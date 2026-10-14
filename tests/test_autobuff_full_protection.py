#!/usr/bin/env python3
"""Regression checks for Full Chemical Protection in the AI buff bot."""

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
AB_SCRIPT = ROOT / "src/map/ab_script.cpp"
AUTOBUFF = ROOT / "src/map/autobuff.cpp"
AUTOATTACK_SCRIPT = ROOT / "src/map/autoattack_script.cpp"
AUTOATTACK = ROOT / "src/map/autoattack.cpp"
AI_SUPPORT_SKILLS = ROOT / "db/custom/ai_support_skill.txt"


class FullChemicalProtectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ab_script = AB_SCRIPT.read_text()
        cls.autobuff = AUTOBUFF.read_text()
        cls.autoattack_script = AUTOATTACK_SCRIPT.read_text()
        cls.autoattack = AUTOATTACK.read_text()
        cls.ai_support_skills = AI_SUPPORT_SKILLS.read_text(encoding="cp874").splitlines()

    def test_skill_is_registered_as_ai_support(self):
        self.assertIn("CR_FULLPROTECTION", self.ai_support_skills)

    def test_menu_and_handler_accept_multi_status_skill(self):
        self.assertIn("skill->nameid == CR_FULLPROTECTION", self.ab_script)
        self.assertIn(
            "skill_get_sc(skill->nameid) == SC_NONE && skill->nameid != CR_FULLPROTECTION",
            self.ab_script,
        )

    def test_autoattack_menu_and_handler_accept_multi_status_skill(self):
        self.assertIn("|| skill->nameid == CR_FULLPROTECTION", self.autoattack_script)
        self.assertIn("skill_id != CR_FULLPROTECTION", self.autoattack_script)

    def test_recasts_for_any_unprotected_equipped_slot(self):
        start = self.autobuff.index("if (buff.skill_id == CR_FULLPROTECTION) {")
        end = self.autobuff.index(
            "return !target->sc.getSCE(skill_get_sc(buff.skill_id));",
            start,
        )
        body = self.autobuff[start:end]
        for equipment in ("EQP_WEAPON", "EQP_SHIELD", "EQP_ARMOR", "EQP_HEAD_TOP"):
            self.assertIn(equipment, body)
        for status in ("SC_CP_WEAPON", "SC_CP_SHIELD", "SC_CP_ARMOR", "SC_CP_HELM"):
            self.assertIn(status, body)
        self.assertIn("pc_checkequip(target, equipment[i]) >= 0", body)
        self.assertIn("!target->sc.getSCE(protections[i])", body)
        self.assertIn("return true;", body)
        self.assertIn("return false;", body)

    def test_autoattack_recasts_for_any_unprotected_equipped_slot(self):
        start = self.autoattack.index("if (autobuff.skill_id == CR_FULLPROTECTION) {")
        end = self.autoattack.index(
            "// Check if the skill can be used and the player is not already under its effect",
            start,
        )
        body = self.autoattack[start:end]
        for equipment in ("EQP_WEAPON", "EQP_SHIELD", "EQP_ARMOR", "EQP_HEAD_TOP"):
            self.assertIn(equipment, body)
        for status in ("SC_CP_WEAPON", "SC_CP_SHIELD", "SC_CP_ARMOR", "SC_CP_HELM"):
            self.assertIn(status, body)
        self.assertIn("pc_checkequip(sd, equipment[i]) >= 0", body)
        self.assertIn("!sd->sc.getSCE(protections[i])", body)
        self.assertIn("needs_buff = true;", body)


if __name__ == "__main__":
    unittest.main()
