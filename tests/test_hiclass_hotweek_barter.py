import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BARTER_PATH = ROOT / "npc/custom-npc/04_craft/craft_all_in_one_barter.yml"


def hotweek_section():
    text = BARTER_PATH.read_text()
    match = re.search(
        r"^  - Name: CRAFT_ALL_IN_ONE_HOTWEEK$"
        r"(?P<body>.*?)"
        r"^  - Name: CRAFT_SHADOW_9$",
        text,
        flags=re.MULTILINE | re.DOTALL,
    )
    assert match is not None
    return match.group("body")


def test_sheet_item_4_rewards_are_in_hotweek_without_zeny_cost():
    body = hotweek_section()
    expected = {
        54: ("Bloody_Branch_40_Box", 1),
        55: ("Refine_Booster_Potion_5", 1),
        56: ("Refine_Booster_Potion_10", 2),
    }

    for index, (item, amount) in expected.items():
        recipe = re.search(
            rf"^      - Index: {index}$"
            rf"(?P<body>.*?)(?=^      - Index: |\Z)",
            body,
            flags=re.MULTILINE | re.DOTALL,
        )
        assert recipe is not None
        recipe_body = recipe.group("body")
        assert re.search(rf"^        Item: {re.escape(item)}(?:\s+#.*)?$", recipe_body, re.MULTILINE)
        assert "Zeny:" not in recipe_body
        assert "Item: Coin_Varlor_V.1 # 1030066" in recipe_body
        assert f"Amount: {amount}" in recipe_body


if __name__ == "__main__":
    test_sheet_item_4_rewards_are_in_hotweek_without_zeny_cost()
    print("HICLASS Hotweek barter tests passed")
