#!/usr/bin/env python3
"""Validate the Legend recipe and exercise the actual asynchronous pet callbacks."""
from pathlib import Path
import subprocess
import tempfile
import unittest

import yaml

ROOT = Path(__file__).resolve().parents[1]


def database(path):
    return yaml.safe_load((ROOT / path).read_bytes())["Body"]


def function(path, signature):
    source = (ROOT / path).read_text()
    start = source.index(signature)
    return source[start:source.index("\n}", start) + 2]


class LegendSynthesisTests(unittest.TestCase):
    def test_recipe_and_rewards(self):
        items = {row["AegisName"]: row for row in database("db/pre-re/item_db_custom.yml")}
        recipe = next(row for row in database("db/import/laphine_synthesis.yml")
                      if row["Item"] == "Combine_Epic_Pet_Laphine")
        self.assertEqual(items[recipe["Item"]]["Id"], 1500180)
        self.assertEqual(items[recipe["Item"]]["Script"].strip(), "laphine_synthesis();")
        self.assertEqual(recipe["RequiredRequirementsCount"], 4)
        self.assertEqual({items[row["Item"]]["Id"] for row in recipe["Requirements"]}, set(range(9283, 9293)))
        self.assertTrue(all(row.get("Amount", 1) == 1 for row in recipe["Requirements"]))
        rewards = next(row for row in database("db/import/item_group_db.yml")
                       if row["Group"] == recipe["RewardGroup"])["SubGroups"]
        self.assertEqual(len(rewards), 1)
        self.assertEqual(rewards[0]["SubGroup"], 1)
        self.assertEqual(len(rewards[0]["List"]), 6)
        self.assertEqual({items[row["Item"]]["Id"] for row in rewards[0]["List"]}, set(range(9293, 9299)))
        self.assertTrue(all(row["Amount"] == 1 and row["Rate"] == 1 for row in rewards[0]["List"]))
        pet_eggs = {row.get("EggItem") for row in database("db/pre-re/pet_db.yml")}
        self.assertTrue(all(row["Item"] in pet_eggs for row in rewards[0]["List"]))

    def test_lua_client_recipe(self):
        script = '''
dofile("data/Lua Files/ItemDBNameTbl.lub")
dofile("data/Lua Files/datainfo/lapineddukddakbox.lub")
local recipe = tblLapineDdukddakBox.sources.Combine_Epic_Pet_Laphine
assert(recipe.ItemID == 1500180 and recipe.NeedCount == 4)
assert(#recipe.SourceItems == 10)
local seen = {}
for _, ingredient in ipairs(recipe.SourceItems) do
    assert(ingredient[2] == 1)
    assert(ingredient[3] >= 9283 and ingredient[3] <= 9292)
    assert(not seen[ingredient[3]])
    seen[ingredient[3]] = true
end
tbl = {}
dofile("client/legend_pet/itemInfo-additions.lua")
assert(tbl[1500180].identifiedResourceName == "Easter_Egg")
'''
        subprocess.run(["lua", "-"], input=script.encode(), cwd=str(ROOT), check=True)

    def test_actual_pet_callbacks(self):
        # Compile the production functions with mock transport/inventory boundaries.
        # The queue and announcement logic below comes from the real source files.
        harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
using uint32 = uint32_t;
using t_itemid = uint32_t;
constexpr int NAME_LENGTH = 24, CHAT_SIZE_MAX = 256;
constexpr int PET_CATCH_FAIL = 0, CARD0_PET = 256, LOG_TYPE_PICKDROP_PLAYER = 1;
struct map_session_data {
    std::vector<bool> pet_synthesis_announcements;
    int catch_target_class = 0;
    struct { char name[NAME_LENGTH] = "Tester"; } status;
    struct { int m = 0, x = 0, y = 0; } bl;
};
struct item { t_itemid nameid; int identify; uint16_t card[4]; };
struct s_pet_db { t_itemid EggID = 9293; int intimate = 250; };
struct PetDB {
    std::shared_ptr<s_pet_db> find(int id) {
        return id == 42 ? std::make_shared<s_pet_db>() : nullptr;
    }
} pet_db;
map_session_data player;
bool connected = true;
int add_result = 0, floor_count = 0, deleted_count = 0;
std::vector<std::string> messages;
map_session_data *map_id2sd(uint32 id) { return id == 1 ? &player : nullptr; }
int CheckForCharServer() { return !connected; }
int inter_fd = 0;
uint32_t packet[128];
char packet_text[128];
#define WFIFOHEAD(fd, length) ((void)0)
#define WFIFOW(fd, offset) packet[offset]
#define WFIFOL(fd, offset) packet[offset]
#define WFIFOB(fd, offset) packet[offset]
#define WFIFOCP(fd, offset) packet_text
#define WFIFOSET(fd, length) ((void)0)
void safestrncpy(char *to, const char *from, size_t count) { std::snprintf(to, count, "%s", from); }
uint16_t GetWord(int value, int word) { return (uint32_t(value) >> (word * 16)) & 0xffff; }
int pet_get_card3_intimacy(int) { return 0; }
void intif_delete_petdata(int) { ++deleted_count; }
int pc_additem(map_session_data *, item *egg, int amount, int) {
    assert(egg->nameid == 9293 && amount == 1 && egg->card[0] == CARD0_PET);
    return add_result;
}
void clif_additem(map_session_data *, int, int, int) {}
template<typename... Args> void map_addflooritem(Args...) { ++floor_count; }
const char *itemdb_name(t_itemid) { return "Medjay_Egg"; }
const char *itemdb_ename(t_itemid) { return "[Legend] MedJay Egg"; }
void intif_broadcast2(const char *message, int, int, int, int, int, int) { messages.emplace_back(message); }
'''
        harness += function("src/map/intif.cpp", "int intif_create_pet(") + "\n"
        harness += function("src/map/pet.cpp", "bool pet_get_egg(") + "\n"
        harness += r'''
void request(bool synthesis) {
    assert(intif_create_pet(1, 10, 42, 1, 9293, 0, 250, 100, 0, 1, "Pet", synthesis) == 1);
}
int main() {
    // Ordinary and synthesis requests can be interleaved before any reply arrives.
    request(false); request(true); request(false);
    assert(messages.empty());
    assert(pet_get_egg(1, 42, 101)); assert(messages.empty());
    assert(pet_get_egg(1, 42, 102)); assert(messages.size() == 1);
    assert(messages.back().find("Tester") != std::string::npos);
    assert(messages.back().find("[Legend] MedJay Egg") != std::string::npos);
    assert(messages.back().find("Medjay_Egg") == std::string::npos);
    assert(pet_get_egg(1, 42, 103)); assert(messages.size() == 1);
    assert(player.pet_synthesis_announcements.empty());

    // Failed creation consumes its queued flag and cannot announce the next egg.
    request(true); request(false);
    assert(!pet_get_egg(1, 0, 0));
    assert(pet_get_egg(1, 42, 104)); assert(messages.size() == 1);

    // Inventory failure keeps the normal floor fallback but does not announce success.
    request(true); add_result = 1;
    assert(pet_get_egg(1, 42, 105)); assert(floor_count == 1);
    assert(messages.size() == 1); add_result = 0;

    // A removed pet definition must not produce a success announcement.
    request(true);
    assert(!pet_get_egg(1, 999, 106)); assert(deleted_count == 1);
    assert(messages.size() == 1 && player.pet_synthesis_announcements.empty());

    // No queued metadata is recorded when the char-server is disconnected.
    connected = false;
    assert(intif_create_pet(1, 10, 42, 1, 9293, 0, 250, 100, 0, 1, "Pet", true) == 0);
    assert(player.pet_synthesis_announcements.empty());
    connected = true;

    // Untracked/legacy replies and offline users do not announce.
    assert(pet_get_egg(1, 42, 107));
    assert(!pet_get_egg(2, 42, 108));
    assert(messages.size() == 1);
}
'''
        with tempfile.TemporaryDirectory(dir=str(ROOT / "tests")) as temporary:
            source = Path(temporary) / "callbacks.cpp"
            binary = Path(temporary) / "callbacks"
            source.write_text(harness)
            subprocess.run(["g++", "-std=c++17", "-Wall", "-Wextra", str(source), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main(verbosity=2)
