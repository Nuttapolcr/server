# CLAUDE.md — กฎสำหรับ AI ทุกตัวที่ทำงานบน Repo นี้

> คลังความรู้อยู่ที่ `docs/knowledge.md` — อ่านควบคู่กันเสมอ

---

## ก่อนเริ่มทำงาน

1. อ่านไฟล์นี้ครบ แล้วอ่าน `docs/knowledge.md` ด้วย
2. Task เกี่ยวกับ item -> grep ตรวจสอบว่า ID มีอยู่แล้วหรือยังก่อนเสมอ อย่าเดา
3. ไม่แน่ใจเรื่อง script pattern หรือ bonus code -> ดู `docs/knowledge.md` ก่อน ห้ามเดา

---

## ระหว่างทำงาน

- **ห้ามแก้ไข `db/re/`** — เป็น read-only reference เท่านั้น งานทั้งหมดอยู่ที่ `db/pre-re/`
- **ห้าม copy script จาก re file** — script ใน re เป็นของ renewal server, custom server มี mechanic ต่างกัน ต้องเขียนใหม่ตาม spec ใน `docs/knowledge.md`
- **Items ใน YAML ต้องเรียง ID ascending** เสมอ
- ถ้า item เป็น [Legend] -> ต้องมี `Gradable: true`
- ถ้า item เป็น [Rookie] -> ไม่ใส่ `Gradable`
- Weight ใน YAML = ตัวเลขใน spreadsheet x 10 (เช่น แสดงผล 220 -> YAML `Weight: 2200`)
- Job name ใน YAML != ชื่อในเกม -> ดูตารางใน `docs/knowledge.md`

---

## การดึงข้อมูล Google Sheets

- ใช้ `mcp__claude_ai_Google_Drive__read_file_content` ด้วย fileId จาก URL — ห้ามใช้ WebFetch เพราะ Sheets ต้องการ auth
- ถ้า output ใหญ่เกิน -> ถูก save เป็น .txt file ให้ใช้ python3 หรือ jq อ่านต่อ
- fileId ของ spreadsheet หลัก: `18jXOdXuyUAcnRlopt6nwZ_U6AursYYIGTvecjY8jirM`

---

## หลังทำงาน — บังคับ

**ถ้าได้รับความรู้ใหม่เกี่ยวกับ server นี้ในระหว่าง session -> ต้องอัพเดตทันที:**

ให้จดเป็น **ความรู้สำหรับอ้างอิง/หลักการแก้งานครั้งต่อไปเท่านั้น** เช่น mapping, ID range,
bonus code, script pattern, workflow rule, external reference. **ห้ามจดเป็น changelog หรือ session log**
เช่น "ทำอะไรไปแล้ว", "เพิ่มอะไรไปแล้ว", "ไฟล์ไหนถูกแก้ในรอบนี้" เว้นแต่ข้อมูลนั้นจำเป็นต่อการตัดสินใจแก้ครั้งต่อไป
ให้เขียนเป็นกฎหรือ reference แทน

| ประเภทความรู้ | อัพเดตที่ไหน |
|---|---|
| item ID ใหม่, stats, job restriction | `docs/knowledge.md` หัวข้อ Item Categories |
| bonus code, skill constant ใหม่ | `docs/knowledge.md` หัวข้อ Script Reference |
| script pattern ใหม่ | `docs/knowledge.md` หัวข้อ Script Patterns |
| กฎการทำงาน, workflow | `CLAUDE.md` (ไฟล์นี้) |
| แหล่งข้อมูลภายนอก, link | `docs/knowledge.md` หัวข้อ References |

ห้ามปล่อยให้ความรู้หายไปกับ session

---

## Deploy Workflow — server-test -> server

- server และ server-test เป็น git repo แยกกัน คนละ remote — deploy = copy ไฟล์ที่เปลี่ยนใน server-test ไปทับ path เดียวกันใน server แล้ว commit แยกกันทั้งสองฝั่ง (commit ฝั่ง server มักขึ้นต้นด้วย "Deploy")
- **ก่อน deploy ทุกครั้งต้องเช็ค `git status`/`git diff` ของ server (production) ก่อนเสมอ** — production อาจมีการแก้ไขที่ยังไม่ commit อยู่แล้ว (hotfix ที่แก้ตรงบน production หรือผ่าน tool อื่น) ถ้ามีไฟล์ที่สองฝั่งแก้ไม่ตรงกัน ต้อง diff ดูเนื้อหาจริงก่อนตัดสินใจ ห้ามเดาแล้ว copy ทับตรงๆ
- บางไฟล์เป็น **live state เฉพาะ environment ไม่ใช่ deployable content** เช่น `npc/custom-npc/07_guild/war_warper.txt` (flag ปราสาท CS2 ปัจจุบันของแต่ละ server ต่างกันโดยธรรมชาติ) — ห้าม copy ทับ ให้ exclude ออกจาก deploy เสมอ
- ถ้า production มี hotfix ที่ไม่เคยเข้า server-test (เคยเจอ: `db/pre-re/skill_nocast_db.txt`) ต้อง backport เข้า server-test ด้วยทุกครั้ง ไม่งั้น deploy รอบถัดไปจะทับ hotfix นั้นทิ้งโดยไม่รู้ตัว
- `src/map/*.cpp`, `*.hpp` เป็น source code — deploy แค่ copy+commit ไม่ทำให้ live process รันโค้ดใหม่ ต้อง `make server` แล้ว restart tmux (`live_login`/`live_char`/`live_map`) เองแยกต่างหาก ห้ามทำอัตโนมัติโดยไม่ถามก่อน เพราะ live server มีผู้เล่นต่ออยู่จริง
