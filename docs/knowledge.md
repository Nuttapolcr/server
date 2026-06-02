# คลังความรู้ — Custom RO Server (pre-re rAthena)

> ไฟล์นี้คือ **ข้อมูล** ไม่ใช่กฎ — กฎอยู่ใน `CLAUDE.md`
> อัพเดตทุกครั้งที่ค้นพบข้อมูลใหม่

---

## ภาพรวม Project

Private Ragnarok Online server รัน **rAthena** โหมด **pre-renewal (pre-re)**

| ไฟล์ | หน้าที่ |
|---|---|
| `db/pre-re/item_db_equip.yml` | ฐานข้อมูลที่ใช้งานจริง (~43k+ lines) |
| `db/re/item_db_equip.yml` | Reference เท่านั้น (~236k lines) ดึง base structure ได้ |

---

## วิธีค้นหา Item

```bash
# ตรวจว่ามีใน pre-re ไหม
grep -n "  - Id: <ID>$" db/pre-re/item_db_equip.yml

# ถ้าไม่มี ดึงจาก re (บอก line number แล้วใช้ sed)
grep -n "  - Id: <ID>$" db/re/item_db_equip.yml
sed -n '<line>,<line+60>p' db/re/item_db_equip.yml
```

---

## รูปแบบ YAML

```yaml
  - Id: 21016
    AegisName: Sinister_Twohand_Sword   # ห้ามมีเว้นวรรค
    Name: Two-Handed Sword of Vicious Mind
    Type: Weapon
    SubType: 2hSword
    Buy: 20
    Weight: 2200           # display weight x 10
    Attack: 300
    MagicAttack: 290       # ใส่เฉพาะเมื่อมีค่า MATK
    Range: 1
    Slots: 3
    Jobs:
      Knight: true
    Locations:
      Both_Hand: true
    WeaponLevel: 4
    EquipLevelMin: 160
    Refineable: true
    Gradable: true         # [Legend] items เท่านั้น
    Script: |
      bonus bStr,5;
```

**Weight:** spreadsheet แสดง "220" -> YAML `Weight: 2200`

---

## Job Name Mapping

| ชื่อในเกม / Spreadsheet | YAML key |
|---|---|
| Lord Knight / Knight | `Knight: true` |
| Paladin / Crusader | `Crusader: true` |
| High Wizard | `Wizard: true` |
| Professor / Scholar | `Sage: true` |
| Mastersmith | `Blacksmith: true` |
| Creator / Biochemist | `Alchemist: true` |
| High Priest | `Priest: true` |
| Champion | `Monk: true` |
| Assassin Cross | `Assassin: true` |
| Stalker | `Rogue: true` |
| Sniper | `Hunter: true` |
| Minstrel | `BardDancer: true` + `Gender: Male` |
| Gypsy | `BardDancer: true` + `Gender: Female` |
| Soul Linker | `SoulLinker: true` |
| TaeKwon Master / Star Gladiator | `StarGladiator: true` |
| Kagerou / Oboro | `Ninja: true` |
| Rebel | `Gunslinger: true` |
| Expanded Super Novice | `SuperNovice: true` |

---

## Location Keys

| Slot | Key |
|---|---|
| อาวุธ 1 มือ | `Right_Hand: true` |
| อาวุธ 2 มือ | `Both_Hand: true` |
| โล่ | `Left_Hand: true` |
| เสื้อเกราะ | `Armor: true` |
| รองเท้า | `Shoes: true` |
| มัพ / ผ้าคลุม | `Garment: true` |
| หัวบน / กลาง / ล่าง | `Head_Top / Head_Mid / Head_Low: true` |

---

## Script Reference — Bonus Codes

```
bonus bStr,N;               # STR+N
bonus bAgi,N;               # AGI+N
bonus bVit,N;               # VIT+N
bonus bInt,N;               # INT+N
bonus bDex,N;               # DEX+N
bonus bLuk,N;               # LUK+N

bonus bBaseAtk,N;           # ATK flat +N
bonus bAtkRate,N;           # ATK +N%
bonus bMatk,N;              # MATK flat +N
bonus bMatkRate,N;          # MATK +N%

bonus bAspdRate,N;          # ASPD +N%
bonus bHPrate,N;            # MaxHP +N%
bonus bMaxHP,N;             # MaxHP flat +N
bonus bSPrate,N;            # MaxSP +N%
bonus bFlee,N;              # FLEE +N
bonus bHit,N;               # HIT +N
bonus bCritical,N;          # Critical rate +N
bonus bCritAtkRate,N;       # Critical damage +N%
bonus bHealPower,N;         # Heal +N%
bonus bLongAtkRate,N;       # Long range damage +N%
bonus bIgnoreMdefRate,N;    # เจาะ MDEF N%
bonus bCastRate,-N;         # ลดร่าย N%
bonus bDelayRate,-N;        # ลดดีเลย์ N%
bonus bUnbreakableWeapon;   # อาวุธไม่แตก

bonus2 bAddClass,Class_Boss,N;   # โจมบอสแรงขึ้น N%
bonus2 bSubClass,Class_Boss,N;   # ป้องกันบอส N%
bonus2 bAddClass,Class_All,N;    # โจมทุก class N%
bonus2 bSkillAtk,"SKILL",N;      # Skill damage +N%
```

---

## Script Reference — Skill Constants (pre-re)

| สกิล | Constant |
|---|---|
| Bowling Bash | `LK_BOWLINGBASH` |
| Shield Chain | `PA_SHIELDCHAIN` |
| Cart Termination | `WS_CARTTERMINATION` |
| Acid Demonstration | `CR_ACIDDEMONSTRATION` |
| Asura Strike | `MO_EXTREMITYFIST` |
| Sonic Blow | `AS_SONICBLOW` |
| Back Stab | `RG_BACKSTAB` |
| Double Strafe | `AC_DOUBLESTRAFE` |
| Arrow Vulcan | `CG_ARROWVULCAN` |
| Throw Arrow | `DC_THROWARROW` |
| Flying Kick | `TK_JUMPKICK` |
| Esma (Soul Linker) | `SL_SMA` |
| Throw Huuma | `NJ_HUUMA` |
| Desperado | `GS_DESPERADO` |
| Holy Light | `AL_HOLYLIGHT` |
| Heal | `AL_HEAL` |

---

## Script Patterns

## Workflow — สร้าง NPC จาก Google Sheet

ใช้เมื่อลูกค้าส่ง Google Sheet แล้วขอให้ “สร้าง NPC ตามลิงก์”

1. อ่าน metadata ของ Google Sheet ก่อนเสมอ เพื่อจับ `gid` เป็นชื่อแท็บจริง
2. อ่านช่วงหัวตารางและแถวข้อมูลเท่าที่จำเป็น เช่น `A1:AC120`
3. แยกข้อมูล 3 กลุ่ม:
   - รายการไอเทม / item id / ราคาแต้ม
   - ชื่อ NPC / sprite id / map x y
   - note จาก Dev Request
4. ตรวจ item id กับ DB จริงใน `db/pre-re/` ก่อนเขียน script
   - ถ้าชื่อกับเลขในชีตไม่ตรงกัน ให้ยึด DB เป็นหลัก และแจ้งในสรุป
   - ตัวอย่าง: `12246` คือ `Mystical Card Album`; `Boss Card Album` ใน custom DB คือ `12698`
5. วาง custom NPC ใน `npc/custom-npc/<category>/`
6. เปิดโหลดไฟล์ใน `npc/custom-npc.conf`
7. ถ้า script มีภาษาไทย ให้แปลงเป็น `WINDOWS-874` ก่อนส่งงาน/commit:

```bash
iconv -f UTF-8 -t WINDOWS-874 input.txt -o /tmp/input.win874
mv /tmp/input.win874 input.txt
```

ถ้าไฟล์เป็น UTF-8 BOM ให้ถอด BOM ก่อน:

```bash
dd bs=1 skip=3 if=npc/custom-npc.conf of=/tmp/custom-npc.conf.nobom
iconv -f UTF-8 -t WINDOWS-874 /tmp/custom-npc.conf.nobom -o /tmp/custom-npc.conf.win874
mv /tmp/custom-npc.conf.win874 npc/custom-npc.conf
```

ยืนยัน encoding โดย decode กลับ:

```bash
iconv -f WINDOWS-874 -t UTF-8 npc/custom-npc/04_craft/boss_point_shop.txt | sed -n '1,30p'
```

8. ตรวจความสะอาด:

```bash
git diff --check
git status --short
```

ถ้าใช้สูตร lookup ใน Google Sheet แล้วต้องแปลงเป็นค่า static:

- อย่า `PASTE_VALUES` เป็นช่วงยาวที่คร่อม merged cells เพราะ Google Sheets API จะ fail ด้วย `partially intersects a merge`
- ให้ paste values เฉพาะ block แถวรายการจริง และหลบ header/title/blank merged rows
- ตรวจด้วย `get_spreadsheet_cells` ว่า `userEnteredValue` เป็น `numberValue` หรือ `stringValue` ไม่ใช่ `formulaValue`

9. ถ้าต้อง commit ให้ stage เฉพาะไฟล์งาน ห้ามรวม dirty file อื่นของผู้ใช้:

```bash
git add npc/custom-npc.conf npc/custom-npc/04_craft/example.txt
git diff --cached --name-only
git commit -m "Add ..."
```

### หมายเหตุจากงาน NPC ย่อย/แลกของบอส

- `NPCย่อยของบอส` ใช้พิกัด `morocc,180,156`, sprite `618`
- `แลกของบอส` ใช้พิกัด `morocc,180,153`, sprite `619`
- แต้มใช้ account variable `#RELIC_POINTS`
- หากใช้ระบบย่อยไอเทม ควรกรองเฉพาะ item สะอาด: ไม่สวมใส่, ไม่ refine, ไม่มี card, ไม่ bound/expire, ไม่มี enchant grade/random option
- ใน environment นี้ `./map-server --help` ทำ license check และ fail ได้ จึงใช้ static checks แทนเมื่อรัน server ไม่ได้

### [Legend] Vicious Mind Weapon — Base Pattern

```
Script: |
  bonus bStr,5; bonus bDex,5;
  bonus2 bAddClass,Class_Boss,20;
  bonus2 bSkillAtk,"SKILL_CONST",20;
  bonus bAspdRate,5;
  .@r = getrefine();
  bonus2 bSkillAtk,"SKILL_CONST",(.@r/3)*3;   # per 3 refines
  if(.@r>=7) bonus bAtkRate,7;
  if(.@r>=9) bonus2 bAddClass,Class_All,10;   # melee; ใช้ bLongAtkRate สำหรับ ranged
  if(.@r>=11) bonus bAspdRate,15;
  .@g = getenchantgrade();
  if(.@g >= ENCHANTGRADE_D) {
      bonus bBaseAtk,20; bonus bAtkRate,5;
      bonus2 bAddClass,Class_Boss,10;
      if(.@g >= ENCHANTGRADE_C) bonus2 bSkillAtk,"SKILL_CONST",(.@r/2)*3;
      else bonus2 bSkillAtk,"SKILL_CONST",(.@r/3)*3;
  }
```

Grade constants: `ENCHANTGRADE_D`, `ENCHANTGRADE_C`, `ENCHANTGRADE_B`, `ENCHANTGRADE_A`

### [Rookie] Weapon — Base Pattern

```
Script: |
  .@r = getrefine();
  bonus bBaseAtk,.@r*15;
  bonus2 bAddClass,Class_Boss,10;
  if(.@r>=5) bonus2 bAddClass,Class_Boss,10;
  if(.@r>=7) { bonus bBaseAtk,30; bonus bAtkRate,15; bonus2 bSubClass,Class_Boss,30; }
```

### [Rookie] Armor Set — Pattern

```
Script: |
  bonus bMaxHPrate,5;
  if (getequipid(EQI_ARMOR) == 15015
   && getequipid(EQI_SHOES) == 2560
   && getequipid(EQI_GARMENT) == 2562) {
      bonus bMaxHP,1000;
      bonus2 bAddClass,Class_Boss,10;
      bonus2 bSubClass,Class_Boss,10;
  }
```

EQI constants: `EQI_ARMOR`, `EQI_SHOES`, `EQI_GARMENT`, `EQI_HAND_R`, `EQI_HAND_L`

### Class-Conditional Script

```
if (BaseJob == Job_Sage) { /* Scholar effects */ }
else if (BaseJob == Job_Wizard) { /* Wizard effects */ }
if (BaseJob == Job_Wizard || BaseJob == Job_Sage) bonus bMatkRate,20;
```

## Pet Database Notes

Google Sheet `gid=2014536238` contains MVP pet bonus data split by rarity.
For `pet_db.yml`, only add pets whose `Mob` exists in `db/pre-re/mob_db.yml`; otherwise the pet loader rejects the entry.
If a sheet pet exists in `db/re/mob_db.yml` but not `db/pre-re/mob_db.yml`, add the mob to pre-re first, then add the pet entry.
Pet entries need a real `EggItem`; before adding a new MVP pet egg, check the custom pet egg ID range starting at `9264`.
In pet sheet descriptions, `"เน€เธเนเธฒเธเธญเธ"` means the pet owner themself. Use `bAddEff2` for self-inflicted attack statuses; use `SC_COMA` with `bAddEff2` for self-inflicted Coma.
For temporary bonuses triggered from a pet `Script`, use `petautobonus`, not equipment `autobonus`, so the effect lifecycle is tied to the active pet.

MVP pet stat mapping:

| Mob | EggItem | Loyal bonus |
|---|---|---|
| `GOLDEN_BUG` | `Golden_Bug_Egg` | STR+2, DEX+2 |
| `MAYA` | `Maya_Egg` | STR+2, AGI+2 |
| `EDDGA` | `Eddga_Egg` | AGI+2, DEX+2 |
| `DARK_LORD` | `Dark_Lord_Egg` | LUK+5, INT+5 |
| `BAPHOMET` | `Baphomet_Egg` | DROP+10% |
| `ORC_LORD` | `Orc_Lord_Egg` | INT+2, SP+100 |
| `LADY_TANEE` | `Lady_Tanee_Egg` | LUK+5, Critical+5 |
| `PHREEONI` | `Phreeoni_Egg` | Flee+5, HIT+5 |
| `ORK_HERO` | `Orc_Hero_Egg` | ATK+10, ATK+1% |
| `AMON_RA` | `Amon_Ra_Egg` | MATK+10, MATK+1% |
| `PHARAOH` | `Pharaoh_Egg` | All stats+1 |
| `BACSOJIN_` | `Bacsojin_Egg` | All stats+1, DROP+20% |
| `OSIRIS` | `Osiris_Egg` | All stats+1, Critical damage+20% |
| `DRACULA` | `Dracula_Egg` | All stats+1, all-size physical damage+10% |
| `DOPPELGANGER` | `Doppelganger_Egg` | All stats+1, boss physical damage+10% |
| `MISTRESS` | `Mistress_Egg` | All stats+1, move speed+30% |
| `MOONLIGHT` | `Moonlight_Egg` | All stats+1, ASPD+5% |
| `DRAKE` | `Drake_Egg` | All stats+1, HP gained on physical kill+100 |
| `KNIGHT_OF_WINDSTORM` | `Knight_Of_Windstorm_Egg` | 20% resist to common statuses |
| `LORD_OF_DEATH` | `Lord_Of_Death_Egg` | All stats+1, SP gained on physical kill+20 |
| `B_SEYREN` | `B_Seyren_Egg` | All stats+3, Critical+20, ASPD+1, Player resist+5% |
| `B_EREMES` | `B_Eremes_Egg` | All stats+1, move speed+30%, Player resist+5% |
| `B_HARWORD` | `B_Harword_Egg` | All stats+1, ASPD+10%, Player resist+5% |
| `B_MAGALETA` | `B_Magaleta_Egg` | Stun resist+50%, self Freeze on attack, Player resist+5% |
| `B_SHECIL` | `B_Shecil_Egg` | Boss damage+40%, self Coma/Curse on attack, Player resist+5% |
| `B_KATRINN` | `B_Katrinn_Egg` | Physical reflect reduction, ATK-10%, Player resist+5% |
| `B_RANDEL` | `B_Randel_Egg` | Magic reflect+100%, MATK-10%, Player resist+5% |
| `B_FLAMEL` | `B_Flamel_Egg` | ATK+20%, ATK+30, ASPD+5%, Player resist+5% |
| `B_CELIA` | `B_Celia_Egg` | MATK+20%, cast time-10%, delay-20%, Player resist+5% |
| `B_CHEN` | `B_Chen_Egg` | 10% chance to inflict common statuses, Player resist+5% |
| `MEDJAY` | `Medjay_Egg` | Physical attacks can grant boss physical damage+100% and Perfect Dodge+50, HP+1000, move speed+30%, Player resist+20% |
| `CP_HEKA` | `Cp_Heka_Egg` | Magic attacks can grant boss magic damage+100% and Perfect Dodge+50, HP+1000, move speed+30%, Player resist+20% |
| `MEDJED` | `Medjed_Egg` | Critical+100, critical damage+30%, self Stun on attack, HP+1000, move speed+30%, Player resist+20% |
| `SPIRIT_R_FLAME_SL` | `Spirit_R_Flame_Sl_Egg` | ATK+150, melee+5%, ranged+10%, HP+1000, move speed+30%, Player resist+20% |
| `SPIRIT_I_WATER_M` | `Spirit_I_Water_M_Egg` | MATK+300, MATK+20%, cast time-30%, delay-30%, HP+1000, move speed+30%, Player resist+20% |
| `SPIRIT_S_WIND_M` | `Spirit_S_Wind_M_Egg` | HP+50%, DEF-30%, MDEF-30%, HP+1000, move speed+30%, Player resist+20% |

Sheet mobs without matching `db/pre-re/mob_db.yml` entries:
`GUIDE_OF_DEAD`, `MD_EP20_BOSS_LASGAND`, `HEROS_F_PEGASUS`.

---

## Source Hooks (C++)

> งานที่แก้ source code โดยตรง ไม่ใช่ item script / YAML — อยู่ใน `src/map/`

### Full Refine Set Bonus

ระบบโบนัสชุดตามระดับตีบวก: ใส่ครบ 6 slot (หัว+เสื้อ+อาวุธ+โล่+เท้า+มัฟ) และทุก slot
ตีบวกถึงระดับเดียวกัน -> ได้โบนัสตาม **min refine** ของทั้งชุด (single-tier แทนที่ ไม่สะสม)

**จุด hook:** `status_calc_pc_sub` ใน `src/map/status.cpp` — แทรกหลัง equipment loop
(`// Parse equipment`) ปิด ก่อน block `EQI_AMMO` มาร์กด้วย `// === Custom: Full Refine Set Bonus ===`

**Slot ที่นับ:** `EQI_HEAD_TOP, EQI_ARMOR, EQI_HAND_R, EQI_HAND_L, EQI_SHOES, EQI_GARMENT`
(หัว = เฉพาะ HEAD_TOP)

**ตาราง tier (pre-re: MAX_REFINE=11 -> มีถึง +11):**

| min refine | DROP | Weight | HP | resist | all stat |
|---|---|---|---|---|---|
| 7 | +10% | +200 | +500 | — | — |
| 8 | +15% | +300 | +700 | — | — |
| 9 | +20% | +400 | +900 | — | — |
| 10 | +25% | +500 | +1200 | 1% | +1 |
| 11 | +25% | +600 | +1500 | 2% | +1 |

**Field ที่ set (ทุกตัวถูก consume หลังจุด hook ใน calc รอบเดียวกัน):**

| โบนัส | C++ field | consume ที่ |
|---|---|---|
| DROP+N% | `sd->indexed_bonus.dropaddrace[RC_ALL] += N` | mob.cpp drop calc |
| Weight+N | `sd->add_max_weight += N` | status.cpp `status_calc_weight` |
| HP+N | `sd->bonus.hp += N` | status.cpp `status_calc_maxhpsp_pc` |
| resist N% (กันกึ่ง) | `subrace[RC_DEMIHUMAN] += N` **และ** `subrace[RC_PLAYER_HUMAN] += N` | battle damage calc |
| all stat +N | `param_bonus[PARAM_STR..LUK] += N` (6 ตัว) | status.cpp stat finalize |

**หมายเหตุสำคัญ:**
- **อาวุธ 2 มือ:** `pc_equipitem` set ทั้ง `equip_index[EQI_HAND_R]` และ `[EQI_HAND_L]`
  เป็น inventory index เดียวกัน (equip ที่ `EQP_ARMS`) -> อ่าน refine ของ HAND_L ได้ค่าเดียวกับอาวุธ
  ตรงตาม spec "อาวุธ2มือ นับเป็น อาวุธ+โล่" โดยไม่ต้องเขียน logic พิเศษ
- การ set `subrace[]` ตรงๆ ข้าม cap `battle_config.limit_race_def` (path script ปกติจะ cap) แต่ค่าน้อยมากจึงไม่มีปัญหา
- ถ้าอนาคตเปิด renewal / เพิ่ม `MAX_REFINE` ค่อยเพิ่ม tier +12~+15 ตาม spreadsheet เดิม

### Collection System — CollectionAmount gate

ระบบ collection: item ที่มี `Flags: Collection: true` เก็บใน collection storage ได้
ถ้ามี `CollectionScript` จะถูก run ตอนคำนวณ stat (`status_calc_pc_additional` ใน `src/map/status.cpp`)

**Tag ระดับ item (sibling ของ Script, ไม่อยู่ใต้ Flags):**

| Tag | ความหมาย | default |
|---|---|---|
| `CollectionScript` | script ที่ run เมื่อ item อยู่ใน collection | — |
| `CollectionEffectStack` | คุมจำนวนครั้งสูงสุดที่ script run ตาม amount (เฉพาะ item แบบ stack) | 1 |
| `CollectionAmount` | **เกณฑ์ขั้นต่ำ**: ต้องมี item นี้ใน collection ครบจำนวน (นับรวมทุก slot) ก่อน `CollectionScript` ถึงทำงาน — gate อย่างเดียว ไม่กระทบ combo | 1 |

**จุด hook:**
- parse: `ItemDatabase::parseBodyNode` ใน `src/map/itemdb.cpp` (ถัดจาก block `CollectionEffectStack`); ค่า < 1 -> warning + default 1
- field: `item_data::collection_amount` (`src/map/itemdb.hpp`)
- helper นับยอดรวม: `pc_premium_storage_total(sd, id)` ใน `src/map/pc.cpp` (บวก amount ทุก slot — ต่างจาก `pc_premium_storage_count` ที่คืน slot แรก)
- gate การ run: `status.cpp` — `if (pc_premium_storage_total(...) >= collection_info->collection_amount)` ครอบ loop `run_script` (ยังเรียก `pc_check_collection_combo` ก่อน gate เสมอ)

---

## Item Categories

### [Legend] Vicious Mind Weapons
`Gradable: true` | `WeaponLevel: 4` | `EquipLevelMin: 160` | `Slots: 3` (ส่วนใหญ่)

| ID | Name | AegisName | ATK/MATK | Weight | Slots | Job |
|---|---|---|---|---|---|---|
| 21016 | Two-Handed Sword of Vicious Mind | Sinister_Twohand_Sword | ATK 300 | 2200 | 3 | Knight, Crusader |
| 1450 | Lance of Vicious Mind | Sinister_Lance | ATK 340 | 2100 | 3 | Crusader |
| 1600 | Rod of Vicious Mind | Sinister_Rod | MATK 290 | 1200 | 2 | Wizard |
| 28605 | Book of Vicious Mind | Sinister_Book | ATK 280 / MATK 300 | 1000 | 2 | Sage, StarGladiator |
| 28107 | Two Handed Axe of Vicious Mind | Sinister_Twohand_Axe | ATK 420 | 3000 | 3 | Blacksmith |
| 13455 | Saber of Vicious Mind | Sinister_Saber | ATK 360 | 1500 | 3 | Alchemist |
| 16041 | Mace of Vicious Mind | Sinister_Mace | MATK 300 | 1400 | 2 | Priest |
| 1800 | Fist of Vicious Mind | Sinister_Fist | ATK 310 | 1800 | 2 | Monk |
| 28008 | Katar of Vicious Mind | Sinister_Katar | ATK 320 | 1500 | 3 | Assassin |
| 28706 | Dagger of Vicious Mind | Sinister_Dagger | ATK 220 | 1200 | 3 | Rogue |
| 18121 | Bow of Vicious Mind | Sinister_Bow | ATK 290 | 1800 | 2 | Hunter |
| 1900 | Violin of Vicious Mind | Sinister_Viollin | ATK 220 | 1300 | 2 | BardDancer (M) |
| 1996 | Wire of Vicious Mind | Sinister_Wire | ATK 220 | 1300 | 2 | BardDancer (F) |
| 2026 | Staff of Vicious Mind | Sinister_Staff | MATK 300 | 1000 | 2 | SoulLinker |
| 13328 | Huuma Shuriken of Vicious Mind | Sinister_Huuma | ATK 240 | 1600 | 2 | Ninja |
| 13128 | Revolver of Vicious Mind | Sinister_Revolver | ATK 250 | 600 | 2 | Gunslinger |
| 13468 | Sword of Vicious Mind | Sinister_Sword_SN | ATK 250 / MATK 250 | 1500 | 3 | SuperNovice |

**หมายเหตุ ID 28605:** มี class-conditional script — Sage ได้ MATK effect, StarGladiator ได้ ATK/Flying Kick effect

### [Rookie] Weapons
`WeaponLevel: 3` | `EquipLevelMin: 1` | `Slots: 1` | ไม่มี Gradable

| ID | Name | AegisName | Subtype | Job |
|---|---|---|---|---|
| 1292 | [Rookie] Katar | Rookie_Katar | Katar | Assassin |
| 1394 | [Rookie] Two-handed Axe | Rookie_Two_Handed_Axe | 2hAxe | Swordman, Merchant |
| 1491 | [Rookie] Spear | Rookie_Lance | 2hSpear | Swordman |
| 1585 | [Rookie] Book | Rookie_Book | Book | Priest, Sage, StarGladiator |
| 2015 | [Rookie] Staff | Rookie_Staff | 2hStaff | Mage, Acolyte, SoulLinker |
| 13071 | [Rookie] Dagger | Rookie_Dagger | Dagger | หลาย class |
| 13115 | [Rookie] Revolver | Rookie_Revolver | Revolver | Gunslinger |
| 21000 | [Rookie] Two-handed Sword | Rookie_Twohand_Sword | 2hSword | Swordman |
| 16019 | [Rookie] Mace | Rookie_Mace | Mace | Novice, Swordman, Acolyte, Merchant |
| 18112 | [Rookie] Bow | Rookie_Bow | Bow | Archer, BardDancer, Hunter, Rogue |
| 13316 | [Rookie] Huuma Shuriken | Rookie_Huuma_Shuriken | Huuma | Ninja |

### [Rookie] Armor Set
| ID | Name | AegisName | Slot | DEF |
|---|---|---|---|---|
| 15015 | [Rookie] Adventure Suit | Rookie_Adv_Suit | Armor | 25 |
| 2149 | [Rookie] Guard | Rookie_Guard | Left_Hand (Shield) | 25 |
| 2560 | [Rookie] Boots | Rookie_Boots | Shoes | 13 |
| 2562 | [Rookie] Muffler | Rookie_Muffler | Garment | 13 |

Set bonus เมื่อใส่ครบ 15015 + 2560 + 2562: MaxHP+1000, Boss damage+10%, Boss defense+10%

---

## References

| แหล่งข้อมูล | รายละเอียด |
|---|---|
| Google Sheets | `https://docs.google.com/spreadsheets/d/18jXOdXuyUAcnRlopt6nwZ_U6AursYYIGTvecjY8jirM/edit` |
| Sheets fileId | `18jXOdXuyUAcnRlopt6nwZ_U6AursYYIGTvecjY8jirM` |
| rAthena docs | `https://github.com/rathena/rathena/wiki` |
