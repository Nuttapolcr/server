import re
import struct
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT_PATH = ROOT / "npc/custom-npc/08_event/valor_mvp_battlefield.txt"
MAPS = ("valor_easy", "valor_norm", "valor_hard")

EPIC_IDS = {
    600017, 630012, 640020, 540020, 520008, 500026, 550024, 560018,
    610021, 510032, 700032, 570018, 580017, 540023, 550026, 650009,
    800003, 500025, 1030018, 1030019, 530013, 700031, 1030051, 1030052,
    452029, 452030, 452031, 452032, 480425, 480426, 480427, 480428,
    470282, 470283, 470284, 470285, 490503, 490504, 490505, 490506,
    490491, 490492, 490493, 490494, 1030010,
}

LEGEND_IDS = {
    21016, 1450, 1600, 28605, 28107, 13455, 16041, 1800, 28008, 28706,
    18121, 1900, 1996, 1029019, 2026, 13328, 13128, 13468, 1030022,
    1030023, 1400, 1030024, 1030053, 1030054, 452033, 452034, 452035,
    452036, 480429, 480430, 480431, 480432, 470286, 470287, 470288,
    470289, 490507, 490508, 490509, 490510, 490495, 490496, 490497,
    490498, 1030011,
}


def read_script() -> str:
    return SCRIPT_PATH.read_text(encoding="cp874")


def parse_array(script, name):
    match = re.search(
        rf"setarray \.@{name}\[0\],(?P<body>.*?);",
        script,
        flags=re.DOTALL,
    )
    assert match is not None
    return {int(value) for value in re.findall(r"\d+", match.group("body"))}


def read_map_cache_entries():
    data = (ROOT / "db/pre-re/map_cache.dat").read_bytes()
    file_size, map_count = struct.unpack_from("<IH2x", data)
    assert file_size == len(data)
    offset = 8
    result = {}
    for _ in range(map_count):
        raw_name, width, height, length = struct.unpack_from("<12shhI", data, offset)
        offset += 20
        name = raw_name.split(b"\0", 1)[0].decode("ascii")
        cells = data[offset : offset + length]
        offset += length
        result[name] = (width, height, cells)
    assert offset == len(data)
    return result


def test_three_maps_are_registered_and_clone_geffen_dungeon():
    maps_conf = (ROOT / "conf/maps_athena.conf").read_text(encoding="cp874")
    map_index = (ROOT / "db/map_index.txt").read_text(encoding="cp874")
    resource_names = (ROOT / "data/resnametable.txt").read_text(encoding="cp874")
    cache = read_map_cache_entries()

    assert "map: valor_pk" not in maps_conf
    for map_name in MAPS:
        assert f"map: {map_name}" in maps_conf
        assert re.search(rf"^{map_name}$", map_index, flags=re.MULTILINE)
        assert f"{map_name}.gat#gef_dun03.gat#" in resource_names
        assert f"{map_name}.rsw#gef_dun03.rsw#" in resource_names
        assert cache[map_name] == cache["gef_dun03"]


def test_boss_counts_respawns_and_special_boss_removal():
    script = read_script()
    rows = re.findall(
        r"^(valor_(?:easy|norm|hard)),0,0,0,0\tmonster\t[^\t]+\t"
        r"(?P<mob_id>\d+),(?P<count>\d+),(?P<delay1>\d+),(?P<delay2>\d+),"
        r"ValorMvpBattlefield::OnValorMobKill$",
        script,
        flags=re.MULTILINE,
    )
    expected = {
        "valor_easy": (1, 40000),
        "valor_norm": (1, 20000),
        "valor_hard": (2, 20000),
    }
    for map_name, (count, delay) in expected.items():
        map_rows = [row for row in rows if row[0] == map_name]
        assert len(map_rows) == 35
        assert all(int(row[2]) == count for row in map_rows)
        assert all(int(row[3]) == delay and int(row[4]) == delay for row in map_rows)
        assert all(int(row[1]) != 1708 for row in map_rows)

    assert "Memory of Thanatos" not in script
    assert "ValorMvpThanatosChest" not in script


def test_difficulty_damage_drop_and_equipment_rules():
    script = read_script()
    battle = (ROOT / "src/map/battle.cpp").read_text()
    mob = (ROOT / "src/map/mob.cpp").read_text()
    treasure = (ROOT / "npc/custom-npc/08_event/mvp_treasure.txt").read_text(encoding="cp874")

    assert parse_array(script, "epic") == EPIC_IDS
    assert parse_array(script, "legend") == LEGEND_IDS
    assert '(.@difficulty == 1 && inarray(.@epic,.@item_id) >= 0)' in script
    assert "OnPCLoadMapEvent:" in script
    assert "addtimer 1000,strnpcinfo(0)+\"::OnGearCheck\";" in script
    for map_name in MAPS:
        assert f"{map_name}\tmapflag\tloadevent" in script

    for map_name, rate in zip(MAPS, (90, 50, 10)):
        assert f'map_mapname2mapid( "{map_name}" )' in battle
        assert f"received_damage_rate = {rate};" in battle
        assert f'map_mapname2mapid("{map_name}")' in mob

    assert "suppress_valor_mvp_drops = mob_is_valor_mvp_battlefield_map" in mob
    for item_id in (1000445, 1000401, 1001140, 1000373):
        assert f"makeitem {item_id},1" in script
    assert "SC_ITEMBOOST" in script
    assert ".@rare_rate = 300;" in script
    assert ".@rare_rate = 500;" in script
    assert ".@rare_rate = 1000;" in script
    assert ".@stone_rate = 500;" in script
    assert ".@stone_rate = 1000;" in script
    assert ".@stone_rate = 2000;" in script
    for map_name in MAPS:
        assert f'.@map$ == "{map_name}"' in treasure


def test_pvp_maps_allow_the_two_requested_potions():
    script = read_script()
    restrictions = (ROOT / "db/pre-re/item_noequip.txt").read_bytes()
    usable_db = (ROOT / "db/pre-re/item_db_usable.yml").read_text()

    for map_name in MAPS:
        assert f"{map_name}\tmapflag\tpvp" in script
    assert not re.search(br"^11548,", restrictions, flags=re.MULTILINE)
    assert not re.search(br"^11549,", restrictions, flags=re.MULTILINE)
    for item_id in (11548, 11549):
        item = re.search(
            rf"^  - Id: {item_id}$.*?(?=^  - Id: )",
            usable_db,
            flags=re.MULTILINE | re.DOTALL,
        )
        assert item is not None
        for map_name in MAPS:
            assert f'"{map_name}"' in item.group(0)


def test_entry_is_free_without_ticket_or_expiry():
    script = read_script()

    assert "เข้าฟรีทุกระดับ ไม่ใช้ตั๋ว" in script
    assert 'select("เข้าดัน Easy:เข้าดัน Normal:เข้าดัน Hard:ยกเลิก")' in script
    assert "1030071" not in script
    assert "#valor_mvp_until" not in script
    assert "OnExpire:" not in script


if __name__ == "__main__":
    test_three_maps_are_registered_and_clone_geffen_dungeon()
    test_boss_counts_respawns_and_special_boss_removal()
    test_difficulty_damage_drop_and_equipment_rules()
    test_pvp_maps_allow_the_two_requested_potions()
    test_entry_is_free_without_ticket_or_expiry()
    print("Valor MVP Battlefield tests passed")
