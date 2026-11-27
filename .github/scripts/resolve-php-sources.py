#!/usr/bin/env python3
"""Resolves the PHP source tarball to build for each PHP minor version.

For each version given, this is the latest stable release from the php.net
releases API or, for a version without a stable release yet, its current
pre-release (alpha, beta or RC) from the php.net pre-release API. It fails
when php.net has neither.

Prints one JSON object, which the release jobs index by matrix.php-version
(and whose keys are the PHP versions the release builds for):
  {"8.4": {"version": "8.4.28", "url": "https://...tar.xz", "sha256": "..."}, ...}

Usage: resolve-php-sources.py [8.2 8.3 ...]   (default: PHP_VERSIONS below)
"""
import json
import sys
import urllib.request

# The PHP versions the release builds for: the one list the release workflow's
# build matrices take theirs from. Keep it in line
# with the PHP version check in config.m4 and "php" in composer.json.
PHP_VERSIONS = ["8.2", "8.3", "8.4", "8.5", "8.6"]


def get_json(url):
    request = urllib.request.Request(url, headers={"User-Agent": "php-debugger-release"})
    with urllib.request.urlopen(request, timeout=30) as response:
        return json.load(response)


def stable(minor):
    release = get_json(f"https://www.php.net/releases/?json&version={minor}")
    if "version" not in release:
        return None
    tarball = next(s for s in release["source"] if s["filename"].endswith(".tar.xz"))
    return {
        "version": release["version"],
        "url": f"https://www.php.net/distributions/{tarball['filename']}",
        "sha256": tarball["sha256"],
    }


def pre_release(minor, builds):
    for version, build in builds.items():
        if not version.startswith(f"{minor}.") or not isinstance(build, dict):
            continue
        release = build.get("release", {})
        if not build.get("active") or not release.get("enabled") or not release.get("sha256_xz"):
            continue
        # e.g. php-8.6.0RC3.tar.xz in the release manager's directory
        name = f"{version}{release['type']}{release['number']}"
        return {
            "version": name,
            "url": f"{release['baseurl']}php-{name}.tar.xz",
            "sha256": release["sha256_xz"],
        }
    return None


sources = {}
builds = None
for minor in sys.argv[1:] or PHP_VERSIONS:
    source = stable(minor)
    if source is None:
        if builds is None:
            builds = get_json("https://www.php.net/pre-release-builds.php?format=json")
        source = pre_release(minor, builds)
    if source is None:
        sys.exit(f"php.net has no stable release or pre-release of PHP {minor}")
    print(f"PHP {minor}: {source['version']} ({source['url']})", file=sys.stderr)
    sources[minor] = source

print(json.dumps(sources))
