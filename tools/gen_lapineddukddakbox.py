#!/usr/bin/env python3
"""
Generate Laphine Ddukddak client data from rAthena laphine_synthesis.yml.

The tool updates only the synthesis entries listed in the input YAML. Existing
client entries in lapineddukddakbox.lub are preserved.
"""

import argparse
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

DEFAULT_SYNTHESIS_DB = ROOT / "db/import/laphine_synthesis.yml"
DEFAULT_OUTPUT = ROOT / "data/Lua Files/datainfo/lapineddukddakbox.lub"
DEFAULT_ITEM_DBS = [
    ROOT / "db/import/item_db.yml",
    ROOT / "db/pre-re/item_db_custom.yml",
    ROOT / "db/pre-re/item_db_etc.yml",
    ROOT / "db/pre-re/item_db_equip.yml",
    ROOT / "db/pre-re/item_db_usable.yml",
    ROOT / "db/re/item_db_etc.yml",
    ROOT / "db/re/item_db_equip.yml",
    ROOT / "db/re/item_db_usable.yml",
]

NEED_SOURCE_OVERRIDES = {
    "Combine_Common_Pet_Laphine": "Uncommon Pet Eggs",
    "Combine_Rare_Pet_Laphine": "Rare Pet Eggs",
}


class ItemInfo:
    def __init__(self, item_id, aegis_name, display_name):
        self.item_id = item_id
        self.aegis_name = aegis_name
        self.display_name = display_name


class Requirement:
    def __init__(self, item, amount=1):
        self.item = item
        self.amount = amount


class SynthesisEntry:
    def __init__(self, item, need_count=1, minimum_refine=0, maximum_refine=0):
        self.item = item
        self.need_count = need_count
        self.minimum_refine = minimum_refine
        self.maximum_refine = maximum_refine
        self.requirements = []


def read_text(path):
    return path.read_text(encoding="latin-1")


def strip_comment(value):
    in_quote = False
    quote = ""
    escaped = False

    for index, char in enumerate(value):
        if escaped:
            escaped = False
            continue
        if in_quote and char == "\\":
            escaped = True
            continue
        if char in ("'", '"'):
            if not in_quote:
                in_quote = True
                quote = char
            elif quote == char:
                in_quote = False
            continue
        if char == "#" and not in_quote:
            return value[:index].rstrip()

    return value.strip()


def parse_scalar(value):
    value = strip_comment(value).strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in ("'", '"'):
        return value[1:-1]
    return value


def parse_int(value, default=0):
    value = parse_scalar(value)
    if value == "":
        return default
    return int(value, 0)


def parse_item_db(path):
    if not path.exists():
        return {}

    items = {}
    current = None

    def finish():
        if not current:
            return
        if "Id" not in current or "AegisName" not in current:
            return
        aegis_name = current["AegisName"]
        items[aegis_name] = ItemInfo(
            item_id=int(current["Id"], 0),
            aegis_name=aegis_name,
            display_name=current.get("Name", aegis_name),
        )

    for raw_line in read_text(path).splitlines():
        id_match = re.match(r"^\s*-\s+Id:\s*(.+?)\s*$", raw_line)
        if id_match:
            finish()
            current = {"Id": parse_scalar(id_match.group(1))}
            continue

        if current is None:
            continue

        field_match = re.match(r"^\s{4}(AegisName|Name):\s*(.+?)\s*$", raw_line)
        if field_match:
            current[field_match.group(1)] = parse_scalar(field_match.group(2))

    finish()
    return items


def build_item_catalog(paths):
    catalog = {}
    for path in paths:
        for aegis_name, info in parse_item_db(path).items():
            catalog.setdefault(aegis_name, info)
    return catalog


def parse_laphine_synthesis(path):
    entries = []
    current = None
    current_requirement = None
    in_requirements = False

    def finish_entry():
        if current is not None:
            entries.append(current)

    for raw_line in read_text(path).splitlines():
        entry_match = re.match(r"^\s{2}-\s+Item:\s*(.+?)\s*$", raw_line)
        if entry_match:
            finish_entry()
            current = SynthesisEntry(item=parse_scalar(entry_match.group(1)))
            current_requirement = None
            in_requirements = False
            continue

        if current is None:
            continue

        if re.match(r"^\s{4}Requirements:\s*$", raw_line):
            in_requirements = True
            current_requirement = None
            continue

        field_match = re.match(
            r"^\s{4}(RequiredRequirementsCount|MinimumRefine|MaximumRefine):\s*(.+?)\s*$",
            raw_line,
        )
        if field_match and not in_requirements:
            key, value = field_match.groups()
            if key == "RequiredRequirementsCount":
                current.need_count = parse_int(value, 1)
            elif key == "MinimumRefine":
                current.minimum_refine = parse_int(value, 0)
            elif key == "MaximumRefine":
                current.maximum_refine = parse_int(value, 0)
            continue

        req_match = re.match(r"^\s{6}-\s+Item:\s*(.+?)\s*$", raw_line)
        if req_match and in_requirements:
            current_requirement = Requirement(item=parse_scalar(req_match.group(1)))
            current.requirements.append(current_requirement)
            continue

        amount_match = re.match(r"^\s{8}Amount:\s*(.+?)\s*$", raw_line)
        if amount_match and current_requirement is not None:
            current_requirement.amount = parse_int(amount_match.group(1), 1)

    finish_entry()
    return entries


def lua_quote(value):
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def need_source_string(entry, catalog):
    if entry.item in NEED_SOURCE_OVERRIDES:
        return NEED_SOURCE_OVERRIDES[entry.item]

    names = []
    for requirement in entry.requirements:
        info = catalog.get(requirement.item)
        names.append(info.display_name if info else requirement.item)
    return ", ".join(names)


def render_entry(entry, catalog):
    item_info = catalog.get(entry.item)
    if item_info is None:
        raise KeyError(f"Cannot resolve synthesis item: {entry.item}")

    source_items = []
    for requirement in entry.requirements:
        requirement_info = catalog.get(requirement.item)
        if requirement_info is None:
            raise KeyError(f"Cannot resolve requirement item: {requirement.item}")
        source_items.append(
            "\t\t\t\t{ "
            + lua_quote(requirement_info.aegis_name)
            + f", {requirement.amount}, {requirement_info.item_id} }}"
        )

    lines = [
        f"\t\t{entry.item} = {{",
        f"\t\t\tItemID = {item_info.item_id},",
        f"\t\t\tNeedCount = {entry.need_count},",
        f"\t\t\tNeedRefineMin = {entry.minimum_refine},",
        f"\t\t\tNeedRefineMax = {entry.maximum_refine},",
        "\t\t\tSourceItems = {",
    ]
    for index, line in enumerate(source_items):
        comma = "," if index + 1 < len(source_items) else ""
        lines.append(line + comma)
    lines.extend(
        [
            "\t\t\t},",
            "\t\t\tNeedSource_String = "
            + lua_quote(need_source_string(entry, catalog)),
            "\t\t},",
        ]
    )
    return "\n".join(lines)


def remove_existing_entries(source, names):
    for name in names:
        pattern = re.compile(
            r"\n\t\t" + re.escape(name) + r"\s*=\s*\{\n.*?\n\t\t\},",
            re.DOTALL,
        )
        source = pattern.sub("", source)
    return source


def update_lub(output_path, entries, catalog):
    original = read_text(output_path)
    newline = "\r\n" if "\r\n" in original else "\n"
    source = original.replace("\r\n", "\n")

    names = [entry.item for entry in entries]
    source = remove_existing_entries(source, names)
    generated = "\n".join(render_entry(entry, catalog) for entry in entries)

    marker = "\n\t},\n\tget_lapine_box = function"
    if marker not in source:
        raise RuntimeError("Cannot find tblLapineDdukddakBox.sources closing marker")

    source = source.replace(marker, "\n" + generated + marker, 1)
    updated = source.replace("\n", newline)

    if updated == original:
        return False

    with output_path.open("w", encoding="latin-1", newline="") as output_file:
        output_file.write(updated)
    return True


def parse_args():
    parser = argparse.ArgumentParser(
        description="Update data/Lua Files/datainfo/lapineddukddakbox.lub from laphine_synthesis.yml."
    )
    parser.add_argument("--synthesis-db", type=Path, default=DEFAULT_SYNTHESIS_DB)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--item-db",
        action="append",
        type=Path,
        default=[],
        help="Additional item DB path. Defaults are always read first.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Validate and print the number of generated entries without writing.",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    item_db_paths = DEFAULT_ITEM_DBS + args.item_db
    catalog = build_item_catalog(item_db_paths)
    entries = parse_laphine_synthesis(args.synthesis_db)

    if not entries:
        raise RuntimeError(f"No synthesis entries found in {args.synthesis_db}")

    for entry in entries:
        render_entry(entry, catalog)

    if args.dry_run:
        print(f"Validated {len(entries)} entries from {args.synthesis_db}")
        return 0

    changed = update_lub(args.output, entries, catalog)
    status = "Updated" if changed else "Already up to date"
    print(f"{status}: {args.output} ({len(entries)} entries)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
