import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT_PATH = ROOT / "npc/custom-npc/08_event/cabinet_jackpot.txt"


def read_script() -> str:
    return SCRIPT_PATH.read_text(encoding="cp874")


def parse_array(script: str, name: str):
    match = re.search(rf"setarray \${name}\[0\], (?P<body>[^;]+);", script)
    assert match is not None
    return [int(value) for value in match.group("body").split(",")]


def test_highlighted_211069_cabinet_changes():
    script = read_script()

    assert "F_Cabinet_ShowTop" not in script
    assert "F_Cabinet_RecordSpin" not in script
    assert "ดูอันดับ 1-10" not in script
    assert "cabinet_hw_zeny_ranking" not in script
    assert "cabinet_hw_cash_ranking" not in script

    assert "CabinetHWZenySpins071069 % 120" in script
    assert ".@next_spins % 120 == 0" in script
    assert "CabinetHWCashSpins071069 % 40" in script
    assert ".@next_spins % 40 == 0" in script
    assert "getitem 1500208" not in script
    assert script.count("getitem 1030066, 1;") == 2

    zeny_items = parse_array(script, "CabinetHWZenyItem")
    zeny_amounts = parse_array(script, "CabinetHWZenyAmount")
    zeny_rates = parse_array(script, "CabinetHWZenyRate")
    assert len(zeny_items) == len(zeny_amounts) == len(zeny_rates) == 44
    assert zeny_items[-16:] == [
        1030043, 1030044, 1030045, 1500170, 1500171, 1500032,
        1500033, 1500034, 1500094, 1500095, 1500096, 1500097,
        1500098, 1500099, 1500199, 1500200,
    ]
    assert zeny_amounts[-16:] == [1] * 16
    assert zeny_rates[-16:] == [30, 30, 30, 50, 50, 20, 20, 20,
                                20, 20, 20, 20, 20, 10, 20, 20]
    assert sum(zeny_rates) == 9730

    cash_items = parse_array(script, "CabinetHWCashItem")
    cash_amounts = parse_array(script, "CabinetHWCashAmount")
    cash_rates = parse_array(script, "CabinetHWCashRate")
    assert len(cash_items) == len(cash_amounts) == len(cash_rates) == 57
    assert cash_rates[:18] == [
        300, 300, 300, 700, 600, 500, 400, 400, 500,
        300, 300, 800, 500, 300, 300, 350, 350, 700,
    ]
    assert cash_items[-28:] == [
        1030043, 1030044, 1030045, 1500168, 1500169, 1500170,
        1500171, 1500172, 1500026, 1500027, 1500028, 1500029,
        1500030, 1500031, 1500032, 1500033, 1500034, 1500094,
        1500095, 1500096, 1500097, 1500098, 1500100, 1500099,
        1500197, 1500198, 1500199, 1500200,
    ]
    assert cash_amounts[-28:] == [1] * 28
    assert cash_rates[-28:] == [30] * 5 + [50] * 17 + [30, 40] + [50] * 4
    assert sum(cash_rates) == 10630


if __name__ == "__main__":
    test_highlighted_211069_cabinet_changes()
    print("Cabinet jackpot tests passed")
