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
    Weight: 2200           # display weight × 10
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

**Weight:** spreadsheet แสดง "220" → YAML `Weight: 2200`

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
