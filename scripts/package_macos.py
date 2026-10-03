#!/usr/bin/env python3
"""Package an existing arm64 build; never compiles or changes the source binary."""
import argparse
import hashlib
import plistlib
import re
import shutil
import subprocess
import sys
from pathlib import Path


def run(*args):
    return subprocess.check_output(args, text=True).strip()


def dependencies(path):
    return [line.strip().split(" (", 1)[0]
            for line in run("otool", "-L", str(path)).splitlines()[1:]]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=Path("build/TowerBuilder"))
    parser.add_argument("--version", default="v0.1")
    parser.add_argument("--output", type=Path, default=Path("dist"))
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("macOS packaging requires macOS tools")
    if not re.fullmatch(r"v[0-9]+(?:\.[0-9]+)*", args.version):
        parser.error("version must be a numeric v-prefixed tag")
    binary = args.binary.resolve(strict=True)
    if run("lipo", "-archs", str(binary)) != "arm64":
        parser.error("this package requires an arm64-only binary")
    build_info = run("vtool", "-show-build", str(binary))
    if not re.search(r"\bminos\s+26\.0\b", build_info):
        parser.error("binary must retain the approved macOS 26.0 deployment target")
    external = [dep for dep in dependencies(binary)
                if not dep.startswith(("/usr/lib/", "/System/Library/"))]
    if len(external) != 1 or not Path(external[0]).name.startswith("libraylib."):
        parser.error("expected raylib as the sole non-system dependency")
    library = Path(external[0]).resolve(strict=True)
    for dep in dependencies(library)[1:]:
        if not dep.startswith(("/usr/lib/", "/System/Library/")):
            parser.error(f"raylib has an unbundled dependency: {dep}")
    license_path = library.parent.parent / "LICENSE"
    if not license_path.is_file():
        parser.error(f"raylib license missing: {license_path}")

    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    name = f"TowerBuilder-{args.version}-macos26plus-arm64"
    staging = output / name
    app = staging / "TowerBuilder.app"
    if staging.exists():
        parser.error(f"staging already exists; choose a fresh output directory: {staging}")
    executable = app / "Contents/MacOS/TowerBuilder"
    frameworks = app / "Contents/Frameworks"
    resources = app / "Contents/Resources"
    executable.parent.mkdir(parents=True)
    frameworks.mkdir()
    resources.mkdir()
    shutil.copy2(binary, executable)
    bundled_library = frameworks / library.name
    shutil.copy2(library, bundled_library)
    shutil.copy2(license_path, resources / "raylib-LICENSE.txt")
    with (app / "Contents/Info.plist").open("wb") as stream:
        plistlib.dump({
            "CFBundleName": "TowerBuilder",
            "CFBundleDisplayName": "TowerBuilder",
            "CFBundleIdentifier": "com.neboer.towerbuilder",
            "CFBundleExecutable": "TowerBuilder",
            "CFBundlePackageType": "APPL",
            "CFBundleShortVersionString": args.version[1:],
            "CFBundleVersion": args.version[1:],
            "LSMinimumSystemVersion": "26.0",
            "NSHighResolutionCapable": True,
        }, stream)
    run("install_name_tool", "-change", external[0],
        f"@executable_path/../Frameworks/{library.name}", str(executable))
    run("install_name_tool", "-id", f"@rpath/{library.name}", str(bundled_library))
    run("codesign", "--force", "--sign", "-", "--timestamp=none", str(bundled_library))
    run("codesign", "--force", "--sign", "-", "--timestamp=none", str(app))
    run("codesign", "--verify", "--deep", "--strict", str(app))
    run(str(executable), "--smoke-test")
    archive = output / f"{name}.zip"
    if archive.exists():
        parser.error(f"archive already exists: {archive}")
    run("ditto", "-c", "-k", "--sequesterRsrc", "--keepParent", str(app), str(archive))
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    checksum = output / f"{name}.zip.sha256"
    checksum.write_text(f"{digest}  {archive.name}\n")
    print(f"Packaged: {archive}")
    print(f"SHA256: {digest}")
    print("Ad-hoc signed only; not Developer ID signed or notarized.")


if __name__ == "__main__":
    main()
