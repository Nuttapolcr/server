#include "autoattack.hpp"
#include "battle.hpp"
#include "log.hpp"
#include "map.hpp" // mmysql_handle
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "skill.hpp"

#include <queue>
#include <cmath>
#include <tuple>
#include <unordered_set>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

using namespace rathena;

// Structure pour representer une cellule
struct Cell {
	int x, y;
	float cost;

	// Comparateur pour la file de priorite (min-heap)
	bool operator>(const Cell& other) const {
		return cost > other.cost;
	}
};

// Forward declarations
static bool algorithm_path_finding(map_session_data* sd, int16_t m, int start_x, int start_y, int target_x, int target_y);
static int aa_move_to_path(std::vector<std::tuple<int, int>>& path, map_session_data* sd);
static void aa_move_path(map_session_data* sd);

// Forward declarations for movement helper utilities
bool has_direction_momentum(map_session_data* sd, int dx, int dy);
bool is_narrow_passage(map_session_data* sd, int x, int y);
bool leads_to_open_area(map_session_data* sd, int start_x, int start_y, int dx, int dy);
bool check_path_ahead_improved(map_session_data* sd, int start_x, int start_y, int dx, int dy, int steps);
bool find_escape_direction(map_session_data* sd, int* escape_x, int* escape_y);
bool find_area_exit(map_session_data* sd, int* exit_x, int* exit_y);
int calculate_smart_position_score(map_session_data* sd, int x, int y);
void add_position_to_history(map_session_data* sd);

std::vector<t_itemid> AA_ITEMIDS = { 97001 }; // Important here, define the item on which you can start autoattack from rental item



// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
bool aa_avoid_collision(struct map_session_data* sd, unsigned int now_tick) {
    if (!sd->aa.target_id) return false;
    
    struct mob_data* target_md = BL_CAST(BL_MOB, map_id2bl(sd->aa.target_id));
    if (!target_md) return false;
    
    if (target_md->ud.dir != -1 && target_md->ud.walktimer != INVALID_TIMER) {
        int mob_dir = target_md->ud.dir;
        int char_dir = sd->ud.dir;
        
        int dir_to_mob = map_calc_dir(&sd->bl, target_md->bl.x, target_md->bl.y);
        
        if (abs(mob_dir - char_dir) == 4 || 
            (abs(mob_dir - char_dir) == 3 && (mob_dir == 0 || char_dir == 0))) {
            
            int avoid_dir = (mob_dir + 2) % 8;
            int avoid_x = sd->bl.x + dirx[avoid_dir] * 3;
            int avoid_y = sd->bl.y + diry[avoid_dir] * 3;
            
            if (map_getcell(sd->bl.m, avoid_x, avoid_y, CELL_CHKNOPASS) == 0) {
                if (unit_walktoxy(&sd->bl, avoid_x, avoid_y, 0)) {
                    sd->aa.last_move = now_tick;
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * Try to escape from current stuck position
 * @param sd Player session data
 * @param now_tick Current tick
 * @return true if escape succeeded
 */
static bool aa_try_escape_stuck(map_session_data* sd, t_tick now_tick) {
	int escape_x, escape_y;
	if (find_escape_direction(sd, &escape_x, &escape_y)) {
		if (unit_walktoxy(&sd->bl, escape_x, escape_y, 0)) {
			sd->aa.last_move = now_tick;
			add_position_to_history(sd);
			sd->aa.lastposition.dx = escape_x - sd->bl.x;
			sd->aa.lastposition.dy = escape_y - sd->bl.y;
			return true;
		}
	}
	
	return false;
}

/**
 * Try to continue in previous direction
 * @param sd Player session data
 * @param now_tick Current tick
 * @return true if movement succeeded
 */
static bool aa_try_continue_direction(map_session_data* sd, t_tick now_tick) {
	if (!battle_config.feature_autoattack_movetype) return false;
	if (sd->aa.lastposition.dx == 0 && sd->aa.lastposition.dy == 0) return false;
	
	int tx = sd->aa.lastposition.dx + sd->bl.x;
	int ty = sd->aa.lastposition.dy + sd->bl.y;
	
	int direction_score = calculate_smart_position_score(sd, tx, ty);
	
	if (map_getcell(sd->bl.m, tx, ty, CELL_CHKPASS) &&
		direction_score > 30 && 
		unit_walktoxy(&sd->bl, tx, ty, 0)) {
		
		sd->aa.last_move = now_tick;
		add_position_to_history(sd);
		return true;
	}
	
	return false;
}

/**
 * Find best movement path using smart pathfinding
 * @param sd Player session data
 * @param now_tick Current tick
 * @param stuck_in_large Whether player is stuck in large area
 * @return true if movement succeeded
 */
static bool aa_smart_pathfinding(map_session_data* sd, t_tick now_tick) {
	int d = battle_config.feature_autoattack_move_min;
	if (d <= 0) d = 1;
	struct path_candidate {
		int x, y, score;
	} candidates[32];
	int candidate_count = 0;
	
	int directions[20][2] = {
		{d,0}, {-d,0}, {0,d}, {0,-d}, {d,d}, {-d,-d}, {d,-d}, {-d,d},
		{d*2,0}, {-d*2,0}, {0,d*2}, {0,-d*2}, {d*2,d*2}, {-d*2,-d*2}, {d*2,-d*2}, {-d*2,d*2},
		{d+1,0}, {-d-1,0}, {0,d+1}, {0,-d-1}
	};
	
	// Collect candidates
	for (int i = 0; i < 20 && candidate_count < 32; i++) {
		int x = sd->bl.x + directions[i][0];
		int y = sd->bl.y + directions[i][1];
		
		if (map_getcell(sd->bl.m, x, y, CELL_CHKPASS)) {
			candidates[candidate_count].x = x;
			candidates[candidate_count].y = y;
			candidates[candidate_count].score = calculate_smart_position_score(sd, x, y);
			
			int step_x = directions[i][0] > 0 ? 1 : (directions[i][0] < 0 ? -1 : 0);
			int step_y = directions[i][1] > 0 ? 1 : (directions[i][1] < 0 ? -1 : 0);
			if (check_path_ahead_improved(sd, x, y, step_x, step_y, AA_CONSTANTS::PATHFIND_RANGE)) {
				candidates[candidate_count].score += 30;
			}
			
			candidate_count++;
		}
	}
	
	// Sort candidates by score (bubble sort)
	for (int i = 0; i < candidate_count - 1; i++) {
		for (int j = i + 1; j < candidate_count; j++) {
			if (candidates[j].score > candidates[i].score) {
				struct path_candidate temp = candidates[i];
				candidates[i] = candidates[j];
				candidates[j] = temp;
			}
		}
	}
	
	// Select best candidate
	int min_score = 20;
	int max_detection_range = battle_config.feature_autoattack_mdetection;
	
	for (int i = 0; i < candidate_count; i++) {
		if (candidates[i].score > min_score) {
			// Check for nearby mobs
			int nearby_mob = 0;
			map_foreachinarea(buildin_autoattack_sub, sd->bl.m,
				candidates[i].x - max_detection_range, candidates[i].y - max_detection_range,
				candidates[i].x + max_detection_range, candidates[i].y + max_detection_range,
				BL_MOB, &nearby_mob, sd->status.char_id);
			
			if (nearby_mob) {
				struct mob_data* found_md = BL_CAST(BL_MOB, map_id2bl(nearby_mob));
				if (found_md && &found_md->bl.prev != nullptr && found_md->status.hp > 0 &&
					!(found_md->sc.option & (OPTION_HIDE | OPTION_CLOAK))) {
					sd->aa.target_id = nearby_mob;
					return true;
				}
			}
			
			// Try to move to this position
			if (unit_walktoxy(&sd->bl, candidates[i].x, candidates[i].y, 0)) {
				sd->aa.last_move = now_tick;
				add_position_to_history(sd);
				
				if (battle_config.feature_autoattack_movetype) {
					sd->aa.lastposition.dx = candidates[i].x - sd->bl.x;
					sd->aa.lastposition.dy = candidates[i].y - sd->bl.y;
				}
				return true;
			}
		}
	}
	
	return false;
}

/**
 * Basic fallback movement (8 directions)
 * @param sd Player session data
 * @param now_tick Current tick
 * @return true if movement succeeded
 */
static bool aa_basic_movement(map_session_data* sd, t_tick now_tick) {
	int basic_directions[8][2] = {
		{1,0}, {-1,0}, {0,1}, {0,-1}, 
		{1,1}, {-1,-1}, {1,-1}, {-1,1}
	};
	
	for (int i = 0; i < 8; i++) {
		int x = sd->bl.x + basic_directions[i][0];
		int y = sd->bl.y + basic_directions[i][1];
		
		if (map_getcell(sd->bl.m, x, y, CELL_CHKPASS)) {
			if (unit_walktoxy(&sd->bl, x, y, 0)) {
				sd->aa.last_move = now_tick;
				add_position_to_history(sd);
				
				if (battle_config.feature_autoattack_movetype) {
					sd->aa.lastposition.dx = basic_directions[i][0];
					sd->aa.lastposition.dy = basic_directions[i][1];
				}
				return true;
			}
		}
	}
	
	return false;
}

/**
 * Random emergency fallback movement
 * @param sd Player session data
 * @param now_tick Current tick
 * @return true if movement succeeded
 */
static bool aa_random_movement(map_session_data* sd, t_tick now_tick) {
	for (int attempts = 0; attempts < 20; attempts++) {
		int dx = (rnd() % 3) - 1;
		int dy = (rnd() % 3) - 1;
		
		if (dx == 0 && dy == 0) continue;
		
		int x = sd->bl.x + dx;
		int y = sd->bl.y + dy;
		
		if (map_getcell(sd->bl.m, x, y, CELL_CHKPASS) &&
			unit_walktoxy(&sd->bl, x, y, 0)) {
			
			sd->aa.last_move = now_tick;
			add_position_to_history(sd);
			
			if (battle_config.feature_autoattack_movetype) {
				sd->aa.lastposition.dx = dx;
				sd->aa.lastposition.dy = dy;
			}
			return true;
		}
	}
	
	return false;
}



// ==================== CORE MOVEMENT FUNCTIONS ====================
void add_position_to_history(map_session_data* sd) { sd->aa.lastposition.x = sd->bl.x; sd->aa.lastposition.y = sd->bl.y; }

bool has_visited_recently(map_session_data* sd, int x, int y) { return (abs(sd->aa.lastposition.x - x) + abs(sd->aa.lastposition.y - y)) <= AA_CONSTANTS::AOE_SIZE; }

// ==================== ANTI-OSCILLATION FUNCTIONS ====================
bool is_backtracking(map_session_data* sd, int x, int y) { return (abs(x - sd->aa.lastposition.x) + abs(y - sd->aa.lastposition.y)) <= 1; }

bool has_direction_momentum(map_session_data* sd, int dx, int dy) {
    if (sd->aa.lastposition.dx == 0 && sd->aa.lastposition.dy == 0) {
        return false;
    }
    int dot_product = (dx * sd->aa.lastposition.dx) + (dy * sd->aa.lastposition.dy);
    return dot_product > 0;
}

// ==================== TERRAIN ANALYSIS FUNCTIONS ====================
bool is_narrow_passage(map_session_data* sd, int x, int y) {
    int passable_directions = 0;
    int directions[8][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {-1,-1}, {1,-1}, {-1,1}};
    
    for (int i = 0; i < 8; i++) {
        int check_x = x + directions[i][0];
        int check_y = y + directions[i][1];
        if (map_getcell(sd->bl.m, check_x, check_y, CELL_CHKPASS)) {
            passable_directions++;
        }
    }
    
    return (passable_directions <= 3);
}

bool is_corridor(map_session_data* sd, int x, int y) {
    int horizontal_passage = 0;
    int vertical_passage = 0;
    
    for (int dx = -AA_CONSTANTS::CORRIDOR_DETECTION_RANGE; dx <= AA_CONSTANTS::CORRIDOR_DETECTION_RANGE; dx++) {
        if (map_getcell(sd->bl.m, x + dx, y, CELL_CHKPASS)) {
            horizontal_passage++;
        }
    }
    
    for (int dy = -AA_CONSTANTS::CORRIDOR_DETECTION_RANGE; dy <= AA_CONSTANTS::CORRIDOR_DETECTION_RANGE; dy++) {
        if (map_getcell(sd->bl.m, x, y + dy, CELL_CHKPASS)) {
            vertical_passage++;
        }
    }
    
    return (horizontal_passage >= 5 || vertical_passage >= 5);
}

bool leads_to_open_area(map_session_data* sd, int start_x, int start_y, int dx, int dy) {
    for (int step = 2; step <= 6; step++) {
        int check_x = start_x + (dx * step);
        int check_y = start_y + (dy * step);
        
        if (!map_getcell(sd->bl.m, check_x, check_y, CELL_CHKPASS)) {
            return false;
        }
        
        int open_spaces = 0;
        for (int ox = -2; ox <= 2; ox++) {
            for (int oy = -2; oy <= 2; oy++) {
                if (map_getcell(sd->bl.m, check_x + ox, check_y + oy, CELL_CHKPASS)) {
                    open_spaces++;
                }
            }
        }
        
        if (open_spaces >= 15) {
            return true;
        }
    }
    return false;
}

bool check_path_ahead_improved(map_session_data* sd, int start_x, int start_y, int dx, int dy, int steps) {
    int valid_steps = 0;
    
    for (int step = 1; step <= steps; step++) {
        int check_x = start_x + (dx * step);
        int check_y = start_y + (dy * step);
        
        if (map_getcell(sd->bl.m, check_x, check_y, CELL_CHKPASS)) {
            valid_steps++;
            if (!has_visited_recently(sd, check_x, check_y)) {
                valid_steps += 2;
            }
        } else {
            break;
        }
    }
    
    return valid_steps >= AA_CONSTANTS::PATHFIND_RANGE;
}

// ==================== AREA DETECTION & ESCAPE ====================
bool find_escape_direction(map_session_data* sd, int* escape_x, int* escape_y) { 
	int escape_dist = 5;
	int directions[8][2] = {{escape_dist,0}, {-escape_dist,0}, {0,escape_dist}, {0,-escape_dist}, {escape_dist,escape_dist}, {-escape_dist,-escape_dist}, {escape_dist,-escape_dist}, {-escape_dist,escape_dist}}; 
	for (int i = 0; i < 8; i++) { 
		int x = sd->bl.x + directions[i][0]; 
		int y = sd->bl.y + directions[i][1]; 
		if (map_getcell(sd->bl.m, x, y, CELL_CHKPASS)) { 
			*escape_x = x; 
			*escape_y = y; 
			return true; 
		} 
	} 
	return false; 
}

bool find_area_exit(map_session_data* sd, int* exit_x, int* exit_y) { int directions[8][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {-1,-1}, {1,-1}, {-1,1}}; for (int i = 0; i < 8; i++) { for (int distance = 3; distance <= 8; distance++) { int x = sd->bl.x + (directions[i][0] * distance); int y = sd->bl.y + (directions[i][1] * distance); if (map_getcell(sd->bl.m, x, y, CELL_CHKPASS)) { *exit_x = x; *exit_y = y; return true; } } } return false; }

// ==================== POSITION SCORING ====================
int calculate_smart_position_score(map_session_data* sd, int x, int y) { 
	int score = 100; 
	int dx = x - sd->bl.x; 
	int dy = y - sd->bl.y; 
	int move_distance = abs(dx) + abs(dy); 
	bool is_narrow = is_narrow_passage(sd, x, y); 
	bool is_corridor_path = is_corridor(sd, x, y); 
	if (!is_narrow && move_distance < AA_CONSTANTS::MIN_MOVE_DISTANCE) score -= 20; 
	if (is_narrow) score += AA_CONSTANTS::NARROW_PATH_BONUS; 
	if (is_corridor_path) score += 80; 
	if (is_backtracking(sd, x, y)) score -= (is_narrow ? 20 : 50); 
	if (has_direction_momentum(sd, dx, dy)) score += AA_CONSTANTS::DIRECTION_PERSISTENCE_BONUS; 
	if (has_visited_recently(sd, x, y)) score -= (is_narrow || is_corridor_path) ? 15 : 30; 
	return score; 
}


void aa_save(map_session_data* sd){
	int i;  
    // aa_common_config
    if (SQL_ERROR == Sql_Query(mmysql_handle, 
        "INSERT INTO `aa_common_config` "
        "(`char_id`, `stopmelee`, `pickup_item_config`, `prio_item_config`, "
        "`aggressive_behavior`, `autositregen_conf`, `autositregen_maxhp`, "
        "`autositregen_minhp`, `autositregen_maxsp`, `autositregen_minsp`, "
        "`tp_use_teleport`, `tp_use_flywing`, `tp_min_hp`, `tp_delay_nomobmeet`, "
        "`tp_mvp`, `tp_miniboss`, `accept_party_request`, `token_siegfried`, "
        "`return_to_savepoint`, `map_mob_selection`, `action_on_end`, "
        "`monster_surround`, `monk_combo`) "
        "VALUES (%u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d) "
        "ON DUPLICATE KEY UPDATE "
        "`stopmelee` = %d, "
        "`pickup_item_config` = %d, "
        "`prio_item_config` = %d, "
        "`aggressive_behavior` = %d, "
        "`autositregen_conf` = %d, "
        "`autositregen_maxhp` = %d, "
        "`autositregen_minhp` = %d, "
        "`autositregen_maxsp` = %d, "
        "`autositregen_minsp` = %d, "
        "`tp_use_teleport` = %d, "
        "`tp_use_flywing` = %d, "
        "`tp_min_hp` = %d, "
        "`tp_delay_nomobmeet` = %d, "
        "`tp_mvp` = %d, "
        "`tp_miniboss` = %d, "
        "`accept_party_request` = %d, "
        "`token_siegfried` = %d, "
        "`return_to_savepoint` = %d, "
        "`map_mob_selection` = %d, "
        "`action_on_end` = %d, "
        "`monster_surround` = %d, "
        "`monk_combo` = %d",
        // INSERT VALUES (23 Ã?Â¨Ã’Â©
        sd->status.char_id,
        sd->aa.stopmelee,
        sd->aa.pickup_item_config,
        sd->aa.prio_item_config,
        sd->aa.mobs.aggressive_behavior,
        sd->aa.autositregen.is_active,
        sd->aa.autositregen.max_hp,
        sd->aa.autositregen.min_hp,
        sd->aa.autositregen.max_sp,
        sd->aa.autositregen.min_sp,
        sd->aa.teleport.use_teleport,
        sd->aa.teleport.use_flywing,
        sd->aa.teleport.min_hp,
        sd->aa.teleport.delay_nomobmeet,
        sd->aa.teleport.tp_mvp,
        sd->aa.teleport.tp_miniboss,
        sd->aa.accept_party_request,
        sd->aa.token_siegfried,
        sd->aa.return_to_savepoint,
        sd->aa.mobs.map,
        sd->aa.action_on_end,
        sd->aa.monster_surround,
        sd->aa.monk_combo,
        // UPDATE VALUES (22 Ã?Â¨Ã’Â - Ã¤?Â¨Ã•Â©Ã?Â§ char_id)
        sd->aa.stopmelee,
        sd->aa.pickup_item_config,
        sd->aa.prio_item_config,
        sd->aa.mobs.aggressive_behavior,
        sd->aa.autositregen.is_active,
        sd->aa.autositregen.max_hp,
        sd->aa.autositregen.min_hp,
        sd->aa.autositregen.max_sp,
        sd->aa.autositregen.min_sp,
        sd->aa.teleport.use_teleport,
        sd->aa.teleport.use_flywing,
        sd->aa.teleport.min_hp,
        sd->aa.teleport.delay_nomobmeet,
        sd->aa.teleport.tp_mvp,
        sd->aa.teleport.tp_miniboss,
        sd->aa.accept_party_request,
        sd->aa.token_siegfried,
        sd->aa.return_to_savepoint,
        sd->aa.mobs.map,
        sd->aa.action_on_end,
        sd->aa.monster_surround,
        sd->aa.monk_combo
    )) {
        Sql_ShowDebug(mmysql_handle);
        ShowError("aa_save: Failed to save config for char_id=%u\n", 
            sd->status.char_id);
    }
    else {
        ShowInfo("aa_save: Successfully saved config for char_id=%u\n", 
            sd->status.char_id);
    }


	//clean aa_items
	if( SQL_ERROR == Sql_Query( mmysql_handle, "DELETE FROM `aa_items` WHERE `char_id` = %d", sd->status.char_id ) ){
		Sql_ShowDebug(mmysql_handle);
	}
	//insert aa_items - 0 - autobuffitems
	if(!sd->aa.autobuffitems.empty()){
		for(auto &itAutobuffitem : sd->aa.autobuffitems){
			if( SQL_ERROR == Sql_Query( mmysql_handle, "INSERT INTO `aa_items` (`char_id`,`type`,`item_id`,`status`) VALUES (%d, 0, %d, %d)", sd->status.char_id, itAutobuffitem.item_id, itAutobuffitem.status) ){
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}
	//insert aa_items - 1 - autopotion
	if(!sd->aa.autopotion.empty()){
		for(auto &itAutopotion : sd->aa.autopotion){
			if( SQL_ERROR == Sql_Query( mmysql_handle, "INSERT INTO `aa_items` (`char_id`,`type`,`item_id`,`min_hp`,`min_sp`) VALUES (%d, 1, %d, %d, %d)", sd->status.char_id, itAutopotion.item_id, itAutopotion.min_hp, itAutopotion.min_sp ) ){
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}
	//insert aa_items - 2 - pickup_item_id
	if(!sd->aa.pickup_item_id.empty()){
		for (i=0; i<sd->aa.pickup_item_id.size(); i++){
			if( SQL_ERROR == Sql_Query( mmysql_handle, "INSERT INTO `aa_items` (`char_id`,`type`,`item_id`) VALUES (%d, 2, %d)", sd->status.char_id, sd->aa.pickup_item_id.at(i) ) ){
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}

	//clean aa_mobs
	if( SQL_ERROR == Sql_Query( mmysql_handle, "DELETE FROM `aa_mobs` WHERE `char_id` = %d", sd->status.char_id ) ){
		Sql_ShowDebug(mmysql_handle);
	}
	//insert aa_mobs
	if(!sd->aa.mobs.id.empty()){
		for (i=0; i<sd->aa.mobs.id.size(); i++){
			if( SQL_ERROR == Sql_Query( mmysql_handle, "INSERT INTO `aa_mobs` (`char_id`,`mob_id`) VALUES (%d, %d)", sd->status.char_id, sd->aa.mobs.id.at(i) ) ){
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}

	//clean aa_skills
	if( SQL_ERROR == Sql_Query( mmysql_handle, "DELETE FROM `aa_skills` WHERE `char_id` = %d", sd->status.char_id ) ){
		Sql_ShowDebug(mmysql_handle);
	}
	//insert aa_skills - 0 - autoheal
	if(!sd->aa.autoheal.empty()){
		for(auto &itAutoheal : sd->aa.autoheal){
			if( SQL_ERROR == Sql_Query( mmysql_handle, "INSERT INTO `aa_skills` (`char_id`,`type`,`skill_id`,`skill_lv`,`min_hp`) VALUES (%d, 0, %d, %d, %d)", sd->status.char_id, itAutoheal.skill_id, itAutoheal.skill_lv, itAutoheal.min_hp ) ){
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}
	//insert aa_skills - 1 - autobuffskills
	if(!sd->aa.autobuffskills.empty()){
		for(auto &itAutobuffskills : sd->aa.autobuffskills){
			if( SQL_ERROR == Sql_Query( mmysql_handle, "INSERT INTO `aa_skills` (`char_id`,`type`,`skill_id`,`skill_lv`) VALUES (%d, 1, %d, %d)", sd->status.char_id, itAutobuffskills.skill_id, itAutobuffskills.skill_lv ) ){
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}
	//insert aa_skills - 2 - autoattackskills
	if(!sd->aa.autoattackskills.empty()){
		for(auto &itAutoattackskills : sd->aa.autoattackskills){
			if( SQL_ERROR == Sql_Query( mmysql_handle, "INSERT INTO `aa_skills` (`char_id`,`type`,`skill_id`,`skill_lv`) VALUES (%d, 2, %d, %d)", sd->status.char_id, itAutoattackskills.skill_id, itAutoattackskills.skill_lv ) ){
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}
	//autobuyitems
	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `aa_autobuyitems` WHERE `char_id` = %d", sd->status.char_id)) {
		Sql_ShowDebug(mmysql_handle);
	}
	if (sd->aa.autobuyitems.size()) {
		for (const auto& it : sd->aa.autobuyitems) {
			if (SQL_ERROR == Sql_Query(mmysql_handle,
				"INSERT INTO `aa_autobuyitems` (`char_id`, `item_id`, `quantity`, `min_trigger`) "
				"VALUES (%d, %d, %d, %d)",
				sd->status.char_id, it.item_id, it.quantity, it.min_trigger))
			{
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}
	// autosellitems
	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `aa_autosellitems` WHERE `char_id` = %d", sd->status.char_id)) {
		Sql_ShowDebug(mmysql_handle);
	}
	if (!sd->aa.autosellitems.empty()) {
		for (const auto& it : sd->aa.autosellitems) {
			if (SQL_ERROR == Sql_Query(mmysql_handle,
				"INSERT INTO `aa_autosellitems` (`char_id`, `item_id`) "
				"VALUES (%d, %d)",
				sd->status.char_id, it.item_id))
			{
				Sql_ShowDebug(mmysql_handle);
			}
		}
	}
}

void aa_load(map_session_data* sd) {
	int type;
	t_tick tick = gettick();

	// aa_common_config
	if (Sql_Query(mmysql_handle,
		"SELECT `stopmelee`,`pickup_item_config`,`prio_item_config`,`aggressive_behavior`,`autositregen_conf`,`autositregen_maxhp`,`autositregen_minhp`,`autositregen_maxsp`,`autositregen_minsp`,`tp_use_teleport`,`tp_use_flywing`,`tp_min_hp`,`tp_delay_nomobmeet`,`tp_mvp`,`tp_miniboss`,`accept_party_request`,`token_siegfried`,`return_to_savepoint`,`map_mob_selection`,`action_on_end`,`monster_surround`,`monk_combo` "
		"FROM `aa_common_config` "
		"WHERE `char_id` = %d ",
		sd->status.char_id) != SQL_SUCCESS)
	{
		Sql_ShowDebug(mmysql_handle);
		return;
	}

	if (Sql_NumRows(mmysql_handle) > 0) {
		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
			char* data;
			Sql_GetData(mmysql_handle, 0, &data, NULL); sd->aa.stopmelee = atoi(data);
			Sql_GetData(mmysql_handle, 1, &data, NULL); sd->aa.pickup_item_config = atoi(data);
			Sql_GetData(mmysql_handle, 2, &data, NULL); sd->aa.prio_item_config = atoi(data);
			Sql_GetData(mmysql_handle, 3, &data, NULL); sd->aa.mobs.aggressive_behavior = atoi(data);
			Sql_GetData(mmysql_handle, 4, &data, NULL); sd->aa.autositregen.is_active = atoi(data);
			Sql_GetData(mmysql_handle, 5, &data, NULL); sd->aa.autositregen.max_hp = atoi(data);
			Sql_GetData(mmysql_handle, 6, &data, NULL); sd->aa.autositregen.min_hp = atoi(data);
			Sql_GetData(mmysql_handle, 7, &data, NULL); sd->aa.autositregen.max_sp = atoi(data);
			Sql_GetData(mmysql_handle, 8, &data, NULL); sd->aa.autositregen.min_sp = atoi(data);
			Sql_GetData(mmysql_handle, 9, &data, NULL); sd->aa.teleport.use_teleport = atoi(data);
			Sql_GetData(mmysql_handle, 10, &data, NULL); sd->aa.teleport.use_flywing = atoi(data);
			Sql_GetData(mmysql_handle, 11, &data, NULL); sd->aa.teleport.min_hp = atoi(data);
			Sql_GetData(mmysql_handle, 12, &data, NULL); sd->aa.teleport.delay_nomobmeet = atoi(data);
			Sql_GetData(mmysql_handle, 13, &data, NULL); sd->aa.teleport.tp_mvp = atoi(data);
			Sql_GetData(mmysql_handle, 14, &data, NULL); sd->aa.teleport.tp_miniboss = atoi(data);
			Sql_GetData(mmysql_handle, 15, &data, NULL); sd->aa.accept_party_request = atoi(data);
			Sql_GetData(mmysql_handle, 16, &data, NULL); sd->aa.token_siegfried = atoi(data);
			Sql_GetData(mmysql_handle, 17, &data, NULL); sd->aa.return_to_savepoint = atoi(data);
			Sql_GetData(mmysql_handle, 18, &data, NULL); sd->aa.mobs.map = atoi(data);
			Sql_GetData(mmysql_handle, 19, &data, NULL); sd->aa.action_on_end = atoi(data);
			Sql_GetData(mmysql_handle, 20, &data, NULL); sd->aa.monster_surround = atoi(data);
			Sql_GetData(mmysql_handle, 21, &data, NULL); sd->aa.monk_combo = atoi(data);
		}
	}
	else {
		sd->aa.stopmelee = 0;
		sd->aa.pickup_item_config = 0;
		sd->aa.prio_item_config = 0;
		sd->aa.mobs.aggressive_behavior = 0;
		sd->aa.autositregen.is_active = 0;
		sd->aa.autositregen.max_hp = 0;
		sd->aa.autositregen.min_hp = 0;
		sd->aa.autositregen.max_sp = 0;
		sd->aa.autositregen.min_sp = 0;
		sd->aa.teleport.use_teleport = 0;
		sd->aa.teleport.use_flywing = 0;
		sd->aa.teleport.min_hp = 0;
		sd->aa.teleport.delay_nomobmeet = 0;
		sd->aa.teleport.tp_mvp = 0;
		sd->aa.teleport.tp_miniboss = 0;
		sd->aa.accept_party_request = 1;
		sd->aa.token_siegfried = 1;
		sd->aa.return_to_savepoint = 1;
		sd->aa.mobs.map = 0;
		sd->aa.action_on_end = 0;
		sd->aa.monster_surround = 0;
		sd->aa.duration_ = 0;
		sd->aa.monk_combo = 0;
	}

	Sql_FreeResult(mmysql_handle);

	// aa_items
	if (Sql_Query(mmysql_handle,
		"SELECT `type`,`item_id`,`min_hp`,`min_sp`,`status` "
		"FROM `aa_items` "
		"WHERE `char_id` = %d ",
		sd->status.char_id) != SQL_SUCCESS)
	{
		Sql_ShowDebug(mmysql_handle);
		return;
	}

	if (Sql_NumRows(mmysql_handle) > 0) {
		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
			char* data;
			Sql_GetData(mmysql_handle, 0, &data, NULL); type = atoi(data);
			switch (type) {
			case 0:
				struct s_autobuffitems autobuffitems;
				autobuffitems.is_active = 1;
				Sql_GetData(mmysql_handle, 1, &data, NULL); autobuffitems.item_id = atoi(data);
				Sql_GetData(mmysql_handle, 4, &data, NULL); autobuffitems.status = atoi(data);
				sd->aa.autobuffitems.push_back(autobuffitems);
				break;
			case 1:
				struct s_autopotion autopotion;
				autopotion.is_active = 1;
				Sql_GetData(mmysql_handle, 1, &data, NULL); autopotion.item_id = atoi(data);
				Sql_GetData(mmysql_handle, 2, &data, NULL); autopotion.min_hp = atoi(data);
				Sql_GetData(mmysql_handle, 3, &data, NULL); autopotion.min_sp = atoi(data);
				sd->aa.autopotion.push_back(autopotion);
				break;
			case 2:
				t_itemid nameid;
				Sql_GetData(mmysql_handle, 1, &data, NULL); nameid = atoi(data);
				sd->aa.pickup_item_id.push_back(nameid);
				break;
			}
		}
	}
	Sql_FreeResult(mmysql_handle);

	//Auto Buy Item
	sd->aa.autobuyitems.clear();
	
	if (Sql_Query(mmysql_handle, "SELECT `item_id`, `quantity`, `min_trigger` FROM `aa_autobuyitems` WHERE `char_id` = %d", sd->status.char_id) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
		return;
	}
	
	if (Sql_NumRows(mmysql_handle)) {
		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
			char* data;
			s_autobuyitems entry = {};
	
			Sql_GetData(mmysql_handle, 0, &data, nullptr); entry.item_id = atoi(data);
			Sql_GetData(mmysql_handle, 1, &data, nullptr); entry.quantity = atoi(data);
			Sql_GetData(mmysql_handle, 2, &data, nullptr); entry.min_trigger = atoi(data);
	
			sd->aa.autobuyitems.push_back(entry);
		}
	}
	
	Sql_FreeResult(mmysql_handle);

	// Auto Sell Item
	sd->aa.autosellitems.clear();
	
	if (Sql_Query(mmysql_handle, "SELECT `item_id` FROM `aa_autosellitems` WHERE `char_id` = %d", sd->status.char_id) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
		return;
	}
	
	if (Sql_NumRows(mmysql_handle)) {
		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
			char* data;
			s_autosellitems entry = {};
	
			Sql_GetData(mmysql_handle, 0, &data, nullptr);
			entry.item_id = atoi(data);
	
			sd->aa.autosellitems.push_back(entry);
		}
	}
	
	Sql_FreeResult(mmysql_handle);

	// aa_mobs
	if (sd->aa.mobs.map == sd->bl.m) {
		if (Sql_Query(mmysql_handle,
			"SELECT `mob_id` "
			"FROM `aa_mobs` "
			"WHERE `char_id` = %d ",
			sd->status.char_id) != SQL_SUCCESS)
		{
			Sql_ShowDebug(mmysql_handle);
			return;
		}

		if (Sql_NumRows(mmysql_handle) > 0) {
			while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
				char* data;
				uint32 mob_id;
				Sql_GetData(mmysql_handle, 0, &data, NULL); mob_id = atoi(data);
				sd->aa.mobs.id.push_back(mob_id);
			}
		}
		Sql_FreeResult(mmysql_handle);
	}
	else
		sd->aa.mobs.map = sd->bl.m;

	// aa_skills
	sd->aa.skill_range = -1;
	if (Sql_Query(mmysql_handle,
		"SELECT `type`,`skill_id`,`skill_lv`,`min_hp`"
		"FROM `aa_skills` "
		"WHERE `char_id` = %d ",
		sd->status.char_id ) != SQL_SUCCESS )
	{
		Sql_ShowDebug(mmysql_handle);
		return;
	}

	if( Sql_NumRows(mmysql_handle) > 0 ){
		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
			char* data;
			Sql_GetData(mmysql_handle, 0, &data, NULL); type = atoi(data);
			switch(type){
				case 0:
					struct s_autoheal autoheal;
					autoheal.is_active = 1;
					Sql_GetData(mmysql_handle, 1, &data, NULL); autoheal.skill_id = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, NULL); autoheal.skill_lv = atoi(data);
					Sql_GetData(mmysql_handle, 3, &data, NULL); autoheal.min_hp = atoi(data);
					autoheal.last_use = 1;
					sd->aa.autoheal.push_back(autoheal);
					break;
				case 1:
					struct s_autobuffskills autobuffskills;
					autobuffskills.is_active = 1;
					Sql_GetData(mmysql_handle, 1, &data, NULL); autobuffskills.skill_id = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, NULL); autobuffskills.skill_lv = atoi(data);
					autobuffskills.last_use = 1;
					sd->aa.autobuffskills.push_back(autobuffskills);
					break;
				case 2:
					struct s_autoattackskills autoattackskills;
					autoattackskills.is_active = 1;
					Sql_GetData(mmysql_handle, 1, &data, NULL); autoattackskills.skill_id = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, NULL); autoattackskills.skill_lv = atoi(data);
					autoattackskills.last_use = 1;
					if (sd->aa.skill_range < 0)
						sd->aa.skill_range = skill_get_range2(&sd->bl, autoattackskills.skill_id, autoattackskills.skill_lv, true);
					else
						sd->aa.skill_range = max(skill_get_range2(&sd->bl, autoattackskills.skill_id, autoattackskills.skill_lv, true), sd->aa.skill_range);
					sd->aa.autoattackskills.push_back(autoattackskills);
					break;
			}
		}
	}
	Sql_FreeResult(mmysql_handle);

	aa_changestate_autoattack(sd, 0);
}

void aa_skill_range_calc(map_session_data* sd) {
	auto& skills = sd->aa.autoattackskills;

	if (skills.empty()) {
		sd->aa.skill_range = -1;
		return;
	}

	auto best_skill = std::min_element(skills.begin(), skills.end(),
		[&sd](const s_autoattackskills& a, const s_autoattackskills& b) {
			return skill_get_range2(&sd->bl, a.skill_id, a.skill_lv, true) <
				skill_get_range2(&sd->bl, b.skill_id, b.skill_lv, true);
		});

	sd->aa.skill_range = skill_get_range2(&sd->bl, best_skill->skill_id, best_skill->skill_lv, true);
}

void aa_mob_ai_search_mvpcheck(struct block_list* bl, struct mob_data* md){
	if (!battle_config.feature_autoattack_teleport_mvp)
		return;

	if (bl->type == BL_PC) {
		TBL_PC* sd = BL_CAST(BL_PC, bl);
		if (sd->state.autoattack) {
			e_mob_bosstype bosstype = md->get_bosstype();
			t_tick now = gettick();

			if (DIFF_TICK(now, sd->aa.last_teleport) < AA_CONSTANTS::BOSS_TELEPORT_DELAY)
				return;

			if (sd->aa.teleport.tp_mvp && bosstype == BOSSTYPE_MVP)
				aa_teleport(sd);
			else if (sd->aa.teleport.tp_miniboss && bosstype == BOSSTYPE_MINIBOSS)
				aa_teleport(sd);
		}
	}
}

void aa_priority_on_hit(map_session_data* sd, struct block_list* src) {
	struct status_data* status = status_get_status_data(&sd->bl);

	if(sd->state.autoattack){

		// Teleport condition if hp is bellow limit
		if ((!sd->aa.teleport.use_teleport || !sd->aa.teleport.use_flywing) // player myst allow teleport or flywing
			&& sd->aa.teleport.min_hp // player must have set min hp value to tp
			&& (status->hp * 100 / sd->aa.teleport.min_hp) < sd->status.max_hp) { // player hp is bellow min hp

			if (aa_teleport(sd))
				return;
		}

		// Keep fighting a monster that is already attacking the player. If the
		// current target is not an attacker (or became invalid), prioritize the
		// new attacker. aa_check_target still honors the user's ignore list.
		if (src->type == BL_MOB) {
			TBL_MOB* current_md = map_id2md(sd->aa.target_id);
			bool current_is_attacker = current_md && current_md->target_id == sd->bl.id &&
				aa_check_target(sd, current_md->bl.id);

			if (!current_is_attacker && aa_check_target(sd, src->id)) {
				if (sd->aa.itempick_id)
					sd->aa.itempick_id = 0;

				aa_target_change(sd, src->id);
			}
		}

		sd->aa.last_hit = gettick();
	}
}

void aa_target_change(map_session_data* sd, int id) {
	struct unit_data* ud = unit_bl2ud(&sd->bl);
	if(ud)
		ud->target = id;

	sd->aa.target_id = id;

	if (id > 0) {
		struct mob_data* md_target = (struct mob_data*)map_id2bl(sd->aa.target_id);
		if (md_target) {
			switch (sd->status.weapon) {
			case W_BOW:
			case W_WHIP:
			case W_MUSICAL:
				aa_arrowchange(sd, md_target);
				break;
			case W_REVOLVER:
			case W_RIFLE:
			case W_GATLING:
			case W_SHOTGUN:
			case W_GRENADE:
				aa_bulletchange(sd, md_target);
				break;
			}
		}
	}
}

void aa_reset_ondead(map_session_data* sd) {
	// not mandatory yet
}

bool aa_canuseskill(map_session_data* sd, uint16 skill_id, uint16 skill_lv) {
    // Ensure the session data is valid
    if (sd == nullptr) 
        return false;

	//Special fix for EDP
	if (skill_id == ASC_EDP && sd->sc.getSCE(SC_EDP))
		return false;

	map_data* mapdata = map_getmapdata(sd->bl.m);
	if (mapdata->getMapFlag(MF_NOSKILL))
		return false;

    const int inf = skill_get_inf(skill_id); // Skill information flags
    const t_tick tick = gettick();          // Current tick time

    // Check if the player has the required skill level
    if (pc_checkskill(sd, skill_id) < skill_lv)
        return false;

    // Check if the player has enough SP to use the skill
    if (skill_get_sp(skill_id, skill_lv) > sd->battle_status.sp)
        return false;

    // Reset idle time if applicable
    if (battle_config.idletime_option & IDLE_USESKILLTOID)
        sd->idletime = tick;

    // Check for conditions that prevent skill usage
    if ((pc_cant_act2(sd) || sd->chatID) && 
        skill_id != RK_REFRESH && 
        !(skill_id == SR_GENTLETOUCH_CURE && (sd->sc.opt1 == OPT1_STONE || sd->sc.opt1 == OPT1_FREEZE || sd->sc.opt1 == OPT1_STUN)) &&
        sd->state.storage_flag && 
        !(inf & INF_SELF_SKILL)) {
        return false;
    }

    // Cannot use skills while sitting
    if (pc_issit(sd))
        return false;

    // Check if the skill is restricted for the player
    if (skill_isNotOk(skill_id, sd))
        return false;

    // Verify skill conditions at the beginning and end of the cast
	// Special fix for EDP
	if (skill_id != ASC_EDP) {
		if (!skill_check_condition_castbegin(sd, skill_id, skill_lv) ||
			!skill_check_condition_castend(sd, skill_id, skill_lv)) {
			return false;
		}
	}

    // Ensure no active skill timer unless it's a valid exception
    if (sd->ud.skilltimer != INVALID_TIMER) {
        if (skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST)
            return false;
    } else if (DIFF_TICK(tick, sd->ud.canact_tick) < 0) {
        if (sd->skillitem != skill_id)
            return false;
    }

    // Costume option disables skill usage
    if (sd->sc.option & OPTION_COSTUME)
        return false;

    // Basilica restrictions
    if (sd->sc.getSCE(SC_BASILICA) && 
        (skill_id != HP_BASILICA || sd->sc.getSCE(SC_BASILICA)->val4 != sd->bl.id)) {
        return false; // Only the caster can stop Basilica
    }

    // Check if a skill menu is open
    if (sd->menuskill_id) {
        if (sd->menuskill_id == SA_TAMINGMONSTER) {
            clif_menuskill_clear(sd); // Cancel pet capture
        } else if (sd->menuskill_id != SA_AUTOSPELL) {
            return false; // Cannot use skills while a menu is open
        }
    }

    // Ensure the skill level does not exceed the player's known level
    skill_lv = min(pc_checkskill(sd, skill_id), skill_lv);

    // Remove invincibility timer if applicable
    pc_delinvincibletimer(sd);

    return true;
}


//sub routine to search item by item on floor
int buildin_autopick_sub(struct block_list *bl, va_list ap) {
    // Extract arguments from va_list
    int* itempick_id = va_arg(ap, int*);
    int src_id = va_arg(ap, int);

	if (*itempick_id > 0)
		return 0;

	if (!bl || bl->type != BL_ITEM)
		return 0;

    // Retrieve sd from player
	map_session_data* sd = map_id2sd(src_id);
    if (!sd || !bl) // Validate both source and target block lists
        return 0;

    // If itempick_id is already set, skip processing
    if (sd->aa.itempick_id != 0)
        return 0;

    // Check item pickup conditions and update itempick_id if valid
	if (!aa_check_item_pickup(sd, bl))
		return 0;

	*itempick_id = bl->id;
    return 1;
}

//Check if an item can be pick up around
bool aa_check_item_pickup(map_session_data *sd, struct block_list *bl) {
    struct flooritem_data* fitem = (struct flooritem_data*)bl;

	if (!bl)
		return false;

    struct party_data* p = (sd->status.party_id) ? party_search(sd->status.party_id) : nullptr;
    t_tick tick = gettick();

    // Validate ownership and party conditions
    if (fitem->first_get_charid > 0 && fitem->first_get_charid != sd->status.char_id) {
        map_session_data *first_sd = map_charid2sd(fitem->first_get_charid);
        if (DIFF_TICK(tick,fitem->first_get_tick) < 0) {
            if (!(p && p->party.item&1 &&
                first_sd && first_sd->status.party_id == sd->status.party_id
                ))
                return false;
        }
        else if (fitem->second_get_charid > 0 && fitem->second_get_charid != sd->status.char_id) {
            map_session_data *second_sd = map_charid2sd(fitem->second_get_charid);
            if (DIFF_TICK(tick, fitem->second_get_tick) < 0) {
                if (!(p && p->party.item&1 &&
                    ((first_sd && first_sd->status.party_id == sd->status.party_id) ||
                    (second_sd && second_sd->status.party_id == sd->status.party_id))
                    ))
                    return false;
            }
            else if (fitem->third_get_charid > 0 && fitem->third_get_charid != sd->status.char_id){
                map_session_data *third_sd = map_charid2sd(fitem->third_get_charid);
                if (DIFF_TICK(tick,fitem->third_get_tick) < 0) {
                    if(!(p && p->party.item&1 &&
                        ((first_sd && first_sd->status.party_id == sd->status.party_id) ||
                        (second_sd && second_sd->status.party_id == sd->status.party_id) ||
                        (third_sd && third_sd->status.party_id == sd->status.party_id))
                        ))
                        return false;
                }
            }
        }
    }

	// Check custom item pickup configuration
    if (sd->aa.pickup_item_config == 1 && !sd->aa.pickup_item_id.empty()) {
        for (const auto& item_id : sd->aa.pickup_item_id) {

			if (item_id == fitem->item.nameid) {
				if (path_search(nullptr, sd->bl.m, sd->bl.x, sd->bl.y, bl->x, bl->y, 1, CELL_CHKNOREACH) &&
					distance_xy(sd->bl.x, sd->bl.y, bl->x, bl->y) < 11){
					return true;
				}
			}
        }

		return false;
    }

    // Verify reachability and distance if no special item pickup instruction
    if (path_search(nullptr, sd->bl.m, sd->bl.x, sd->bl.y, bl->x, bl->y, 1, CELL_CHKNOREACH) && 
        distance_xy(sd->bl.x, sd->bl.y, bl->x, bl->y) < 11) {
        return true;
    }

	return false;
}

unsigned int aa_check_item_pickup_onfloor(map_session_data* sd) {
	// Retrieve the block list associated with the current item pick ID
	struct block_list* bl = map_id2bl(sd->aa.itempick_id);

	// Check if the current item can be picked up
	if (!bl || bl->type != BL_ITEM || !aa_check_item_pickup(sd, bl)) {
		sd->aa.itempick_id = 0; // Reset the item pick ID
		int itempick_id_ = 0;

		// Iterate through the detection radius to find a suitable item
		for (int radius = 0; radius < battle_config.feature_autoattack_pdetection; ++radius) {
			map_foreachinarea(
				buildin_autopick_sub,
				sd->bl.m,
				sd->bl.x - radius,
				sd->bl.y - radius,
				sd->bl.x + radius,
				sd->bl.y + radius,
				BL_ITEM,
				&itempick_id_,
				sd->bl.id
			);

			// If an item is found, set the item pick ID and break
			if (itempick_id_) {
				sd->aa.itempick_id = itempick_id_;
				break;
			}
		}
	}

	return sd->aa.itempick_id;
}

bool aa_check_target(map_session_data* sd, unsigned int id) {
	if (id == 0)
		return false;

	struct block_list* bl = map_id2bl(id); // Retrieve the block list for the target ID
	if (!bl)
		return false;

	// Check kill-steal protection
	if (battle_config.ksprotection && mob_ksprotected(&sd->bl, bl))
		return false;

	//target dead
	if (status_isdead(bl))
		return false;

	//not enemy
	if (battle_check_target(&sd->bl, bl, BCT_ENEMY) <= 0 || !status_check_skilluse(&sd->bl, bl, 0, 0))
		return false;

	//can't attack
	if (!unit_can_attack(&sd->bl, bl->id))
		return false;

	status_data* sstatus = status_get_status_data(&sd->bl);
	int32 range = (sd->aa.stopmelee == 0 || (sd->aa.stopmelee == 2 && sstatus->sp < 100))
		? (sd->aa.skill_range >= 0 ? min(sstatus->rhw.range, sd->aa.skill_range) : sstatus->rhw.range)
		: (sd->aa.skill_range >= 0 ? sd->aa.skill_range : 1);

	// Verification du chemin et de la distance
	if (!path_search(nullptr, sd->bl.m, sd->bl.x, sd->bl.y, bl->x, bl->y, 0, CELL_CHKNOPASS))
		return false;

	// Verification du chemin et de la distance
	if (distance_bl(&sd->bl, bl) > AREA_SIZE)
		return false;

	// Verification de l'etat de la cible
	TBL_MOB* md = map_id2md(bl->id);
	if (!md || md->status.hp <= 0 || md->special_state.ai)
		return false;

	// Check for hidden or cloaked state
	if (md->sc.option & (OPTION_HIDE | OPTION_CLOAK))
		return false;
	
	if (!sd->aa.mobs.id.empty() &&
		std::find(sd->aa.mobs.id.begin(), sd->aa.mobs.id.end(), md->mob_id) != sd->aa.mobs.id.end()) {
			if (sd->aa.mobs.aggressive_behavior || (!sd->aa.mobs.aggressive_behavior && md->target_id != sd->bl.id)) // check if aggressive and target the bot
				return false;
	}

	// Verification du chemin et de la distance
	if (!battle_check_range(&sd->bl, bl, AREA_SIZE))
		return false;

	if (!check_distance_bl(&sd->bl, bl, range) &&
		!unit_can_reach_bl(&sd->bl, bl, range, 1, nullptr, nullptr) &&
		!unit_walktobl(&sd->bl, bl, range, 1))
		return false;

	// Default to valid target if all checks pass
	return true;
}

int buildin_autoattack_sub(struct block_list* bl, va_list ap) {
	// Retrieve arguments passed via the va_list
	int* target_id = va_arg(ap, int*);
	int src_id = va_arg(ap, int);

	if (*target_id > 0) {
		return 0;
	}

	if (!bl || bl->type != BL_MOB)
		return 0;

	// Retrieve sd
	map_session_data* sd = map_id2sd(src_id);

	// Validate source and target blocks
	if (!sd)
		return 0;

	// Verify target eligibility
	if (!aa_check_target(sd, bl->id)) {
		return 0;
	}

	*target_id = bl->id;
	return 1;
}

struct s_aa_attacker_candidate {
	int id = 0;
	int distance = INT_MAX;
};

static int aa_find_attacker_sub(struct block_list* bl, va_list ap) {
	s_aa_attacker_candidate* candidate = va_arg(ap, s_aa_attacker_candidate*);
	int src_id = va_arg(ap, int);
	map_session_data* sd = map_id2sd(src_id);
	TBL_MOB* md = BL_CAST(BL_MOB, bl);

	if (!candidate || !sd || !md || md->target_id != sd->bl.id || status_isdead(bl))
		return 0;

	if (md->sc.option & (OPTION_HIDE | OPTION_CLOAK))
		return 0;

	if (sd->aa.mobs.aggressive_behavior &&
		std::find(sd->aa.mobs.id.begin(), sd->aa.mobs.id.end(), md->mob_id) != sd->aa.mobs.id.end())
		return 0;

	int target_distance = distance_bl(&sd->bl, bl);
	if (target_distance < candidate->distance) {
		candidate->id = bl->id;
		candidate->distance = target_distance;
	}

	return 1;
}

static int aa_find_nearest_attacker(map_session_data* sd) {
	s_aa_attacker_candidate candidate;
	int radius = battle_config.feature_autoattack_mdetection;

	map_foreachinarea(
		aa_find_attacker_sub,
		sd->bl.m,
		sd->bl.x - radius,
		sd->bl.y - radius,
		sd->bl.x + radius,
		sd->bl.y + radius,
		BL_MOB,
		&candidate,
		sd->bl.id
	);

	return candidate.id;
}

int buildin_autoattack_monsters_sub(struct block_list* bl, va_list ap) {
	// Retrieve arguments passed via the va_list
	std::unordered_set<int>* counted_monsters = va_arg(ap, std::unordered_set<int>*);
	int src_id = va_arg(ap, int);

	if (!bl || bl->type != BL_MOB)
		return 0;

	// Retrieve sd
	map_session_data* sd = map_id2sd(src_id);
	TBL_MOB* md = map_id2md(bl->id);

	// Validate source and target blocks
	if (!sd || !md)
		return 0;

	//md->target_id=bl->id;
	if (sd->bl.id == md->target_id && counted_monsters->find(md->bl.id) == counted_monsters->end())
		counted_monsters->insert(md->bl.id);  // Ajouter l'ID du monstre dans le set

	return 1;
}

unsigned int aa_check_target_alive(map_session_data* sd) {
	// Validate the current target
	if (!aa_check_target(sd, sd->aa.target_id)) {
		if (sd->aa.mobs.map != sd->bl.m) {
			sd->aa.mobs.map = sd->bl.m;
			sd->aa.mobs.id.clear();
		}

		int target_id_ = 0;
		aa_target_change(sd, 0);

		// When surrounded, prefer a monster that is actively attacking the
		// player instead of oscillating between arbitrary nearby targets.
		target_id_ = aa_find_nearest_attacker(sd);
		if (target_id_ && aa_check_target(sd, target_id_))
			aa_target_change(sd, target_id_);

		// Search for a new target within detection radius
		for (int radius = 0; !sd->aa.target_id && radius < battle_config.feature_autoattack_mdetection; ++radius) {
			map_foreachinarea(
				buildin_autoattack_sub,
				sd->bl.m,
				sd->bl.x - radius,
				sd->bl.y - radius,
				sd->bl.x + radius,
				sd->bl.y + radius,
				BL_MOB,
				&target_id_,
				sd->bl.id
			);

			// If a target is found, set the target ID and break
			if (target_id_) {
				aa_target_change(sd, target_id_);
				break;
			}
		}
	}

	return sd->aa.target_id;
}

int aa_check_surround_monster(map_session_data* sd) {
    std::unordered_set<int> counted_monsters;  // Set pour stocker les ID des monstres deja comptes

    // Search for a new target within detection radius
    for (int radius = 0; radius < battle_config.feature_autoattack_mdetection; ++radius) {
        map_foreachinarea(
            buildin_autoattack_monsters_sub,
            sd->bl.m,
            sd->bl.x - radius,
            sd->bl.y - radius,
            sd->bl.x + radius,
            sd->bl.y + radius,
            BL_MOB,
            &counted_monsters,  // Passer le set en parametre
            sd->bl.id
        );
    }

    return static_cast<int>(counted_monsters.size());
}

bool aa_teleport(map_session_data* sd) {
	// Early exit if teleportation is disabled globally or via configuration
	if (!sd->state.autoattack || !battle_config.feature_autoattack_teleport)
		return false;

	map_data* mapdata = map_getmapdata(sd->bl.m);
	if (mapdata->getMapFlag(MF_NOTELEPORT))
		return false;

	bool flywing_used = false;

	// Check if teleport skill can be used
	if (!sd->aa.teleport.use_teleport && sd->status.sp > 20 && !mapdata->getMapFlag(MF_NOSKILL)) {
		if (pc_checkskill(sd, AL_TELEPORT) > 0) {
			skill_consume_requirement(sd, AL_TELEPORT, 1, 2);
			pc_randomwarp(sd, CLR_TELEPORT);
			status_heal(&sd->bl, 0, -skill_get_sp(AL_TELEPORT, 1), 1);
			flywing_used = true;
		}
	}

	// Check for Fly Wing usage if teleport was not used
	if (!flywing_used && !sd->aa.teleport.use_flywing) {
		static const int flywing_item_ids[] = { 601 }; // Prioritized Fly Wing item IDs
		int inventory_index = -1;
		bool requires_consumption = false;

		for (int item_id : flywing_item_ids) {
			inventory_index = pc_search_inventory(sd, item_id);
			if (inventory_index >= 0) {
				if (item_id == 601) {
					requires_consumption = true; // Fly Wing (601) requires consumption
				}
				break;
			}
		}

		if (inventory_index >= 0) {
			// The AI performs the warp itself. Keep a rented Fly Wing until its
			// rental timer expires; consume only a normal Fly Wing.
			const bool is_rental_flywing = sd->inventory.u.items_inventory[inventory_index].expire_time > 0;
			if (requires_consumption && !is_rental_flywing) {
				pc_delitem(sd, inventory_index, 1, 0, 0, LOG_TYPE_OTHER);
			}
			pc_randomwarp(sd, CLR_TELEPORT);
			flywing_used = true;
		}
	}

	// Finalize teleportation actions
	if (flywing_used) {
		// Reset value
		sd->aa.target_id = 0;
		aa_target_change(sd, 0);
		sd->aa.last_teleport = gettick();
		sd->aa.last_attack = gettick();
		sd->aa.last_move = gettick();
		sd->aa.last_hit = gettick();
		sd->aa.lastposition.x = sd->bl.x;
		sd->aa.lastposition.y = sd->bl.y;

		// Action after tp
		aa_status_checkmapchange(sd);
		pc_delinvincibletimer(sd);
		clif_parse_LoadEndAck(0, sd);
	}

	return flywing_used;
}


void aa_refill_arrows(map_session_data* sd, int current_ammo_id) {
	if (!sd)
		return;
	
	if (!sd->state.autoattack)
		return;
	
	int16 ammo_idx = sd->equip_index[EQI_AMMO];
	if (ammo_idx < 0)
		return;
	
	int ammo_amount = sd->inventory.u.items_inventory[ammo_idx].amount;
	
	if (ammo_amount >= 10)
		return;
	
	struct ammo_container_pair {
		unsigned short ammo_id;      // ID CÃ??Ã?Â¹/Ã…?xÃ??
		unsigned short container_id; // ID Ã??Ã??
		int amount;                  // Ã?â€?Ã??Ã?Â·Ã•Â¨Ã¤Â´Â©
	};
	
	ammo_container_pair pairs[] = {
		// ===== ARROWS (Ã…?xÃ??) =====
		{1750, 12004, 500},  // Arrow
		{1751, 12009, 500},  // Silver Arrow
		{1752, 12008, 500},  // Fire Arrow
		{1753, 12006, 500},  // Steel Arrow
		{1754, 12012, 500},  // Crystal Arrow
		{1755, 12010, 500},  // Arrow of Wind
		{1756, 12011, 500},  // Stone Arrow
		{1757, 12014, 500},  // Immaterial Arrow
		{1762, 12015, 500},  // Rusty Arrow
		{1765, 12007, 500},  // Arrow of Shadow
		// 1766 Ã¤?Â¨U Quiver
		{1767, 12013, 500},  // Sleep Arrow
		{1770, 12005, 500},  // Sharp Arrow
		{1772, 12183, 500},  // Holy Arrow
		{1773, 12575, 500},  // Hunting Arrow
		{1774, 12576, 500},  // Siege Arrow
		
		// ===== BULLETS (CÃ??Ã?Â¹?â€”Ã?Â© =====
		{13200, 12149, 500}, // Bullet
		{13201, 12151, 500}, // Silver Bullet
		{13215, 22744, 500}, // Bullet (Fire)
		{13216, 22745, 500}, // Bullet (Ice)
		{13217, 22746, 500}, // Bullet (Wind)
		{13218, 22747, 500}, // Bullet (Earth)
		{13219, 22748, 500}, // Bullet (Dark)
		{13220, 22749, 500}, // Bullet (Holy)
		{13221, 22738, 500}, // Bullet (Poison)
		{13228, 23123, 500}, // Bullet Type 1
		{13229, 23124, 500}, // Bullet Type 2
		{13230, 23125, 500}, // Bullet Type 3
		{13231, 23126, 500}, // Bullet Type 4
		{13232, 23127, 500}  // Bullet Type 5
	};
	
	for (const auto& pair : pairs) {
		if (pair.ammo_id == current_ammo_id) {
			int16 container_idx = pc_search_inventory(sd, pair.container_id);
			
			if (container_idx < 0) {
				if (ammo_amount == 0) {
					char msg[100];
					sprintf(msg, "Out of ammo! No Container (ID:%d) available!", pair.container_id);
					clif_displaymessage(sd->fd, msg);
				}
				return;
			}
			
			if (pc_delitem(sd, container_idx, 1, 0, 0, LOG_TYPE_CONSUME) != 0)
				return;
			
			struct item item_tmp = {};
			item_tmp.nameid = pair.ammo_id;
			item_tmp.identify = 1;
			
			if (pc_additem(sd, &item_tmp, pair.amount, LOG_TYPE_CONSUME) == 0) {
				char msg[100];
				sprintf(msg, "Opened Container! Received %d ammo (ID:%d)", pair.amount, pair.ammo_id);
				clif_displaymessage(sd->fd, msg);
			}
			return;
		}
	}
	
	if (ammo_amount == 0) {
		clif_displaymessage(sd->fd, "This ammo type has no container!");
	}
}

int aa_ammochange(map_session_data* sd, struct mob_data* md,
	const unsigned short* ammoIds,     // Liste des ID des munitions
	const unsigned short* ammoElements, // elements associes
	const unsigned short* ammoAtk,     // Puissances d'attaque des munitions
	size_t ammoCount,                  // Nombre de types de munitions
	int rqAmount = 0,                  // Quantite requise (0 si non applicable)
	const unsigned short* ammoLevels = nullptr // Niveaux minimum requis (facultatif)
) {
	if (DIFF_TICK(sd->canequip_tick, gettick()) > 0)
		return 0; // Cooldown

	int bestIndex = -1;
	int bestPriority = -1;
	int bestElement = -1;
	bool isEquipped = false;

	for (size_t i = 0; i < ammoCount; ++i) {
		int16 index = pc_search_inventory(sd, ammoIds[i]);
		if (index < 0) continue; // Munition non trouvee

		// Check qty (only for kunai atm)
		if (rqAmount > 0 && sd->inventory.u.items_inventory[index].amount < rqAmount)
			continue;

		// Check required level
		if (ammoLevels && sd->status.base_level < ammoLevels[i])
			continue;

		int priority = ammoAtk[i];
		if (aa_elemstrong(md, ammoElements[i]))
			priority += 500; // Bonus for the strong elem

		if (aa_elemallowed(md, ammoElements[i]) && priority > bestPriority) {
			bestPriority = priority;
			bestIndex = index;
			isEquipped = pc_checkequip2(sd, ammoIds[i], EQI_AMMO, EQI_AMMO + 1);
			bestElement = ammoElements[i];
		}
	}

	if (bestIndex > -1) {
		if (!isEquipped)
			pc_equipitem(sd, bestIndex, EQP_AMMO);
		return bestElement; // return the best elem
	}

	clif_displaymessage(sd->fd, "No suitable ammunition left!");
	return -1;
}

void aa_arrowchange(map_session_data* sd, struct mob_data* md) {
	constexpr unsigned short arrows[] = {
		1750, 1751, 1752, 1753, 1754, 1755, 1756, 1757, 1762, 1765, 1766, 1767, 1770, 1772, 1773, 1774
	};
	constexpr unsigned short arrowElements[] = {
		ELE_NEUTRAL, ELE_HOLY, ELE_FIRE, ELE_NEUTRAL, ELE_WATER, ELE_WIND, ELE_EARTH, ELE_GHOST,
		ELE_NEUTRAL, ELE_POISON, ELE_HOLY, ELE_DARK, ELE_NEUTRAL, ELE_HOLY, ELE_NEUTRAL, ELE_NEUTRAL
	};
	constexpr unsigned short arrowAtk[] = {
		25, 30, 30, 40, 30, 30, 30, 30, 30, 50, 50, 30, 30, 50, 45, 35
	};

	aa_ammochange(sd, md, arrows, arrowElements, arrowAtk, std::size(arrows));
}

void aa_bulletchange(map_session_data* sd, mob_data* md) {
	constexpr unsigned short bullets[] = {
		13200, 13201, 13215, 13216, 13217, 13218, 13219, 13220, 13221, 13228, 13229, 13230, 13231, 13232
	};
	constexpr unsigned short bulletElements[] = {
		ELE_NEUTRAL, ELE_HOLY, ELE_NEUTRAL, ELE_FIRE, ELE_WATER, ELE_WIND, ELE_EARTH, ELE_HOLY,
		ELE_HOLY, ELE_FIRE, ELE_WIND, ELE_WATER, ELE_POISON, ELE_DARK
	};
	constexpr unsigned short bulletAtk[] = {
		25, 15, 50, 40, 40, 40, 40, 40, 15, 20, 20, 20, 20, 20
	};
	constexpr unsigned short bulletLevels[] = {
		1, 1, 100, 100, 100, 100, 100, 100, 1, 1, 1, 1, 1, 1
	};

	aa_ammochange(sd, md, bullets, bulletElements, bulletAtk, std::size(bullets), 0, bulletLevels);
}

void aa_kunaichange(map_session_data* sd, struct mob_data* md, int rqamount) {
	constexpr unsigned short kunaiIds[] = {
		13255, 13256, 13257, 13258, 13259, 13294
	};
	constexpr unsigned short kunaiElements[] = {
		ELE_WATER, ELE_EARTH, ELE_WIND, ELE_FIRE, ELE_POISON, ELE_NEUTRAL
	};
	constexpr unsigned short kunaiAtk[] = {
		30, 30, 30, 30, 30, 50
	};

	aa_ammochange(sd, md, kunaiIds, kunaiElements, kunaiAtk, std::size(kunaiIds), rqamount);
}

void aa_cannonballchange(map_session_data* sd, struct mob_data* md) {
	constexpr unsigned short cannonballIds[] = {
		18000, 18001, 18002, 18003, 18004
	};
	constexpr unsigned short cannonballElements[] = {
		ELE_NEUTRAL, ELE_HOLY, ELE_DARK, ELE_GHOST, ELE_NEUTRAL
	};
	constexpr unsigned short cannonballAtk[] = {
		100, 120, 120, 120, 250
	};

	aa_ammochange(sd, md, cannonballIds, cannonballElements, cannonballAtk, std::size(cannonballIds));
}

// Determines if an element is strong against the target mob's defense element
bool aa_elemstrong(const mob_data* md, int ele) {
	if (!md)
		return false;

	const int def_ele = md->status.def_ele;
	const int ele_lv = md->status.ele_lv;

	// Define rules for each element
	switch (ele) {
	case ELE_GHOST:
		return (def_ele == ELE_UNDEAD && ele_lv >= 2) || (def_ele == ELE_GHOST);

	case ELE_FIRE:
		return def_ele == ELE_UNDEAD || def_ele == ELE_EARTH;

	case ELE_WATER:
		return (def_ele == ELE_UNDEAD && ele_lv >= 3) || (def_ele == ELE_FIRE);

	case ELE_WIND:
		return def_ele == ELE_WATER;

	case ELE_EARTH:
		return def_ele == ELE_WIND;

	case ELE_HOLY:
		return (def_ele == ELE_POISON && ele_lv >= 3) ||
			(def_ele == ELE_DARK) ||
			(def_ele == ELE_UNDEAD);

	case ELE_DARK:
		return def_ele == ELE_HOLY;

	case ELE_POISON:
		return (def_ele == ELE_UNDEAD && ele_lv >= 2) ||
			(def_ele == ELE_GHOST) ||
			(def_ele == ELE_NEUTRAL);

	case ELE_UNDEAD:
		return (def_ele == ELE_HOLY && ele_lv >= 2);

	case ELE_NEUTRAL:
		return false;

	default:
		return false;
	}
}



// Determines if an element is allowed against the target mob's defense element
bool aa_elemallowed(struct mob_data* md, int ele) {
	if (!md)
		return true; // Default to allowed if the mob data is invalid

	const int def_ele = md->status.def_ele;
	const int ele_lv = md->status.ele_lv;

	// Check for White Imprison status, applicable to most elements
	if (md->sc.getSCE(SC_WHITEIMPRISON)) {
		if (ele != ELE_GHOST) // Exception for Ghost element
			return false;
	}

	switch (ele) {
	case ELE_GHOST:
		return !((def_ele == ELE_NEUTRAL && ele_lv >= 2) ||
			(def_ele == ELE_FIRE && ele_lv >= 3) ||
			(def_ele == ELE_WATER && ele_lv >= 3) ||
			(def_ele == ELE_WIND && ele_lv >= 3) ||
			(def_ele == ELE_EARTH && ele_lv >= 3) ||
			(def_ele == ELE_POISON && ele_lv >= 3) ||
			(def_ele == ELE_HOLY && ele_lv >= 2) ||
			(def_ele == ELE_DARK && ele_lv >= 2));

	case ELE_FIRE:
	case ELE_WATER:
	case ELE_WIND:
	case ELE_EARTH:
		if (def_ele == ele || // Same element
			(def_ele == ELE_HOLY && ele_lv >= 2) ||
			(def_ele == ELE_DARK && ele_lv >= 3))
			return false;

		if (ele == ELE_EARTH && def_ele == ELE_UNDEAD && ele_lv >= 4)
			return false;

		return true;

	case ELE_HOLY:
		return def_ele != ELE_HOLY;

	case ELE_DARK:
		return !(def_ele == ELE_POISON ||
			def_ele == ELE_DARK ||
			def_ele == ELE_UNDEAD);

	case ELE_POISON:
		return !((def_ele == ELE_WATER && ele_lv >= 3) ||
			(def_ele == ELE_GHOST && ele_lv >= 3) ||
			(def_ele == ELE_POISON) ||
			(def_ele == ELE_UNDEAD) ||
			(def_ele == ELE_HOLY && ele_lv >= 2) ||
			(def_ele == ELE_DARK));

	case ELE_UNDEAD:
		return !((def_ele == ELE_WATER && ele_lv >= 3) ||
			(def_ele == ELE_FIRE && ele_lv >= 3) ||
			(def_ele == ELE_WIND && ele_lv >= 3) ||
			(def_ele == ELE_EARTH && ele_lv >= 3) ||
			(def_ele == ELE_POISON && ele_lv >= 1) ||
			(def_ele == ELE_UNDEAD) ||
			(def_ele == ELE_DARK));

	case ELE_NEUTRAL:
		return !(def_ele == ELE_GHOST && ele_lv >= 2);

	default:
		return true; // Default to allowed for unsupported elements
	}
}

//Monk Combo
void check_and_cast_chain_combo(struct map_session_data *sd) {
    const int TRIPLE_ATTACK_SKILL_ID = 263; // Skill ID for Triple Attack
    const int CHAIN_COMBO_SKILL_ID = 272;   // Skill ID for Chain Combo
	int chaincombo_lv = pc_checkskill(sd, 272);
	
    if (sd->sc.getSCE(SC_COMBO) && sd->sc.getSCE(SC_COMBO)->val1 == TRIPLE_ATTACK_SKILL_ID) {
        bool used_triple_attack = true;

        int chain_combo_skill_lv = pc_checkskill(sd, CHAIN_COMBO_SKILL_ID);

        if (used_triple_attack && chain_combo_skill_lv > 0) {
            int status_change = skill_get_sc(CHAIN_COMBO_SKILL_ID);
            if (!sd->sc.getSCE(status_change)) {
                unit_skilluse_id(&sd->bl, sd->bl.id, CHAIN_COMBO_SKILL_ID, chain_combo_skill_lv);
				
				switch(chaincombo_lv){
#ifndef RENEWAL				
				case 1: status_heal(&sd->bl, 0, -11, 0); break;
				case 2: status_heal(&sd->bl, 0, -12, 0); break;
				case 3: status_heal(&sd->bl, 0, -13, 0); break;
				case 4: status_heal(&sd->bl, 0, -14, 0); break;
				case 5: status_heal(&sd->bl, 0, -15, 0); break;
#else	
				case 1: status_heal(&sd->bl, 0, -5, 0); break;
				case 2: status_heal(&sd->bl, 0, -6, 0); break;
				case 3: status_heal(&sd->bl, 0, -7, 0); break;
				case 4: status_heal(&sd->bl, 0, -8, 0); break;
				case 5: status_heal(&sd->bl, 0, -9, 0); break;
#endif				
				}
            }
        }
    }
}


void aa_buyitem(struct map_session_data *sd) {
	// Auto Buy Items
	if (sd->aa.autobuyitems.size()) {
		
		// Get MC_DISCOUNT skill level
		int discount_lv = pc_checkskill(sd, MC_DISCOUNT);
		
		for (const auto& entry : sd->aa.autobuyitems) {
			t_itemid itemid = entry.item_id;
			int buy_quantity = entry.quantity;
			int min_trigger = entry.min_trigger;
			char flag = 0;
	
			auto item_data_ptr = item_db.find(itemid);
			if (!item_data_ptr)
				continue;
	
			item_data* item_data = item_data_ptr.get();
	
			int item_index = pc_search_inventory(sd, itemid);
			int current_amount = (item_index >= 0) ? sd->inventory.u.items_inventory[item_index].amount : 0;
	
			if (current_amount < min_trigger) {
				int price = item_data->value_buy;
				if (price <= 0)
					continue;
				
				// Apply MC_DISCOUNT bonus
				if (discount_lv > 0) {
					int discount_percent = 0;
					
					// MC_DISCOUNT discount rates
					switch (discount_lv) {
						case 1: discount_percent = 7; break;
						case 2: discount_percent = 9; break;
						case 3: discount_percent = 11; break;
						case 4: discount_percent = 13; break;
						case 5: discount_percent = 15; break;
						case 6: discount_percent = 17; break;
						case 7: discount_percent = 19; break;
						case 8: discount_percent = 21; break;
						case 9: discount_percent = 23; break;
						case 10: discount_percent = 24; break;
						default: discount_percent = 0; break;
					}
					
					// Ã…Â´Ã?â€?Ã?â€? (Ã…Âº %)
					price = price - (price * discount_percent / 100);
				}
				
				int total_weight = item_data->weight * buy_quantity;
				if ((sd->weight + total_weight) > (sd->max_weight * 80 / 100))
					continue;
	
				int total_cost = price * buy_quantity;
				if (sd->status.zeny < total_cost)
					continue;
	
				pc_payzeny(sd, total_cost, LOG_TYPE_CONSUME);
	
				struct item item_tmp;
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = itemid;
				item_tmp.identify = 1;
				
				if ((flag = pc_additem(sd, &item_tmp, buy_quantity, LOG_TYPE_SCRIPT)))
					clif_additem(sd, 0, 0, flag);
			}
		}
	}
}	


void aa_sellitem(struct map_session_data *sd) {
	// Auto-sell items when item count reaches threshold
	
	if (battle_config.feature_autoattack_sell_count > 0 && !sd->aa.autosellitems.empty()) {
		
		// Get MC_OVERCHARGE skill level
		int overcharge_lv = pc_checkskill(sd, MC_OVERCHARGE);
		
		for (const auto& entry : sd->aa.autosellitems) {
			t_itemid itemid = entry.item_id;
	
			auto item_data_ptr = item_db.find(itemid);
			if (!item_data_ptr)
				continue;
	
			item_data* id = item_data_ptr.get();
			int item_index = pc_search_inventory(sd, itemid);
			if (item_index < 0)
				continue;
	
			int current_amount = sd->inventory.u.items_inventory[item_index].amount;
			
			if (current_amount < battle_config.feature_autoattack_sell_count)
				continue;
	
			int price = id->value_sell;
			
			// Apply MC_OVERCHARGE bonus
			if (overcharge_lv > 0) {
				int bonus_percent = 0;
				
				// MC_OVERCHARGE bonus rates
				switch (overcharge_lv) {
					case 1: bonus_percent = 7; break;
					case 2: bonus_percent = 9; break;
					case 3: bonus_percent = 11; break;
					case 4: bonus_percent = 13; break;
					case 5: bonus_percent = 15; break;
					case 6: bonus_percent = 17; break;
					case 7: bonus_percent = 19; break;
					case 8: bonus_percent = 21; break;
					case 9: bonus_percent = 23; break;
					case 10: bonus_percent = 24; break;
					default: bonus_percent = 0; break;
				}
				
				price = price + (price * bonus_percent / 100);
			}
						
			// Cap price based on item type and battle_config
			if (id->type == IT_HEALING && battle_config.max_sale_healing && price > battle_config.max_sale_healing)
				price = battle_config.max_sale_healing;
			else if (id->type == IT_USABLE && battle_config.max_sale_usable && price > battle_config.max_sale_usable)
				price = battle_config.max_sale_usable;
			else if (id->type == IT_WEAPON && battle_config.max_sale_weapon && price > battle_config.max_sale_weapon)
				price = battle_config.max_sale_weapon;
			else if (id->type == IT_ARMOR && battle_config.max_sale_armor && price > battle_config.max_sale_armor)
				price = battle_config.max_sale_armor;
			else if (battle_config.max_sale_etc && price > battle_config.max_sale_etc)
				price = battle_config.max_sale_etc;
			
			int total_price = price * current_amount;
	
			pc_delitem(sd, item_index, current_amount, 0, 0, LOG_TYPE_SCRIPT);
			pc_getzeny(sd, total_price, LOG_TYPE_SCRIPT);
			clif_updatestatus(sd, SP_ZENY);
		}
	}
}

// 0 nothing - 1 pick up - 2 heal
int aa_status(map_session_data* sd) {
	if (!sd) return -1;

	if(sd->state.storage_flag){
		std::string msg = "Automessage - Storage open, close it first!";
		aa_message(sd, "Storage", msg.data(), 300, nullptr);
		status_change_end(&sd->bl, SC_AUTOATTACK);
		return 0;
	}

	struct party_data* p = (sd->status.party_id) ? party_search(sd->status.party_id) : nullptr;

	if (pc_is90overweight(sd)) {
		std::string msg = "Automessage - I'm overweight - System Off!";
		aa_message(sd, "Overweight", msg.data(), 300, p);
		status_change_end(&sd->bl, SC_AUTOATTACK);
		return 0;
	}

	struct status_data* status = status_get_status_data(&sd->bl);
	t_tick last_tick = gettick();

	//if surrounded by too much monsters
	if (sd->aa.monster_surround && aa_check_surround_monster(sd) > sd->aa.monster_surround) {
		if (aa_teleport(sd))
			return 2;
	}

	// Brain, what the bot need to do during the loop
	// Priority 1 - rest (sit / stand)
	aa_status_rest(sd, status, last_tick);
	if (pc_issit(sd)) return 1;
	
	//Auto But Item
	aa_buyitem(sd);
	
	//Auto Sell Item
	aa_sellitem(sd);

	// Priority 2 - Buff item
	if (aa_status_buffitem(sd, last_tick))
		return 2;

	// Priority 3 - potion. Item use is independent from combat, so a
	// successful potion must not consume the whole AI tick.
	aa_status_potion(sd, status);

	// Priority 4 - heal
	if (aa_status_heal(sd, status, last_tick)) return 3;

	// Priority 5 - Buffs
	if (aa_status_buffs(sd, last_tick)) return 5;

	// Intermediate priority
	aa_status_checkteleport_delay (sd, last_tick);
	aa_status_check_reset(sd, last_tick);

	// Check targets
	if (sd->aa.target_id && !aa_check_target(sd, sd->aa.target_id))
		aa_target_change(sd, 0);

	if (sd->aa.itempick_id) // Already an item id to pick up
		aa_check_item_pickup(sd, map_id2bl(sd->aa.itempick_id)); // Check the validity of it

	// Priority 6 - Pick up
	if (battle_config.feature_autoattack_pickup && aa_status_pickup(sd, last_tick))
		return 6;

	if(!sd->aa.target_id) // no item to pick up so lf for a target to attack
		aa_check_target_alive(sd);

	//Monk Combo
	int lv_chain_combo = pc_checkskill(sd, 272);
	if (lv_chain_combo > 0 && sd->aa.monk_combo) {
		check_and_cast_chain_combo(sd);
	}
			
	// Priority 7 - Attack skill and melee
	if (sd->aa.target_id) {
		struct mob_data* md_target = (struct mob_data* )map_id2bl(sd->aa.target_id);

		if (aa_status_attack(sd, md_target, last_tick)) {
			sd->aa.last_attack = last_tick;
			return 7;
		}
		else if (aa_status_melee(sd, md_target, last_tick, status)) {
			return 8;
		}
		return 9;
	}

	// Priority 8 - Move
	if (battle_config.feature_autoattack_movetype == 2)
		aa_move_path(sd);
	else
		aa_move_short(sd, last_tick);

	aa_status_checkmapchange(sd);

	return 10;
}

bool aa_status_checkteleport_delay(map_session_data* sd, t_tick last_tick) {
	t_tick attack_ = DIFF_TICK(last_tick, sd->aa.last_attack);
	t_tick pick_ = DIFF_TICK(last_tick, sd->aa.last_pick);

	if (!sd->aa.teleport.delay_nomobmeet)
		return false;

	if (sd->aa.target_id || sd->aa.itempick_id)
		return false;

	if (sd->aa.teleport.use_teleport && sd->aa.teleport.use_flywing)
		return false;

	if (pick_ < 2000 || attack_ < 2000)
		return false;

	if (attack_ > sd->aa.teleport.delay_nomobmeet) {
		struct unit_data* ud;

		if ((ud = unit_bl2ud(&sd->bl)) == nullptr)
			return false;

		if (ud->skilltimer != INVALID_TIMER)
			return false; // Can't teleport while casting

		return aa_teleport(sd);
	}

	return false;
}

// Check if reset of item or target is need
bool aa_status_check_reset(map_session_data* sd, t_tick last_tick) {
	if (unit_is_walking(&sd->bl))
		return false;

	if (sd->aa.target_id) {
		t_tick attack_ = DIFF_TICK(last_tick, sd->aa.last_attack);
		if (attack_ > 7500) {
			sd->aa.target_id = 0;
			aa_move_short(sd, last_tick); // Force move
			sd->aa.last_attack = last_tick;
			return true;
		}
	}
	else if (sd->aa.itempick_id) {
		t_tick pick_ = DIFF_TICK(last_tick, sd->aa.last_pick);
		if (pick_ > 5000) {
			sd->aa.itempick_id = 0; // If not walking
			aa_move_short(sd, last_tick); // Force move
			sd->aa.last_pick = last_tick;
			return true;
		}
	}

	return false;
}

bool aa_status_pickup(map_session_data* sd, t_tick last_tick) {
	if (sd->aa.pickup_item_config == 2) return false;

	bool should_look_for_item = false;

	if (sd->aa.prio_item_config) { // Loot priority
		should_look_for_item = true;
	} else { // Fight priority
		if (sd->aa.target_id) {
			sd->aa.itempick_id = 0;
			return false;
		}
		aa_check_target_alive(sd);
		if (!sd->aa.target_id) {
			should_look_for_item = true;
		}
	}

	if (should_look_for_item) {
		if (!sd->aa.itempick_id) {
			aa_check_item_pickup_onfloor(sd);
			if (sd->aa.itempick_id) {
				if (sd->aa.prio_item_config) aa_target_change(sd, 0);
				sd->aa.last_pick = last_tick;
			}
		} else if (sd->aa.prio_item_config) {
			aa_target_change(sd, 0);
		}
	}

	if (!sd->aa.itempick_id) return false;

	struct block_list* fitem_bl = map_id2bl(sd->aa.itempick_id);
	if (!fitem_bl) {
		sd->aa.itempick_id = 0;
		return false;
	}

	struct flooritem_data* fitem = (struct flooritem_data*)fitem_bl;
	if (check_distance_bl(&sd->bl, fitem_bl, 2)) {
		if (DIFF_TICK(last_tick, sd->aa.last_pick) < battle_config.feature_autoattack_pickup_delay) return true;

		if (pc_takeitem(sd, fitem)) {
			sd->aa.itempick_id = 0;
			sd->aa.last_pick = last_tick;
		}
		return true;
	} else {
		if (unit_walktobl(&sd->bl, fitem_bl, 1, 1)) return true;
	}

	return false;
}

//Auto-heal skill
bool aa_status_heal(map_session_data* sd, const status_data* status, t_tick last_tick) {
	// Check if auto-healing is enabled and the list of auto-healing skills is not empty
	if (!battle_config.feature_autoattack_autoheal || sd->aa.autoheal.empty()) {
		return false;
	}

	// Check if the global skill cooldown allows skill usage
	if (last_tick < sd->aa.skill_cd) {
		return false;
	}

	// Iterate through the list of auto-healing skills
	for (const auto& autoheal : sd->aa.autoheal) {
		// Ensure the skill can be used, and the current HP meets the trigger condition
		int hp_percentage = (status->hp * 100) / sd->status.max_hp;
		if (!aa_canuseskill(sd, autoheal.skill_id, autoheal.skill_lv) || hp_percentage >= autoheal.min_hp) {
			continue;
		}

		// Check if the skill's individual cooldown has expired
		if (last_tick < autoheal.last_use) {
			continue;
		}

		// Attempt to use the healing skill
		if (unit_skilluse_id(&sd->bl, sd->bl.id, autoheal.skill_id, autoheal.skill_lv)) {
			// unit_skilluse_id() consumes the skill requirements. Applying them
			// here as well would consume catalyst items twice.

			// Apply global cooldown if applicable
			if (battle_config.feature_autoattack_bskill_delay > 0) {
				t_tick skill_delay = skill_get_delay(autoheal.skill_id, autoheal.skill_lv);
				if (skill_delay < battle_config.feature_autoattack_bskill_delay) {
					t_tick new_cd = last_tick + battle_config.feature_autoattack_bskill_delay + skill_get_cast(autoheal.skill_id, autoheal.skill_lv);
					if (sd->aa.skill_cd < new_cd) {
						sd->aa.skill_cd = new_cd;
					}
				}
			}

			return true; // Healing skill successfully used
		}
	}

	return false; // No healing skill was used
}

//Healing potions
bool aa_status_potion(map_session_data* sd, const status_data* status) {
	// Check if the auto-potion feature is enabled
	if (!battle_config.feature_autoattack_autopotion || sd->aa.autopotion.empty() || sd->st != nullptr || sd->npc_id != 0)
		return false;

	// Iterate through the auto-potion configuration
	for (const auto& potion : sd->aa.autopotion) {
		// These Death Match potions must never be used by Auto Attack.
		if (potion.item_id == 11548 || potion.item_id == 11549)
			continue;

		const status_data* current_status = status_get_status_data(&sd->bl);
		const bool needs_hp = potion.min_hp > 0 &&
			get_percentage(current_status->hp, current_status->max_hp) < potion.min_hp;
		const bool needs_sp = potion.min_sp > 0 &&
			get_percentage(current_status->sp, current_status->max_sp) < potion.min_sp;

		if (!needs_hp && !needs_sp)
			continue;

		const int index = pc_search_inventory(sd, potion.item_id);
		if (index >= 0 && pc_useitem(sd, index))
			return true;
	}

	return false;
}

// Automatically sit to rest or stand when conditions are met
bool aa_status_rest(map_session_data* sd, const status_data* status, t_tick last_tick) {
    if (!battle_config.feature_autoattack_sittorest || !sd->aa.autositregen.is_active)
        return false; // Return early if the feature or regen is inactive

    // Calculate the time since the last hit
    t_tick time_since_last_hit = DIFF_TICK(last_tick, sd->aa.last_hit);

    // Check overweight status based on game mode
    bool is_overweight = 
#ifdef RENEWAL
        pc_is70overweight(sd);
#else
        pc_is50overweight(sd);
#endif

    // Calculate HP and SP percentages
    int hp_percentage = (status->hp * 100) / status->max_hp;
    int sp_percentage = (status->sp * 100) / status->max_sp;

    // Determine sit conditions
    bool needs_hp_regen = (sd->aa.autositregen.min_hp > 0 && hp_percentage < sd->aa.autositregen.min_hp);
    bool needs_sp_regen = (sd->aa.autositregen.min_sp > 0 && sp_percentage < sd->aa.autositregen.min_sp);
    bool can_sit = !pc_issit(sd) && (needs_hp_regen || needs_sp_regen) && time_since_last_hit >= 5000 && !is_overweight;

    // Determine stand conditions
    bool regen_complete = 
        (sd->aa.autositregen.min_hp > 0 && hp_percentage >= sd->aa.autositregen.max_hp) &&
        (sd->aa.autositregen.min_sp > 0 && sp_percentage >= sd->aa.autositregen.max_sp);
    bool can_stand = pc_issit(sd) && (regen_complete || time_since_last_hit < 5000 || is_overweight);

    // Execute actions based on conditions
    if (can_sit) {
        pc_setsit(sd);
        skill_sit(sd, 1);
        clif_sitting(&sd->bl);
    } else if (can_stand && pc_setstand(sd, false)) {
        skill_sit(sd, 0);
        clif_standing(&sd->bl);
    }

    return true; // Indicate that the function was processed
}

// Automatically use buff skills
bool aa_status_buffs(map_session_data* sd, t_tick last_tick) {

	if (!battle_config.feature_autoattack_buffskill || sd->aa.autobuffskills.empty())
		return false; // Return early if the feature is disabled or no buffs are configured

	if (last_tick < sd->aa.skill_cd) 
		return false; // Return if skill cooldown is active

	for (auto& autobuff : sd->aa.autobuffskills) {
		if (last_tick < autobuff.last_use || !autobuff.is_active) {
			continue; // Skip inactive buffs or buffs on cooldown
		}

		// Check if the skill can be used and the player is not already under its effect
		if (aa_canuseskill(sd, autobuff.skill_id, autobuff.skill_lv) &&
			!sd->sc.getSCE(skill_get_sc(autobuff.skill_id))) {

			// Handle specific cases for special skills
			if ((autobuff.skill_id == MO_CALLSPIRITS || autobuff.skill_id == CH_SOULCOLLECT) && sd->spiritball == 5)
				continue; // Skip if spirit balls are already maxed

			if (autobuff.skill_id == SA_AUTOSPELL) {
				handle_autospell(sd, autobuff.skill_lv);
				continue; // Skip further processing as autospell handling is done
			}

			// Use the skill
			if (unit_skilluse_id(&sd->bl, sd->bl.id, autobuff.skill_id, autobuff.skill_lv)) {
				// The normal skill pipeline consumes SP and catalyst items once.
				// Handle skill cooldown adjustment
				if (battle_config.feature_autoattack_bskill_delay &&
					skill_get_delay(autobuff.skill_id, autobuff.skill_lv) < battle_config.feature_autoattack_bskill_delay) {
					sd->aa.skill_cd = last_tick + battle_config.feature_autoattack_bskill_delay +
						skill_get_cast(autobuff.skill_id, autobuff.skill_lv);
				}

				return true; // Return true once a skill is used
			}
		}
	}

	return false; // Return false if no skills were used
}

void aa_token_respawn(map_session_data* sd, int flag) {
	if (flag && sd && sd->state.autoattack) {
		//token of siefried
		if (sd->aa.token_siegfried && pc_revive_item(sd))
			return;

		// return to the save point
		if (sd->aa.return_to_savepoint) {
			struct party_data* p = (sd->status.party_id) ? party_search(sd->status.party_id) : nullptr;

			pc_respawn(sd, CLR_OUTSIGHT);
			std::string msg = "Automessage - I'm dead, returning to save point - System Off!";
			aa_message(sd, "Dead", msg.data(), 300, p);
			status_change_end(&sd->bl, SC_AUTOATTACK);
		}
	}
}

// Helper function to handle SA_AUTOSPELL logic
void handle_autospell(map_session_data* sd, int skill_lv) {
	short random_skill;

	//Already in use with right lv
	if (sd->sc.getSCE(SC_AUTOSPELL) && sd->sc.getSCE(SC_AUTOSPELL)->val1 == skill_lv)
		return;

	sd->menuskill_val = pc_checkskill(sd, SA_AUTOSPELL);

#ifdef RENEWAL
	switch (skill_lv) {
	case 1:
	case 2:
	case 3:
		random_skill = rand() % 3;
		skill_autospell(sd, random_skill == 0 ? MG_FIREBOLT : (random_skill == 1 ? MG_COLDBOLT : MG_LIGHTNINGBOLT));
		break;
	case 4:
	case 5:
	case 6:
		random_skill = rand() % 2;
		skill_autospell(sd, random_skill == 0 ? MG_FIREBALL : MG_SOULSTRIKE);
		break;
	case 7:
	case 8:
	case 9:
		random_skill = rand() % 2;
		skill_autospell(sd, random_skill == 0 ? MG_FROSTDIVER : WZ_EARTHSPIKE);
		break;
	case 10:
		random_skill = rand() % 2;
		skill_autospell(sd, random_skill == 0 ? MG_THUNDERSTORM : WZ_HEAVENDRIVE);
		break;
	}
#else
	switch (skill_lv) {
	case 1:
		skill_autospell(sd, MG_NAPALMBEAT);
		break;
	case 2:
	case 3:
	case 4:
		random_skill = rand() % 3;
		skill_autospell(sd, random_skill == 0 ? MG_FIREBOLT : (random_skill == 1 ? MG_COLDBOLT : MG_LIGHTNINGBOLT));
		break;
	case 5:
	case 6:
	case 7:
		skill_autospell(sd, MG_SOULSTRIKE);
		break;
	case 8:
	case 9:
		skill_autospell(sd, MG_FIREBALL);
		break;
	case 10:
		skill_autospell(sd, MG_FROSTDIVER);
		break;
	}
#endif

	sd->menuskill_id = 0;
	sd->menuskill_val = 0;
}

// true if have the status
bool aa_checkactivestatus(map_session_data* sd, sc_type type) {
	if (!sd)
		return 0;

	if (sd->sc.getSCE(type))
		return true;

	return false;
}

// Automatically use buff items
bool aa_status_buffitem(map_session_data* sd, t_tick last_tick) {
    // Dead characters must not consume buff items while waiting to respawn.
    if (pc_isdead(sd) || !battle_config.feature_autoattack_buffitems || sd->aa.autobuffitems.empty() || sd->st != nullptr || sd->npc_id != 0) {
        return false;
    }

    // Iterate through configured auto-buff items
    for (auto& autobuffitem : sd->aa.autobuffitems) {
        // Skip items on cooldown or inactive items
        if (!autobuffitem.is_active)
            continue;

		// Check if the player have the status
		if (aa_checkactivestatus(sd, (sc_type)autobuffitem.status))
			continue;

        // Check inventory for the item and use it if available
        int inventory_index = pc_search_inventory(sd, autobuffitem.item_id);
        if (inventory_index >= 0 && pc_useitem(sd, inventory_index))
            return true;
    }

	return false;
}

// Handles auto-attack skills in combat
bool aa_status_attack(map_session_data* sd, struct mob_data* md_target, t_tick last_tick) {
	if (!md_target)
		return false;

	if (!battle_config.feature_autoattack_attackskill || last_tick < sd->aa.attack_skill_cd || sd->aa.autoattackskills.empty())
		return false;

	t_tick attack_delay = DIFF_TICK(last_tick, sd->aa.last_attack);
	time_t current_time = time(NULL);
	bool flywing_used = false;

	// Start at a random entry without allocating and reseeding a random engine
	// on every 100 ms combat tick. Repeated std::random_device calls could stall
	// the AI loop for several seconds on hosts with limited entropy.
	const size_t skill_count = sd->aa.autoattackskills.size();
	const size_t start_index = rnd() % skill_count;

	// Process each skill once, wrapping around from the random start entry.
	for (size_t offset = 0; offset < skill_count; ++offset) {
		const auto& skill = sd->aa.autoattackskills[(start_index + offset) % skill_count];
		if (!skill.is_active || !aa_canuseskill(sd, skill.skill_id, skill.skill_lv))
			continue;

		// Handle specific skill requirements
		switch (skill.skill_id) {
		case NC_ARMSCANNON:
		case GN_CARTCANNON:
			aa_cannonballchange(sd, md_target);
			break;
		case NJ_KUNAI:
			aa_kunaichange(sd, md_target, 1);
			break;
		case KO_HAPPOKUNAI:
			aa_kunaichange(sd, md_target, 8);
			break;
		}

		int skill_range = std::max(2, abs(skill_get_range(skill.skill_id, skill.skill_lv)));

		// Check range and pathfinding
		if (!check_distance_bl(&sd->bl, &md_target->bl, skill_range) ||
			!path_search_long(nullptr, sd->bl.m, sd->bl.x, sd->bl.y, md_target->bl.x, md_target->bl.y, CELL_CHKWALL)) {
			if (unit_walktobl(&sd->bl, &md_target->bl, skill_range, 1)) {
				return true;
			}
			continue;
		}

		// Execute skill
		bool skill_used = false;
		if (skill_get_inf(skill.skill_id) & INF_ATTACK_SKILL) {
			skill_used = unit_skilluse_id(&sd->bl, sd->aa.target_id, skill.skill_id, skill.skill_lv);
		}
		else if (skill_get_inf(skill.skill_id) & INF_GROUND_SKILL) {
			skill_used = unit_skilluse_pos(&sd->bl, md_target->bl.x, md_target->bl.y, skill.skill_id, skill.skill_lv);
		}
		else if (skill_get_inf(skill.skill_id) & INF_SELF_SKILL) {
			if (check_distance_bl(&sd->bl,&md_target->bl, 2)) {
				if (skill_is_combo(skill.skill_id) && !sd->sc.getSCE(SC_COMBO)) {
					continue;
				}
				skill_used = unit_skilluse_id(&sd->bl, sd->bl.id, skill.skill_id, skill.skill_lv);
			}
			else {
				if (unit_walktobl(&sd->bl, &md_target->bl, 2, 1)) {
					return true;
				}
				continue;
			}
		}

		if (!skill_used)
			continue;

		// Update cooldowns. unit_skilluse_id()/unit_skilluse_pos() already use
		// the normal skill pipeline and consume requirements exactly once.
		sd->idletime = current_time;

		if (battle_config.feature_autoattack_askill_delay &&
			skill_get_delay(skill.skill_id, skill.skill_lv) < battle_config.feature_autoattack_askill_delay) {
			sd->aa.attack_skill_cd = last_tick + battle_config.feature_autoattack_askill_delay +
				skill_get_cast(skill.skill_id, skill.skill_lv);
			sd->aa.last_attack = last_tick;
		}

		return true;
	}

	return false;
}

bool aa_status_melee(map_session_data* sd, struct mob_data* md_target, t_tick last_tick, const status_data* status) {
	bool is_attacking = false;
	if (!md_target)
		return false;

	if (sd->aa.stopmelee == 0 || (sd->aa.stopmelee == 2 && status->sp < 100)) {
		if (!unit_attack(&sd->bl, sd->aa.target_id, 1)) {
			sd->aa.last_attack = last_tick;
			return true;
		}
	}
	return false;
}

//Move
bool aa_move_short(map_session_data* sd, t_tick last_tick) {
	if (unit_is_walking(&sd->bl)) return false;

	t_tick now_tick = gettick();
	bool target_active = false;

	// Check target validity and range
	if (sd->aa.target_id && !sd->aa.itempick_id) {
		unsigned int valid_target = aa_check_target_alive(sd);
		if (valid_target) {
			struct mob_data* md = BL_CAST(BL_MOB, map_id2bl(valid_target));
			if (md) {
				const status_data* sstatus = status_get_status_data(&sd->bl);
				int range = sstatus ? sstatus->rhw.range : 1;

				if (distance_xy(sd->bl.x, sd->bl.y, md->bl.x, md->bl.y) <= range && 
					path_search(NULL, sd->bl.m, sd->bl.x, sd->bl.y, md->bl.x, md->bl.y, 1, CELL_CHKNOREACH)) {
					return false; // In range, no move needed
				}

				target_active = true;
				// Move to target
				if (aa_avoid_collision(sd, now_tick)) return true;
				if (unit_walktoxy(&sd->bl, md->bl.x, md->bl.y, 0)) {
					sd->aa.last_move = now_tick;
					add_position_to_history(sd);
					return true;
				}
			}
		} else {
			sd->aa.target_id = 0;
		}
	}

	// Check idle/move conditions
	if (!target_active) {
		if (pc_issit(sd) || sd->aa.itempick_id) return false;
		
		t_tick attack_tick = DIFF_TICK(now_tick, sd->aa.last_hit);
		int idle_tick = (int)(time(NULL) - sd->idletime);
		
		if (attack_tick <= 15000 && idle_tick <= AA_CONSTANTS::IDLE_THRESHOLD) return false;
	}

	// Teleport logic
	t_tick tele_tick = DIFF_TICK(now_tick, sd->aa.last_teleport);
	if ((!sd->aa.teleport.use_teleport || !sd->aa.teleport.use_flywing) && 
		sd->aa.teleport.delay_nomobmeet && tele_tick > sd->aa.teleport.delay_nomobmeet && 
		!sd->aa.target_id) {
		if (aa_teleport(sd)) return true;
	}

	t_tick move_tick = DIFF_TICK(now_tick, sd->aa.last_move);
	if (move_tick <= AA_CONSTANTS::MOVE_DELAY) return false;

	// Movement algorithms
	if (aa_try_escape_stuck(sd, now_tick)) return true;
	if (aa_try_continue_direction(sd, now_tick)) return true;
	if (aa_smart_pathfinding(sd, now_tick)) return true;
	if (aa_basic_movement(sd, now_tick)) return true;

	return aa_random_movement(sd, now_tick);
}

static void aa_move_path(map_session_data* sd) {
	if (unit_is_walking(&sd->bl)) {
		return;
	}

	int max_attempt = 2;
	int x = 0, y = 0;
	bool found = false;

	if (sd->aa.path.empty()) {
		for (int i = 0; i < max_attempt && !found; i++)
			found = aa_get_random_coords(sd->bl.m, x, y);

		if (found) {
			algorithm_path_finding(sd, sd->bl.m, sd->bl.x, sd->bl.y, x, y);
		}
	}

	if (aa_move_to_path(sd->aa.path, sd) == 0) {
		sd->aa.path.clear();
	}
}

void aa_status_checkmapchange(map_session_data* sd) {
	if (sd->bl.m != sd->aa.lastposition.map) {

		// Action after tp
		if (sd->state.autotrade) {
			pc_delinvincibletimer(sd);
			clif_parse_LoadEndAck(0, sd);
		}

		status_change_end(&sd->bl, SC_AUTOATTACK, INVALID_TIMER);
	}
}

void aa_moblist_reset_mapchange(map_session_data* sd) {
	// Reinit mapindex for mob selection if map changed
	if (sd->aa.mobs.map != sd->bl.m) {
		sd->aa.mobs.id.clear();
		sd->aa.mobs.map = sd->bl.m;
	}
}

bool aa_message(map_session_data* sd, std::string key, char* message, int delay, struct party_data* p) {
	if (!sd)
		return false;

	// Reference au vector party_msg pour une utilisation simplifiee
	auto& msg_list = sd->aa.msg_list;

	if (msg_list.empty()) {
		// Ajouter directement le message et le delai si le vecteur est vide
		msg_list.emplace_back(key, gettick() + delay * 1000);
	}
	else {
		// Rechercher si le message pour cette cle a deja ete envoye
		auto it = std::find_if(msg_list.begin(), msg_list.end(),
			[&key](const std::pair<std::string, t_tick>& msg) { return msg.first == key; });

		if (it != msg_list.end()) {
			// Verifier le delai avant d'envoyer un nouveau message
			if (gettick() < it->second) {
				return false;  // Ne pas envoyer si le delai n'est pas encore atteint
			}
			// Mettre a jour le delai du message
			it->second = gettick() + delay * 1000;
		}
		else {
			// Ajouter une nouvelle entree avec le delai du message
			msg_list.emplace_back(key, gettick() + delay * 1000);
		}
	}

	if(p)
		party_send_message(sd, message, (int)strlen(message) + 1);
	else
		clif_displaymessage(sd->fd, message);

	return true;
}

void aa_getusablepotions(map_session_data* sd, t_itemid* inventory_potion_id, int* inventory_potion_amount, char inventory_potion_name[MAX_INVENTORY][ITEM_NAME_LENGTH + 1], int* amount) {
	*amount = 0;

	if (!sd) return;

	for (int i = 0; i < MAX_INVENTORY; ++i) {
		const auto& inv_item = sd->inventory.u.items_inventory[i];

		if (auto item_data = item_db.find(inv_item.nameid)) {
			if (item_data->type == IT_HEALING) {
				inventory_potion_id[*amount] = inv_item.nameid;
				inventory_potion_amount[*amount] = inv_item.amount;
				safestrncpy(inventory_potion_name[*amount], item_data->ename.c_str(), ITEM_NAME_LENGTH);
				inventory_potion_name[*amount][ITEM_NAME_LENGTH] = '\0'; // Assurer la terminaison de la cha??ne

				(*amount)++;
			}
		}
	}
}

uint32 aa_getrental_search_inventory(map_session_data* sd, t_itemid nameid) {
	short i;
	uint32 expire_time = 0;
	nullpo_retr(-1, sd);

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].nameid == nameid) {
			if (sd->inventory.u.items_inventory[i].expire_time > 0) {
				expire_time = sd->inventory.u.items_inventory[i].expire_time;
			}
		}
	}

	return expire_time;
}

int aa_get_random_coords(int16 m, int &x, int &y){
	int16 i = 0;

	struct map_data* mapdata = map_getmapdata(m);

	//int32 edge = battle_config.map_edge_size;
	int32 edge = 15;
	do {
		x = rnd_value<int16>(edge, mapdata->xs - edge - 1);
		y = rnd_value<int16>(edge, mapdata->ys - edge - 1);
	} while ((map_getcell(m, x, y, CELL_CHKNOPASS) || (!battle_config.teleport_on_portal && npc_check_areanpc(1, m, x, y, 1))) && (i++) < 1000);

	if (i < 1000)
		return 1;
	else
		return 0;
}

// Fonction pour calculer le cout heuristique (distance de Manhattan)
float heuristic(int x1, int y1, int x2, int y2) {
	return static_cast<float>(std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));
}

static bool algorithm_path_finding(map_session_data* sd, int16_t m, int start_x, int start_y, int target_x, int target_y) {
	sd->aa.path.clear();
	sd->aa.path_index = 0;

	struct map_data* mapdata = map_getmapdata(m);
	int max_attempt = 0, max_attempt2 = 0;

	auto linear_index = [mapdata](int x, int y) {
		return y * mapdata->xs + x;
		};

	// Priority queue to explore cells
	std::priority_queue<Cell, std::vector<Cell>, std::greater<Cell>> open_list;

	// Maps for costs and parent tracking
	std::unordered_map<int, float> cost_so_far;
	std::unordered_map<int, int> came_from;

	open_list.push({ start_x, start_y, 0.0f });
	cost_so_far[linear_index(start_x, start_y)] = 0.0f;

	// Movement directions (dx, dy) with corresponding costs
	const int dxs[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
	const int dys[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	const float costs[8] = { 1.0f, 1.414f, 1.0f, 1.414f, 1.0f, 1.414f, 1.0f, 1.414f };

	std::unordered_set<int> visited;

	while (!open_list.empty() && max_attempt < 15000) {
		max_attempt++;
		Cell current = open_list.top();
		open_list.pop();

		// Verifier si la cible est atteinte
		if (current.x == target_x && current.y == target_y) {
			break;
		}

		// Verifier si la cellule a deja ete visitee
		int current_index = linear_index(current.x, current.y);
		if (visited.count(current_index)) {
			continue;
		}
		visited.insert(current_index);

		// Explorer les voisins
		for (int i = 0; i < 8; ++i) {
			int next_x = current.x + dxs[i];
			int next_y = current.y + dys[i];

			// Verifier les limites de la carte
			if (next_x < 0 || next_y < 0 || next_x >= mapdata->xs || next_y >= mapdata->ys) {
				continue;
			}

			// Verifier si la cellule est accessible
			if (map_getcell(m, next_x, next_y, CELL_CHKNOPASS)) {
				continue;
			}

			// Calculer le cout
			float new_cost = cost_so_far[current_index] + costs[i];
			int next_index = linear_index(next_x, next_y);

			// Ajouter la cellule si le cout est inferieur ou si elle n'a pas ete visitee
			if (!cost_so_far.count(next_index) || new_cost < cost_so_far[next_index]) {
				cost_so_far[next_index] = new_cost;
				float priority = new_cost + heuristic(next_x, next_y, target_x, target_y);
				open_list.push({ next_x, next_y, priority });
				came_from[next_index] = current_index;
			}
		}
	}

	if (max_attempt >= 15000)
		return false;

	// Reconstruct the path
	int current_index = linear_index(target_x, target_y);

	while (current_index != linear_index(start_x, start_y) && max_attempt2 < 350) {
		int x = current_index % mapdata->xs;
		int y = current_index / mapdata->xs;
		sd->aa.path.push_back({ x, y });
		current_index = came_from[current_index];
		max_attempt2++;
	}
	if (max_attempt2 >= 350 || sd->aa.path.size() < AA_WALK_CELL) {
		sd->aa.path.clear();
		return false;
	}

	sd->aa.path.push_back({ start_x, start_y });
	std::reverse(sd->aa.path.begin(), sd->aa.path.end());

	sd->aa.path_index = 0;

	return !sd->aa.path.empty();
}

// 0 - Path to recalculate - 1 - Walking to next cell - 2 moving
static int aa_move_to_path(std::vector<std::tuple<int, int>>& path, map_session_data* sd) {
	int case_to_walk = AA_WALK_CELL;

	// Verifier si le pion est en deplacement
	if (unit_is_walking(&sd->bl)) {
		return 2;
	}

	// Verifier si le chemin est termine
	if (path.empty()) {
		return 0;
	}

	if(sd->aa.path_index >= static_cast<int>(path.size())) {
		return 0;
	}

	// Verifier si le pion est deja a la prochaine case cible
	const auto& current_target = path[sd->aa.path_index];
	if (std::make_tuple(sd->bl.x, sd->bl.y) == current_target) {
		if (sd->aa.path_index + AA_WALK_CELL >= static_cast<int>(path.size())) {
			return 0;
		}

		if (static_cast<int>(path.size()) - (sd->aa.path_index - 1) < case_to_walk) {
			case_to_walk = static_cast<int>(path.size()) - (sd->aa.path_index - 1);
		}

		sd->aa.path_index += case_to_walk; // Avancer a la prochaine case

		while (case_to_walk > 1) {
			const auto& next_target = path[sd->aa.path_index];
			int target_x = std::get<0>(next_target);
			int target_y = std::get<1>(next_target);
			if (!unit_walktoxy(&sd->bl, target_x, target_y, 4)) {
				case_to_walk--;
				sd->aa.path_index--;
			}
			else {
				return 1;
			}
		}
	}
	else{
		const auto& next_target = path[path.size()-1];
		int target_x = std::get<0>(next_target);
		int target_y = std::get<1>(next_target);
		return algorithm_path_finding(sd, sd->bl.m, sd->bl.x, sd->bl.y, target_x, target_y);
	}

	return 0;
}

// 0 = init, 1 = start, 2 = stop
bool aa_changestate_autoattack(map_session_data* sd, int flag) {
	map_data* mapdata;
	map_session_data* pl_sd;
	struct unit_data* ud;
	struct s_mapiterator* iter;
	int ip_limitation = 0, gepard_limitation = 0;

	switch (flag) {
	case 1:
		if (battle_config.feature_autoattack_iplimit) {
			iter = mapit_getallusers();
			for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter))
			{
				if (pl_sd->bl.id != sd->bl.id && session[sd->fd]->client_addr == pl_sd->aa.client_addr && pl_sd->state.autoattack)
					ip_limitation++;

				if (ip_limitation > battle_config.feature_autoattack_iplimit) {
					std::string msg = "There is already an account using autoattack";
					aa_message(sd, "FlagOff", msg.data(), 5, nullptr);
					mapit_free(iter);
					return false;
				}
			}
			mapit_free(iter);
		}

		if (battle_config.feature_autoattack_gepardlimit) {
			iter = mapit_getallusers();
			for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter))
			{
				//Uncomment this if you have gepard and want to set a limit
				//if (pl_sd->bl.id != sd->bl.id && session[sd->fd]->gepard_info.unique_id == pl_sd->aa.unique_id && pl_sd->state.autoattack)
				//	gepard_limitation++;

				if (gepard_limitation > battle_config.feature_autoattack_gepardlimit) {
					std::string msg = "There is already an account using autoattack";
					aa_message(sd, "FlagOff", msg.data(), 5, nullptr);
					mapit_free(iter);
					return false;
				}
			}
			mapit_free(iter);
		}

		mapdata = map_getmapdata(sd->bl.m);
		if (!battle_config.feature_autoattack_allow_town && mapdata->getMapFlag(MF_TOWN)) {
			std::string msg = "Automessage - Autoattack is not allowed in town!";
			aa_message(sd, "FlagOff", msg.data(), 5, nullptr);
			return false;
		}

		if (!battle_config.feature_autoattack_allow_pvp && mapdata->getMapFlag(MF_PVP)) {
			std::string msg = "Automessage - Autoattack is not allowed in pvp map!";
			aa_message(sd, "FlagOff", msg.data(), 5, nullptr);
			return false;
		}

		if (!battle_config.feature_autoattack_allow_gvg && mapdata_flag_gvg2(mapdata)) {
			std::string msg = "Automessage - Autoattack is not allowed in woe!";
			aa_message(sd, "FlagOff", msg.data(), 5, nullptr);
			return false;
		}

		if (!battle_config.feature_autoattack_allow_bg && mapdata->getMapFlag(MF_BATTLEGROUND)) {
			std::string msg = "Automessage - Autoattack is not allowed in battleground map!";
			aa_message(sd, "FlagOff", msg.data(), 5, nullptr);
			return false;
		}

		if (battle_config.feature_autoattack_hateffect) {
			for (const auto& effectID : AA_HATEFFECTS) {
				if (effectID <= HAT_EF_MIN || effectID >= HAT_EF_MAX)
					continue;
				auto it = util::vector_get(sd->hatEffects, effectID);
				if (it != sd->hatEffects.end())
					continue;
				sd->hatEffects.push_back(effectID);
				if (!sd->state.connect_new)
					clif_hat_effect_single(sd, effectID, true);
			}
		}

		if (battle_config.feature_autoattack_prefixname) {
			char temp_name[NAME_LENGTH];
			safestrncpy(temp_name, AA_CONSTANTS::PREFIX_NAME, sizeof(temp_name));
			strncat(temp_name, sd->status.name, sizeof(temp_name) - strlen(temp_name) - 1);
			safestrncpy(sd->fakename, temp_name, sizeof(sd->fakename));

			clif_name_area(&sd->bl);
			if (sd->disguise) // Another packet should be sent so the client updates the name for sd
				clif_name_self(&sd->bl);
		}

		sd->state.autoattack = true;
	case 0:
		sd->aa.lastposition.map = sd->bl.m;
		sd->aa.lastposition.x = sd->bl.x;
		sd->aa.lastposition.y = sd->bl.y;
		sd->aa.lastposition.dx = 0;
		sd->aa.lastposition.dy = 0;
		sd->aa.last_hit = gettick();
		sd->aa.last_teleport = gettick();
		sd->aa.last_move = gettick();
		sd->aa.last_attack = gettick();
		sd->aa.last_pick = gettick();
		sd->aa.skill_cd = 0;
		sd->aa.attack_skill_cd = 0;
		sd->aa.attack_target_id = 0;
		aa_target_change(sd, 0);
		sd->aa.itempick_id = 0;
		sd->aa.path.clear();
		sd->aa.path_index = 0;
		sd->aa.client_addr = session[sd->fd]->client_addr;
		// Uncomment this if you have gepard and want to set a limit
		//sd->aa.unique_id = session[sd->fd]->gepard_info.unique_id;
		break;

	case 2:
		if (battle_config.feature_autoattack_prefixname && sd->fakename[0]) {
			sd->fakename[0] = '\0';
			clif_name_area(&sd->bl);
			if (sd->disguise)
				clif_name_self(&sd->bl);
		}

		if (sd->aa.action_on_end == 1) {
			pc_setpos(sd, mapindex_name2id(sd->status.save_point.map), sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT); // return to save point
			sd->aa.lastposition.x = sd->status.save_point.x;
			sd->aa.lastposition.y = sd->status.save_point.y;
			sd->aa.lastposition.map = sd->bl.m;

			if (sd->state.autotrade) {
				pc_delinvincibletimer(sd);
				clif_parse_LoadEndAck(0, sd);
			}
		}
		
		if (sd->aa.action_on_end == 2) { //logout
			if (session_isActive(sd->fd))
				clif_authfail_fd(sd->fd, 10);
			else
				map_quit(sd);
		}


		if (battle_config.feature_autoattack_hateffect) {
			for (const auto& effectID : AA_HATEFFECTS) {
				if (effectID <= HAT_EF_MIN || effectID >= HAT_EF_MAX)
					continue;
				auto it = util::vector_get(sd->hatEffects, effectID);
				if (it == sd->hatEffects.end())
					continue;
				util::vector_erase_if_exists(sd->hatEffects, effectID);
				if (!sd->state.connect_new)
					clif_hat_effect_single(sd, effectID, false);
			}
		}
		sd->state.autoattack = false;
		break;
	}

	return true;
}

bool aa_party_request(map_session_data* sd) {
	party_join(sd, sd->party_invite);
	return true;
}
