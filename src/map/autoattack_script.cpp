#include "autoattack_script.hpp"
#include "autoattack.hpp"
#include "battle.hpp"
#include "script.hpp"
#include "itemdb.hpp"

#include <common/db.hpp>
#include <common/showmsg.hpp>

#include <cstdlib> // atoi, strtol, strtoll, exit
#include <sstream>
#include <string>
#include <vector>
#include <set>

//std::vector<t_itemid> ai_item_buy;

bool find_aa_skill_heal(map_session_data* sd, int skill_id, s_autoheal& found_buff) {
	auto it = std::find_if(sd->aa.autoheal.begin(), sd->aa.autoheal.end(),
		[skill_id](const s_autoheal& v) { return v.skill_id == skill_id; });

	if (it != sd->aa.autoheal.end()) {
		found_buff = *it;
		return true;
	}
	return false;
}

bool find_aa_skill_buff(map_session_data* sd, int skill_id, s_autobuffskills& found_buff) {
	auto it = std::find_if(sd->aa.autobuffskills.begin(), sd->aa.autobuffskills.end(),
		[skill_id](const s_autobuffskills& v) { return v.skill_id == skill_id; });

	if (it != sd->aa.autobuffskills.end()) {
		found_buff = *it;
		return true;
	}
	return false;
}

bool find_aa_skill_attack(map_session_data* sd, int skill_id, s_autoattackskills& found_buff) {
	auto it = std::find_if(sd->aa.autoattackskills.begin(), sd->aa.autoattackskills.end(),
		[skill_id](const s_autoattackskills& v) { return v.skill_id == skill_id; });

	if (it != sd->aa.autoattackskills.end()) {
		found_buff = *it;
		return true;
	}
	return false;
}

bool find_aa_mob(map_session_data* sd, int mob_id) {
	auto it = std::find_if(sd->aa.mobs.id.begin(), sd->aa.mobs.id.end(),
		[mob_id](const int& v) { return v == mob_id; });

	if (it != sd->aa.mobs.id.end())
		return true;

	return false;
}

// Fonction pour recuperer les monstres de la map
std::map<int, std::string> get_unique_monsters_onmap(TBL_PC* sd, const char* monster_menu_state, script_state* st) {
	std::map<int, std::string> unique_mobs;
	if (!sd->aa.mobs.id.empty()) {
		for (const auto& itAutoattackmobs : sd->aa.mobs.id) {
			auto mob = mob_db.find(itAutoattackmobs);
			if (!mob) continue;

			std::ostringstream menu_temp;
			menu_temp << "^008000(ON)^000000 [" << mob->id << "] " << mob->name.c_str();

			unique_mobs[itAutoattackmobs] = menu_temp.str();

			set_reg_num(st, sd, reference_uid(add_str(monster_menu_state), itAutoattackmobs), monster_menu_state, 1, nullptr);
		}
	}

	// add other mobs
	unsigned short mapindex = sd->mapindex;
	s_mapiterator* it = mapit_geteachmob();
	bool mob_searched[MAX_MOB_DB2];
	memset(mob_searched, 0, MAX_MOB_DB2);

	while (true) {
		TBL_MOB* md = (TBL_MOB*)mapit_next(it);
		if (md == NULL)
			break;

		if (md->bl.m != sd->bl.m || md->status.hp <= 0)
			continue;

		if (mob_searched[md->db->id] == true)
			continue; // Already found, skip it

		std::shared_ptr<s_mob_db> mob = mob_db.find(md->db->id);
		if (!mob)
			continue;

		mob_searched[md->db->id] = true;

		if (unique_mobs.count(md->db->id) == 0) {
			std::ostringstream mob_temp;
			mob_temp << "^FF0000(OFF)^000000 [" << mob->id << "] " << mob->name.c_str();
			unique_mobs[mob->id] = mob_temp.str();

			set_reg_num(st, sd, reference_uid(add_str(monster_menu_state), mob->id), monster_menu_state, 0, nullptr);
		}
	}
	mapit_free(it);

	return unique_mobs;
}

// Fonction pour recuperer les competences uniques
std::map<int, std::string> get_unique_attack_skills(TBL_PC* sd, const char* skill_menu_state, script_state* st) {
	std::map<int, std::string> unique_skills;
	if (!sd->aa.autoattackskills.empty()) {
		for (const auto& itAutoattackskills : sd->aa.autoattackskills) {
			auto skill = skill_db.find(itAutoattackskills.skill_id);
			if (!skill) continue;

			std::ostringstream menu_temp;
			menu_temp << "^008000(ON)^000000 " << skill->desc << " - Lv " << itAutoattackskills.skill_lv;

			unique_skills[itAutoattackskills.skill_id] = menu_temp.str();

			set_reg_num(st, sd, reference_uid(add_str(skill_menu_state), itAutoattackskills.skill_id), skill_menu_state, 1, nullptr);
		}
	}

	// add other skills
	for (int i = 0; i < MAX_SKILL; ++i) {
		if (sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0
			//uncomment the following line to prevent a skill to be shown in the list
			//&& sd->status.skill[i].id != AL_RUWACH
			) {
			auto skill = skill_db.find(sd->status.skill[i].id);
			if (!skill) continue;

			e_cast_type type = skill_get_casttype(sd->status.skill[i].id);
			if ((skill->skill_type != BF_NONE
				&& skill->inf != INF_PASSIVE_SKILL
				&& !skill_get_nk(sd->status.skill[i].id, NK_NODAMAGE)
				&& (type != CAST_NODAMAGE || skill->inf & INF_SELF_SKILL)
				)
				// Add skills like the following lines to force the skill to show in the list
				|| skill->nameid == KN_BRANDISHSPEAR
				|| skill->nameid == CG_TAROTCARD
				) {

				if (unique_skills.count(skill->nameid) == 0) {
					std::ostringstream skill_temp;
					skill_temp << "^FF0000(OFF)^000000 " << skill->desc << " - Lv " << std::to_string(sd->status.skill[i].lv);
					unique_skills[skill->nameid] = skill_temp.str();

					set_reg_num(st, sd, reference_uid(add_str(skill_menu_state), skill->nameid), skill_menu_state, 0, nullptr);
				}
			}
		}
	}
	return unique_skills;
}

// Fonction pour recuperer les competences uniques
std::map<int, std::string> get_unique_buff_skills(TBL_PC* sd, const char* skill_menu_state, script_state* st) {
	std::map<int, std::string> unique_skills;
	if (!sd->aa.autobuffskills.empty()) {
		for (const auto& itAutobuffskills : sd->aa.autobuffskills) {
			auto skill = skill_db.find(itAutobuffskills.skill_id);
			if (!skill) continue;

			std::ostringstream menu_temp;
			menu_temp << "^008000(ON)^000000 " << skill->desc << " - Lv " << itAutobuffskills.skill_lv;

			unique_skills[itAutobuffskills.skill_id] = menu_temp.str();

			set_reg_num(st, sd, reference_uid(add_str(skill_menu_state), itAutobuffskills.skill_id), skill_menu_state, 1, nullptr);
		}
	}

	// add other skills
	for (int i = 0; i < MAX_SKILL; ++i) {
		if (sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0
			//Add skill like the following line to prevent a skill to be shown in the list
			&& sd->status.skill[i].id != AL_RUWACH
			) {
			auto skill = skill_db.find(sd->status.skill[i].id);
			if (!skill) continue;

			e_cast_type type = skill_get_casttype(sd->status.skill[i].id);
			if ((type == CAST_NODAMAGE
				&& skill->inf & (INF_SUPPORT_SKILL | INF_SELF_SKILL)
				&& skill_get_sc(skill->nameid) != SC_NONE)
				|| skill->nameid == MO_CALLSPIRITS
				|| skill->nameid == CH_SOULCOLLECT
				|| skill->nameid == SA_AUTOSPELL ) {

				if (unique_skills.count(skill->nameid) == 0) {
					std::ostringstream skill_temp;
					skill_temp << "^FF0000(OFF)^000000 " << skill->desc << " - Lv " << std::to_string(sd->status.skill[i].lv);
					unique_skills[skill->nameid] = skill_temp.str();

					set_reg_num(st, sd, reference_uid(add_str(skill_menu_state), skill->nameid), skill_menu_state, 0, nullptr);
				}
			}
		}
	}
	return unique_skills;
}

std::string format_skill_heal_status(const s_autoheal& sheal, std::shared_ptr<s_skill_db> skill) {
	std::ostringstream os;
	os << (sheal.is_active ? "^008000(ON)^000000 " : "^FF0000(OFF)^000000 ");
	os << "[" << skill->desc << "] - Lv " << sheal.skill_lv << " - HP % " << sheal.min_hp;
	return os.str();
}

std::string format_skill_buff_status(const s_autobuffskills& sbuff, std::shared_ptr<s_skill_db> skill) {
	std::ostringstream os;
	os << (sbuff.is_active ? "^008000(ON)^000000 " : "^FF0000(OFF)^000000 ");
	os << "[" << skill->desc << "]";
	os << (sbuff.is_active ? " - Lv " + std::to_string(sbuff.skill_lv) : "");
	return os.str();
}

std::string format_skill_attack_status(const s_autoattackskills& sattack, std::shared_ptr<s_skill_db> skill) {
	std::ostringstream os;
	os << (sattack.is_active ? "^008000(ON)^000000 " : "^FF0000(OFF)^000000 ");
	os << "[" << skill->desc << "]";
	os << (sattack.is_active ? " - Lv " + std::to_string(sattack.skill_lv) : "");
	return os.str();
}

std::string format_mob_status(const int& mob_id) {
	std::ostringstream os;
	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);
	if(mob)
		os << "^008000(ON)^000000 [" << mob->id << "] " << mob->name.c_str();
	return os.str();
}

void format_skill_list(map_session_data* sd, std::ostringstream& menu, const std::map<int, std::string>& unique_skills, const char* skill_menu_ids, script_state* st) {
	bool first_done = false;
	int j = 0;

	for (const auto& uskill : unique_skills) {
		if (first_done)
			menu << ":";
		else
			first_done = true;

		menu << uskill.second;

		set_reg_num(st, sd, reference_uid(add_str(skill_menu_ids), ++j), skill_menu_ids, uskill.first, nullptr);
	}
}

void set_skill_state(script_state* st, map_session_data* sd, const char* state_name, int skill_id, bool is_active) {
	set_reg_num(st, sd, reference_uid(add_str(state_name), skill_id), state_name, is_active ? 1 : 0, nullptr);
}

void handle_autoattack_heal(std::ostringstream& os_buf, int index, int extra_index, script_state* st, map_session_data* sd) {

	std::shared_ptr<s_skill_db> skill;

	const char* player_heal_list = ".@player_heal_list$";
	const char* player_heal_ids = ".@player_heal_ids";
	const char* player_heal_state = ".@player_heal_state";
	int j = 0;

	std::ostringstream menu;

	if (index == 0) {
		skill = skill_db.find(extra_index);
		if (!skill)
			return;

		s_autoheal sheal;
		if (find_aa_skill_heal(sd, extra_index, sheal)) {
			os_buf << format_skill_heal_status(sheal, skill);

			set_reg_str(st, sd, reference_uid(add_str(player_heal_list), 0), player_heal_list, menu.str().c_str(), nullptr);
		}
	}
	else if (index > 0) {
		skill = skill_db.find(index);
		if (!skill)
			return;

		s_autoheal sheal;
		if (find_aa_skill_heal(sd, index, sheal)) {
			os_buf << format_skill_heal_status(sheal, skill);
			set_skill_state(st, sd, ".@skill_menu_state", index, true);
		}
		else {
			os_buf << "^FF0000(OFF)^000000 " << skill->desc << " - Max lv " << std::to_string(pc_checkskill(sd, index));
			set_skill_state(st, sd, ".@skill_menu_state", index, false);
		}
	}

	return;
}

void handle_autoattack_potions(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd) {
	const char* potion_menu_list = ".@potion_menu_list$";
	const char* potion_menu_ids = ".@potion_menu_ids";
	const char* potion_menu_state = ".@potion_menu_state";
	std::ostringstream menu;
	bool first_done = false;

	int j = 0;

	std::map<int, std::string> unique_items;
	const auto is_disabled_potion = [](t_itemid item_id) {
		return item_id == 11548 || item_id == 11549;
	};

	// Helper function to build potion item string
	auto buildPotionItemString = [](const std::string& status, const std::string& item_name, short amount = 0, uint16 min_hp = 0, uint16 min_sp = 0) {
		std::ostringstream item_str;
		item_str << status << " " << item_name;
		if (min_hp > 0)
			item_str << " - HP < " << min_hp << "%";
		if (min_sp > 0)
			item_str << " - SP < " << min_sp << "%";
		if (amount > 0)
			item_str << " - x" << amount;
		return item_str.str();
		};

	// Process active autobuff potions
	for (const auto& itAutopotion : sd->aa.autopotion) {
		if (is_disabled_potion(itAutopotion.item_id))
			continue;
		if (auto item_data = item_db.find(itAutopotion.item_id)) {
			unique_items[itAutopotion.item_id] = buildPotionItemString("^008000(ON)^000000", item_data->ename, 0, itAutopotion.min_hp, itAutopotion.min_sp);
			set_reg_num(st, sd, reference_uid(add_str(potion_menu_state), itAutopotion.item_id), potion_menu_state, 1, nullptr);
		}
	}

	// Process potions in inventory
	t_itemid inventory_potion_id[MAX_INVENTORY];
	int inventory_potion_amount[MAX_INVENTORY];
	char inventory_potion_name[MAX_INVENTORY][ITEM_NAME_LENGTH + 1];
	int amount = 0;

	aa_getusablepotions(sd, inventory_potion_id, inventory_potion_amount, inventory_potion_name, &amount);

	for (int i = 0; i < amount; ++i) {
		if (is_disabled_potion(inventory_potion_id[i]))
			continue;
		if (unique_items.count(inventory_potion_id[i]) > 0) {
			unique_items[inventory_potion_id[i]] += " - x" + std::to_string(inventory_potion_amount[i]); // append the amount
		}
		else {
			unique_items[inventory_potion_id[i]] = buildPotionItemString("^FF0000(OFF)^000000", inventory_potion_name[i], inventory_potion_amount[i]);
			set_reg_num(st, sd, reference_uid(add_str(potion_menu_state), inventory_potion_id[i]), potion_menu_state, 0, nullptr);
		}
	}

	for (const auto& uitem : unique_items) {
		if (first_done)
			menu << ":";
		else
			first_done = true;

		menu << uitem.second;
		set_reg_num(st, sd, reference_uid(add_str(potion_menu_ids), ++j), potion_menu_ids, uitem.first, nullptr);
	}

	set_reg_str(st, sd, reference_uid(add_str(potion_menu_list), 0), potion_menu_list, menu.str().c_str(), nullptr);

	return;
}



void handle_autoattack_itempickup(std::ostringstream& os_buf, int index, int extra_index, script_state* st, map_session_data* sd) {
	std::shared_ptr<item_data> item_data;
	switch(sd->aa.pickup_item_config){
		case 0:
			os_buf << "Pick up every item on the floor \n";
			break;
		case 1:
			os_buf << "Pick up only items on the list \n";
			break;
		case 2:
			os_buf << "Don't pick up any item \n";
			break;	
	}
	if(sd->aa.pickup_item_id.size() > 0){
		for (int i=0; i<sd->aa.pickup_item_id.size(); i++){
			if( ( item_data = item_db.find(sd->aa.pickup_item_id.at(i)) ) == NULL )
				continue;
			os_buf << "[" << item_data->name.c_str() << "] ^i[" << item_data->nameid << "]\n";
		}
	} else
		os_buf << "\n No item to pick up in the list \n";
	return;
}


void handle_autoattack_monsterselection(std::ostringstream& os_buf, int index, int extra_index, script_state* st, map_session_data* sd) {
	std::shared_ptr<s_skill_db> skill;

	const char* mobs_menu_list = ".@mobs_menu_list$";
	const char* mobs_menu_ids = ".@mobs_menu_ids";
	const char* mobs_menu_state = ".@mobs_menu_state";

	if (index == 0) { // Monster menu
		std::ostringstream menu;
		std::map<int, std::string> unique_mobs = get_unique_monsters_onmap(sd, mobs_menu_state, st);

		format_skill_list(sd, menu, unique_mobs, mobs_menu_ids, st); // works for mobs also
		set_reg_str(st, sd, reference_uid(add_str(mobs_menu_list), 0), mobs_menu_list, menu.str().c_str(), nullptr);
	}
	else if (index > 0 && extra_index == 1) { // Player list menu
		const char* player_buff_list = ".@player_buff_list$";
		const char* player_buff_ids = ".@player_buff_ids";
		const char* player_buff_state = ".@player_buff_state";

		if (find_aa_mob(sd, index)) 
			os_buf << format_mob_status(index);
		else
			os_buf << "^FF0000(OFF)^000000 " << skill_db.find(index)->desc << "\n";

	}
	else {
		std::shared_ptr<s_mob_db> mob = mob_db.find(index);
		if (mob)
			os_buf << "^FF0000(OFF)^000000 [" << mob->id << "] " << mob->name.c_str();
	}

	return;
}

void handle_autoattack_attack(std::ostringstream& os_buf, int index, int extra_index, script_state* st, map_session_data* sd) {
	std::shared_ptr<s_skill_db> skill;

	const char* skill_menu_list = ".@skill_menu_list$";
	const char* skill_menu_ids = ".@skill_menu_ids";
	const char* skill_menu_state = ".@skill_menu_state";

	if (index == 0) { // Skill menu
		std::ostringstream menu;
		std::map<int, std::string> unique_skills = get_unique_attack_skills(sd, skill_menu_state, st);
		format_skill_list(sd, menu, unique_skills, skill_menu_ids, st);
		set_reg_str(st, sd, reference_uid(add_str(skill_menu_list), 0), skill_menu_list, menu.str().c_str(), nullptr);
	}
	else if (index > 0 && extra_index == 1) { // Player list menu
		const char* player_buff_list = ".@player_buff_list$";
		const char* player_buff_ids = ".@player_buff_ids";
		const char* player_buff_state = ".@player_buff_state";

		s_autoattackskills found_attack;
		if (find_aa_skill_attack(sd, index, found_attack)) {
			os_buf << format_skill_attack_status(found_attack, skill_db.find(index));

			//std::map<int, std::string> unique_players = get_unique_players(sd, found_attack, party_id, player_buff_state, st, os_buf);
			std::map<int, std::string> unique_players;
			std::ostringstream menu;
			format_skill_list(sd, menu, unique_players, player_buff_ids, st);

			set_reg_str(st, sd, reference_uid(add_str(player_buff_list), 0), player_buff_list, menu.str().c_str(), nullptr);
		}
		else {
			os_buf << "^FF0000(OFF)^000000 " << skill_db.find(index)->desc << "\n";
		}
	}
	else {
		os_buf << "^FF0000(" << skill_db.find(index)->nameid << ")^000000 " << skill_db.find(index)->desc;
	}

	return;
}

void handle_autoattack_buff(std::ostringstream& os_buf, int index, int extra_index, script_state* st, map_session_data* sd) {
	std::shared_ptr<s_skill_db> skill;

	const char* skill_menu_list = ".@skill_menu_list$";
	const char* skill_menu_ids = ".@skill_menu_ids";
	const char* skill_menu_state = ".@skill_menu_state";

	if (index == 0) { // Skill menu
		std::ostringstream menu;
		std::map<int, std::string> unique_skills = get_unique_buff_skills(sd, skill_menu_state, st);
		format_skill_list(sd, menu, unique_skills, skill_menu_ids, st);
		set_reg_str(st, sd, reference_uid(add_str(skill_menu_list), 0), skill_menu_list, menu.str().c_str(), nullptr);
	}
	else if (index > 0 && extra_index == 1) { // Player list menu
		const char* player_buff_list = ".@player_buff_list$";
		const char* player_buff_ids = ".@player_buff_ids";
		const char* player_buff_state = ".@player_buff_state";

		s_autobuffskills found_buff;
		if (find_aa_skill_buff(sd, index, found_buff)) {
			os_buf << format_skill_buff_status(found_buff, skill_db.find(index));

			//std::map<int, std::string> unique_players = get_unique_players(sd, found_buff, party_id, player_buff_state, st, os_buf);
			std::map<int, std::string> unique_players;
			std::ostringstream menu;
			format_skill_list(sd, menu, unique_players, player_buff_ids, st);

			set_reg_str(st, sd, reference_uid(add_str(player_buff_list), 0), player_buff_list, menu.str().c_str(), nullptr);
		}
		else {
			os_buf << "^FF0000(OFF)^000000 " << skill_db.find(index)->desc << "\n";
		}
	}
	else {
		os_buf << "^FF0000(" << skill_db.find(index)->nameid << ")^000000 " << skill_db.find(index)->desc;
	}

	return;
}


void handle_autoattack_buy(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd) {
    char buffer[256];
    std::shared_ptr<item_data> item_data;
    
    if (index == 0) { // Show configured items to buy (for delete menu)
        if (sd->aa.autobuyitems.size()) {
            int discount_lv = pc_checkskill(sd, MC_DISCOUNT);
            std::ostringstream menu;
            int menu_index = 0;
            
            const char* buy_menu_list = ".@buy_menu_list$";
            const char* buy_menu_ids = ".@buy_menu_ids";
            
            for (const auto& entry : sd->aa.autobuyitems) {
                t_itemid itemid = entry.item_id;
                if ((item_data = item_db.find(itemid)) == NULL)
                    continue;
                
                std::string item_name = item_data->name;
                int price = item_data->value_buy;
                
                // Apply MC_DISCOUNT bonus
                if (discount_lv > 0) {
                    int discount_percent = 0;
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
                    }
                    price = price - (price * discount_percent / 100);
                }
                
                if (discount_lv > 0) {
                    safesnprintf(buffer, sizeof(buffer),
                        "-> ^i[%u]\n"
                        "    Qty: ^FF0000%d^000000 pcs | BW: ^FF0000%d^000000 pcs\n"
                        "    price: ^00AA00%d^000000 Zeny ^AAAAAA(DC)^000000\n",
                        itemid,
                        entry.quantity,
                        entry.min_trigger,
                        price
                    );
                } else {
                    safesnprintf(buffer, sizeof(buffer),
                        "-> ^i[%u]\n"
                        "    Qty: ^FF0000%d^000000 pcs | BW: ^FF0000%d^000000 pcs\n"
                        "    price: ^00AA00%d^000000 Zeny\n",
                        itemid, 
                        entry.quantity,
                        entry.min_trigger,
                        price
                    );
                }
                
                os_buf << buffer;
                
                // Build menu string for delete menu
                if (menu_index > 0)
                    menu << ":";
                menu << item_name;
                
                // Store item ID in array
                set_reg_num(st, sd, reference_uid(add_str(buy_menu_ids), menu_index), buy_menu_ids, itemid, nullptr);
                menu_index++;
            }
            
            // Set menu list variable
            set_reg_str(st, sd, reference_uid(add_str(buy_menu_list), 0), buy_menu_list, menu.str().c_str(), nullptr);
            
        } else {
            os_buf << "No items configured for auto-buy";
        }
    }
    else if (index == -1) { // Show available items list (for adding)
        if (ai_item_buy.size()) {
            std::ostringstream menu;
            int menu_index = 0;
            
            const char* buy_add_menu_list = ".@buy_add_menu_list$";
            const char* buy_add_menu_ids = ".@buy_add_menu_ids";
            
            for (t_itemid itemid : ai_item_buy) {
                if ((item_data = item_db.find(itemid)) == NULL)
                    continue;
                
                safesnprintf(buffer, sizeof(buffer), "-> [%s] ^i[%u]\n", item_data->name.c_str(), itemid);
                os_buf << buffer;
                
                // Build menu string
                if (menu_index > 0)
                    menu << ":";
                menu << item_data->name;
                
                // Store item ID in array
                set_reg_num(st, sd, reference_uid(add_str(buy_add_menu_ids), menu_index), buy_add_menu_ids, itemid, nullptr);
                menu_index++;
            }
            
            // Set menu list variable
            set_reg_str(st, sd, reference_uid(add_str(buy_add_menu_list), 0), buy_add_menu_list, menu.str().c_str(), nullptr);
            
        } else {
            os_buf << "No items available for purchase";
        }
    }
    
    return;
}

void handle_autoattack_sell(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd) {
    char buffer[256];
    std::shared_ptr<item_data> item_data;
    
    if (index == 0) { // Show configured items to sell (for delete menu)
        if (sd->aa.autosellitems.size()) {
            int overcharge_lv = pc_checkskill(sd, MC_OVERCHARGE);
            std::ostringstream menu;
            int menu_index = 0;
            
            const char* sell_menu_list = ".@sell_menu_list$";
            const char* sell_menu_ids = ".@sell_menu_ids";
            
            for (const auto& entry : sd->aa.autosellitems) {
                t_itemid itemid = entry.item_id;
                if ((item_data = item_db.find(itemid)) == NULL)
                    continue;
                
                std::string item_name = item_data->name;
                int price = item_data->value_sell;
                
                // Apply MC_OVERCHARGE bonus
                if (overcharge_lv > 0) {
                    int overcharge_percent = 0;
                    switch (overcharge_lv) {
                        case 1: overcharge_percent = 7; break;
                        case 2: overcharge_percent = 9; break;
                        case 3: overcharge_percent = 11; break;
                        case 4: overcharge_percent = 13; break;
                        case 5: overcharge_percent = 15; break;
                        case 6: overcharge_percent = 17; break;
                        case 7: overcharge_percent = 19; break;
                        case 8: overcharge_percent = 21; break;
                        case 9: overcharge_percent = 23; break;
                        case 10: overcharge_percent = 24; break;
                    }
                    price = price + (price * overcharge_percent / 100);
                }
                
                if (overcharge_lv > 0) {
                    safesnprintf(buffer, sizeof(buffer),
                        "-> ^i[%u] | price: ^00AA00%d^000000 Zeny ^AAAAAA(OC)^000000\n",
                        itemid,
                        price
                    );
                } else {
                    safesnprintf(buffer, sizeof(buffer),
                        "-> ^i[%u] | price: ^00AA00%d^000000 Zeny\n",
                        itemid,
                        price
                    );
                }
                
                os_buf << buffer;
                
                // Build menu string for delete menu
                if (menu_index > 0)
                    menu << ":";
                menu << item_name;
                
                // Store item ID in array
                set_reg_num(st, sd, reference_uid(add_str(sell_menu_ids), menu_index), sell_menu_ids, itemid, nullptr);
                menu_index++;
            }
            
            // Set menu list variable
            set_reg_str(st, sd, reference_uid(add_str(sell_menu_list), 0), sell_menu_list, menu.str().c_str(), nullptr);
            
        } else {
            os_buf << "No items configured for auto-sell";
        }
    }
    else if (index == -1) { // Show items in inventory (for adding)
        std::ostringstream menu;
        int menu_index = 0;
        std::set<t_itemid> unique_items;
        
        const char* sell_add_menu_list = ".@sell_add_menu_list$";
        const char* sell_add_menu_ids = ".@sell_add_menu_ids";
        
        for (int i = 0; i < MAX_INVENTORY; i++) {
            if (sd->inventory.u.items_inventory[i].nameid == 0)
                continue;
                
            t_itemid itemid = sd->inventory.u.items_inventory[i].nameid;
            
            if (unique_items.count(itemid) > 0)
                continue;
                
            if ((item_data = item_db.find(itemid)) == NULL)
                continue;
            
            if (item_data->type != IT_ETC && item_data->type != IT_HEALING)
                continue;
            
            unique_items.insert(itemid);
            
            safesnprintf(buffer, sizeof(buffer), "-> [%s] ^i[%u]\n", item_data->name.c_str(), itemid);
            os_buf << buffer;
            
            // Build menu string
            if (menu_index > 0)
                menu << ":";
            menu << item_data->name;
            
            // Store item ID in array
            set_reg_num(st, sd, reference_uid(add_str(sell_add_menu_ids), menu_index), sell_add_menu_ids, itemid, nullptr);
            menu_index++;
        }
        
        if (menu_index > 0) {
            set_reg_str(st, sd, reference_uid(add_str(sell_add_menu_list), 0), sell_add_menu_list, menu.str().c_str(), nullptr);
        } else {
            os_buf << "No sellable items in inventory (ETC, HEALING)";
        }
    }
    
    return;
}


void handle_autoattack_sitrest(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd) {
	if (sd->aa.autositregen.is_active) {
		os_buf << "^FF0000(Enabled)^000000 \n";

		if (sd->aa.autositregen.min_hp > 0)
			os_buf << "HP - Sit if hp < " << sd->aa.autositregen.min_hp << "% - Stand if hp >= " << sd->aa.autositregen.max_hp << "% \n";

		if (sd->aa.autositregen.min_sp)
			os_buf << "SP - Sit if sp < " << sd->aa.autositregen.min_sp << "% - Stand if sp >= " << sd->aa.autositregen.max_sp << "% \n";
	}
	else
		os_buf << "^FF0000(Disabled)^000000 \n";

}

void handle_autoattack_teleport(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd) {
	if (sd->aa.teleport.min_hp > 0)
		os_buf << "^008000(ON)^000000 Minimum HP value for emergency teleport set to " << sd->aa.teleport.min_hp << "%\n";
	else
		os_buf << "^FF0000(OFF)^000000 Minimum HP value for emergency teleport \n";
	

	if (sd->aa.teleport.delay_nomobmeet > 0)
		os_buf << "^008000(ON)^000000 Teleport delay if no monster is encountered within that time to " << sd->aa.teleport.delay_nomobmeet/1000 << "s\n";
	else
		os_buf << "^FF0000(OFF)^000000 Teleport delay if no monster is encountered within that time \n";

	if (sd->aa.monster_surround > 0)
		os_buf << "^008000(ON)^000000 Teleport if more than " << sd->aa.monster_surround << " attack you \n";

	os_buf << (sd->aa.teleport.tp_mvp ? "^008000(ON)^000000" : "^FF0000(OFF)^000000")
		<< " Teleport when an MVP is found\n";
	os_buf << (sd->aa.teleport.tp_miniboss ? "^008000(ON)^000000" : "^FF0000(OFF)^000000")
		<< " Teleport when a Mini Boss is found\n";
}

void handle_autoattack_items(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd) {
	const char* buffitem_menu_state = ".@buffitem_menu_state";
	auto item_data = item_db.find(index);

	if (item_data && item_data->type == IT_USABLE) {
		auto itAutoattackitem = std::find_if(sd->aa.autobuffitems.begin(), sd->aa.autobuffitems.end(),
			[index](const s_autobuffitems& v) { return v.item_id == index; });

		bool is_autoattack_item = (itAutoattackitem != sd->aa.autobuffitems.end());
		os_buf << (is_autoattack_item ? "^008000(ON)^000000 " : "^FF0000(OFF)^000000 ")
			<< item_data->ename.c_str();
		set_reg_num(st, sd, reference_uid(add_str(buffitem_menu_state), index), buffitem_menu_state, is_autoattack_item, nullptr);
	}
}

void handle_potions(std::ostringstream& os_buf, map_session_data* sd) {
	os_buf << "[Potions] - " << (sd->aa.state_potions ? "Enabled" : "Disabled");
}

void handle_token_of_siegfried(std::ostringstream& os_buf, map_session_data* sd) {
	os_buf << "[Token of Siegfried] - " << (sd->aa.token_siegfried ? "Enabled" : "Disabled");
}

void handle_priorize_loot_fight(std::ostringstream& os_buf, map_session_data* sd) {
	os_buf << (sd->aa.prio_item_config ? "[Item Pickup] Priorized loot over fight" : "[Item Pickup] Priorized fight over loot"); // - 0 Fight - 1 Loot
}

void handle_party_request(std::ostringstream& os_buf, map_session_data* sd) {
	os_buf << "[Auto accept party request] - " << (sd->aa.accept_party_request ? "Enabled" : "Disabled");
}

void handle_monk_combo(std::ostringstream& os_buf, map_session_data* sd) {
	os_buf << "[Monk Combo] - " << (sd->aa.monk_combo ? "Enabled" : "Disabled");
}


void handle_return_to_savepoint(std::ostringstream& os_buf, map_session_data* sd) {
	os_buf << "[Return to save point on death] - " << (sd->aa.return_to_savepoint ? "Enabled" : "Disabled");
}

bool handleAutoattack_fromitem(map_session_data* sd, t_itemid item_id, t_tick max_duration) {
	t_tick duration_ = 0;
	std::shared_ptr<item_data> id;

	switch (battle_config.feature_autoattack_duration_type) {
	case 0:
		uint32 item_expire_time;

		id = item_db.find(item_id);

		if (!id)
			return SCRIPT_CMD_FAILURE;

		item_expire_time = aa_getrental_search_inventory(sd, item_id);

		if (item_expire_time == 0) {
			ShowError("autobuff_fromitem: Item not found in the inventory or is not a consumed delay.\n", item_id);
			return SCRIPT_CMD_FAILURE;
		}

		duration_ = DIFF_TICK(item_expire_time, time(NULL));

		if (duration_ >= max_duration)
			duration_ = max_duration * 1000;
		else
			duration_ = duration_ * 1000;
		break;

	case 1:
		if (sd->aa.duration_ <= 0) {
			std::string msg = "Automessage - You don't have timer left on autoattack system!";
			aa_message(sd, "TimerOut", msg.data(), 300, nullptr);
		}
		else
			duration_ = sd->aa.duration_;
		break;
	}

	if(duration_ > 0)
		status_change_start(&sd->bl, &sd->bl, SC_AUTOATTACK, 10000, 0, 0, 0, 0, duration_, SCSTART_NOAVOID);

	return SCRIPT_CMD_SUCCESS;
}

void handleAutoHeal(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() == 5) {
		bool is_active = static_cast<bool>(std::stoi(result.at(1)));
		uint16 skill_id = static_cast<uint16>(std::stoi(result.at(2)));
		uint16 skill_lv = static_cast<uint16>(std::stoi(result.at(3)));
		uint16 min_hp = static_cast<uint16>(std::stoi(result.at(4)));
		t_tick last_use = 1;

		auto skill = skill_db.find(skill_id);
		if (!skill || (skill->nameid != AL_HEAL && skill->nameid != AB_HIGHNESSHEAL && skill->nameid != AB_CHEAL))
			return;

		if (!is_active) {
			sd->aa.autoheal.erase(
				std::remove_if(sd->aa.autoheal.begin(), sd->aa.autoheal.end(),
					[skill_id](const s_autoheal& v) {
						return v.skill_id == skill_id;
					}),
				sd->aa.autoheal.end());
			return;
		}

		auto itAutoheal = std::find_if(sd->aa.autoheal.begin(), sd->aa.autoheal.end(),
			[skill_id](const s_autoheal& v) { return v.skill_id == skill_id; });

		if (itAutoheal != sd->aa.autoheal.end()) {
			itAutoheal->is_active = is_active;
			itAutoheal->skill_lv = skill_lv;
			itAutoheal->min_hp = min_hp;
		}
		else {
			s_autoheal autoheal{ is_active, skill_id, skill_lv, min_hp, last_use };
			sd->aa.autoheal.push_back(autoheal);
		}
	}
}

void handleAutoPotion(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 5)
		return; // Invalid input

	bool is_active = static_cast<bool>(std::stoi(result.at(1)));
	t_itemid nameid = std::stoi(result.at(2));
	uint16 min_hp = static_cast<uint16>(std::stoi(result.at(3)));
	uint16 min_sp = static_cast<uint16>(std::stoi(result.at(4)));

	auto item_data = item_db.find(nameid);
	if (!item_data || item_data->type != IT_HEALING)
		return; // Item doesn't exist or isn't a healing item
	if (nameid == 11548 || nameid == 11549) {
		sd->aa.autopotion.erase(
			std::remove_if(sd->aa.autopotion.begin(), sd->aa.autopotion.end(), [nameid](const s_autopotion& v) {
				return v.item_id == nameid;
			}),
			sd->aa.autopotion.end()
		);
		return;
	}

	if (!is_active || (min_hp == 0 && min_sp == 0)) {
		sd->aa.autopotion.erase(
			std::remove_if(sd->aa.autopotion.begin(), sd->aa.autopotion.end(), [nameid](const s_autopotion& v) {
				return v.item_id == nameid;
				}),
			sd->aa.autopotion.end()
		);
	}
	else {
		auto itAutopotion = std::find_if(sd->aa.autopotion.begin(), sd->aa.autopotion.end(), [nameid](const s_autopotion& v) {
			return v.item_id == nameid;
			});

		if (itAutopotion != sd->aa.autopotion.end()) {
			itAutopotion->is_active = is_active;
			itAutopotion->min_hp = min_hp;
			itAutopotion->min_sp = min_sp;
		}
		else {
			s_autopotion autobuff_potions = { is_active, nameid, min_hp, min_sp };
			sd->aa.autopotion.push_back(autobuff_potions);
		}
	}
}

void handleAutoBuffSkills(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() == 4) {
		bool is_active = static_cast<bool>(std::stoi(result.at(1)));
		uint16 skill_id = static_cast<uint16>(std::stoi(result.at(2)));
		uint16 skill_lv = static_cast<uint16>(std::stoi(result.at(3)));

		auto skill = skill_db.find(skill_id);
		if (!skill)
			return; // Invalid skill

		if (skill_id != MO_CALLSPIRITS && skill_id != CH_SOULCOLLECT && skill_id != CH_SOULCOLLECT) { // Bypass if one of this skill
			if (!(skill->inf & (INF_SUPPORT_SKILL | INF_SELF_SKILL)))
				return; // not a support/self skill

			if (skill_get_sc(skill->nameid) == SC_NONE)
				return; // not status linked to the skill
		}

		if (!is_active) {
			sd->aa.autobuffskills.erase(
				std::remove_if(sd->aa.autobuffskills.begin(), sd->aa.autobuffskills.end(), [skill_id](const s_autobuffskills& v) {
					return v.skill_id == skill_id;
					}),
				sd->aa.autobuffskills.end()
			);
		}
		else {
			auto itBuffSkill = std::find_if(sd->aa.autobuffskills.begin(), sd->aa.autobuffskills.end(), [skill_id](const s_autobuffskills& v) {
				return v.skill_id == skill_id;
				});

			if (itBuffSkill != sd->aa.autobuffskills.end()) {
				itBuffSkill->is_active = is_active;
				itBuffSkill->skill_lv = skill_lv;
			}
			else {
				s_autobuffskills buffskills = { is_active, skill_id, skill_lv };
				sd->aa.autobuffskills.push_back(buffskills);
			}
		}
	}
}

void handleAutoAttackSkills(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() == 4) {
		bool is_active = static_cast<bool>(std::stoi(result.at(1)));
		uint16 skill_id = static_cast<uint16>(std::stoi(result.at(2)));
		uint16 skill_lv = static_cast<uint16>(std::stoi(result.at(3)));

		auto skill = skill_db.find(skill_id);
		if (!skill)
			return; // Invalid skill

		if (skill_id != KN_BRANDISHSPEAR && skill_id != CG_TAROTCARD) { // Bypass if one of this skill
			e_cast_type type = skill_get_casttype(skill_id);

			if (skill->skill_type == BF_NONE)
				return; // not a attack skill

			if (skill_get_nk(skill_id, NK_NODAMAGE))
				return; // not damage skill

			if (type == CAST_NODAMAGE && !(skill->inf & INF_SELF_SKILL))
				return; // not damage skill
		}

		if (!is_active) {
			sd->aa.autoattackskills.erase(
				std::remove_if(sd->aa.autoattackskills.begin(), sd->aa.autoattackskills.end(), [skill_id](const s_autoattackskills& v) {
					return v.skill_id == skill_id;
					}),
				sd->aa.autoattackskills.end()
			);
		}
		else {
			auto itAttackSkill = std::find_if(sd->aa.autoattackskills.begin(), sd->aa.autoattackskills.end(), [skill_id](const s_autoattackskills& v) {
				return v.skill_id == skill_id;
				});

			if (itAttackSkill != sd->aa.autoattackskills.end()) {
				itAttackSkill->is_active = is_active;
				itAttackSkill->skill_lv = skill_lv;
			}
			else {
				s_autoattackskills attackskills = { is_active, skill_id, skill_lv };
				sd->aa.autoattackskills.push_back(attackskills);
			}
		}
		aa_skill_range_calc(sd);
	}
}

void handleAutoAttackItems(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 4)
		return; // Invalid input

	bool is_active = static_cast<bool>(std::stoi(result.at(1)));
	t_itemid item_id = std::stoi(result.at(2));
	int item_status = std::stoi(result.at(3));

	if (item_status <= SC_NONE || item_status >= SC_MAX)
		return; // Invalid status

	auto item_data = item_db.find(item_id);
	if (!item_data || item_data->type != IT_USABLE)
		return; // Item doesn't exist or isn't usable

	if (!is_active) {
		sd->aa.autobuffitems.erase(
			std::remove_if(sd->aa.autobuffitems.begin(), sd->aa.autobuffitems.end(), [item_id](const s_autobuffitems& v) {
				return v.item_id == item_id;
				}),
			sd->aa.autobuffitems.end()
		);
	}
	else {
		auto itBuffItem = std::find_if(sd->aa.autobuffitems.begin(), sd->aa.autobuffitems.end(), [item_id](const s_autobuffitems& v) {
			return v.item_id == item_id;
			});

		if (itBuffItem != sd->aa.autobuffitems.end()) {
			itBuffItem->is_active = is_active;
			itBuffItem->status = item_status;
		}
		else {
			s_autobuffitems buffitems = { is_active, item_id, item_status };
			sd->aa.autobuffitems.push_back(buffitems);
		}
	}
}

void handleAutoAttackPotionState(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool state_autoattack_potions = std::stoi(result.at(1));
	sd->aa.state_potions = state_autoattack_potions;
}

void handleReturnToSavepoint(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool return_to_savepoint = std::stoi(result.at(1));
	sd->aa.return_to_savepoint = return_to_savepoint;
}

void handleResetAutoAttackConfig(map_session_data* sd) {

	sd->aa.pickup_item_id.clear();
	sd->aa.pickup_item_config = 0;
	sd->aa.prio_item_config = 0;
	sd->aa.mobs.id.clear();
	sd->aa.mobs.aggressive_behavior = 0;
	sd->aa.autobuffitems.clear();
	sd->aa.stopmelee = 0;
	sd->aa.autoattackskills.clear();
	sd->aa.autobuffskills.clear();
	sd->aa.autositregen.is_active = 0;
	sd->aa.autopotion.clear();
	sd->aa.autoheal.clear();
	sd->aa.state_potions = true;
	sd->aa.return_to_savepoint = false;
	sd->aa.token_siegfried = false;
	sd->aa.msg_list.clear();
	sd->aa.monk_combo = 0;
	sd->aa.autosellitems.clear();
	sd->aa.autobuyitems.clear();
	sd->aa.action_on_end = 0;
	sd->aa.teleport.tp_mvp = false;
	sd->aa.teleport.tp_miniboss = false;
	sd->aa.attack_skill_cd = 0;

}

void handleTokenOfSiegfried(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool autobuff_token_siegfried = std::stoi(result.at(1));
	sd->aa.token_siegfried = autobuff_token_siegfried;
}

void handleTeleportFlywing(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool tp_flywing = std::stoi(result.at(1));
	sd->aa.teleport.use_flywing = tp_flywing;
}

void handleTeleportSkill(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool tp_skill = std::stoi(result.at(1));
	sd->aa.teleport.use_teleport = tp_skill;
}

void handleTeleportMvp(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return;

	sd->aa.teleport.tp_mvp = std::stoi(result.at(1)) != 0;
}

void handleTeleportMiniboss(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return;

	sd->aa.teleport.tp_miniboss = std::stoi(result.at(1)) != 0;
}

void handleMeleeAttack(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	int melee_attack = std::stoi(result.at(1));
	sd->aa.stopmelee = melee_attack;
}

void handleAcceptPartyRequest(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool autobuff_accept_party_request = std::stoi(result.at(1));
	sd->aa.accept_party_request = autobuff_accept_party_request;
}

void handleMonkCombo(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool monk_combo = std::stoi(result.at(1));
	sd->aa.monk_combo = monk_combo;
}


void handleAutoBuy(const std::vector<std::string>& result, map_session_data* sd) {
    if (result.size() == 5) {
        int is_active = std::stoi(result.at(1));
        
        // Check for clear all command first
        if (is_active == -1) {
            // Clear all items from buy list
            sd->aa.autobuyitems.clear();
            return;
        }
        
        t_itemid item_id = std::stoi(result.at(2));
        int quantity = std::stoi(result.at(3));
        int min_trigger = std::stoi(result.at(4));
        
        auto item_data = item_db.find(item_id);
        if (!item_data)
            return; // Invalid item
        
        // Check if item is in allowed buy list
        bool valid = std::find(ai_item_buy.begin(), ai_item_buy.end(), item_id) != ai_item_buy.end();
        if (!valid)
            return; // Item not allowed for auto-buy
        
        if (!is_active) {
            // If is_active = 0, remove from list
            sd->aa.autobuyitems.erase(
                std::remove_if(sd->aa.autobuyitems.begin(), sd->aa.autobuyitems.end(),
                    [item_id](const s_autobuyitems& s) { return s.item_id == item_id; }),
                sd->aa.autobuyitems.end()
            );
            return;
        }
        
        auto it = std::find_if(sd->aa.autobuyitems.begin(), sd->aa.autobuyitems.end(),
            [item_id](const s_autobuyitems& s) { return s.item_id == item_id; });
        
        if (it != sd->aa.autobuyitems.end()) {
            it->quantity = quantity;
            it->min_trigger = min_trigger;
        } else {
            s_autobuyitems entry = {};
            entry.item_id = item_id;
            entry.quantity = quantity;
            entry.min_trigger = min_trigger;
            sd->aa.autobuyitems.push_back(entry);
        }
    }
}

void handleAutoSell(const std::vector<std::string>& result, map_session_data* sd) {
    if (result.size() == 3) {
        int is_active = std::stoi(result.at(1));
        
        // Check for clear all command first
        if (is_active == -1) {
            // Clear all items from sell list
            sd->aa.autosellitems.clear();
            return;
        }
        
        t_itemid item_id = std::stoi(result.at(2));
        
        auto item_data = item_db.find(item_id);
        if (!item_data)
            return; // Invalid item
        
        if (!is_active) {
            // If is_active = 0, remove from list
            sd->aa.autosellitems.erase(
                std::remove_if(sd->aa.autosellitems.begin(), sd->aa.autosellitems.end(),
                    [item_id](const s_autosellitems& s) { return s.item_id == item_id; }),
                sd->aa.autosellitems.end()
            );
            return;
        }
        
        // Check if item already exists
        auto it = std::find_if(sd->aa.autosellitems.begin(), sd->aa.autosellitems.end(),
            [item_id](const s_autosellitems& s) { return s.item_id == item_id; });
        
        if (it == sd->aa.autosellitems.end()) {
            // Check limit (max 10 items)
            if (sd->aa.autosellitems.size() >= 10) {
                clif_displaymessage(sd->fd, "Auto-sell list is full (maximum 10 items).");
                return;
            }
            
            s_autosellitems entry = {};
            entry.item_id = item_id;
            sd->aa.autosellitems.push_back(entry);
        }
        // If already exists, no need to update (only has item_id)
    }
}

void handleIgnoreAggressiveMonster(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool aggressive_behavior = std::stoi(result.at(1));
	sd->aa.mobs.aggressive_behavior = aggressive_behavior;
}

void handleItemPickupConfiguration(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	int pickup_item_config = std::stoi(result.at(1));
	sd->aa.pickup_item_config = pickup_item_config;
}

void handleActionOnEnd(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	int action_on_end = std::stoi(result.at(1));
	sd->aa.action_on_end = action_on_end;
}

void handleMonsterSurround(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	int monster_surround = std::stoi(result.at(1));
	sd->aa.monster_surround = monster_surround;
}

void handleItemPickupSelection(const std::vector<std::string>& result, map_session_data* sd) {
	if(result.size() == 3){ // id = 9 - item pickup (is_active;item_id)
		int i;
		int status = std::stoi(result.at(1));
		t_itemid nameid = std::stoi(result.at(2));
		bool item_found = false;
		std::shared_ptr<item_data> item_data;

		switch(status){
			case -1:
				sd->aa.pickup_item_id.clear();
				break;
			case 0:
				if(!sd->aa.pickup_item_id.empty()){
					for(i=0;i<sd->aa.pickup_item_id.size(); i++){
						if(sd->aa.pickup_item_id.at(i) == nameid){
							sd->aa.pickup_item_id.erase(sd->aa.pickup_item_id.begin()+i);
							break;
						}
					}
				}
				break;
			case 1:
				//check if item exist
				if( ( item_data = item_db.find(nameid) ) == NULL )
					return;

				if(!sd->aa.pickup_item_id.empty()){
					for(i=0;i<sd->aa.pickup_item_id.size(); i++){
						if(sd->aa.pickup_item_id.at(i) == nameid){
							item_found = true;
							break;
						}
					}
				}
				if(!item_found)
					sd->aa.pickup_item_id.push_back(nameid);

				break;
		}
	}
}

void handleMonsterSelection(const std::vector<std::string>& result, map_session_data* sd) {
    if (result.size() != 3)
        return; // Invalid input

    uint32_t mob_id = std::stoul(result.at(1));
    bool active = std::stoi(result.at(2));

    auto& mob_ids = sd->aa.mobs.id;
    auto it = std::find(mob_ids.begin(), mob_ids.end(), mob_id);

    if (active) {
        if (it == mob_ids.end()) {
            // Add the mob_id if not already present
            mob_ids.push_back(mob_id);
        }
    } else {
        if (it != mob_ids.end()) {
            // Remove the mob_id if it exists
            mob_ids.erase(it);
        }
    }
}

void handlePriorizeLootFight(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	bool autobuff_priorizelootfight = std::stoi(result.at(1));
	sd->aa.prio_item_config = autobuff_priorizelootfight;
}

void handleSitRestHp(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 3)
		return; // Invalid input

	int min_hp = std::stoi(result.at(1));
	int max_hp = std::stoi(result.at(2));

	if (min_hp == 0 && sd->aa.autositregen.min_sp == 0)
		sd->aa.autositregen.is_active = 0;
	else
		sd->aa.autositregen.is_active = 1;

	sd->aa.autositregen.min_hp = min_hp;
	sd->aa.autositregen.max_hp = max_hp;
}

void handleSitRestSp(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 3)
		return; // Invalid input

	int min_sp = std::stoi(result.at(1));
	int max_sp = std::stoi(result.at(2));

	if (min_sp == 0 && sd->aa.autositregen.min_hp == 0)
		sd->aa.autositregen.is_active = 0;
	else
		sd->aa.autositregen.is_active = 1;

	sd->aa.autositregen.min_sp = min_sp;
	sd->aa.autositregen.max_sp = max_sp;
}

void handleTeleportMinHp(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	int tp_min_hp = std::stoi(result.at(1));

	sd->aa.teleport.min_hp = tp_min_hp;
}

void handleTeleportNoMobMeet(const std::vector<std::string>& result, map_session_data* sd) {
	if (result.size() != 2)
		return; // Invalid input

	int tp_delay_nomobmeet = std::stoi(result.at(1));

	sd->aa.teleport.delay_nomobmeet = tp_delay_nomobmeet;
}

extern int handleGetautoattackint(map_session_data* sd, const int value) {
	int result = 0;

	switch (value) {
	case 0:
		result = static_cast<int>(sd->aa.autoheal.size());
		break;
	case 2:
		result = static_cast<int>(sd->aa.autoattackskills.size());
		break;
	case 3:
		result = static_cast<int>(sd->aa.autobuffskills.size());
		break;
	case 4:
		result = sd->aa.state_potions;
		break;
	case 5:
		result = static_cast<int>(sd->aa.autobuffitems.size());
		break;
	case 6:
		result = sd->aa.return_to_savepoint;
		break;
	case 7:
		result = sd->aa.token_siegfried;
		break;
	case 8:
		result = sd->aa.accept_party_request;
		break;
	case 9:
		result = sd->aa.pickup_item_config;
		break;
	case 10:
		result = sd->aa.prio_item_config;
		break;
	case 11:
		result = sd->aa.stopmelee;
		break;
	case 12:
		result = sd->aa.teleport.use_flywing;
		break;
	case 13:
		result = sd->aa.teleport.use_teleport;
		break;
	case 14:
		if (sd->aa.autositregen.is_active) {
			switch (sd->aa.autositregen.min_hp) {
			case 75: result = 1;
				break;
			case 50: result = 2;
				break;
			case 25: result = 3;
				break;
			default:
				result = 0;
				break;
			}
		} else
			result = sd->aa.autositregen.is_active;
		break;
	case 15:
		if (sd->aa.autositregen.is_active) {
			switch (sd->aa.autositregen.min_sp) {
				case 75 : result = 1;
					break;
				case 50 : result = 2;
					break;
				case 25 : result = 3;
					break;
				default:
					result = 0;
					break;
			}
		} else
			result = sd->aa.autositregen.is_active;
		break;
	case 16:
		result = sd->aa.mobs.aggressive_behavior;
		break;
	case 17:
		result = static_cast<int>(sd->aa.pickup_item_id.size());
		break;
	case 18:
		result = sd->aa.action_on_end;
		break;
	case 19:
		result = sd->aa.monk_combo;
		break;
	case 20:
		result = static_cast<int>(sd->aa.autobuyitems.size());
		break;
	case 21:
		result = static_cast<int>(sd->aa.autosellitems.size());
		break;
	case 22:
		result = sd->aa.teleport.tp_mvp;
		break;
	case 23:
		result = sd->aa.teleport.tp_miniboss;
		break;
	default:
		return result;
		break;
	}

	return result;
}
