#!/usr/bin/env python3
"""Update third-party versions and source archives from upstream tags."""

from __future__ import annotations

import argparse
import glob
import json
import os
import re
import sys
import urllib.error
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Any


@dataclass(frozen=True)
class Dependency:
    name: str
    version_variable: str
    repo: str
    tag_regex: str
    tag_template: str
    download_url_template: str
    archive_name_template: str
    archive_glob: str
    auto_update: bool
    sync_archive: bool
    version_dash: bool = False
    notes: str = ""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--metadata",
        default="external/third_party_sources.json",
        help="Path to dependency metadata JSON.",
    )
    parser.add_argument(
        "--versions",
        default="external/third_party_versions.cmake",
        help="Path to third-party version CMake file.",
    )
    parser.add_argument(
        "--sources-dir",
        default="external/sources",
        help="Directory containing source tarballs.",
    )
    parser.add_argument(
        "--check-updates",
        action="store_true",
        help="Check upstream tags and update versions file when newer versions are found.",
    )
    parser.add_argument(
        "--sync-archives",
        action="store_true",
        help="Ensure source archives match version file.",
    )
    parser.add_argument(
        "--prune",
        action="store_true",
        help="Delete superseded archives matching archive_glob.",
    )
    parser.add_argument(
        "--report-only",
        action="store_true",
        help="Do not write files or download archives; only report planned actions.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Return non-zero if any archive sync step fails.",
    )
    return parser.parse_args()


def read_metadata(path: Path) -> list[Dependency]:
    data = json.loads(path.read_text(encoding="utf-8"))
    return [Dependency(**dep) for dep in data["dependencies"]]


def read_versions(path: Path) -> tuple[str, dict[str, str]]:
    text = path.read_text(encoding="utf-8")
    versions: dict[str, str] = {}
    for match in re.finditer(r"^\s*set\(\s*([A-Z0-9_]+)\s+([^\s\)]+)\s*\)", text, re.MULTILINE):
        versions[match.group(1)] = match.group(2)
    return text, versions


def write_versions(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8")


def normalize_version(version: str, dep: Dependency) -> str:
    return version.replace("-", ".") if dep.version_dash else version


def semver_key(version: str) -> tuple[int, int, int] | None:
    match = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", version)
    if not match:
        return None
    return tuple(int(part) for part in match.groups())


def github_api_get(url: str) -> Any:
    headers = {
        "Accept": "application/vnd.github+json",
        "User-Agent": "grape-third-party-updater",
    }
    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if token:
        headers["Authorization"] = "Bearer " + token
    request = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(request, timeout=30) as response:  # noqa: S310
        return json.loads(response.read().decode("utf-8"))


def fetch_latest_version(dep: Dependency) -> str | None:
    pattern = re.compile(dep.tag_regex)
    latest: tuple[int, int, int] | None = None
    latest_version: str | None = None
    page = 1

    while page <= 5:
        try:
            tags = github_api_get(f"https://api.github.com/repos/{dep.repo}/tags?per_page=100&page={page}")
        except urllib.error.HTTPError:
            return None
        except urllib.error.URLError:
            return None
        if not tags:
            break
        for tag in tags:
            match = pattern.match(tag["name"])
            if not match:
                continue
            version = normalize_version(match.group("version"), dep)
            key = semver_key(version)
            if key is None:
                continue
            if latest is None or key > latest:
                latest = key
                latest_version = version
        page += 1

    return latest_version


def format_tokens(version: str) -> dict[str, str]:
    return {"version": version, "version_dash": version.replace(".", "-")}


def apply_version_updates(
    source_text: str,
    updates: dict[str, str],
) -> str:
    updated_text = source_text
    for variable, version in updates.items():
        updated_text = re.sub(
            rf"(^\s*set\(\s*{re.escape(variable)}\s+)([^\s\)]+)(\s*\))",
            rf"\g<1>{version}\g<3>",
            updated_text,
            flags=re.MULTILINE,
        )
    return updated_text


def download(url: str, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    request = urllib.request.Request(url, headers={"User-Agent": "grape-third-party-updater"})
    with urllib.request.urlopen(request, timeout=120) as response, destination.open("wb") as file:  # noqa: S310
        file.write(response.read())


def sync_archives(
    dependencies: list[Dependency],
    versions: dict[str, str],
    sources_dir: Path,
    prune: bool,
    report_only: bool,
) -> tuple[list[str], list[str]]:
    actions: list[str] = []
    errors: list[str] = []

    for dep in dependencies:
        current_version = versions.get(dep.version_variable)
        if current_version is None:
            errors.append(f"{dep.name}: missing version variable {dep.version_variable}")
            continue

        tokens = format_tokens(current_version)
        archive_name = dep.archive_name_template.format(**tokens)
        archive_path = sources_dir / archive_name

        if dep.sync_archive:
            if not archive_path.exists():
                url = dep.download_url_template.format(**tokens)
                actions.append(f"download {archive_name} from {url}")
                if not report_only:
                    try:
                        download(url, archive_path)
                    except urllib.error.URLError as exc:
                        errors.append(f"{dep.name}: failed to download {url}: {exc}")
            else:
                actions.append(f"keep {archive_name}")
        else:
            actions.append(f"skip sync for {dep.name}")

        if prune:
            pattern = str(sources_dir / dep.archive_glob)
            for path_str in glob.glob(pattern):
                path = Path(path_str)
                if path.name == archive_name:
                    continue
                actions.append(f"remove {path.name}")
                if not report_only:
                    path.unlink(missing_ok=True)

    return actions, errors


def main() -> int:
    args = parse_args()

    metadata_path = Path(args.metadata)
    versions_path = Path(args.versions)
    sources_dir = Path(args.sources_dir)

    dependencies = read_metadata(metadata_path)
    source_text, versions = read_versions(versions_path)

    do_check = args.check_updates or (not args.check_updates and not args.sync_archives)
    do_sync = args.sync_archives or (not args.check_updates and not args.sync_archives)

    updates: dict[str, str] = {}
    if do_check:
        for dep in dependencies:
            if not dep.auto_update:
                print(f"[skip] {dep.name}: auto-update disabled")
                continue
            current = versions.get(dep.version_variable)
            if current is None:
                print(f"[warn] {dep.name}: missing variable {dep.version_variable}")
                continue
            latest = fetch_latest_version(dep)
            if latest is None:
                print(f"[warn] {dep.name}: could not resolve latest version")
                continue
            current_key = semver_key(current)
            latest_key = semver_key(latest)
            if current_key is None or latest_key is None:
                print(f"[warn] {dep.name}: non-semver value current={current}, latest={latest}")
                continue
            if latest_key > current_key:
                updates[dep.version_variable] = latest
                print(f"[update] {dep.name}: {current} -> {latest}")
            else:
                print(f"[ok] {dep.name}: {current}")

        if updates:
            new_text = apply_version_updates(source_text, updates)
            if args.report_only:
                print("[report] version file changes pending")
            else:
                write_versions(versions_path, new_text)
                source_text = new_text
                _, versions = read_versions(versions_path)
        else:
            print("[ok] no version updates found")

    errors: list[str] = []
    if do_sync:
        actions, errors = sync_archives(
            dependencies=dependencies,
            versions=versions,
            sources_dir=sources_dir,
            prune=args.prune,
            report_only=args.report_only,
        )
        for action in actions:
            print(f"[sync] {action}")

    if errors:
        for error in errors:
            print(f"[error] {error}", file=sys.stderr)
        return 1 if args.strict else 0

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
