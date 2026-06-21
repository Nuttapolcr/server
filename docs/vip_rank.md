# VIP Rank Server System

- Rank config: `db/custom/vip_rank.yml`
- SQL migration: `sql-files/upgrades/vip_rank.sql`
- Runtime: `src/map/vip_rank.hpp` and `src/map/vip_rank.cpp`
- Account data loads from `pc_authok` in `src/map/pc.cpp`.
- Bonuses apply from `status_calc_pc_sub` in `src/map/status.cpp`.
- `Script` uses normal item bonus syntax and applies on every map.
- `PveScript` uses normal item bonus syntax and is skipped when `map_flag_vs2` is true.
- `WoeScript` uses normal item bonus syntax and applies only when `map_flag_gvg2` is true: GvG, WoE, TE, and castle maps.
- rAthena weight uses x10 units, so `bonus bAddMaxWeight,10000;` displays as +1,000 Weight.
- Each Rank reads its `Icon` from YAML and accepts any valid `EFST_*` constant.
- Use `Icon: EFST_BLANK` to hide a Rank icon. Rank 0 has no icon.
- Icons are sent directly instead of starting `SC_MEMBER*`, because the legacy statuses contain separate bonus scripts.

## Script API

- `vip_rank_info(type)`
  - `VIP_RANK_INFO_RANK`
  - `VIP_RANK_INFO_DONATION_TOTAL`
  - `VIP_RANK_INFO_BOX_SPEED`
- `vip_rank_box_time(base_ms, minimum_ms)`
- `vip_rank_update(amount, operation, reference, reason)`
  - operation `0`: set
  - operation `1`: add
  - operation `2`: refund

## Commands

- `@viprank [character name|account ID]`
- `@viprankset <amount> <character name|account ID>`
- `@viprankadd <amount> <character name|account ID>`
- `@viprankrefund <amount> <character name|account ID>`
- `@reloadviprankdb`
