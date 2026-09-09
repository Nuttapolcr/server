# MA 9/9/69 production deployment

Scope: all pending test-server changes, including Sarah/Fenrir access and card, Abyss F4 monsters/items/client assets, bot consumables, PvP party rules, monster density, crafting menus/recipes, HW rewards/season rankings, Collection HP Absorb and magic drain, item/cash-shop/pet/reward changes, character-bound starter rentals, and castle selection updates.

Production-only teleport-scroll behavior and the extra Fadhgridh castle entry were merged into test before verification. The existing test source check preventing autoattack consumables on NOAUTOATTACK maps is included so deployed source matches the tested binary.

Configuration values deployed from the reviewed files:
- HW Zeny price 500,000; jackpot contribution 30,000. Cash price 50; contribution 5.
- New guarantee counters and ranking tables use season 090969, keeping old data and jackpot totals.
- Dungeon density multipliers apply to ordinary monsters; Hydrolancer, Detardeurus and Ktullanux retain their prior counts.
- Bloody Branch box 1500167 now grants 30 branches, matching the latest pending item DB edit.

Database migration: `sql-files/upgrades/2026-09-09-hw-cabinet.sql`, idempotent creation of the two new ranking tables. The existing inventory.bound column supports the starter-rental change and needs no ALTER TABLE. No player counters or ranking history are deleted.

Validation: map and char builds passed; final item databases, all barter imports and affected NPC scripts loaded in an isolated test map-server; expected counts matched on all nine changed/new dungeon maps; new magic-drain constant and relevant shops loaded; PvP NOPARTY removal checked; Windows-874 and Lua syntax passed. Existing Fan in Mouth/Blood Sucker armor-level warnings remain. Individual task validations are in the conversation and related task notes.

Files and atomically replaced map-server/char-server binaries are deployed without signaling, reloading, stopping or starting services. Configuration files, login-server binary and environment-specific settings are not part of this release. New code requires a later separately authorized server start/restart. Production map-server was not running during the initial process check.

Client assets are copied under server/client as deployment artifacts, not installed into player clients or the gmtool asset source. See `doc/abyss_04.md` for missing map assets and unsupported Dragon Power enchant definitions, and `doc/hp_absorb.md` for the itemInfo delta and drain stacking. Do not replace a current itemInfo table with the older complete Abyss staging snapshot; merge the supplied deltas instead.

Rollback backup and hashes: `/home/valor_ro/server-test/.deploy-ma-090969/`. Database additions are safe to retain during a file rollback. Earlier task notes describing production as unchanged refer to the pre-deployment implementation stage.
