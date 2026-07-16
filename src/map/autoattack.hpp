#ifndef AUTOATTACK_HPP
#define AUTOATTACK_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "path.hpp"
#include "status.hpp"

#include <common/mmo.hpp>
#include <common/database.hpp>
#include <common/db.hpp>

constexpr auto AA_WALK_CELL = 13;

namespace AA_CONSTANTS {
	constexpr int MOVE_DELAY = 100;            // 500ms movement delay
	constexpr int IDLE_THRESHOLD = 15;         // 15 seconds idle threshold
	constexpr int CORRIDOR_DETECTION_RANGE = 3;
	constexpr int AOE_SIZE = 2;
	constexpr const char* PREFIX_NAME = "[AUTO]";
	
	// Movement Behavior Settings  
	constexpr int MIN_MOVE_DISTANCE = 2;
	constexpr int PATHFIND_RANGE = 4;
	constexpr int MAX_ATTEMPTS = 50;

	// Scoring System Settings
	constexpr int EXPLORATION_BONUS = 80;
	constexpr int DIRECTION_PERSISTENCE_BONUS = 30;
	constexpr int BACKTRACK_PENALTY = 80;
	constexpr int NARROW_PATH_BONUS = 100;
}

const std::vector<int> AA_HATEFFECTS = { 97, 131, 123, 31, 3 };
extern std::vector<t_itemid> AA_ITEMIDS;

class map_session_data;
struct block_list;

float heuristic(int x1, int y1, int x2, int y2);

enum aa_current_order {
	AA_NOTHING = 0,
	AA_MOVE_TO_ITEM = 1,
	AA_MOVE_TO_MONSTER = 2
};

struct s_autoattackskills {
	bool is_active;
	uint16 skill_id;
	uint16 skill_lv;
	t_tick last_use;
};

struct s_autobuffskills {
	bool is_active;
	uint16 skill_id;
	uint16 skill_lv;
	t_tick last_use;
};

struct s_autoheal {
	bool is_active;
	uint16 skill_id;
	uint16 skill_lv;
	uint16 min_hp;
	t_tick last_use;
};

struct s_autopotion {
	bool is_active;
	t_itemid item_id;
	uint16 min_hp;
	uint16 min_sp;
};

struct s_autositregen {
	bool is_active;
	uint16 max_hp;
	uint16 min_hp;
	uint16 max_sp;
	uint16 min_sp;
};

struct s_autobuffitems {
	bool is_active;
	t_itemid item_id;
	int status;
};

struct s_lastposition {
	int map; // Previous map on Map Change
	short x,y;
	short dx,dy;
};

struct s_targetposition {
	int map;
	short x, y;
	short dx, dy;
};

struct s_teleport {
	bool use_teleport;
	bool use_flywing;
	uint16 min_hp;
	unsigned int delay_nomobmeet;
	bool tp_mvp;
	bool tp_miniboss;
};

struct s_mobs {
	int map;
	std::vector<uint32> id;
	bool aggressive_behavior; //0 attack - 1 ignore
};

struct s_autobuyitems {
	t_itemid item_id;
	int quantity;
	int min_trigger;
};

struct s_autosellitems {
	t_itemid item_id;
};


struct s_autoattack {
	time_t last_teleport;
	time_t last_move;
	time_t last_attack;
	time_t last_pick;
	t_tick skill_cd;
	t_tick last_hit;
	int attack_target_id;
	int target_id;
	int itempick_id;
	int stopmelee;
	bool accept_party_request;
	bool return_to_savepoint;
	bool token_siegfried;
	bool monk_combo;
	unsigned int pickup_item_config;
	unsigned int prio_item_config;
	struct s_teleport teleport;
	struct s_lastposition lastposition;
	struct s_targetposition targetposition;
	struct s_autositregen autositregen;
	struct s_mobs mobs;
	bool state_potions;
	std::vector<s_autoheal> autoheal;
	std::vector<s_autopotion> autopotion;
	std::vector<s_autobuffskills> autobuffskills;
	std::vector<s_autoattackskills> autoattackskills;
	std::vector<s_autobuffitems> autobuffitems;
	std::vector<t_itemid> pickup_item_id;
	std::vector<std::pair<std::string, t_tick>> msg_list;
	std::vector<std::tuple<int, int>> path;
	std::vector<s_autobuyitems> autobuyitems;
	std::vector<s_autosellitems> autosellitems;
	int path_index;
	int action_on_end;
	int monster_surround;
	t_tick duration_;
	unsigned int unique_id;
	uint32 client_addr; // remote client address
	int skill_range;
};

void aa_save(map_session_data* sd);
void aa_load(map_session_data* sd);
void aa_skill_range_calc(map_session_data* sd);
void aa_mob_ai_search_mvpcheck(struct block_list* bl, struct mob_data* md);
void aa_priority_on_hit(map_session_data* sd, struct block_list* src);
void aa_target_change(map_session_data* sd, int id);
void aa_reset_ondead(map_session_data* sd);
bool aa_canuseskill(map_session_data* sd, uint16 skill_id, uint16 skill_lv);
int buildin_autopick_sub(struct block_list* bl, va_list ap);
bool aa_check_item_pickup(map_session_data* sd, struct block_list* bl);
unsigned int aa_check_item_pickup_onfloor(map_session_data* sd);
bool aa_check_target(map_session_data* sd, unsigned int id);
int buildin_autoattack_sub(struct block_list* bl, va_list ap);
int buildin_autoattack_monsters_sub(struct block_list* bl, va_list ap);
unsigned int aa_check_target_alive(map_session_data* sd);
int aa_check_surround_monster(map_session_data* sd);
bool aa_teleport(map_session_data* sd);
int aa_ammochange(map_session_data* sd, struct mob_data* md, const unsigned short* ammoIds, const unsigned short* ammoElements, const unsigned short* ammoAtk, size_t ammoCount, int rqAmount, const unsigned short* ammoLevels);
void aa_arrowchange(map_session_data* sd, struct mob_data* md);
void aa_bulletchange(map_session_data* sd, mob_data* md);
void aa_kunaichange(map_session_data* sd, struct mob_data* md, int rqamount);
void aa_cannonballchange(map_session_data* sd, struct mob_data* md);
bool aa_elemstrong(const mob_data* md, int ele);
bool aa_elemallowed(struct mob_data* md, int ele);
int aa_status(map_session_data* sd);
bool aa_status_checkteleport_delay(map_session_data* sd, t_tick last_tick);
bool aa_status_check_reset(map_session_data* sd, t_tick last_tick);
bool aa_status_pickup(map_session_data* sd, t_tick last_tick);
bool aa_status_heal(map_session_data* sd, const status_data* status, t_tick last_tick);
bool aa_status_potion(map_session_data* sd, const status_data* status);
bool aa_status_rest(map_session_data* sd, const status_data* status, t_tick last_tick);
bool aa_status_buffs(map_session_data* sd, t_tick last_tick);
void aa_token_respawn(map_session_data* sd, int flag);
void handle_autospell(map_session_data* sd, int skill_lv);
bool aa_checkactivestatus(map_session_data* sd, sc_type type);
bool aa_status_buffitem(map_session_data* sd, t_tick last_tick);
bool aa_status_attack(map_session_data* sd, struct mob_data* md_target, t_tick last_tick);
bool aa_status_melee(map_session_data* sd, struct mob_data* md_target, t_tick last_tick, const status_data* status);
bool aa_move_short(map_session_data* sd, t_tick last_tick);
void aa_status_checkmapchange(map_session_data* sd);
bool aa_message(map_session_data* sd, std::string key, char* message, int delay, struct party_data* p);
void aa_getusablepotions(map_session_data* sd, t_itemid* inventory_potion_id, int* inventory_potion_amount, char inventory_potion_name[MAX_INVENTORY][ITEM_NAME_LENGTH + 1], int* amount);
uint32 aa_getrental_search_inventory(map_session_data* sd, t_itemid nameid);
bool aa_changestate_autoattack(map_session_data* sd, int flag);

int aa_get_random_coords(int16 m, int& x, int& y);
bool aa_party_request(map_session_data* sd);
void aa_moblist_reset_mapchange(map_session_data* sd);
void aa_refill_arrows(map_session_data* sd, int current_ammo_id);

#endif /* AUTOATTACK_HPP */
