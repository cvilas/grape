#!/usr/bin/env python3
"""Print a positive GCC job count; diagnostics go to stderr.

GCC_MEMORY_RESERVE_GIB (default 1, permits 0) and GCC_MEMORY_PER_JOB_GIB
(default 2, must be positive) are integer GiB heuristics, not measured compiler
usage. Values must fit in signed 64-bit bytes. MemAvailable's kB means KiB;
swap is never included. Cgroup limits/usage are bytes.

Inspect the process's memory cgroup and all ancestors visible in its mount.
Ancestors hidden by a cgroup namespace/bind mount cannot be inspected.
One translation unit or concurrent workloads may still exceed the budget:
this is not an OOM guarantee, especially when falling back to one job.
"""

import os
from pathlib import Path
import re
import subprocess
import sys


GIB = 1024**3
MAX_BYTES = 2**63 - 1
# Linux v1 PAGE_COUNTER_MAX (64-bit, 4 KiB pages); larger sentinels also
# represent effectively unlimited memory and cannot reduce MemAvailable.
V1_UNLIMITED = (MAX_BYTES // 4096) * 4096


def unsigned(value, maximum=MAX_BYTES):
    if not re.fullmatch(r"[0-9]{1,20}", value):
        raise ValueError("expected a bounded unsigned decimal integer")
    number = int(value)
    if number > maximum:
        raise ValueError("integer exceeds supported range")
    return number


def budgets(environ):
    reserve = unsigned(environ.get("GCC_MEMORY_RESERVE_GIB", "1"), MAX_BYTES // GIB)
    per_job = unsigned(environ.get("GCC_MEMORY_PER_JOB_GIB", "2"), MAX_BYTES // GIB)
    if per_job == 0:
        raise ValueError("GCC_MEMORY_PER_JOB_GIB must be positive")
    return reserve * GIB, per_job * GIB


def host_available(meminfo):
    entries = [line for line in meminfo.splitlines() if line.startswith("MemAvailable:")]
    if len(entries) != 1:
        raise ValueError("missing or duplicate MemAvailable")
    match = re.fullmatch(r"MemAvailable:\s+([0-9]+)\s+kB\s*", entries[0])
    if not match:
        raise ValueError("malformed MemAvailable")
    return unsigned(match[1], MAX_BYTES // 1024) * 1024


def unescape_mount(value):
    return re.sub(r"\\([0-7]{3})", lambda match: chr(int(match[1], 8)), value)


def cgroup_directories(cgroup, mountinfo):
    """Return (current directory, mount boundary, v2) for memory hierarchies."""
    memberships = {}
    for line in cgroup.splitlines():
        _, controllers, location = line.split(":", 2)
        if not controllers or "memory" in controllers.split(","):
            path = Path(location)
            if not path.is_absolute() or ".." in path.parts:
                raise ValueError("invalid cgroup path")
            memberships[not controllers] = path

    found = set()
    directories = []
    for line in mountinfo.splitlines():
        before, after = line.split(" - ", 1)
        fields, filesystem = before.split(), after.split()
        v2 = filesystem[0] == "cgroup2"
        if not v2 and not (
            filesystem[0] == "cgroup" and "memory" in filesystem[2].split(",")
        ):
            continue
        if v2 not in memberships:
            continue
        root, mount = (Path(unescape_mount(fields[index])) for index in (3, 4))
        location = memberships[v2]
        if not root.is_absolute() or not mount.is_absolute() or ".." in mount.parts:
            raise ValueError("invalid cgroup mount")
        if location.is_relative_to(root):
            relative = location.relative_to(root)
        elif location == Path("/"):
            # A cgroup namespace may report "/" with a subtree mount root.
            relative = Path(".")
        else:
            continue
        directories.append((mount / relative, mount, v2))
        found.add(v2)
    if memberships.keys() - found:
        raise ValueError("cannot locate the process's memory cgroup mount")
    return directories


def effective_available(host, cgroup, mountinfo):
    available = host
    for current, boundary, v2 in cgroup_directories(cgroup, mountinfo):
        while True:
            limit_file = current / ("memory.max" if v2 else "memory.limit_in_bytes")
            # v2's root (or a hierarchy without a memory controller) has no
            # memory.max. Still inspect ancestors for enabled controllers.
            if v2 and not limit_file.exists():
                pass
            else:
                raw_limit = limit_file.read_text().strip()
                limit = (
                    None if v2 and raw_limit == "max"
                    else unsigned(raw_limit, 2**64 - 1)
                )
                if limit is not None and (v2 or limit < V1_UNLIMITED):
                    usage_file = current / (
                        "memory.current" if v2 else "memory.usage_in_bytes"
                    )
                    usage = unsigned(usage_file.read_text().strip())
                    available = min(available, max(0, limit - usage))
            if current == boundary:
                break
            current = current.parent
    return available


def choose_jobs(cpus, available, reserve, per_job):
    return max(1, min(cpus, max(0, available - reserve) // per_job))


def main():
    cpus = host = effective = reserve = per_job = None
    jobs = 1
    try:
        reserve, per_job = budgets(os.environ)
        cpus = unsigned(subprocess.check_output(["nproc"], text=True).strip())
        if cpus == 0:
            raise ValueError("nproc must report a positive CPU count")
        host = host_available(Path("/proc/meminfo").read_text())
        effective = effective_available(
            host,
            Path("/proc/self/cgroup").read_text(),
            Path("/proc/self/mountinfo").read_text(),
        )
        jobs = choose_jobs(cpus, effective, reserve, per_job)
        if effective < reserve + per_job:
            print("Insufficient memory budget: using the minimum one job.", file=sys.stderr)
    except (OSError, ValueError, IndexError, subprocess.SubprocessError) as error:
        print(f"GCC budget detection failed: {error}; using one job.", file=sys.stderr)

    print(
        f"GCC budget: CPUs={cpus}, host available bytes={host}, "
        f"effective available bytes={effective}, reserve bytes={reserve}, "
        f"per-job bytes={per_job}, jobs={jobs}\n"
        "Heuristic only; one translation unit or concurrent workloads may exceed "
        "the budget. Only visible cgroup ancestors are inspected.",
        file=sys.stderr,
    )
    print(jobs)


if __name__ == "__main__":
    main()
