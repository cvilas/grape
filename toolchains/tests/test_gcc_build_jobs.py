import contextlib
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import gcc_build_jobs as budget


class BudgetTests(unittest.TestCase):
    def test_cpu_and_memory_bounds(self):
        for cpus, memory, expected in (
            (4, 32 * budget.GIB, 4),
            (8, 8 * budget.GIB, 3),
            (8, 5 * budget.GIB - 1, 1),
            (8, 3 * budget.GIB, 1),
            (8, budget.GIB, 1),
            (8, 0, 1),
        ):
            with self.subTest(cpus=cpus, memory=memory):
                self.assertEqual(
                    budget.choose_jobs(cpus, memory, budget.GIB, 2 * budget.GIB),
                    expected,
                )

    def test_memavailable_units_and_no_swap(self):
        self.assertEqual(
            budget.host_available("MemAvailable: 1048576 kB\nSwapFree: 999999999 kB\n"),
            budget.GIB,
        )
        self.assertEqual(budget.host_available("MemAvailable: 0 kB\n"), 0)

    def test_invalid_memavailable(self):
        for value in (
            "", "MemFree: 123 kB", "MemAvailable: -1 kB",
            "MemAvailable: 1 MB", "MemAvailable: 1.5 kB",
            "MemAvailable: nope kB", "MemAvailable: 1 kB trailing",
            "MemAvailable: 1 kB\nMemAvailable: 2 kB",
            f"MemAvailable: {2**63} kB",
        ):
            with self.subTest(value=value), self.assertRaises(ValueError):
                budget.host_available(value)

    def test_tunable_budgets(self):
        self.assertEqual(budget.budgets({}), (budget.GIB, 2 * budget.GIB))
        self.assertEqual(
            budget.budgets({"GCC_MEMORY_RESERVE_GIB": "0", "GCC_MEMORY_PER_JOB_GIB": "1"}),
            (0, budget.GIB),
        )
        self.assertEqual(budget.choose_jobs(8, 8 * budget.GIB, 0, budget.GIB), 8)

    def test_invalid_parameters(self):
        for name in ("GCC_MEMORY_RESERVE_GIB", "GCC_MEMORY_PER_JOB_GIB"):
            for value in ("", "-1", "1.5", "1+1", " 2", "max", "9" * 100, str(2**63)):
                with self.subTest(name=name, value=value), self.assertRaises(ValueError):
                    budget.budgets({name: value})
        with self.assertRaises(ValueError):
            budget.budgets({"GCC_MEMORY_PER_JOB_GIB": "0"})

    def run_main(self, meminfo="MemAvailable: 8388608 kB\n", cpus="4\n", environ=None):
        stdout, stderr = io.StringIO(), io.StringIO()
        files = {
            "/proc/meminfo": meminfo,
            "/proc/self/cgroup": "",
            "/proc/self/mountinfo": "",
        }

        def read_text(path):
            value = files[str(path)]
            if isinstance(value, Exception):
                raise value
            return value

        with patch.dict(budget.os.environ, environ or {}, clear=True), \
                patch.object(budget.subprocess, "check_output", return_value=cpus), \
                patch.object(Path, "read_text", read_text), \
                contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            budget.main()
        return stdout.getvalue(), stderr.getvalue()

    def test_main_success_and_logging(self):
        output, log = self.run_main()
        self.assertEqual(output, "3\n")
        for value in ("CPUs=4", "host available bytes=", "effective available bytes=",
                      "reserve bytes=1073741824", "per-job bytes=2147483648",
                      "jobs=3", "Heuristic only"):
            self.assertIn(value, log)

    def test_main_safe_fallbacks(self):
        for memory in ("", "MemAvailable: bad kB", FileNotFoundError("missing")):
            with self.subTest(memory=memory):
                output, log = self.run_main(meminfo=memory)
                self.assertEqual(output, "1\n")
                self.assertIn("using one job", log)
        for cpus in ("0", "-1", "bad", str(2**63)):
            with self.subTest(cpus=cpus):
                self.assertEqual(self.run_main(cpus=cpus)[0], "1\n")
        self.assertEqual(
            self.run_main(environ={"GCC_MEMORY_PER_JOB_GIB": "0"})[0], "1\n"
        )

    def test_main_low_memory_visible(self):
        output, log = self.run_main(meminfo="MemAvailable: 1 kB\n")
        self.assertEqual(output, "1\n")
        self.assertIn("minimum one job", log)

    def test_missing_nproc(self):
        with patch.object(budget.subprocess, "check_output", side_effect=FileNotFoundError), \
                contextlib.redirect_stdout(io.StringIO()) as output, \
                contextlib.redirect_stderr(io.StringIO()):
            budget.main()
        self.assertEqual(output.getvalue(), "1\n")


class CgroupTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.mount = Path(self.temporary.name) / "memory mount"
        self.current = self.mount / "runner" / "job"
        self.current.mkdir(parents=True)

    def mountinfo(self, v2=True, root="/"):
        mount = str(self.mount).replace(" ", r"\040")
        filesystem = "cgroup2 cgroup rw" if v2 else "cgroup cgroup rw,memory"
        return f"1 0 0:1 {root} {mount} rw - {filesystem}\n"

    def write_budget(self, directory, limit, usage, v2=True):
        (directory / ("memory.max" if v2 else "memory.limit_in_bytes")).write_text(str(limit))
        (directory / ("memory.current" if v2 else "memory.usage_in_bytes")).write_text(str(usage))

    def available(self, v2=True, root="/", location="/runner/job"):
        membership = f"0::{location}\n" if v2 else f"4:cpu,memory:{location}\n"
        return budget.effective_available(
            16 * budget.GIB, membership, self.mountinfo(v2, root)
        )

    def test_v2_finite_child_and_limiting_parent(self):
        self.write_budget(self.current, 10 * budget.GIB, budget.GIB)
        self.write_budget(self.current.parent, 8 * budget.GIB, 3 * budget.GIB)
        self.assertEqual(self.available(), 5 * budget.GIB)
        self.assertEqual(
            budget.choose_jobs(8, self.available(), budget.GIB, 2 * budget.GIB), 2
        )

    def test_v2_unlimited_child_finite_root(self):
        self.write_budget(self.current, "max", 0)
        self.write_budget(self.mount, 4 * budget.GIB, budget.GIB)
        self.assertEqual(self.available(), 3 * budget.GIB)

    def test_v2_unlimited_and_absent_controller(self):
        self.assertEqual(self.available(), 16 * budget.GIB)
        self.write_budget(self.current, "max", 0)
        self.assertEqual(self.available(), 16 * budget.GIB)

    def test_exhausted_limit(self):
        self.write_budget(self.current, budget.GIB, 2 * budget.GIB)
        self.assertEqual(self.available(), 0)

    def test_v1_finite_parent_and_unlimited_child(self):
        for unlimited in (budget.V1_UNLIMITED, 2**64 - 1):
            with self.subTest(unlimited=unlimited):
                self.write_budget(self.current, unlimited, 0, False)
                self.write_budget(self.current.parent, 6 * budget.GIB, 2 * budget.GIB, False)
                self.write_budget(self.mount, budget.V1_UNLIMITED, 0, False)
                self.assertEqual(self.available(False), 4 * budget.GIB)
        self.write_budget(self.current, 3 * budget.GIB, budget.GIB, False)
        self.assertEqual(self.available(False), 2 * budget.GIB)

    def test_subtree_mount_and_namespace_root(self):
        self.write_budget(self.current, 4 * budget.GIB, budget.GIB)
        self.assertEqual(
            self.available(root="/container", location="/container/runner/job"),
            3 * budget.GIB,
        )
        self.write_budget(self.mount, 2 * budget.GIB, budget.GIB)
        self.assertEqual(self.available(root="/container", location="/"), budget.GIB)

    def test_invalid_limit_or_usage(self):
        for limit, usage in (("bad", "0"), ("-1", "0"), ("100", "bad"),
                             (str(2**64), "0")):
            with self.subTest(limit=limit, usage=usage), self.assertRaises(ValueError):
                self.write_budget(self.current, limit, usage)
                self.available()
        self.write_budget(self.current, "100", "0")
        (self.current / "memory.current").unlink()
        with self.assertRaises(OSError):
            self.available()

    def test_unmapped_or_invalid_cgroup(self):
        for location in ("/outside/job", "/../job", "relative"):
            with self.subTest(location=location), self.assertRaises(ValueError):
                self.available(root="/container", location=location)
        with self.assertRaises(ValueError):
            budget.effective_available(budget.GIB, "0::/runner\n", "")

    def test_cgroup_detection_failure_falls_back(self):
        with patch.object(budget, "effective_available", side_effect=ValueError("bad cgroup")):
            self.assertEqual(BudgetTests().run_main()[0], "1\n")


if __name__ == "__main__":
    unittest.main()
