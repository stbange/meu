"""The same native/core/web configure, build and test entry point on Windows/Linux."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / ".tools"
VERSIONS = json.loads((ROOT / "tools/toolchains.json").read_text())


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("target", choices=("native", "core", "web"))
    parser.add_argument("--test", action="store_true")
    parser.add_argument("--smoke", action="store_true", help="render and capture three native frames")
    parser.add_argument("--run", action="store_true")
    parser.add_argument("--release", action="store_true", help="omit the read-only browser test bridge")
    parser.add_argument("--sanitizers", action="store_true", help="native ASan/UBSan (Linux)")
    parser.add_argument("--build-dir", type=Path, help="override the generated build directory")
    args = parser.parse_args()
    host = "windows-x86_64" if os.name == "nt" else "linux-x86_64"
    suffix = ".exe" if os.name == "nt" else ""
    cmake = TOOLS / ("cmake-" + VERSIONS["cmake"] + "-" + host) / "bin" / ("cmake" + suffix)
    ctest = cmake.with_name("ctest" + suffix)
    ninja = TOOLS / "ninja" / ("ninja" + suffix)
    if not cmake.exists() or not ninja.exists():
        raise SystemExit("Run python tools/bootstrap.py" + (" --web" if args.target == "web" else "") + " first.")
    build = ROOT / "build" / (args.target + ("-release" if args.release else "") +
                              ("-sanitizers" if args.sanitizers else ""))
    if args.build_dir:
        build = (ROOT / args.build_dir).resolve()
    env = os.environ.copy()
    paths = [str(cmake.parent), str(ninja.parent)]
    configure = [str(cmake), "-S", str(ROOT), "-B", str(build), "-G", "Ninja",
                 "-DCMAKE_BUILD_TYPE=" + ("Release" if args.release else "RelWithDebInfo"),
                 "-DCMAKE_MAKE_PROGRAM=" + str(ninja),
                 "-DMEU_DIAGNOSTICS=" + ("OFF" if args.release else "ON"),
                 "-DMEU_BUILD_CLIENT=" + ("OFF" if args.target == "core" else "ON"),
                 "-DMEU_SANITIZERS=" + ("ON" if args.sanitizers else "OFF")]
    if args.target == "web":
        sdk = TOOLS / ("emsdk-" + VERSIONS["emsdk_commit"])
        if not (sdk / ".emscripten").exists():
            raise SystemExit("Run python tools/bootstrap.py --web first.")
        env["EMSDK"] = str(sdk)
        env["EM_CONFIG"] = str(sdk / ".emscripten")
        env["EMSDK_PYTHON"] = sys.executable
        paths.insert(0, str(sdk / "upstream/emscripten"))
        configure += ["-DCMAKE_TOOLCHAIN_FILE=" + str(sdk / "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")]
    else:
        compiler = str(TOOLS / "w64devkit/bin/gcc.exe") if os.name == "nt" else shutil.which("gcc")
        if not compiler:
            raise SystemExit("Install GCC 14.2.0 or use the documented Linux reference container.")
        version = subprocess.check_output([compiler, "-dumpfullversion"], text=True).strip()
        if version != VERSIONS["gcc"]:
            raise SystemExit("Reference compiler is GCC " + VERSIONS["gcc"] + "; found " + version +
                             ". See docs/build-and-test.md for the pinned Linux container or manual CMake setup.")
        paths.insert(0, str(Path(compiler).parent))
        configure += ["-DCMAKE_C_COMPILER=" + compiler]
        if os.name == "nt":
            # Identify GCC from an object, not a disposable a.exe. Link and
            # runtime suitability are tested separately by CMake and CTest.
            configure += ["-DCMAKE_C_COMPILER_ID_FLAGS_ALWAYS=-c"]
            if args.target == "native":
                configure += ["-DCMAKE_CXX_COMPILER_ID_FLAGS_ALWAYS=-c"]
    env["PATH"] = os.pathsep.join(paths + [env.get("PATH", "")])
    print("Configure", args.target, flush=True)
    subprocess.run(configure, cwd=ROOT, env=env, check=True)
    subprocess.run([str(cmake), "--build", str(build), "--parallel", "4"], cwd=ROOT, env=env, check=True)
    if args.test:
        subprocess.run([str(ctest), "--test-dir", str(build), "--output-on-failure"], cwd=ROOT, env=env, check=True)
    if args.smoke or args.run:
        if args.target != "native":
            raise SystemExit("--smoke/--run are native-only; serve the web build with tools/serve.py.")
        command = [str(build / "bin" / ("meu" + suffix))]
        if args.smoke:
            captures = ROOT / "test-results"
            captures.mkdir(exist_ok=True)
            command += ["--smoke-test", "--capture", str(captures / "native-room.bmp")]
        subprocess.run(command, cwd=ROOT, env=env, check=True, timeout=30 if args.smoke else None)


if __name__ == "__main__":
    main()
