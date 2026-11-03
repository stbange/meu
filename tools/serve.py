"""Serve only the selected web distribution, defaulting to this computer."""
import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=4174)
    parser.add_argument("--directory", type=Path, default=ROOT / "build/web/bin")
    args = parser.parse_args()
    directory = args.directory.resolve()
    if not (directory / "index.html").exists():
        raise SystemExit("Build the web client first: python tools/build.py web --test")
    handler = partial(SimpleHTTPRequestHandler, directory=str(directory))
    with ThreadingHTTPServer((args.host, args.port), handler) as server:
        print("MEU: http://" + args.host + ":" + str(args.port), flush=True)
        server.serve_forever()


if __name__ == "__main__":
    main()
