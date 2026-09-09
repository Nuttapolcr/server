# Abyss Lake F4 — 9 September 2026 maintenance, item 2

Implemented in `/home/valor_ro/server-test`. Production files and running services were not changed.

## Entry and spawns

Use **Warper → Dungeons → Abyss Lakes → Abyss Lakes 4**. The destination is `abyss_04,169,159`; the map cache confirms that cell is walkable. Both the map index and map configuration already register the map.

| ID | Monster | Count | Respawn | HP | Base EXP | Job EXP |
|---|---|---:|---:|---:|---:|---:|
| 20612 | Purple Ferus | 25 | 5 seconds | 3,404,262 | 243,162 | 170,213 |
| 20613 | Treasure Mimic | 15 | 5 seconds | 3,436,608 | 245,472 | 171,830 |
| 20614 | Black Acidus | 35 | 5 seconds | 3,455,504 | 246,822 | 172,775 |
| 20615 | Silver Acidus | 35 | 5 seconds | 3,456,051 | 246,822 | 172,775 |
| 20616 | Bone Ferus | 50 | 5 seconds | 3,474,420 | 248,173 | 173,721 |
| 20617 | Bone Acidus | 50 | 5 seconds | 3,491,700 | 249,407 | 174,585 |
| 20618 | Bone Detardeurus | 1 | 3 hours | 88,466,828 | 6,150,908 | 4,305,636 |

Sources: [map and population](https://www.divine-pride.net/database/map/abyss_04), individual monster pages `/database/monster/20612` through `/20618`, read on 7 September 2026. Values use the default monster pages and the 100% EXP rows. Individual monster pages specify element level 4; the map summary displays 5 and is not used for that field. The Bone monsters use the Undead race. The boss receives 10% damage.

Each monster has eight normal drops. MVP rewards are Dragon's Treasure 50%, Stable Dragon's Power 40%, and Turbulent Dragon's Power 30%. Rates in the new database are **base rates**; the existing server multipliers and drop bonuses still apply. No global battle settings were changed.

## Files and activation

- `db/pre-re/mob_db.yml`: seven monsters stored directly in the main Pre-Renewal monster database.
- `db/pre-re/item_db_etc.yml` and `db/pre-re/item_db_usable.yml`: 17 card/material items and five usable/container items, stored directly in the main files.
- `db/pre-re/item_group_db.yml`: Dragon's Treasure's 15 outcomes, matching the [reference probabilities](https://www.divine-pride.net/database/item/23985).
- `db/pre-re/item_combos.yml`: five combinations involving the new cards and existing cards.
- `npc/pre-re/mobs/dungeons/abyss_04.txt`: population and `nomemo` flag.
- `npc/custom-npc/11_system/WarperRE.txt`: fourth floor menu and navigation entry; the previous Sarah and Fenrir change is preserved.

The MOB_DB parser now accepts an optional pair, `MagicAttack` and `MagicAttack2`, for explicit minimum/maximum MATK. Both fields are required when either is specified; an inverted range is rejected. With neither field, or with both set to zero, existing INT-based calculation remains in use. These explicit bounds are also applied when monster base status is recalculated. Physical ATK uses the existing Pre-Renewal minimum/maximum fields. Other combat calculations remain those of this server.

`make -j4 map` completed successfully. The newly built test binary is on disk; the running test map-server has not been restarted. **Activating the new MATK fields requires starting the rebuilt binary, not only reloading the scripts/database.** Production promotion and production restart/reload remain separate actions.

## Client staging and remaining limitations

`client/abyss_04/system/itemInfo.lub` is a staged copy of the existing gmtool client file with 15 missing records appended. The seven existing card records and all other original bytes are preserved. `client/abyss_04/itemInfo-additions.lua` contains only those additions, for applying to a newer client table if needed. The source file SHA-256 was `37a5652af8413fb3e22bf8b8d30f7fe595a5ed4ca1699de4dd87e9558f691fd7`. English additions and original resource names come from [zackdreaver/ROenglishRE](https://github.com/zackdreaver/ROenglishRE/blob/master/Ragnarok/System/itemInfo.lua), retaining its attribution.

This client staging contains item descriptions only. The actual game client must have the map's GAT/GND/RSW files and corresponding monster/item graphics. The gmtool GRF copies inspected here contain the `abyss_04` minimap, but no `abyss_04` GAT/GND/RSW files. No map or monster graphics were fabricated, and no client files outside `server-test` were modified.

Dragon's Power items 23981, 100142, 100144, and 100145 retain their original `laphine_upgrade()` scripts. The existing server has no implemented upgrade rules for these items; their enchant functionality is not enabled by this change. Dragon's Treasure itself has a populated, load-tested reward group. This change covers the requested monster stats, EXP, drops, and entry; it does not add monster skill rows, armor crafting NPCs, or enchant distributions that are absent from the existing database.

## Validation

- Isolated native `map-server --run-once`, using the test database and a copy of the rebuilt binary: exit 0, no error messages, `ABYSS_VERIFY_PASS`.
- 211 spawned monsters, with the expected count for every ID. Dynamic spawning was disabled only in the isolated validation configuration to populate maps without a connected character.
- All seven monsters' HP, level, six primary stats, DEF/MDEF, ATK bounds, base/job EXP, and spawned MATK bounds matched the source fixture.
- All 56 normal drop IDs and base probabilities matched the fixture. Drop multipliers were normalized only in the isolated check; production/test configuration files retain their existing rates.
- A control Poring without explicit MATK retained the ordinary INT-based MATK calculation.
- Native loader accepted 22 new items, one reward group, and five card combinations. After consolidation into the main databases, moved records were checked against the original records for equality; the separate Abyss YAML files and their imports were removed.
- Both staged client Lua files pass `luac -p`.
- Final modified `.txt` and `.conf` files pass `scripts/ensure_windows874.py`; `git diff --check` passes.
- Existing production/test map-server PIDs remained running throughout. An in-game client session was not performed.
