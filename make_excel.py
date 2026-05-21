from openpyxl import Workbook
from openpyxl.styles import (
    PatternFill, Font, Alignment, Border, Side, GradientFill
)
from openpyxl.utils import get_column_letter

wb = Workbook()
ws = wb.active
ws.title = "Features"

# ── colour palette ──────────────────────────────────────────────────────────
DARK_BG   = "1E2A3A"   # sheet title row
CAT_COLORS = {          # one accent per category
    "ระบบหลัก":               ("1A3A5C", "FFFFFF"),
    "ระบบเสริมอาวุธ":         ("2E4057", "E8F4FD"),
    "ระบบคราฟต์":             ("1B5E20", "E8F5E9"),
    "ระบบ VIP / Premium":     ("4A0E8F", "F3E5FF"),
    "ระบบอีเวนต์":            ("BF360C", "FFF3E0"),
    "ระบบ Ranking":           ("E65100", "FFF8E1"),
    "ระบบเควส":               ("00695C", "E0F2F1"),
    "ระบบ Guild / WoE":       ("4527A0", "EDE7F6"),
    "ระบบ PvP / PK":          ("880E4F", "FCE4EC"),
    "ระบบร้านค้า":            ("01579B", "E1F5FE"),
    "ระบบ Security / Anti-Cheat": ("37474F", "ECEFF1"),
    "ระบบ GM / Admin":        ("263238", "F5F5F5"),
    "ระบบอื่นๆ":              ("33691E", "F9FBE7"),
    "ระบบ CBT":               ("6D4C41", "EFEBE9"),
}

ENGINE_FILL = PatternFill("solid", fgColor="FFF9C4")   # yellow tint for engine rows
NPC_FILL    = PatternFill("solid", fgColor="FFFFFF")

def thin_border(top=True, bottom=True, left=True, right=True):
    s = Side(style="thin", color="BDBDBD")
    n = Side(style=None)
    return Border(
        top=s if top else n,
        bottom=s if bottom else n,
        left=s if left else n,
        right=s if right else n,
    )

def thick_bottom():
    return Border(
        top=Side(style=None),
        bottom=Side(style="medium", color="455A64"),
        left=Side(style=None),
        right=Side(style=None),
    )

# ── data ────────────────────────────────────────────────────────────────────
data = [
    # (category, system, description, note)
    ("ระบบหลัก","Auto Attack (autoattack.cpp)","บอทโจมตีอัตโนมัติระดับ Engine พร้อม Pathfinding / AoE / Corridor Detection / Scoring","C++ Engine"),
    ("ระบบหลัก","Auto Buff (autobuff.cpp)","บัฟปาร์ตี้อัตโนมัติ รองรับ Self / Leader / Named / All พร้อม Heal / Potion / Resurrection / Teleport","C++ Engine"),
    ("ระบบหลัก","Charm System","ระบบเครื่องราง ติดตาม refine / grade / hrefine / hgrade ผ่าน script commands (charmamount ฯลฯ)","C++ Engine"),
    ("ระบบหลัก","Collection System (collection.txt)","ระบบสะสมไอเทม ติดตาม colrefine / colenchantgrade","NPC + Engine"),
    ("ระบบหลัก","Refine Bonus (refineoption.txt)","ปรับ Option อุปกรณ์ด้วยการ Refine","NPC Script"),
    ("ระบบหลัก","Pet Evolution (evolution.txt)","ระบบวิวัฒนาการเพ็ต / มอนสเตอร์","NPC Script"),
    ("ระบบหลัก","Rune System (rune.cpp + runeui.txt)","RuneBook / RuneSet / RuneDecompose พร้อม UI","C++ Engine"),
    ("ระบบหลัก","Aura System (aura.cpp)","Aura Effects โหลดจาก YAML รองรับซ่อน/แสดง","C++ Engine"),
    ("ระบบหลัก","Stall System (stall.cpp)","แผงลอย NPC ถาวร ซื้อ/ขาย มีภาษีอัตโนมัติ 5%","C++ Engine"),

    ("ระบบเสริมอาวุธ","card_remover / npc_remove_card","ถอดการ์ดจากอุปกรณ์","NPC Script"),
    ("ระบบเสริมอาวุธ","card_remover_option","ถอดการ์ดพร้อม Random Option","NPC Script"),
    ("ระบบเสริมอาวุธ","enchant_costume / enchant_costume2","เสริม Costume ด้วย Enchant","NPC Script"),
    ("ระบบเสริมอาวุธ","enchant_master","ระบบ Enchant หลัก","NPC Script"),
    ("ระบบเสริมอาวุธ","enchant_option","เสริม Random Option","NPC Script"),
    ("ระบบเสริมอาวุธ","enchant_pick_01–04","เลือก Enchant แบบต่างๆ (4 แบบ)","NPC Script"),
    ("ระบบเสริมอาวุธ","enchant_remove_01–04","ถอด Enchant (4 แบบ)","NPC Script"),
    ("ระบบเสริมอาวุธ","lock_random_option","ล็อค Random Option","NPC Script"),
    ("ระบบเสริมอาวุธ","npc_grade","ระบบ Grade อุปกรณ์","NPC Script"),
    ("ระบบเสริมอาวุธ","socket_enchant","เพิ่มช่องใส่การ์ด (Socket)","NPC Script"),
    ("ระบบเสริมอาวุธ","reset_option / reset_option_obt","รีเซ็ต Random Option","NPC Script"),
    ("ระบบเสริมอาวุธ","reset_refine_option","รีเซ็ต Option ของ Refine","C++ Engine"),
    ("ระบบเสริมอาวุธ","c_autobonus / c_autobonus2 / c_autobonus3","Custom AutoBonus จาก Script","C++ Engine"),

    ("ระบบคราฟต์","craft_master / craft_master1–3","คราฟต์อุปกรณ์หลายระดับ","NPC Script"),
    ("ระบบคราฟต์","craftcharm","คราฟต์เครื่องราง","NPC Script"),
    ("ระบบคราฟต์","craftcoslower / craftcoslower2","คราฟต์ Costume ระดับต่ำ","NPC Script"),
    ("ระบบคราฟต์","Monster_Craft","คราฟต์จากไอเทมมอนสเตอร์ (YAML)","NPC Script"),
    ("ระบบคราฟต์","event_exchange_item","แลกไอเทม Event","NPC Script"),
    ("ระบบคราฟต์","merge_ore","รวมแร่ธาตุ","NPC Script"),

    ("ระบบ VIP / Premium","vip / vip_config / vip_package / vip_shop","ระบบ VIP ครบวงจร (config / package / shop)","NPC Script"),
    ("ระบบ VIP / Premium","Premi / Premi2","ระบบ Premium","NPC Script"),
    ("ระบบ VIP / Premium","premium_service / premium_service3","บริการ Premium","NPC Script"),
    ("ระบบ VIP / Premium","Member_Bonus / Member_Farm","โบนัสสมาชิก / ฟาร์มสมาชิก","NPC Script"),
    ("ระบบ VIP / Premium","Party_Bonus_System / Party_Pass_System","โบนัสปาร์ตี้ / บัตรปาร์ตี้","NPC Script"),
    ("ระบบ VIP / Premium","Buffs / guild_buff / Sulatan_Buff","บัฟ VIP / บัฟกิลด์ / บัฟพิเศษ","NPC Script"),
    ("ระบบ VIP / Premium","npc_bonus_time","ระบบ Bonus Time","NPC Script"),
    ("ระบบ VIP / Premium","npc_gympass","Gym Pass (เพิ่ม Weight)","NPC Script"),
    ("ระบบ VIP / Premium","bellnpc / function_bell","ระฆัง NPC แจ้งเตือน","NPC Script"),
    ("ระบบ VIP / Premium","changesuit","เปลี่ยนชุด (Costume)","NPC Script"),
    ("ระบบ VIP / Premium","premiumstorage2","คลังเก็บของ Premium","NPC Script"),

    ("ระบบอีเวนต์","casino_mushroom / casino_poringslot","คาสิโนเห็ด / สล็อต Poring","NPC Script"),
    ("ระบบอีเวนต์","npc_jackpot_cash / npc_jackpot_zeny","แจ็กพอต Cash / Zeny","NPC Script"),
    ("ระบบอีเวนต์","spin_cash / spin_zeny","สปินรับรางวัล Cash / Zeny","NPC Script"),
    ("ระบบอีเวนต์","random_box / random_egg1","กล่องสุ่ม / ไข่สุ่ม","NPC Script"),
    ("ระบบอีเวนต์","codeitem / npc_item_code","กรอกโค้ดรับไอเทม","NPC Script"),
    ("ระบบอีเวนต์","event_luckyrooms","ห้องโชคดี","NPC Script"),
    ("ระบบอีเวนต์","event_playbox","กล่องเล่นเกม Event","NPC Script"),
    ("ระบบอีเวนต์","killboss_auto_item","รับไอเทมอัตโนมัติเมื่อฆ่า Boss","NPC Script"),
    ("ระบบอีเวนต์","Dungeon_of_the_Day / dotd","Dungeon ประจำวัน","NPC Script"),
    ("ระบบอีเวนต์","Juti_Stage","Stage จูติ","NPC Script"),
    ("ระบบอีเวนต์","AutoDe_dice","ลูกเต๋าอัตโนมัติ","NPC Script"),
    ("ระบบอีเวนต์","egg_water_full","ระบบไข่/น้ำ (เต็ม)","NPC Script"),
    ("ระบบอีเวนต์","OldGlastHeim / All-Mvp-Old","Glast Heim เก่า / MVP รุ่นเก่า","NPC Script"),
    ("ระบบอีเวนต์","reward_cbt","รางวัล CBT","NPC Script"),

    ("ระบบ Ranking","ranking","Ranking ทั่วไป","NPC Script"),
    ("ระบบ Ranking","ranking_mvp / ranking_mvp_sor","Ranking MVP","NPC Script"),
    ("ระบบ Ranking","ranking_pvp","Ranking PvP","NPC Script"),
    ("ระบบ Ranking","rank_donate","Ranking บริจาค","NPC Script"),
    ("ระบบ Ranking","rank_shop","ร้านแลกรางวัล Rank","NPC Script"),
    ("ระบบ Ranking","rank_upgrade","อัปเกรดด้วยแต้ม Rank","NPC Script"),
    ("ระบบ Ranking","rank_register","ลงทะเบียน Ranking","NPC Script"),

    ("ระบบเควส","daily","เควสประจำวัน","NPC Script"),
    ("ระบบเควส","breeder / rentbreeder","ระบบเพาะพันธุ์ / เช่าเพาะพันธุ์","NPC Script"),
    ("ระบบเควส","rental_npc","เช่า NPC","NPC Script"),
    ("ระบบเควส","Skill_Exclusive","สกิลพิเศษเฉพาะ","NPC Script"),
    ("ระบบเควส","CreateElementalConverter","สร้าง Elemental Converter","NPC Script"),
    ("ระบบเควส","remove_foodbuff","ลบบัฟจากอาหาร","NPC Script"),

    ("ระบบ Guild / WoE","woe_controller","ควบคุม War of Emperium","NPC Script"),
    ("ระบบ Guild / WoE","guildcastles","ปราสาทกิลด์","NPC Script"),
    ("ระบบ Guild / WoE","Guild_Maneger","จัดการกิลด์","NPC Script"),
    ("ระบบ Guild / WoE","guildassistance / guildhelper","ผู้ช่วยกิลด์","NPC Script"),
    ("ระบบ Guild / WoE","guildroom","ห้องกิลด์","NPC Script"),
    ("ระบบ Guild / WoE","lockguild","ล็อคกิลด์","NPC Script"),
    ("ระบบ Guild / WoE","party_01–05","ระบบปาร์ตี้ (5 ระบบ)","NPC Script"),
    ("ระบบ Guild / WoE","agitstart3 / agitend3 / gvgon3 / gvgoff3","War of Emperium ประเภทที่ 3","C++ Engine"),

    ("ระบบ PvP / PK","PVPRoom / pvp_random","ห้อง PvP / PvP สุ่ม","NPC Script"),
    ("ระบบ PvP / PK","PK.txt","ระบบ Player Kill","NPC Script"),
    ("ระบบ PvP / PK","pkpass / pkpass_end","กำหนดโซน PK ชั่วคราวจาก Script","C++ Engine"),

    ("ระบบร้านค้า","Shop_All / Shop_General","ร้านค้าทั่วไป / ร้านรวม","NPC Script"),
    ("ระบบร้านค้า","cashshop","ร้านค้า Cash","NPC Script"),
    ("ระบบร้านค้า","card_seller / card_trader","ร้านขาย/แลกการ์ด","NPC Script"),
    ("ระบบร้านค้า","megaphone","ขายโทรโข่ง","NPC Script"),
    ("ระบบร้านค้า","item_preview","ดูตัวอย่างไอเทม","NPC Script"),
    ("ระบบร้านค้า","npc_gift_vouncher","คูปองของขวัญ","NPC Script"),
    ("ระบบร้านค้า","hw_flash_sale","Flash Sale","NPC Script"),
    ("ระบบร้านค้า","shop_special_cash","ร้านพิเศษ Cash","NPC Script"),
    ("ระบบร้านค้า","upg_item_cash","อัปเกรดไอเทมด้วย Cash","NPC Script"),
    ("ระบบร้านค้า","ex_scroll_shop","ร้านขาย Scroll พิเศษ","NPC Script"),
    ("ระบบร้านค้า","Cos_Sell_Back","ขายคืน Costume","NPC Script"),
    ("ระบบร้านค้า","open_stall (stall.cpp)","เปิดร้าน Stall จาก Script","C++ Engine"),

    ("ระบบ Security / Anti-Cheat","macro_detector (macro_login / macro_monster)","ตรวจจับ Macro ตอน Login / ฆ่ามอนสเตอร์","NPC + Engine"),
    ("ระบบ Security / Anti-Cheat","Gepard Shield (@gepard_block_* / @gepard_unblock_*)","บล็อก/ปลดบล็อกผู้เล่นด้วย nick / account_id / unique_id","C++ Engine"),
    ("ระบบ Security / Anti-Cheat","Packet Obfuscation (clif_obfuscation.hpp)","เข้ารหัส Packet ด้วย 3 Key","C++ Engine"),
    ("ระบบ Security / Anti-Cheat","Packet Shuffle (clif_shuffle.hpp)","สลับ Packet ID ตาม Client Version","C++ Engine"),
    ("ระบบ Security / Anti-Cheat","auto_clean_log / auto_clean_guild","ลบ Log / ข้อมูลกิลด์เก่าอัตโนมัติ","NPC Script"),
    ("ระบบ Security / Anti-Cheat","auto_kill_mob","กวาดล้างมอนสเตอร์อัตโนมัติ","NPC Script"),

    ("ระบบ GM / Admin","@nerf / @nerfset / @nerfclear / @nerflist","ปรับ Drop Rate per-map แยกประเภท (etc / equip / weapon / card)","C++ Engine"),
    ("ระบบ GM / Admin","@afk","AFK System พร้อมหมวก AFK + timeout อัตโนมัติ","C++ Engine"),
    ("ระบบ GM / Admin","gm_service","บริการ GM","NPC Script"),
    ("ระบบ GM / Admin","@reload_color_db / @colornick","ระบบสี Nickname","C++ Engine"),
    ("ระบบ GM / Admin","@reloadrunedb","Reload Rune Database","C++ Engine"),
    ("ระบบ GM / Admin","@guildalliance","จัดการ Guild Alliance","C++ Engine"),

    ("ระบบอื่นๆ","boss_announce","ประกาศ Boss เกิด","NPC Script"),
    ("ระบบอื่นๆ","stylist / superstylist","ช่างแต่งหน้า","NPC Script"),
    ("ระบบอื่นๆ","system_dressroom","ห้องลองชุด","NPC Script"),
    ("ระบบอื่นๆ","change_gender","เปลี่ยนเพศ","NPC Script"),
    ("ระบบอื่นๆ","union","ระบบสหภาพ","NPC Script"),
    ("ระบบอื่นๆ","natural_weather","ระบบสภาพอากาศ","NPC Script"),
    ("ระบบอื่นๆ","Scripts_City_Clock","นาฬิกาในเมือง","NPC Script"),
    ("ระบบอื่นๆ","WarperRE / WarperPRE / warper_mvp","Warper หลายประเภท + MVP","NPC Script"),
    ("ระบบอื่นๆ","goldpc","ระบบ Gold PC (รายได้จากเวลาออนไลน์)","NPC Script"),
    ("ระบบอื่นๆ","autoloot / has_autoloot","เก็บไอเทมอัตโนมัติ","C++ Engine"),
    ("ระบบอื่นๆ","pc_battle_stats","เมนูตรวจสอบ Equipment / Stats / Zeny / Item List","C++ Engine"),
    ("ระบบอื่นๆ","setdialogalign / setdialogsize / setdialogpos","ปรับ UI หน้าต่าง Dialog ของ NPC","C++ Engine"),

    ("ระบบ CBT","CBT-Weapon / CBT-Armor / CBT-Card ฯลฯ","ชุด NPC ทดสอบครบทุกประเภทอุปกรณ์","NPC Script"),
    ("ระบบ CBT","cbt_main / cbt_level","ระบบ CBT หลัก + ระดับ","NPC Script"),
    ("ระบบ CBT","test_damage","ทดสอบความเสียหาย","NPC Script"),
]

# ── Title Row (row 1) ────────────────────────────────────────────────────────
ws.merge_cells("A1:D1")
title_cell = ws["A1"]
title_cell.value = "rAthena Private Server — Feature List"
title_cell.font = Font(name="Calibri", bold=True, size=16, color="FFFFFF")
title_cell.fill = PatternFill("solid", fgColor=DARK_BG)
title_cell.alignment = Alignment(horizontal="center", vertical="center")
ws.row_dimensions[1].height = 36

# ── Legend Row (row 2) ───────────────────────────────────────────────────────
ws.merge_cells("A2:B2")
ws["A2"].value = "  🟡  C++ Engine   |   ⬜  NPC Script"
ws["A2"].font = Font(name="Calibri", size=10, italic=True, color="555555")
ws["A2"].fill = PatternFill("solid", fgColor="F5F5F5")
ws["A2"].alignment = Alignment(horizontal="left", vertical="center")
ws.merge_cells("C2:D2")
ws["C2"].value = f"รวมทั้งหมด {len(data)} ระบบ"
ws["C2"].font = Font(name="Calibri", size=10, italic=True, color="555555")
ws["C2"].fill = PatternFill("solid", fgColor="F5F5F5")
ws["C2"].alignment = Alignment(horizontal="right", vertical="center")
ws.row_dimensions[2].height = 22

# ── Header Row (row 3) ───────────────────────────────────────────────────────
headers = ["หมวดหมู่", "ระบบ / ไฟล์", "คำอธิบาย", "ประเภท"]
header_fill = PatternFill("solid", fgColor="263238")
for col_idx, h in enumerate(headers, 1):
    cell = ws.cell(row=3, column=col_idx, value=h)
    cell.font = Font(name="Calibri", bold=True, size=11, color="FFFFFF")
    cell.fill = header_fill
    cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
    cell.border = thin_border()
ws.row_dimensions[3].height = 28

# ── Data Rows ────────────────────────────────────────────────────────────────
current_cat = None
cat_start_row = 4

for row_idx, (cat, system, desc, note) in enumerate(data, start=4):
    cat_header_bg, cat_header_fg = CAT_COLORS.get(cat, ("455A64", "FFFFFF"))

    # Insert category separator when category changes
    if cat != current_cat:
        if current_cat is not None:
            # bottom border on previous last data row
            for c in range(1, 5):
                prev = ws.cell(row=row_idx - 1, column=c)
                prev.border = Border(
                    top=prev.border.top,
                    left=prev.border.left,
                    right=prev.border.right,
                    bottom=Side(style="medium", color="90A4AE"),
                )

        # Category header spanning all columns
        ws.insert_rows(row_idx)
        ws.merge_cells(f"A{row_idx}:D{row_idx}")
        cat_cell = ws.cell(row=row_idx, column=1, value=f"  {cat}")
        cat_cell.font = Font(name="Calibri", bold=True, size=12, color=cat_header_fg)
        cat_cell.fill = PatternFill("solid", fgColor=cat_header_bg)
        cat_cell.alignment = Alignment(horizontal="left", vertical="center")
        cat_cell.border = Border(
            top=Side(style="medium", color="FFFFFF"),
            bottom=Side(style="medium", color="FFFFFF"),
        )
        ws.row_dimensions[row_idx].height = 26

        current_cat = cat
        row_idx += 1  # next row for actual data

    # Determine fill
    row_fill = ENGINE_FILL if "Engine" in note else NPC_FILL

    for col_idx, val in enumerate([cat, system, desc, note], 1):
        cell = ws.cell(row=row_idx, column=col_idx, value=val)
        cell.font = Font(name="Calibri", size=10,
                         color="1565C0" if col_idx == 2 else "212121")
        cell.fill = row_fill
        cell.alignment = Alignment(vertical="center", wrap_text=(col_idx == 3),
                                   horizontal="left")
        cell.border = thin_border()

    ws.row_dimensions[row_idx].height = 22

# ── Column widths ────────────────────────────────────────────────────────────
ws.column_dimensions["A"].width = 28
ws.column_dimensions["B"].width = 38
ws.column_dimensions["C"].width = 58
ws.column_dimensions["D"].width = 16

# ── Freeze panes ─────────────────────────────────────────────────────────────
ws.freeze_panes = "A4"

# ── Summary Sheet ─────────────────────────────────────────────────────────────
ws2 = wb.create_sheet("สรุปจำนวน")
ws2.column_dimensions["A"].width = 30
ws2.column_dimensions["B"].width = 18

# title
ws2.merge_cells("A1:B1")
t = ws2["A1"]
t.value = "สรุปจำนวนระบบแยกตามหมวดหมู่"
t.font = Font(name="Calibri", bold=True, size=14, color="FFFFFF")
t.fill = PatternFill("solid", fgColor=DARK_BG)
t.alignment = Alignment(horizontal="center", vertical="center")
ws2.row_dimensions[1].height = 32

# header
for col, label in enumerate(["หมวดหมู่", "จำนวนระบบ"], 1):
    c = ws2.cell(row=2, column=col, value=label)
    c.font = Font(name="Calibri", bold=True, size=11, color="FFFFFF")
    c.fill = PatternFill("solid", fgColor="37474F")
    c.alignment = Alignment(horizontal="center", vertical="center")
    c.border = thin_border()
ws2.row_dimensions[2].height = 24

# count per category
from collections import Counter
counts = Counter(row[0] for row in data)
total = 0
for r_idx, (cat, cnt) in enumerate(counts.items(), start=3):
    bg, fg = CAT_COLORS.get(cat, ("455A64", "FFFFFF"))
    c1 = ws2.cell(row=r_idx, column=1, value=cat)
    c1.font = Font(name="Calibri", bold=True, size=10, color=fg)
    c1.fill = PatternFill("solid", fgColor=bg)
    c1.alignment = Alignment(horizontal="left", vertical="center", indent=1)
    c1.border = thin_border()

    c2 = ws2.cell(row=r_idx, column=2, value=cnt)
    c2.font = Font(name="Calibri", bold=True, size=11, color="212121")
    c2.fill = PatternFill("solid", fgColor="ECEFF1")
    c2.alignment = Alignment(horizontal="center", vertical="center")
    c2.border = thin_border()
    ws2.row_dimensions[r_idx].height = 22
    total += cnt

# total row
tr = len(counts) + 3
c1 = ws2.cell(row=tr, column=1, value="รวมทั้งหมด")
c1.font = Font(name="Calibri", bold=True, size=11, color="FFFFFF")
c1.fill = PatternFill("solid", fgColor="1E2A3A")
c1.alignment = Alignment(horizontal="center", vertical="center")
c1.border = thin_border()

c2 = ws2.cell(row=tr, column=2, value=total)
c2.font = Font(name="Calibri", bold=True, size=12, color="FFFFFF")
c2.fill = PatternFill("solid", fgColor="1E2A3A")
c2.alignment = Alignment(horizontal="center", vertical="center")
c2.border = thin_border()
ws2.row_dimensions[tr].height = 26

# ── Save ──────────────────────────────────────────────────────────────────────
output_path = "/home/roserver/server/features.xlsx"
wb.save(output_path)
print(f"Saved → {output_path}")
