#!/usr/bin/env python3
"""Validate the complete, reversible pet egg and Collection barter."""
from pathlib import Path
import unittest

import yaml


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_PAIRS = [
    ("Golden_Bug_Egg", "Col_Golden_Bug"),
    ("Maya_Egg", "Col_Maya"),
    ("Eddga_Egg", "Col_Eddga"),
    ("Dark_Lord_Egg", "Col_Dark_Lord"),
    ("Baphomet_Egg", "Col_Baphomet"),
    ("Orc_Lord_Egg", "Col_Orc_Lord"),
    ("Lady_Tanee_Egg", "Col_Lady_Tanee"),
    ("Phreeoni_Egg", "Col_Phreeoni"),
    ("Orc_Hero_Egg", "Col_Orc_Hero"),
    ("Amon_Ra_Egg", "Col_Amon_Ra"),
    ("Pharaoh_Egg", "Col_Pharaoh"),
    ("Bacsojin_Egg", "Col_Bacsojin"),
    ("Osiris_Egg", "Col_Osiris"),
    ("Dracula_Egg", "Col_Dracula"),
    ("Doppelganger_Egg", "Col_Doppelganger"),
    ("Mistress_Egg", "Col_Mistress"),
    ("Moonlight_Egg", "Col_Moonlight"),
    ("Drake_Egg", "Col_Drake"),
    ("Knight_Of_Windstorm_Egg", "Col_Stormy_Knight"),
    ("Lord_Of_Death_Egg", "Col_Lord_of_Death"),
    ("B_Seyren_Egg", "Col_Seyren"),
    ("B_Eremes_Egg", "Col_Eremes"),
    ("B_Harword_Egg", "Col_Howard"),
    ("B_Magaleta_Egg", "Col_Margaretha"),
    ("B_Shecil_Egg", "Col_Cecil"),
    ("B_Katrinn_Egg", "Col_Kathryne"),
    ("B_Randel_Egg", "Col_Randel"),
    ("B_Flamel_Egg", "Col_Flamel"),
    ("B_Celia_Egg", "Col_Celia"),
    ("B_Chen_Egg", "Col_Chen"),
    ("Medjay_Egg", "Col_MedJay"),
    ("Cp_Heka_Egg", "Col_MedJee"),
    ("Medjed_Egg", "Col_MedJed"),
    ("Spirit_R_Flame_Sl_Egg", "Col_Spirit_Flame"),
    ("Spirit_I_Water_M_Egg", "Col_Spirit_Water"),
    ("Spirit_S_Wind_M_Egg", "Col_Spirit_Wind"),
]


def load_yaml(path):
    return yaml.safe_load((ROOT / path).read_bytes())


class PetCollectionBarterTests(unittest.TestCase):
    def test_all_pairs_are_one_to_one_and_reversible(self):
        body = load_yaml("npc/custom-npc/04_craft/pet_collection_barters.yml")["Body"]
        shops = {shop["Name"]: shop for shop in body}
        egg_shop = shops["HICLASS_PET_EGG_TO_COLLECTION"]
        collection_shop = shops["HICLASS_PET_COLLECTION_TO_EGG"]

        self.assertFalse(egg_shop["CraftBarter"])
        self.assertFalse(collection_shop["CraftBarter"])
        self.assertEqual([row["Index"] for row in egg_shop["Items"]], list(range(36)))
        self.assertEqual([row["Index"] for row in collection_shop["Items"]], list(range(36)))

        egg_to_collection = []
        for row in egg_shop["Items"]:
            self.assertNotIn("Zeny", row)
            self.assertEqual(len(row["RequiredItems"]), 1)
            required = row["RequiredItems"][0]
            self.assertEqual(required, {"Index": 0, "Item": required["Item"], "Amount": 1})
            egg_to_collection.append((required["Item"], row["Item"]))

        collection_to_egg = []
        for row in collection_shop["Items"]:
            self.assertNotIn("Zeny", row)
            self.assertEqual(len(row["RequiredItems"]), 1)
            required = row["RequiredItems"][0]
            self.assertEqual(required, {"Index": 0, "Item": required["Item"], "Amount": 1})
            collection_to_egg.append((row["Item"], required["Item"]))

        self.assertEqual(egg_to_collection, EXPECTED_PAIRS)
        self.assertEqual(collection_to_egg, EXPECTED_PAIRS)

    def test_items_and_pet_definitions_exist(self):
        items = {}
        for path in sorted((ROOT / "db/pre-re").glob("item_db*.yml")):
            document = yaml.safe_load(path.read_bytes())
            for row in document.get("Body", []):
                items[row["AegisName"]] = row
        pet_eggs = {
            row["EggItem"]
            for row in load_yaml("db/pre-re/pet_db.yml")["Body"]
        }

        for egg, collection in EXPECTED_PAIRS:
            self.assertEqual(items[egg]["Type"], "PetEgg")
            self.assertIn(egg, pet_eggs)
            self.assertEqual(items[collection]["Type"], "Etc")
            self.assertTrue(items[collection]["Flags"]["Collection"])

        self.assertEqual([items[egg]["Id"] for egg, _ in EXPECTED_PAIRS[-6:]], list(range(9293, 9299)))
        self.assertEqual([items[item]["Id"] for _, item in EXPECTED_PAIRS[-6:]], list(range(1001755, 1001761)))

    def test_npc_is_enabled_at_configured_coordinate(self):
        menu = (ROOT / "npc/custom-npc/04_craft/pet_collection_barter_menu.txt").read_text(encoding="cp874")
        self.assertIn("morocc,139,144,4\tscript\tNPCแลกไข่ Collection\t841,{", menu)
        self.assertIn('callshop "HICLASS_PET_EGG_TO_COLLECTION",1;', menu)
        self.assertIn('callshop "HICLASS_PET_COLLECTION_TO_EGG",1;', menu)

        custom_conf = (ROOT / "npc/custom-npc.conf").read_text(encoding="cp874")
        barter_imports = (ROOT / "npc/pre-re/merchants/barters.yml").read_text()
        self.assertIn("npc: npc/custom-npc/04_craft/pet_collection_barter_menu.txt", custom_conf)
        self.assertIn("Path: npc/custom-npc/04_craft/pet_collection_barters.yml", barter_imports)


if __name__ == "__main__":
    unittest.main(verbosity=2)
