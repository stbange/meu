"""Install checksum-pinned, per-checkout tools without changing global settings."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tarfile
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / ".tools"
MANIFEST = json.loads((ROOT / "tools/toolchains.json").read_text())


def download(key):
    record = MANIFEST["downloads"][key]
    destination = TOOLS / "downloads" / key
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        with destination.open("rb") as source:
            if hashlib.file_digest(source, "sha256").hexdigest() == record["sha256"]:
                return destination
        destination.unlink()
    pending = destination.with_suffix(".partial")
    print("Download", record["url"], flush=True)
    urllib.request.urlretrieve(record["url"], pending)
    with pending.open("rb") as source:
        digest = hashlib.file_digest(source, "sha256").hexdigest()
    if digest != record["sha256"]:
        pending.unlink()
        raise RuntimeError("Checksum mismatch for " + key)
    pending.replace(destination)
    return destination


def extract(archive, destination, kind):
    destination.mkdir(parents=True, exist_ok=True)
    if kind == "zip":
        with zipfile.ZipFile(archive) as source:
            source.extractall(destination)
    elif kind == "tar":
        subprocess.run(["tar", "-xf", str(archive), "-C", str(destination)], check=True)
    else:
        # The Windows 2022 system tar lacks this archive's LZMA codec.
        # Extract with the pinned standalone reader, without running an installer.
        reader = TOOLS / "7zr.exe"
        if not reader.exists():
            shutil.copyfile(download("7zr-windows"), reader)
        subprocess.run([str(reader), "x", "-y", str(archive), "-o" + str(destination)], check=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--web", action="store_true", help="also install the pinned Emscripten SDK")
    args = parser.parse_args()
    if platform.machine().lower() not in ("amd64", "x86_64"):
        raise SystemExit("This bootstrap targets x86-64 Windows/Linux. Other hosts can configure CMake manually.")
    host = "windows" if os.name == "nt" else "linux"
    if host == "linux" and sys.platform != "linux":
        raise SystemExit("Use a Windows/Linux host or configure CMake manually.")
    cmake_dir = TOOLS / ("cmake-" + MANIFEST["cmake"] + "-" +
                         ("windows-x86_64" if host == "windows" else "linux-x86_64"))
    cmake_executable = cmake_dir / "bin" / ("cmake.exe" if os.name == "nt" else "cmake")
    if not cmake_executable.exists():
        extract(download("cmake-" + host), TOOLS, "zip" if host == "windows" else "tar")
    ninja = TOOLS / "ninja" / ("ninja.exe" if os.name == "nt" else "ninja")
    if not ninja.exists():
        extract(download("ninja-" + host), ninja.parent, "zip")
        if os.name != "nt":
            ninja.chmod(0o755)
    if os.name == "nt" and not (TOOLS / "w64devkit/bin/gcc.exe").exists():
        extract(download("gcc-windows"), TOOLS, "sfx")
    if args.web:
        sdk = TOOLS / ("emsdk-" + MANIFEST["emsdk_commit"])
        if not (sdk / "emsdk.py").exists():
            extract(download("emsdk"), TOOLS, "zip")
        subprocess.run([sys.executable, str(sdk / "emsdk.py"), "install", MANIFEST["emscripten"]], check=True)
        subprocess.run([sys.executable, str(sdk / "emsdk.py"), "activate", MANIFEST["emscripten"]], check=True)
    subprocess.run([str(cmake_executable), "--version"], check=True)
    subprocess.run([str(ninja), "--version"], check=True)
    print("MEU tools are ready in", TOOLS)


if __name__ == "__main__":
    main()
