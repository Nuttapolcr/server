////////////////////PET Refine////////////////////////
ให้ไข่ ใบนั้นๆ สามารถตีบวก + ตีเกรดได้
Refineable: true
Gradable: true
วิธีการใช้งาน
สำหรับสคริปใน pet_db.yml
// สำหรับดึงตีบวก
.@pr = petrefine();
// สำหรับดึงเกรด
.@pg = petgrade();

////////////////////////AddMonsterLevelDropItem//////////////////////////////
bonus3 bAddMonsterLevelDropItem,<level>,<item id>,<chance n/100%>;
bonus4 bAddMonsterLevelDropItemRace,<race>,<level>,<item id>,<chance n/100%>;
bonus4 bAddMonsterLevelDropItemClass,<class>,<level>,<item id>,<chance n/100%>;
bonus3 bAddMonsterLevelDropGroup,<level>,<Group ID>,<chance n/100%>;
bonus4 bAddMonsterLevelDropGroupRace,<race>,<level>,<Group ID>,<chance n/100%>;
bonus4 bAddMonsterLevelDropGroupClass,<class>,<level>,<Group ID>,<chance n/100%>;

////////////////////////New Mapflag//////////////////////////////
MF_NOFALCON
MF_NORIDING
MF_NOMADOGEAR
MF_NOCART
MF_NOWUG

/////////////////////////////////////////////////////////////////////////////////////

ตัวอย่าง itemdb

    # กำหนดแบบฝากได้แค่ 1 ชิ้น
  - Id: 2220
    AegisName: Hat
    Name: Hat
    Type: Armor
    Buy: 1000
    Weight: 200
    Defense: 2
    Locations:
      Head_Top: true
    ArmorLevel: 1
    Refineable: true
    View: 16
    Flags:
      Collection: true # << กำหนดให้เป็น collection แบบ 1 ชิ้น
    CollectionScript: | << โบนัสของ collection
      bonus bWis,1;

    # กำหนดแบบฝากได้หลายชิ้น 
  - Id: 4001
    AegisName: Poring_Card
    Name: Poring Card
    Type: Card
    Buy: 20
    Weight: 10
    Locations:
      Armor: true
    Flags:
      BuyingStore: true
      DropEffect: CLIENT
      CollectionStack: true # << สำหรับ collection แบบส่งผลหลายชิ้น
    CollectionEffectStack: 5 # << กำหนด จำนวน stack ที่จะให้ bonus , เช่น 5 ชิ้น , หากไม่กำหนด จะมีค่าเป็น 1
    CollectionScript: | << โบนัสของ collection
      bonus bSpl,1;

---

คำสั่งเพิ่มเติม สำหรับใช้งาน ใน item script

getcolrefine(); สำหรับดึงค่า ตีบวกของไอเท็มนั้นๆออกมา 
getcolenchantgrade(); สำหรับดึงค่า ระดับ เกรด ของไอเท็มนั้นๆออกมา
