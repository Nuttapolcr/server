#!/usr/bin/env python3
"""Validate City Titan's canonical PRE record and drop table."""

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PRE_MOBS = ROOT / "db/pre-re/mob_db.yml"
IMPORT_MOBS = ROOT / "db/import/mob_db.yml"
PRE_SKILLS = ROOT / "db/pre-re/mob_skill_db.txt"
IMPORT_SKILLS = ROOT / "db/import/mob_skill_db.txt"


class CityTitanDropTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.pre_mobs = PRE_MOBS.read_text()
        cls.import_mobs = IMPORT_MOBS.read_text()
        cls.pre_skills = PRE_SKILLS.read_text(encoding="cp874")
        cls.import_skills = IMPORT_SKILLS.read_text(encoding="cp874")

    def test_monster_record_is_only_in_canonical_pre_database(self):
        self.assertEqual(len(re.findall(r"^  - Id: 31100$", self.pre_mobs, re.MULTILINE)), 1)
        self.assertNotRegex(self.import_mobs, r"^  - Id: 31100$")

    def test_memory_of_rudolf_card_is_not_a_city_titan_drop(self):
        start = self.pre_mobs.index("  - Id: 31100\n")
        match = re.search(r"^  - Id: \d+$", self.pre_mobs[start + 1 :], re.MULTILINE)
        end = start + 1 + match.start() if match else len(self.pre_mobs)
        titan = self.pre_mobs[start:end]
        self.assertIn("AegisName: CITY_THANATOS", titan)
        self.assertIn("Item: Thanatos_Card", titan)
        self.assertNotIn("Memory_of_Rudolf_Card", titan)

    def test_skill_rows_are_only_in_canonical_pre_database(self):
        pre_rows = re.findall(r"^31100,", self.pre_skills, re.MULTILINE)
        import_rows = re.findall(r"^31100,", self.import_skills, re.MULTILINE)
        self.assertEqual(len(pre_rows), 25)
        self.assertEqual(import_rows, [])


if __name__ == "__main__":
    unittest.main()
