"""Shared helpers for client lub generators.

Provides an item_db YAML catalog (AegisName -> id/display name) and the
ability to patch data/Lua Files/ItemDBNameTbl.lub when a generator emits
AegisNames the client does not yet know about.
"""

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ITEM_DB_NAME_TBL = ROOT / "data/Lua Files/ItemDBNameTbl.lub"

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


class ItemInfo:
    def __init__(self, item_id, aegis_name, display_name):
        self.item_id = item_id
        self.aegis_name = aegis_name
        self.display_name = display_name


def _strip_comment(value):
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


def _strip_quotes(value):
    value = _strip_comment(value).strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in ("'", '"'):
        return value[1:-1]
    return value


def parse_item_db(path):
    """Return {AegisName: ItemInfo} for a single item_db YAML."""
    if not path.exists():
        return {}
    items = {}
    current = None

    def finish():
        if not current or "Id" not in current or "AegisName" not in current:
            return
        aegis = current["AegisName"]
        items[aegis] = ItemInfo(
            item_id=int(current["Id"], 0),
            aegis_name=aegis,
            display_name=current.get("Name", aegis),
        )

    for raw_line in path.read_text(encoding="latin-1").splitlines():
        id_match = re.match(r"^\s*-\s+Id:\s*(.+?)\s*$", raw_line)
        if id_match:
            finish()
            current = {"Id": _strip_quotes(id_match.group(1))}
            continue
        if current is None:
            continue
        field_match = re.match(r"^\s{4}(AegisName|Name):\s*(.+?)\s*$", raw_line)
        if field_match:
            current[field_match.group(1)] = _strip_quotes(field_match.group(2))

    finish()
    return items


def build_item_catalog(paths=None):
    """Merge multiple item_db YAMLs into one AegisName -> ItemInfo dict."""
    catalog = {}
    for path in paths or DEFAULT_ITEM_DBS:
        for aegis, info in parse_item_db(path).items():
            catalog.setdefault(aegis, info)
    return catalog


_IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")


def _is_identifier(name):
    return _IDENT_RE.fullmatch(name) is not None


def _lua_key(name):
    """Render an AegisName as a Lua table key (bare or bracketed string)."""
    if _is_identifier(name):
        return name
    escaped = name.replace("\\", "\\\\").replace('"', '\\"')
    return f'["{escaped}"]'


def existing_namelist(path=ITEM_DB_NAME_TBL):
    """Return AegisNames currently registered in the table.

    Picks up both identifier-style keys (`Name = id`) and bracketed string
    keys (`["Name"] = id`).
    """
    text = path.read_text(encoding="latin-1")
    names = set()
    for match in re.finditer(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*-?\d+",
                             text, re.MULTILINE):
        names.add(match.group(1))
    for match in re.finditer(r'^\s*\[\s*"((?:[^"\\]|\\.)*)"\s*\]\s*=\s*-?\d+',
                             text, re.MULTILINE):
        raw = match.group(1)
        names.add(raw.replace('\\"', '"').replace("\\\\", "\\"))
    return names


def add_missing_names(additions, path=ITEM_DB_NAME_TBL):
    """Append `<name> = <id>,` rows just before the table's closing brace.

    `additions` is an iterable of (aegis_name, item_id) pairs. Names that
    already appear in the file (or earlier in the input) are skipped.
    Returns the pairs actually written.
    """
    existing = existing_namelist(path)
    pending = []
    chosen = set()
    for name, item_id in additions:
        if name in existing or name in chosen:
            continue
        pending.append((name, int(item_id)))
        chosen.add(name)

    if not pending:
        return []

    with path.open("r", encoding="latin-1", newline="") as src:
        text = src.read()
    newline = "\r\n" if "\r\n" in text else "\n"
    body = text.replace("\r\n", "\n")

    end_match = re.search(r"\n\}\s*\nItemDB_To_ItemID\b", body)
    if not end_match:
        raise RuntimeError(f"Cannot find closing brace of ItemDBNameTbl in {path}")

    insert_at = end_match.start()
    before = body[:insert_at].rstrip()
    after = body[insert_at:]

    if not before.endswith(","):
        before = before + ","

    block = "".join(f"\n\t{_lua_key(name)} = {item_id}," for name, item_id in pending)
    new_body = before + block + after
    new_text = new_body.replace("\n", newline)
    with path.open("w", encoding="latin-1", newline="") as out:
        out.write(new_text)
    return pending
