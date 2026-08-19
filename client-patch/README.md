# Stylist Dress Room client patch

ไฟล์ที่ต้องบรรจุใน GRF ฝั่งไคลเอนต์ด้วย internal path เดิม:

`data\LuaFiles514\Lua Files\dressroom\jobdresslist.lub`

การแก้ไข:

- สีผมชาย/หญิง: เหลือ ID `0-8`
- สีชุดชาย/หญิง: เหลือ ID `0, 2-7` (ไม่มี ID `1` ใน Stylist DB)
- ไม่เปลี่ยนรายการทรงผม, ชุดอาชีพ หรือตารางอื่น
- เก็บ encoding เป็น Windows-874 และ line endings เป็น CRLF

การนำไปทดสอบต้องใส่ไฟล์นี้ใน GRF/THOR ที่ client โหลดก่อน `rdata.grf` โดยใช้ internal path ข้างต้น การวางไฟล์ไว้ในโฟลเดอร์นี้เป็นเพียง staging และยังไม่ได้แก้ GRF ที่ใช้งานจริง

Checksums:

- ต้นฉบับ member จาก `rdata.grf`: `ca325955bb0470e522fa13a3a4579b65d0731b488395b8e764db26e633734f7c`
- ไฟล์ staged หลังแก้: `d9a1d3d923b56921581b688b1c2f6627357a74e6bb1c1cf474dc5cec3caaa31c`
