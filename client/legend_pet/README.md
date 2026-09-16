# Epic to Legend pet synthesis

Implemented in server-test for HICLASS MA 16/9/69, item 4.

At `maintown,178,157`, open the existing Laphine NPC, choose Pet, then Epic.
Select any four separate Epic eggs from IDs 9283-9292 (matching types are allowed).
One egg from IDs 9293-9298 is guaranteed; all six outcomes have equal weight.
No zeny fee is added. A server-wide announcement includes the player's name and
reward after the new pet egg is successfully added to inventory.

## Client files

- Deploy `data/Lua Files/ItemDBNameTbl.lub` to the same client path.
- Deploy `data/Lua Files/datainfo/lapineddukddakbox.lub` to the same client path.
- Merge `client/legend_pet/itemInfo-additions.lua` into the client's `itemInfo.lub`
  after its `tbl` definition. It adds the UI trigger, item 1500180, using the
  existing `Easter_Egg` resource.

These files are staged in server-test only. The gmtool client files are unchanged.

## Server files

The change adds `Combine_Epic_Pet_Laphine` (1500180), the synthesis recipe,
`PET_COMBINATION_EPIC_REWARD`, and the NPC menu. The new group enum is appended
so existing group IDs retain their values. The map-server must be rebuilt and
started with the updated binary during an explicitly authorized deployment/restart.
File or database reload alone does not load the new C++ constant and pet callback.
No SQL schema migration is required.

## In-game acceptance checks

1. Mix four Epic eggs of different kinds, then four of the same kind: exactly
   four eggs are consumed and exactly one of the six Legend eggs is obtained.
2. Try fewer eggs, a non-Epic egg, and reusing the same inventory slot: reject
   the request without consuming ingredients.
3. Verify that the reward can hatch as its corresponding pet and the successful
   combination produces one announcement visible to another player.
4. Verify Common/Rare combinations and ordinary pet egg creation still work and
   do not produce the new Legend combination announcement.

Local data, Lua syntax, compilation, and callback regression checks are separate
from these in-game checks; no live process is reloaded as part of this task.

## Completed local validation

- `make -C src/map -j4`: map-server build passed.
- `python3 -B tests/test_legend_pet_synthesis.py`: recipe, client Lua, and actual
  pet callback regression checks passed (including interleaved requests, failed
  creation, full inventory, missing pet definitions, and disconnected transport).
- The native rAthena script parser accepted both final NPC bodies and the new
  trigger script in an offline harness; map-server startup and SQL were not run.
- `luac -p` passed for both client data files and the itemInfo merge snippet.
- Windows-874 validation passed for `npc/custom-npc/04_craft/laphine_synthesis.txt`.
- `git diff --check` passed.

The production files and running map-server remain unchanged. In-game acceptance
checks above still require the updated server binary and client files.
