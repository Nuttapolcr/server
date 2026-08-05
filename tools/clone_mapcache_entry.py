#!/usr/bin/env python3
"""Append a map-cache entry under a new name, reusing an existing map's cells."""

import argparse
import struct
from pathlib import Path


# main_header is written with the C compiler's two-byte tail padding.
HEADER = struct.Struct("<IH2x")
ENTRY = struct.Struct("<12shhI")


def map_name(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("ascii")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("cache", type=Path)
    parser.add_argument("source")
    parser.add_argument("destination")
    args = parser.parse_args()

    destination = args.destination.encode("ascii")
    if len(destination) >= 12:
        raise SystemExit("destination map name must be at most 11 characters")

    data = bytearray(args.cache.read_bytes())
    file_size, map_count = HEADER.unpack_from(data)
    if file_size != len(data):
        raise SystemExit(f"invalid cache size: header={file_size}, actual={len(data)}")

    offset = HEADER.size
    source_entry = None
    for _ in range(map_count):
        name, width, height, length = ENTRY.unpack_from(data, offset)
        if map_name(name) == args.destination:
            print(f"{args.destination} already exists in {args.cache}")
            return
        if map_name(name) == args.source:
            source_entry = data[offset : offset + ENTRY.size + length]
        offset += ENTRY.size + length

    if source_entry is None:
        raise SystemExit(f"source map {args.source!r} not found")

    cloned = bytearray(source_entry)
    cloned[:12] = destination.ljust(12, b"\0")
    data.extend(cloned)
    HEADER.pack_into(data, 0, len(data), map_count + 1)
    args.cache.write_bytes(data)
    print(f"cloned {args.source} to {args.destination} in {args.cache}")


if __name__ == "__main__":
    main()
