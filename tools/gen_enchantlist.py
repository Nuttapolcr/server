#!/usr/bin/env python3
"""
Generate client EnchantList.lub entries from rAthena item_enchant.yml.

By default this updates only the Table[id] blocks present in the input YAML:
existing client entries are preserved, matching IDs are replaced, and new IDs
are appended to the end of the file.
"""

import argparse
import re
from pathlib import Path

from _itemdb_lub import (
    add_missing_names,
    build_item_catalog,
    existing_namelist,
)


ROOT = Path(__file__).resolve().parents[1]

DEFAULT_ENCHANT_DB = ROOT / "db/pre-re/item_enchant.yml"
DEFAULT_OUTPUT = ROOT / "data/Lua Files/Enchant/EnchantList.lub"


class ParseError(Exception):
    pass


def read_text(path, encoding="utf-8"):
    return path.read_text(encoding=encoding)


def write_text(path, text, encoding="latin-1"):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding=encoding, newline="") as output:
        output.write(text)


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
    if value.lower() == "true":
        return True
    if value.lower() == "false":
        return False
    if value in ("null", "~"):
        return None
    if re.fullmatch(r"-?0x[0-9a-fA-F]+|-?\d+", value):
        return int(value, 0)
    return value


def parse_key_value(raw_line):
    match = re.match(r"^(\s*)([^:]+):(?:\s*(.*?))?\s*$", raw_line)
    if not match:
        raise ParseError(f"Cannot parse YAML line: {raw_line}")
    indent, key, value = match.groups()
    return len(indent), key.strip(), parse_scalar(value or "")


def parse_material_line(raw_line):
    match = re.match(r"^\s*-\s+Material:\s*(.+?)\s*$", raw_line)
    if not match:
        raise ParseError(f"Cannot parse material line: {raw_line}")
    return {"Material": parse_scalar(match.group(1)), "Amount": 1}


def parse_item_line(raw_line):
    match = re.match(r"^\s*-\s+Item:\s*(.+?)\s*$", raw_line)
    if not match:
        raise ParseError(f"Cannot parse item line: {raw_line}")
    return {"Item": parse_scalar(match.group(1))}


def parse_item_enchant(path):
    entries = []
    current = None
    section = None
    slot = None
    slot_section = None
    material = None
    enchant_group = None
    enchant_item = None
    perfect = None
    upgrade = None

    def finish_entry():
        if current is not None:
            entries.append(current)

    for line_no, raw_line in enumerate(read_text(path).splitlines(), 1):
        line = strip_comment(raw_line)
        if not line:
            continue

        entry_match = re.match(r"^\s{2}-\s+Id:\s*(.+?)\s*$", raw_line)
        if entry_match:
            finish_entry()
            current = {
                "Id": parse_scalar(entry_match.group(1)),
                "TargetItems": [],
                "MinimumRefine": 0,
                "MinimumEnchantgrade": 0,
                "AllowRandomOptions": True,
                "Order": [],
                "Slots": [],
            }
            section = None
            slot = None
            slot_section = None
            material = None
            enchant_group = None
            enchant_item = None
            perfect = None
            upgrade = None
            continue

        if current is None:
            continue

        try:
            indent, key, value = parse_key_value(raw_line)
        except ParseError as exc:
            raise ParseError(f"{path}:{line_no}: {exc}") from exc

        if indent == 4:
            slot = None
            slot_section = None
            material = None
            enchant_group = None
            enchant_item = None
            perfect = None
            upgrade = None
            if key in ("TargetItems", "Reset", "Order", "Slots"):
                section = key
                if key == "Reset":
                    current["Reset"] = {"Chance": 0, "Price": 0, "Materials": []}
                continue
            current[key] = value
            continue

        if section == "TargetItems" and indent == 6:
            current["TargetItems"].append(key)
            continue

        if section == "Reset":
            if indent == 6 and key in ("Chance", "Price"):
                current["Reset"][key] = value
                continue
            if indent == 6 and key == "Materials":
                material = None
                continue
            if indent == 8 and re.match(r"^\s*-\s+Material:", raw_line):
                material = parse_material_line(raw_line)
                current["Reset"]["Materials"].append(material)
                continue
            if indent == 10 and key == "Amount" and material is not None:
                material["Amount"] = value
                continue

        if section == "Order" and indent == 6 and re.match(r"^\s*-\s+Slot:", raw_line):
            current["Order"].append(parse_scalar(raw_line.split(":", 1)[1]))
            continue

        if section != "Slots":
            continue

        if indent == 6 and re.match(r"^\s*-\s+Slot:", raw_line):
            slot = {
                "Slot": parse_scalar(raw_line.split(":", 1)[1]),
                "Materials": [],
                "EnchantgradeBonus": [],
                "Enchants": [],
                "PerfectEnchants": [],
                "Upgrades": [],
            }
            current["Slots"].append(slot)
            slot_section = None
            material = None
            enchant_group = None
            enchant_item = None
            perfect = None
            upgrade = None
            continue

        if slot is None:
            continue

        if indent == 8:
            material = None
            enchant_group = None
            enchant_item = None
            perfect = None
            upgrade = None
            if key in ("Materials", "EnchantgradeBonus", "Enchants", "PerfectEnchants", "Upgrades"):
                slot_section = key
                continue
            slot[key] = value
            continue

        if slot_section == "Materials":
            if indent == 10 and re.match(r"^\s*-\s+Material:", raw_line):
                material = parse_material_line(raw_line)
                slot["Materials"].append(material)
                continue
            if indent == 12 and key == "Amount" and material is not None:
                material["Amount"] = value
                continue

        if slot_section == "EnchantgradeBonus":
            if indent == 10 and re.match(r"^\s*-\s+Enchantgrade:", raw_line):
                enchant_group = {"Enchantgrade": parse_scalar(raw_line.split(":", 1)[1]), "Chance": 0}
                slot["EnchantgradeBonus"].append(enchant_group)
                continue
            if indent == 12 and key == "Chance" and enchant_group is not None:
                enchant_group["Chance"] = value
                continue

        if slot_section == "Enchants":
            if indent == 10 and re.match(r"^\s*-\s+Enchantgrade:", raw_line):
                enchant_group = {
                    "Enchantgrade": parse_scalar(raw_line.split(":", 1)[1]),
                    "Items": [],
                }
                slot["Enchants"].append(enchant_group)
                enchant_item = None
                continue
            if indent == 12 and key == "Items":
                continue
            if indent == 14 and re.match(r"^\s*-\s+Item:", raw_line):
                if enchant_group is None:
                    raise ParseError(f"{path}:{line_no}: Item without Enchantgrade")
                enchant_item = parse_item_line(raw_line)
                enchant_item["Chance"] = 0
                enchant_group["Items"].append(enchant_item)
                continue
            if indent == 16 and key == "Chance" and enchant_item is not None:
                enchant_item["Chance"] = value
                continue

        if slot_section == "PerfectEnchants":
            if indent == 10 and re.match(r"^\s*-\s+Item:", raw_line):
                perfect = parse_item_line(raw_line)
                perfect["Price"] = 0
                perfect["Materials"] = []
                slot["PerfectEnchants"].append(perfect)
                material = None
                continue
            if indent == 12 and key == "Price" and perfect is not None:
                perfect["Price"] = value
                continue
            if indent == 12 and key == "Materials":
                material = None
                continue
            if indent == 14 and re.match(r"^\s*-\s+Material:", raw_line):
                if perfect is None:
                    raise ParseError(f"{path}:{line_no}: Material without perfect enchant")
                material = parse_material_line(raw_line)
                perfect["Materials"].append(material)
                continue
            if indent == 16 and key == "Amount" and material is not None:
                material["Amount"] = value
                continue

        if slot_section == "Upgrades":
            if indent == 10 and re.match(r"^\s*-\s+Enchant:", raw_line):
                upgrade = {"Enchant": parse_scalar(raw_line.split(":", 1)[1]), "Materials": []}
                slot["Upgrades"].append(upgrade)
                material = None
                continue
            if indent == 12 and key in ("Upgrade", "Price") and upgrade is not None:
                upgrade[key] = value
                continue
            if indent == 12 and key == "Materials":
                material = None
                continue
            if indent == 14 and re.match(r"^\s*-\s+Material:", raw_line):
                if upgrade is None:
                    raise ParseError(f"{path}:{line_no}: Material without upgrade")
                material = parse_material_line(raw_line)
                upgrade["Materials"].append(material)
                continue
            if indent == 16 and key == "Amount" and material is not None:
                material["Amount"] = value
                continue

    finish_entry()
    return entries


def lua_quote(value):
    return '"' + str(value).replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n") + '"'


def lua_bool(value):
    return "true" if value else "false"


def lua_materials(materials):
    if not materials:
        return ""
    return ", " + ", ".join(
        f"{{{lua_quote(material['Material'])}, {int(material.get('Amount', 1))}}}"
        for material in materials
    )


def caution_text(entry):
    lines = []
    refine = int(entry.get("MinimumRefine") or 0)
    if refine > 0:
        lines.append(f"Only items with +{refine} Refine or higher can be enchanted")
    lines.append("Success Chance: 100%")
    reset = entry.get("Reset")
    if reset:
        chance = int(reset.get("Chance") or 0) / 1000
        lines.append(f"Reset Chance: {chance:g}%")
    return "\n".join(lines)


def render_entry(entry):
    entry_id = int(entry["Id"])
    order = entry.get("Order") or [slot["Slot"] for slot in entry.get("Slots", [])]
    lines = [
        f"Table[{entry_id}] = CreateEnchantInfo()",
        f"Table[{entry_id}]:SetSlotOrder({', '.join(str(int(slot)) for slot in order)})",
    ]

    for item in entry.get("TargetItems", []):
        lines.append(f"Table[{entry_id}]:AddTargetItem({lua_quote(item)})")

    lines.append(
        f"Table[{entry_id}]:SetCondition({int(entry.get('MinimumRefine') or 0)}, "
        f"{int(entry.get('MinimumEnchantgrade') or 0)})"
    )
    lines.append(f"Table[{entry_id}]:ApproveRandomOption({lua_bool(entry.get('AllowRandomOptions', True))})")

    reset = entry.get("Reset")
    if reset:
        lines.append(
            f"Table[{entry_id}]:SetReset(true, {int(reset.get('Chance') or 0)}, "
            f"{int(reset.get('Price') or 0)}{lua_materials(reset.get('Materials', []))} )"
        )
    else:
        lines.append(f"Table[{entry_id}]:SetReset(false, 0, 0)")

    lines.append(f"Table[{entry_id}]:SetCaution({lua_quote(caution_text(entry))})")

    for slot in entry.get("Slots", []):
        slot_id = int(slot["Slot"])
        price = int(slot.get("Price") or 0)
        if slot.get("Materials"):
            lines.append(
                f"Table[{entry_id}].Slot[{slot_id}]:SetRequire({price}{lua_materials(slot['Materials'])} )"
            )
        if "Chance" in slot:
            lines.append(f"Table[{entry_id}].Slot[{slot_id}]:SetSuccessRate({int(slot.get('Chance') or 0)})")
        for bonus in slot.get("EnchantgradeBonus", []):
            lines.append(
                f"Table[{entry_id}].Slot[{slot_id}]:SetGradeBonus("
                f"{int(bonus.get('Enchantgrade') or 0)}, {int(bonus.get('Chance') or 0)})"
            )
        for group in slot.get("Enchants", []):
            grade = int(group.get("Enchantgrade") or 0)
            for item in group.get("Items", []):
                lines.append(
                    f"Table[{entry_id}].Slot[{slot_id}]:SetEnchant("
                    f"{grade}, {lua_quote(item['Item'])}, {int(item.get('Chance') or 0)})"
                )
        for item in slot.get("PerfectEnchants", []):
            lines.append(
                f"Table[{entry_id}].Slot[{slot_id}]:AddPerfectEnchant("
                f"{lua_quote(item['Item'])}, {int(item.get('Price') or 0)}"
                f"{lua_materials(item.get('Materials', []))} )"
            )
        for upgrade in slot.get("Upgrades", []):
            lines.append(
                f"Table[{entry_id}].Slot[{slot_id}]:AddUpgradeEnchant("
                f"{lua_quote(upgrade['Enchant'])}, {lua_quote(upgrade.get('Upgrade', ''))}, "
                f"{int(upgrade.get('Price') or 0)}{lua_materials(upgrade.get('Materials', []))} )"
            )

    return "\n".join(lines)


def remove_table_blocks(text, entry_ids):
    wanted = {int(entry_id) for entry_id in entry_ids}
    removed = set()

    pattern = re.compile(
        r"(?:\n{0,2})Table\[(\d+)\] = CreateEnchantInfo\(\)\n"
        r".*?(?=\n{2}Table\[\d+\] = CreateEnchantInfo\(\)|\Z)",
        re.S,
    )

    def replace(match):
        table_id = int(match.group(1))
        if table_id not in wanted:
            return match.group(0)
        removed.add(table_id)
        return ""

    return pattern.sub(replace, text).rstrip(), removed


def merge_output(existing_text, entries, replace_all=False):
    rendered = [render_entry(entry) for entry in sorted(entries, key=lambda e: int(e["Id"]))]
    entry_ids = [entry["Id"] for entry in entries]

    if replace_all or not existing_text:
        header = "-- Generated from rAthena item_enchant.yml by tools/gen_enchantlist.py\n"
        base = header.rstrip()
        removed = set()
    else:
        base, removed = remove_table_blocks(existing_text, entry_ids)

    if base:
        return base.rstrip() + "\n\n" + "\n\n".join(rendered).rstrip() + "\n", removed
    return "\n\n".join(rendered).rstrip() + "\n", removed


def collect_aegis_names(entries):
    """Yield every AegisName the rendered EnchantList.lub will reference."""
    for entry in entries:
        for target in entry.get("TargetItems", []):
            yield target
        for material in (entry.get("Reset") or {}).get("Materials", []):
            yield material["Material"]
        for slot in entry.get("Slots", []):
            for material in slot.get("Materials", []):
                yield material["Material"]
            for group in slot.get("Enchants", []):
                for item in group.get("Items", []):
                    yield item["Item"]
            for perfect in slot.get("PerfectEnchants", []):
                yield perfect["Item"]
                for material in perfect.get("Materials", []):
                    yield material["Material"]
            for upgrade in slot.get("Upgrades", []):
                yield upgrade["Enchant"]
                if upgrade.get("Upgrade"):
                    yield upgrade["Upgrade"]
                for material in upgrade.get("Materials", []):
                    yield material["Material"]


def sync_item_name_table(entries):
    """Add missing AegisNames to ItemDBNameTbl.lub so the client can resolve them."""
    existing = existing_namelist()
    pending = []
    seen = set()
    for name in collect_aegis_names(entries):
        if name in existing or name in seen:
            continue
        seen.add(name)
        pending.append(name)

    if not pending:
        return []

    catalog = build_item_catalog()
    additions = []
    unresolved = []
    for name in pending:
        info = catalog.get(name)
        if info is None:
            unresolved.append(name)
            continue
        additions.append((name, info.item_id))

    if unresolved:
        print("warning: cannot resolve item id for: " + ", ".join(sorted(unresolved)))

    return add_missing_names(additions)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate data/Lua Files/Enchant/EnchantList.lub from item_enchant.yml."
    )
    parser.add_argument("-i", "--input", type=Path, default=DEFAULT_ENCHANT_DB, help="item_enchant.yml path")
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_OUTPUT, help="EnchantList.lub path")
    parser.add_argument("--replace-all", action="store_true", help="write only entries from the input YAML")
    parser.add_argument("--dry-run", action="store_true", help="parse and report without writing")
    return parser.parse_args()


def main():
    args = parse_args()
    entries = parse_item_enchant(args.input)
    if not entries:
        raise SystemExit(f"No Body entries found in {args.input}")

    existing = ""
    if args.output.exists() and not args.replace_all:
        existing = read_text(args.output, encoding="latin-1")

    merged, replaced = merge_output(existing, entries, replace_all=args.replace_all)

    print(
        f"input={args.input} entries={len(entries)} "
        f"ids={int(entries[0]['Id'])}-{int(entries[-1]['Id'])} "
        f"replace_existing={len(replaced)} append_new={len(entries) - len(replaced)}"
    )
    if args.dry_run:
        return

    write_text(args.output, merged, encoding="latin-1")
    print(f"wrote={args.output}")

    added = sync_item_name_table(entries)
    if added:
        joined = ", ".join(name for name, _ in added)
        print(f"ItemDBNameTbl.lub: added {len(added)} entries ({joined})")


if __name__ == "__main__":
    main()
