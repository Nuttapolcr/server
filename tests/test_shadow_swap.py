#!/usr/bin/env python3
"""Regression checks for the five Shadow swap categories."""

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "npc/custom-npc/04_craft/weapon_swap.txt"
ITEM_DB = ROOT / "db/pre-re/item_db_custom.yml"


class ShadowSwapTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = SCRIPT.read_text(encoding="cp874")
        cls.item_db = ITEM_DB.read_text()

    def test_shadow_menu_and_fixed_cost(self):
        self.assertIn("S_SelectShadow:", self.source)
        for category in (
            "Shadow Armor",
            "Shadow Shield",
            "Shadow Shoes",
            "Shadow Earring",
            "Shadow Pendant",
        ):
            self.assertIn(category, self.source)
        self.assertRegex(
            self.source,
            r"else if \(\.@category >= 7\) \{\s*"
            r"\.@card_cost = 2;\s*"
            r"\.@zeny_cost = 5000000;",
        )

    def test_shadow_category_boundaries_and_item_ids(self):
        self.assertIn(
            "setarray .CategoryStart[0], 0, 72, 84, 96, 108, 120, 132, 146, 149, 152, 155, 159;",
            self.source,
        )
        self.assertIn(
            "setarray .CategoryCount[0], 24, 4, 4, 4, 4, 4, 14, 3, 3, 3, 4, 4;",
            self.source,
        )

        expected_groups = (
            (1500035, 1500039, 1500043),
            (1500057, 1500058, 1500059),
            (1500036, 1500040, 1500044),
            (1500037, 1500041, 1500045, 1500055),
            (1500038, 1500042, 1500046, 1500056),
        )
        item_block = self.source.split("setarray .SwapItem[0],", 1)[1].split("waitingroom", 1)[0]
        item_ids = [int(value) for value in re.findall(r"\b\d+\b", item_block)]
        expected_ids = [item_id for group in expected_groups for item_id in group]
        self.assertEqual(item_ids[-len(expected_ids):], expected_ids)
        for item_id in expected_ids:
            self.assertEqual(self.item_db.count("  - Id: {}\n".format(item_id)), 1)

    def test_all_random_options_are_preserved(self):
        for option_index in range(1, 6):
            self.assertIn("@inventorylist_option_id{}".format(option_index), self.source)
            self.assertIn("@inventorylist_option_value{}".format(option_index), self.source)
            self.assertIn("@inventorylist_option_parameter{}".format(option_index), self.source)
        self.assertIn("getitembound4 .@target_id", self.source)


if __name__ == "__main__":
    unittest.main()
