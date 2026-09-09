# Collection HP Absorb (MA 9/9/69, task 10)

Item 1500173 (`Collection_HP_Absorb`) is the third CABINET_GUARANTEE recipe.
One item costs three Coin Valor V.4 (1500166), with no Zeny fee.
The sheet repeats Bloody Branch (40) Box twice; this task adds only the requested new item and preserves the existing single Branch recipe.

The existing Charm item and CharmMaxEffect: 3 are preserved. Its CharmScript excludes MF_GVG and MF_GVG_CASTLE, matching the existing client NO GVG description, and grants:

```text
bonus2 bHPDrainRate,100,10;
bonus2 bMagicHPDrainRate,100,10;
```

Chance uses 1/1000 units: 100 is 10%. Healing uses percent: 10 is 10% of calculated damage, following the existing drain calculation (minimum 1 HP on a successful positive hit). Normal attacks and weapon skills use the existing physical drain path. Magic skills use the new separate bonus. Self hits, reflected magic, and non-positive damage do not trigger the new magic effect. Hybrid weapon/magic skills use the existing physical path once.

Bonuses follow existing additive drain stacking: one copy contributes 100 chance units and 10 healing percent; three active copies contribute 300 and 30 before other bonuses. No change was made to the existing physical drain bonus or the collection stack limit. The new magic fields are reset by the existing status bonus reset on recalculation.

Client delta: `client/hp_absorb/itemInfo-additions.lua`. It preserves the existing gmtool resource and item metadata and mirrors the identified description into both description tables. Apply after the existing `tbl` entries before returning the itemInfo table. The gmtool source was read only.

Validation:
- `make -j4 map` passed (existing unrelated compiler warnings remain).
- Compiled isolated tests using the production drain functions, bonus switch branch, and magic skill hook: enumerated all 1000 RNG outcomes (exactly 100 successes), checked healing amount, no effect without a bonus, null/self/reflect/hybrid/no-damage exclusions, integer saturation, bonus accumulation and reset.
- Isolated native map-server loaded final item DB, new script constant and barter shop successfully (`ABSORB10_OK`).
- Lua syntax and mirrored client descriptions passed; recipe IDs, amount and placement verified.
- No logged-in-player combat test was performed. No running test or production server was reloaded or restarted.

Activation needs the newly built map-server binary; script reload alone cannot add the new bonus implementation. Production and the client source remain unchanged.
