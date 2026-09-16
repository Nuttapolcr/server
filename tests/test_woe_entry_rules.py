#!/usr/bin/env python3
"""Validate WoE outsider eviction and Chemical Protection removal rules."""
from pathlib import Path
import unittest

import yaml


ROOT = Path(__file__).resolve().parents[1]
CHEMICAL_PROTECTION = ("SC_CP_WEAPON", "SC_CP_SHIELD", "SC_CP_ARMOR", "SC_CP_HELM")


def read_cp874(path):
    return (ROOT / path).read_text(encoding="cp874")


class WoeEntryRuleTests(unittest.TestCase):
    def test_valorfight_2_stone_evicts_only_outsiders(self):
        script = read_cp874("npc/custom-npc/07_guild/valor_fight_2.txt")
        event = script[script.index("OnStoneKilled:"):script.index("S_Award:")]

        self.assertIn("setcastledata .map$,1,.@guild_id;", event)
        self.assertIn("MapRespawnGuildID .map$,.@guild_id,2;", event)
        self.assertNotIn("MapRespawnGuildID .map$,.@guild_id,6;", event)
        self.assertLess(
            event.index("setcastledata .map$,1,.@guild_id;"),
            event.index("MapRespawnGuildID .map$,.@guild_id,2;"),
        )

    def test_all_castle_entries_end_chemical_protection(self):
        source = (ROOT / "src/map/pc.cpp").read_text()
        start = source.index("enum e_setpos pc_setpos(")
        end = source.index("enum e_setpos pc_setpos_savepoint(", start)
        setpos = source[start:end]

        self.assertIn("mapdata->getMapFlag( MF_GVG_CASTLE )", setpos)
        self.assertIn("mapdata->getMapFlag( MF_GVG_TE_CASTLE )", setpos)
        for status in CHEMICAL_PROTECTION:
            self.assertIn(f"status_change_end( &sd->bl, {status} );", setpos)

        status_rows = {
            row["Status"]: row
            for row in yaml.safe_load((ROOT / "db/pre-re/status.yml").read_bytes())["Body"]
        }
        for status in ("Cp_Weapon", "Cp_Shield", "Cp_Armor", "Cp_Helm"):
            self.assertTrue(status_rows[status]["Flags"]["NoClearbuff"])
            self.assertTrue(status_rows[status]["Flags"]["RemoveChemicalProtect"])

    def test_valor_maps_are_castles(self):
        valor_1 = read_cp874("npc/custom-npc/07_guild/valor_fight.txt")
        valor_2 = read_cp874("npc/custom-npc/07_guild/valor_fight_2.txt")
        normal_castles = read_cp874("npc/mapflag/gvg.txt")

        self.assertIn("hero_ent1\tmapflag\tgvg_castle", valor_1)
        self.assertIn("hero_ent2\tmapflag\tgvg_castle", valor_2)
        self.assertIn("prtg_cas01\tmapflag\tgvg_castle", normal_castles)


if __name__ == "__main__":
    unittest.main(verbosity=2)
