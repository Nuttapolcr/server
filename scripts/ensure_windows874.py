#!/usr/bin/env python3
"""Validate or atomically convert .txt/.conf files to Windows-874."""

import os
import sys
import tempfile
from pathlib import Path


ALLOWED_SUFFIXES = {".txt", ".conf"}


def ensure_windows874(path: Path) -> None:
    if path.suffix.lower() not in ALLOWED_SUFFIXES:
        raise ValueError(f"unsupported extension: {path}")

    raw = path.read_bytes()
    original_mode = path.stat().st_mode
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        # Already Windows-874 (or invalid input). Strict decoding confirms it.
        raw.decode("cp874")
        return

    try:
        converted = text.encode("cp874")
    except UnicodeEncodeError as exc:
        raise ValueError(f"contains characters not representable in Windows-874: {path}") from exc

    with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as temporary:
        temporary.write(converted)
        temporary_path = Path(temporary.name)
    try:
        os.chmod(temporary_path, original_mode)
        os.replace(temporary_path, path)
    finally:
        if temporary_path.exists():
            temporary_path.unlink()


def main() -> int:
    if len(sys.argv) < 2:
        print("Usage: ensure_windows874.py <file> [<file> ...]", file=sys.stderr)
        return 2

    try:
        for name in sys.argv[1:]:
            ensure_windows874(Path(name))
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"Windows-874 validation failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
