 #include "autobuff.hpp"
 #include "battle.hpp"
 #include "log.hpp"
 #include "party.hpp"
 #include "pc.hpp"
 #include "status.hpp"
 #include "unit.hpp"
 #include "clif.hpp"
 
 #include <vector>
 #include <map>
 
 #include <common/nullpo.hpp>
 #include <common/socket.hpp>
 #include <common/showmsg.hpp>
 #include <common/utils.hpp>
 
 using namespace rathena;
 
 // Private message command processing
 int processPrivateCommands(map_session_data* sd, enum sc_type type,
 	map_session_data* leader_sd, party_data* p) {
 	if (sd->ab.order_msg.empty() || sd->ab.allow_pm_cmd == AB_PM_DISABLEPM)
 		return -1;
 
 	auto& front = sd->ab.order_msg.front();
 	auto [text, sender] = front;
 	if (sender == sd) {
 		std::string msg = "Autobuff : you can't send message to yourself";
 		ab_partymessage(sd, "Cmd", msg.data(), 5);
 		sd->ab.order_msg.pop_front();
 		return 0;
 	}
 
 	std::istringstream iss(text);
 	std::vector<std::string> words{ std::istream_iterator<std::string>{iss}, {} };
 	std::ostringstream err;
 
 	switch (sd->ab.allow_pm_cmd) {
 	case AB_PM_ALLOWPMFROMLEADER:
 		if (!leader_sd || sender->status.char_id != leader_sd->status.char_id)
 			err << "Autobuff : Only the leader of the party can command me!";
 		break;
 	case AB_PM_ALLOWPM: {
 		bool in_party = std::any_of(std::begin(p->party.member), std::end(p->party.member),
 			[&](auto& m) { return m.char_id == sender->status.char_id; });
 		if (!in_party)
 			err << "Autobuff : Only party members can command me!";
 	}   break;
 	case AB_PM_NAMEDPLAYER: {
 		bool in_party = std::any_of(std::begin(p->party.member), std::end(p->party.member),
 			[&](auto& m) { return m.char_id == sender->status.char_id; });
 		bool named_ok = std::any_of(sd->ab.allow_pm_char_id.begin(), sd->ab.allow_pm_char_id.end(),
 			[&](int id) { return id == sender->status.char_id; });
 		if (!in_party || !named_ok)
 			err << "Autobuff : Only specific members of the party can command me!";
 	}   break;
 	default: break;
 	}
 
 	bool should_pop = true;
 
 	if (err.tellp()) {
 		ab_partymessage(sd, "Cmd", err.str().data(), 5);
 	}
 	else {
 		ab_commandHandler handler;
 		if (auto it = handler.command_map.find(words[0]); it != handler.command_map.end()) {
 			int cmd_result = it->second(sd, sender, words);
 			if (cmd_result == 2)
 				should_pop = false;
 		}
 		else {
 			std::string msg = "Autobuff : Unknown command";
 			ab_partymessage(sd, "Cmd", msg.data(), 5);
 		}
 	}
 
 	if (should_pop && !sd->ab.order_msg.empty())
 		sd->ab.order_msg.pop_front();
 
 	return 0;
 }
 
 // Follow (movement) logic restored with full thresholds
 void processFollow(bool skip, map_session_data* sd, party_data* p) {
 	if (skip || sd->state.ab_stay || sd->state.ab_stop || pc_issit(sd) || pc_isdead(sd)
 		|| sd->ab.following_player <= 0)
 		return;
 
 	auto* target = map_charid2sd(sd->ab.following_player);
 	if (!target) {
 		std::string msg = "Autobuff : Player to follow is offline, I don't follow anyone !";
 		ab_partymessage(sd, "FollowPlayerOffline", msg.data(), 600);
 		sd->state.ab_stay = true;
 		return;
 	}
 	bool in_party = std::any_of(std::begin(p->party.member), std::end(p->party.member),
 		[&](auto& m) { return m.char_id == target->status.char_id; });
 	if (!in_party) {
 		std::string msg = "Autobuff : Player to follow is offline, I don't follow anyone !";
 		ab_partymessage(sd, "FollowPlayerOffline", msg.data(), 600);;
 		return;
 	}
 
 	int dist = distance_bl(&target->bl, &sd->bl);
 	// Different map
 	if (target->bl.m != sd->bl.m) {
 		if (battle_config.feature_autobuff_teleportiflost >= 2 || sd->ab.autositregen.status || (sd->ab.emergency_tp && sd->ab.emergency_timer >= gettick())) {
 			sd->ab.autositregen.status = false;
 			sd->ab.emergency_tp = false;
 			sd->ab.emergency_timer = 0;
 			if (pc_setpos(sd, target->mapindex, target->bl.x, target->bl.y, CLR_TELEPORT) == SETPOS_OK) {
 				pc_delinvincibletimer(sd);
 				if (sd->state.autotrade)
 					clif_parse_LoadEndAck(sd->fd, sd);
 			}
 		}
 		else {
 			std::string msg = "Autobuff : too far away from the autobuff - You're on a different map!";
 			ab_partymessage(sd, "TooFarFromFollow", msg.data(), 600);
 		}
 	}
 	// Teleport if too far (>14)
 	else if (dist > 14) {
 		if (battle_config.feature_autobuff_teleportiflost >= 1 || sd->ab.autositregen.status || (sd->ab.emergency_tp && sd->ab.emergency_timer >= gettick())) {
 			sd->ab.autositregen.status = false;
 			sd->ab.emergency_tp = false;
 			sd->ab.emergency_timer = 0;
 			if (pc_setpos(sd, target->mapindex, target->bl.x, target->bl.y, CLR_TELEPORT) == SETPOS_OK && sd->state.autotrade)
 				clif_parse_LoadEndAck(sd->fd, sd);
 		}
 		else {
 			std::string msg = "Autobuff : too far away from the autobuff";
 			ab_partymessage(sd, "TooFarFromFollow", msg.data(), 600);
 		}
 	}
 	// Walk to maintain distance
 	else if (dist > sd->ab.dist_to_leader && (!sd->ab.emergency_timer || (sd->ab.emergency_tp && sd->ab.emergency_timer >= gettick())) ) {
 		int dest_x = target->bl.x - sd->ab.dist_to_leader * dirx[target->ud.dir];
 		int dest_y = target->bl.y - sd->ab.dist_to_leader * diry[target->ud.dir];
 		bool walked = false;
 		if ((abs(dest_x - sd->bl.x) > 2) || (abs(dest_y - sd->bl.y) > 2)) {
 			if (!target->ud.walktimer)
 				walked = ab_walk(&sd->bl, dest_x + rand() % 5 - 2, dest_y + rand() % 5 - 2, 8);
 			else
 				walked = ab_walk(&sd->bl, dest_x, dest_y, 8);
 		}
 		if (!walked && (abs(sd->bl.x - target->bl.x) > 6 || abs(sd->bl.y - target->bl.y) > 6)) {
 			walkpath_data wpd;
 			if (path_search(&wpd, sd->bl.m, sd->bl.x, sd->bl.y,
 				target->bl.x, target->bl.y, 0, CELL_CHKNOPASS)) {
 				ab_walk(&sd->bl,
 					sd->bl.x + dirx[wpd.path[0]],
 					sd->bl.y + diry[wpd.path[0]],
 					8);
 			}
 		}
 		sd->ab.autositregen.status = false;
 		sd->ab.emergency_tp = false;
 		sd->ab.emergency_timer = 0;
 	}
 }
 
 // Resurrection logic with full GvG exclusions and gem checks
 void processResurrection(map_session_data* sd, t_tick last_tick, bool& skip) {
 	if (skip || !sd->ab.autobuff_resurection || pc_checkskill(sd, ALL_RESURRECTION) <= 0)
 		return;
 	// Exclude all GvG zones
 	if (map_getmapflag(sd->bl.m, MF_GVG) || map_getmapflag(sd->bl.m, MF_GVG_TE)
 		|| map_getmapflag(sd->bl.m, MF_GVG_CASTLE) || map_getmapflag(sd->bl.m, MF_GVG_TE_CASTLE)
 		|| map_getmapflag(sd->bl.m, MF_BATTLEGROUND))
 		return;
 
 	int lvl = pc_checkskill(sd, ALL_RESURRECTION);
 	int gem_idx = pc_search_inventory(sd, ITEMID_BLUE_GEMSTONE);
 	int gems = gem_idx >= 0 ? sd->inventory.u.items_inventory[gem_idx].amount : 0;
 	if (gems < 8) {
 		std::string msg = "I'm low on Blue Gemstones!";
 		ab_partymessage(sd, "LowGemstone", msg.data(), 100);
 	}
 
 	if (!ab_canuseskill(sd, ALL_RESURRECTION, lvl))
 		return;
 
 	int dead_id = 0;
 	map_foreachinarea(ab_targetresu, sd->bl.m,
 		sd->bl.x - AREA_SIZE, sd->bl.y - AREA_SIZE,
 		sd->bl.x + AREA_SIZE, sd->bl.y + AREA_SIZE,
 		BL_PC, &dead_id, sd->bl.id);
 	if (dead_id <= 0)
 		return;
 
 	auto* target = map_id2sd(dead_id);
 	if (!target || gems <= 0) {
 		std::string msg = "I'm out of Blue Gemstones!";
 		ab_partymessage(sd, "OutGemstone", msg.data(), 100);
 		return;
 	}
 
 	auto skill = skill_db.find(ALL_RESURRECTION);
 	int ab_range = (skill->inf & INF_SELF_SKILL) ? 10
 		: skill_get_range(ALL_RESURRECTION, lvl);
 	ab_range = (ab_range == 0) ? 2 : abs(ab_range);
 
 	if (!check_distance(target->bl.x - sd->bl.x, target->bl.y - sd->bl.y, ab_range)
 		|| !path_search_long(nullptr, sd->bl.m, sd->bl.x, sd->bl.y,
 			target->bl.x, target->bl.y, CELL_CHKWALL)) {
 		if (!unit_walktobl(&sd->bl, &target->bl, ab_range, 1))
 			return;
 		skip = true;
 		return;
 	}
 
 	if (unit_skilluse_id(&sd->bl, dead_id, ALL_RESURRECTION, lvl)) {
 		skip = true;
 		skill_consume_requirement(sd, ALL_RESURRECTION, lvl, 2);
 		sd->ab.skill_cd = std::max(sd->ab.skill_cd, last_tick + std::max(battle_config.feature_autobuff_bskill_delay, skill_delayfix(&sd->bl, ALL_RESURRECTION, lvl)));
 	}
 }
 
 // Automatically sit to rest or stand when conditions are met
 bool ab_status_rest(map_session_data* sd, const status_data* status, t_tick last_tick) {
 	if (!sd->ab.autositregen.is_active)
 		return false;
 
 	uint16 overweight_percent = 0;
 	if (sd->max_weight > 0)
 		overweight_percent = (uint16)(sd->weight * 100 / sd->max_weight);
 	bool is_overweight = (overweight_percent >= 90);
 
 	int hp_percentage = (status->hp * 100) / status->max_hp;
 	int sp_percentage = (status->sp * 100) / status->max_sp;
 
 	bool needs_hp_regen = (sd->ab.autositregen.min_hp > 0 && hp_percentage < sd->ab.autositregen.min_hp);
 	bool needs_sp_regen = (sd->ab.autositregen.min_sp > 0 && sp_percentage < sd->ab.autositregen.min_sp);
 	bool can_sit = !pc_issit(sd) && (needs_hp_regen || needs_sp_regen) && !is_overweight;
 
 	bool regen_complete = true;
 	if (sd->ab.autositregen.min_hp > 0 && hp_percentage < sd->ab.autositregen.max_hp)
 		regen_complete = false;
 	if (sd->ab.autositregen.min_sp > 0 && sp_percentage < sd->ab.autositregen.max_sp)
 		regen_complete = false;
 	bool can_stand = pc_issit(sd) && (regen_complete || is_overweight);
 
 	if (can_sit) {
 		sd->ab.autositregen.status = true;
 		pc_setsit(sd);
 		skill_sit(sd, 1);
 		clif_sitting(&sd->bl);
 	}
 	else if (can_stand && pc_setstand(sd, false)) {
 		skill_sit(sd, 0);
 		clif_standing(&sd->bl);
 	}
 
 	return true;
 }
 
 void ab_action_after_hit(map_session_data* sd, struct block_list* src) {
 	struct status_data* status = status_get_status_data(&sd->bl);
 	if (sd->state.autobuff) {
 
 		// Teleport condition if hp is bellow limit
 		if (sd->ab.emergency_tp_perc // player must have set min hp value to tp
 			&& (status->hp * 100 / sd->ab.emergency_tp_perc) < sd->status.max_hp) { // player hp is bellow min hp
 
 			if (ab_teleport(sd))
 				return;
 		}
 	}
 }
 
 void ab_action_before_hit(map_session_data* sd, struct block_list* src, uint32 damage) {
 	struct status_data* status = status_get_status_data(&sd->bl);
 	if (sd->state.autobuff) {
 		if (sd->ab.emergency_tp_damage && damage > sd->ab.emergency_tp_damage) {
 			if (ab_teleport(sd))
 				return;
 		}
 	}
 }
 
 
 void ab_buyitem(struct map_session_data *sd) {
 	// Auto Buy Items
 	if (sd->ab.autobuyitems.size()) {
 		
 		// Get MC_DISCOUNT skill level
 		int discount_lv = pc_checkskill(sd, MC_DISCOUNT);
 		
 		for (const auto& entry : sd->ab.autobuyitems) {
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
 					
 					// ลด๏ฟฝาค๏ฟฝ (ลบ %)
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
 
 
 void ab_sellitem(struct map_session_data *sd) {
 	// Auto-sell items when item count reaches threshold
 	
 	if (battle_config.feature_autoattack_sell_count > 0 && !sd->ab.autosellitems.empty()) {
 		
 		// Get MC_OVERCHARGE skill level
 		int overcharge_lv = pc_checkskill(sd, MC_OVERCHARGE);
 		
 		for (const auto& entry : sd->ab.autosellitems) {
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
 
 //Brain
 int ab_status(map_session_data* sd, enum sc_type type) {
 	auto handle_error = [&](std::string msg) {
 		status_change_end(&sd->bl, type);
 		ab_partymessage(sd, "Error", msg.data(), 1);
 		return 0;
 		};
 
 	auto* p = sd->status.party_id ? party_search(sd->status.party_id) : nullptr;
 	if (!p) {
 		status_change_end(&sd->bl, type);
 		clif_displaymessage(sd->fd, "Autobuff : OFF - You can't enable it when you're not in a party");
 		return 0;
 	}
 
 	if (battle_config.feature_autobuff_duration_type) {
 		if (sd->ab.duration_ <= 0) {
 			std::string msg = "Automessage - You don't have timer left on autobuff system!";
 			ab_partymessage(sd, "TimerOut", msg.data(), 5);
 			return -1;
 		}
 
 		sd->ab.duration_ = sd->ab.duration_ - battle_config.feature_autobuff_timer;
 		pc_setaccountreg(sd, add_str("#ab_duration"), sd->ab.duration_);
 	}
 
 	bool skip = pc_cant_act(sd) || pc_isdead(sd);
 
 	// Leader & online count
 	map_session_data* leader_sd = nullptr;
 	int player_pos = -1, leader_pos = -1, nb_online = 0;
 	for (int i = 0; i < MAX_PARTY; ++i) {
 		auto& m = p->party.member[i];
 		if (!m.char_id) continue;
 		auto* msd = map_charid2sd(m.char_id);
 		if (!msd) continue;
 		if (msd->status.char_id != sd->status.char_id) ++nb_online;
 		else player_pos = i;
 		if (party_isleader(msd)) {
 			leader_sd = msd;
 			leader_pos = i;
 		}
 	}
 	if (!nb_online && !sd->ab.autobuff_disable_alone)
 		return handle_error("Autobuff : OFF - You're the only online player in the party");
 
 	// Class check
 	int base = sd->class_ & MAPID_BASEMASK;
 	int upper = sd->class_ & MAPID_UPPERMASK;
 	if (base != MAPID_ACOLYTE && upper != MAPID_CRUSADER
 		&& upper != MAPID_BARDDANCER && upper != MAPID_ALCHEMIST)
 		return handle_error("Autobuff : OFF - Only an acolyte, crusader, bard, dancer and alchemist base job can use autobuff");
 
 	if (sd->regen.state.overweight) {
 		std::string msg = "Autobuff : I'm overweight !";
 		ab_partymessage(sd, "Overweight", msg.data(), 300);
 	}
 
 	// PM disabled purge + message
 	if (!sd->ab.order_msg.empty() && sd->ab.allow_pm_cmd == AB_PM_DISABLEPM) {
 		std::string msg = "Autobuff : Private message command is disabled";
 		ab_partymessage(sd, "Cmd", msg.data(), 300);
 		sd->ab.order_msg.clear();
 	}
 
 	// Private commands
 	processPrivateCommands(sd, type, leader_sd, p);
 
 	//check if logout from command
 	if (!sd)
 		return 0;
 
 	t_tick last_tick = gettick();
 	struct status_data* curent_status = status_get_status_data(&sd->bl);
 	ab_status_rest(sd, curent_status, last_tick);
 
 	// Skip if he received stop order or is sit or dead
 	if (sd->state.ab_stop || pc_issit(sd))
 		skip = true;
 
 	//Auto But Item
 	ab_buyitem(sd);
 	
 	//Auto Sell Item
 	ab_sellitem(sd);
 
 	// Resurrection
 	processResurrection(sd, last_tick, skip);
 
 	// Movement (follow)
 	processFollow(skip, sd, p);
 
 	/* Auto buff skills */
 	if (sd->ab.priorize_buff) {
 		auto_buff_action(skip, sd, leader_sd, p, leader_pos);
 		auto_heal_action(skip, sd, leader_sd, p, leader_pos);
 	}
 	else {
 		auto_heal_action(skip, sd, leader_sd, p, leader_pos);
 		auto_buff_action(skip, sd, leader_sd, p, leader_pos);
 	}
 
 	/* Auto buff items */
 	if (!skip && !sd->ab.autobuff_buffitems.empty()) {
 		for (auto& itAutobuffitem : sd->ab.autobuff_buffitems) {
 			if (!sd->sc.getSCE(itAutobuffitem.status)) {
 				int i_ = pc_search_inventory(sd, itAutobuffitem.item_id);
 				if (i_ >= 0)
 					pc_useitem(sd, i_);
 			}
 		}
 	}
 
 	// Auto buff potions
 	if (!skip && sd->ab.state_autobuff_potions && !sd->ab.autobuff_potions.empty()) {
 		for (auto& potion : sd->ab.autobuff_potions) {
 			auto check_and_use_potion = [&](int current_stat, int max_stat, int threshold) {
 				if (get_percentage(current_stat, max_stat) < threshold) {
 					int index = pc_search_inventory(sd, potion.item_id);
 					if (index >= 0) {
 						pc_useitem(sd, index);
 					}
 				}
 				};
 
 			// Check HP and SP conditions
 			check_and_use_potion(sd->battle_status.hp, sd->battle_status.max_hp, potion.min_hp);
 			check_and_use_potion(sd->battle_status.sp, sd->battle_status.max_sp, potion.min_sp);
 		}
 	}
 
 	return 1;
 }
 
 void auto_heal_action(bool& skip, map_session_data* sd, map_session_data* leader_sd, struct party_data* p, int leader_pos) {
 	std::shared_ptr<s_skill_db> skill;
 	t_tick last_tick = gettick();
 	time_t last_time = time(NULL);
 
 	if (skip || sd->ab.autobuff_heal.empty() || last_tick < sd->ab.skill_cd)
 		return;
 
 	for (auto& heal : sd->ab.autobuff_heal) {
 		if (skip) break;
 
 		if (!heal.is_active || !ab_canuseskill(sd, heal.skill_id, heal.skill_lv) || pc_checkskill(sd, heal.skill_id) <= 0)
 			continue;
 
 		skill = skill_db.find(heal.skill_id);
 		bool heal_used = false;
 		bool can_walk = true;
 
 		const bool is_self_skill = skill->inf & INF_SELF_SKILL;
 		const bool is_potionpitcher5 = (heal.skill_id == AM_POTIONPITCHER && heal.skill_lv == 5);
 		int ab_skill_range = skill_get_range(heal.skill_id, heal.skill_lv);
 		ab_skill_range = (ab_skill_range == 0) ? 2 : std::abs(ab_skill_range);
 		if (is_self_skill) ab_skill_range = 10;
 
 		auto should_heal = [&](const map_session_data* target) -> bool {
 			if (!target || target->bl.m != sd->bl.m || pc_isdead(target))
 				return false;
 			int current = is_potionpitcher5 ? target->battle_status.sp : target->battle_status.hp;
 			int maximum = is_potionpitcher5 ? target->battle_status.max_sp : target->battle_status.max_hp;
 			return get_percentage(current, maximum) < heal.min_hp;
 		};
 
 		auto use_heal_skill = [&](map_session_data* target) -> bool {
 			if (heal.skill_id == PR_SANCTUARY) {
 				int gem_idx = pc_search_inventory(sd, ITEMID_BLUE_GEMSTONE);
 				int gems = gem_idx >= 0 ? sd->inventory.u.items_inventory[gem_idx].amount : 0;
 				if (gems <= 0) {
 					std::string msg = "I'm out of Blue Gemstones!";
 					ab_partymessage(sd, "OutGemstone", msg.data(), 100);
 					return false;
 				}
 				else if (gems < 8) {
 					std::string msg = "I'm low on Blue Gemstones!";
 					ab_partymessage(sd, "LowGemstone", msg.data(), 100);
 				}
 			}
 			if (heal.skill_id == CR_SLIMPITCHER || heal.skill_id == PR_SANCTUARY)
 				return unit_skilluse_pos(&sd->bl, target->bl.x, target->bl.y, heal.skill_id, heal.skill_lv);
 			else
 				return unit_skilluse_id(&sd->bl, is_self_skill ? sd->bl.id : target->bl.id, heal.skill_id, heal.skill_lv);
 		};
 
 		auto try_heal_target = [&](map_session_data* target) -> bool {
 			if (!check_distance_bl(&sd->bl, &target->bl, ab_skill_range) ||
 				!path_search_long(nullptr, sd->bl.m, sd->bl.x, sd->bl.y, target->bl.x, target->bl.y, CELL_CHKWALL)) {
 				can_walk = unit_walktobl(&sd->bl, &target->bl, ab_skill_range, 1);
 				if (!can_walk) return false;
 				skip = true;
 				return false;
 			}
 			heal_used = use_heal_skill(target);
 			return heal_used;
 		};
 
 		switch (heal.priorize_heal) {
 		case AB_SELF:
 			if (should_heal(sd)) {
 				heal_used = use_heal_skill(sd);
 			}
 			break;
 
 		case AB_LEADER:
 			if (leader_sd && leader_sd->bl.m == sd->bl.m && !pc_isdead(leader_sd)) {
 				if (should_heal(leader_sd))
 					try_heal_target(leader_sd);
 			}
 			break;
 
 		case AB_NAMEDPLAYER:
 			for (const auto& char_id : heal.priorize_heal_char_id) {
 				map_session_data* target_sd = nullptr;
 
 				for (int i = 0; i < MAX_PARTY; i++) {
 					if (p->party.member[i].char_id == char_id) {
 						target_sd = map_charid2sd(char_id);
 						break;
 					}
 				}
 				if (!target_sd || !should_heal(target_sd)) continue;
 				if (try_heal_target(target_sd)) break;
 			}
 			break;
 
 		case AB_ALL:
 		case AB_EVERYONEEXCEPTSELF:
 			for (int i = 0; i < MAX_PARTY; i++) {
 				map_session_data* target_sd = map_charid2sd(p->party.member[i].char_id);
 				if (!target_sd || (heal.priorize_heal == AB_EVERYONEEXCEPTSELF && target_sd == sd)) continue;
 				if (!should_heal(target_sd)) continue;
 				if (try_heal_target(target_sd)) break;
 			}
 			break;
 		}
 
 		if (heal_used) {
 			skip = true;
 			sd->idletime = last_time;
 			skill_consume_requirement(sd, heal.skill_id, heal.skill_lv, 2);
 			sd->ab.skill_cd = std::max(sd->ab.skill_cd, last_tick + std::max(battle_config.feature_autobuff_bskill_delay, skill_delayfix(&sd->bl, heal.skill_id, heal.skill_lv)));
 		}
 	}
 }
 
 void auto_buff_action(bool& skip, map_session_data* sd, map_session_data* leader_sd, struct party_data* p, int leader_pos) {
 	std::shared_ptr<s_skill_db> skill;
 	t_tick last_tick = gettick();
 	bool encore = false, song = false;
 
 #ifndef RENEWAL
 	if (sd->sc.getSCE(SC_DANCING)) {
 		unit_skilluse_id(&sd->bl, sd->bl.id, BD_ADAPTATION, 1);
 	}
 #endif
 
 	if (skip || sd->ab.autobuff_buffskills.empty() || last_tick < sd->ab.skill_cd)
 		return;
 
 	for (auto& buff : sd->ab.autobuff_buffskills) {
 		if (skip) break;
 
 		if (!buff.is_active || !ab_canuseskill(sd, buff.skill_id, buff.skill_lv) || pc_checkskill(sd, buff.skill_id) <= 0)
 			continue;
 
 		skill = skill_db.find(buff.skill_id);
 		int ab_skill_range = (skill->inf & INF_SELF_SKILL) ? 10 : skill_get_range(buff.skill_id, buff.skill_lv);
 		ab_skill_range = ab_skill_range == 0 ? 2 : std::abs(ab_skill_range);
 
 		bool buff_used = false;
 		bool can_walk = true;
 
 		if (skill->inf2[INF2_ISSONG] || skill->inf2[INF2_ISENSEMBLE]) {
 			song = true;
 			encore = (sd->skill_id_dance == buff.skill_id && sd->skill_lv_dance == buff.skill_lv);
 		}
 
		auto needs_buff = [&](map_session_data* target) -> bool {
			if (!target || pc_isdead(target) || target->bl.m != sd->bl.m)
				return false;
			if (buff.skill_id == CR_FULLPROTECTION) {
				const unsigned int equipment[] = { EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP };
				const sc_type protections[] = { SC_CP_WEAPON, SC_CP_SHIELD, SC_CP_ARMOR, SC_CP_HELM };

				for (size_t i = 0; i < ARRAYLENGTH(equipment); ++i) {
					if (pc_checkequip(target, equipment[i]) >= 0 && !target->sc.getSCE(protections[i]))
						return true;
				}
				return false;
			}
			return !target->sc.getSCE(skill_get_sc(buff.skill_id));
			};
 
 		auto valid_target = [&](map_session_data* target) -> bool {
 			if (!target) return false;
 			if (target->bl.m != sd->bl.m || pc_isdead(target) ) return false;
 			if (target == sd && skill->inf2[INF2_NOTARGETSELF]) return false;
 			return true;
 			};
 
 		auto use_buff_on_target = [&](map_session_data* target) -> bool {
 			if (!check_distance(sd->bl.x - target->bl.x, sd->bl.y - target->bl.y, ab_skill_range) ||
 				!path_search_long(nullptr, sd->bl.m, sd->bl.x, sd->bl.y, target->bl.x, target->bl.y, CELL_CHKWALL)) {
 				can_walk = unit_walktobl(&sd->bl, &target->bl, ab_skill_range, 1);
 				if (!can_walk) {
 					skip = true;
 					return false;
 				}
 			}
 			buff_used = unit_skilluse_id(
 				&sd->bl,
 				(skill->inf & INF_SELF_SKILL) ? sd->bl.id : target->bl.id,
 				encore ? BD_ENCORE : buff.skill_id,
 				encore ? 1 : buff.skill_lv
 			);
 			return buff_used;
 			};
 
 		switch (buff.priorize_buff) {
 		case AB_SELF:
 			if (needs_buff(sd)) {
 				buff_used = unit_skilluse_id(&sd->bl, sd->bl.id, encore ? BD_ENCORE : buff.skill_id, encore ? 1 : buff.skill_lv);
 			}
 			break;
 
 		case AB_LEADER:
 			if (leader_sd && valid_target(leader_sd) && needs_buff(leader_sd)) {
 				use_buff_on_target(leader_sd);
 			}
 			break;
 
 		case AB_NAMEDPLAYER:
 			for (auto char_id : buff.priorize_buff_char_id) {
 				map_session_data* target_sd = map_charid2sd(char_id);
 				if (!valid_target(target_sd) || !needs_buff(target_sd)) continue;
 				if (use_buff_on_target(target_sd)) break;
 			}
 			break;
 
 		case AB_ALL:
 		case AB_EVERYONEEXCEPTSELF:
 			for (int i = 0; i < MAX_PARTY; ++i) {
 				map_session_data* target_sd = map_charid2sd(p->party.member[i].char_id);
 				if (!valid_target(target_sd)) continue;
 				if (buff.priorize_buff == AB_EVERYONEEXCEPTSELF && target_sd == sd) continue;
 				if (needs_buff(target_sd) && use_buff_on_target(target_sd)) break;
 			}
 			break;
 		}
 
 		if (buff_used) {
 			skip = true;
 			sd->ab.skill_cd = std::max(sd->ab.skill_cd, last_tick + std::max(battle_config.feature_autobuff_bskill_delay, skill_delayfix(&sd->bl, buff.skill_id, buff.skill_lv)));
 		}
 	}
 }
 
 
 void ab_token_respawn(block_list* target, int flag) {
 	if (target->type == BL_PC && flag) {
 		map_session_data* ab_psd = (TBL_PC*)target;
 		if (ab_psd && ab_psd->sc.getSCE(SC_AUTOBUFF)) {
 			if (ab_psd->ab.autobuff_token_siegfried)
 				pc_revive_item(ab_psd);
 			else if (ab_psd->ab.return_to_savepoint)
 				pc_respawn(ab_psd, CLR_OUTSIGHT);
 		}
 	}
 	return;
 }
 
 bool ab_walk(struct block_list* bl, short x, short y, unsigned char flag) {
 	struct unit_data* ud = NULL;
 	ud = unit_bl2ud(bl);
 
 	if (ud == NULL)
 		return 0;
 
 	// start the new walk if we're more  than 3 cells of the destination
 	if ((abs(x - ud->to_x) > 2) || (abs(y - ud->to_y) > 2) || (ud->walktimer == INVALID_TIMER)) {
 		return unit_walktoxy(bl, x, y, flag);
 	}
 
 	return 0;
 }
 
 bool ab_partymessage(map_session_data* sd, std::string key, char* message, int delay) {
 	if (!sd)
 		return false;
 
 	auto& party_msg = sd->ab.party_msg;
 
 	if (party_msg.empty()) {
 		party_msg.emplace_back(key, gettick() + delay * 1000);
 	}
 	else {
 		auto it = std::find_if(party_msg.begin(), party_msg.end(),
 			[&key](const std::pair<std::string, t_tick>& msg) { return msg.first == key; });
 
 		if (it != party_msg.end()) {
 			if (gettick() < it->second) {
 				return false; 
 			}
 			it->second = gettick() + delay * 1000;
 		}
 		else {
 			party_msg.emplace_back(key, gettick() + delay * 1000);
 		}
 	}
 
 	party_send_message(sd, message, (int)strlen(message) + 1);
 
 	return true;
 }
 
 bool ab_canuseskill(map_session_data* sd, uint16 skill_id, uint16 skill_lv) {
 	int inf = skill_get_inf(skill_id);
 	t_tick tick = gettick();
 
 	if (pc_checkskill(sd, skill_id) < skill_lv)
 		return false;
 
 	if (skill_get_sp(skill_id, skill_lv) > sd->battle_status.sp)
 		return false;
 
 	if (battle_config.idletime_option & IDLE_USESKILLTOID)
 		sd->idletime = tick;
 
 	if ((pc_cant_act2(sd) || sd->chatID) && skill_id != RK_REFRESH && !(skill_id == SR_GENTLETOUCH_CURE &&
 		(sd->sc.opt1 == OPT1_STONE || sd->sc.opt1 == OPT1_FREEZE || sd->sc.opt1 == OPT1_STUN)) &&
 		sd->state.storage_flag && !(inf & INF_SELF_SKILL))
 		return false;
 
 	if (pc_issit(sd))
 		return false;
 
 	if (skill_isNotOk(skill_id, sd))
 		return false;
 
 	if (!skill_check_condition_castbegin(sd, skill_id, skill_lv) || !skill_check_condition_castend(sd, skill_id, skill_lv))
 		return false;
 
 	if (sd->ud.skilltimer != INVALID_TIMER) {
 		if (skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST)
 			return false;
 	}
 	else if (DIFF_TICK(tick, sd->ud.canact_tick) < 0) {
 		if (sd->skillitem != skill_id)
 			return false;
 	}
 
 	if (sd->sc.option & OPTION_COSTUME)
 		return false;
 
 	if (sd->sc.getSCE(SC_BASILICA) && (skill_id != HP_BASILICA || sd->sc.getSCE(SC_BASILICA)->val4 != sd->bl.id))
 		return false; // On basilica only caster can use Basilica again to stop it.
 
 	if (sd->menuskill_id) {
 		if (sd->menuskill_id == SA_TAMINGMONSTER) {
 			clif_menuskill_clear(sd); //Cancel pet capture.
 		}
 		else if (sd->menuskill_id != SA_AUTOSPELL)
 			return false; //Can't use skills while a menu is open.
 	}
 
 	skill_lv = min(pc_checkskill(sd, skill_id), skill_lv); //never trust client
 
 	pc_delinvincibletimer(sd);
 
 	return true;
 }
 
 int ab_targetresu(struct block_list* bl, va_list ap) {
 	nullpo_ret(bl);
 	int* tbl_dead_id = va_arg(ap, int*);
 	map_session_data* party_sd = nullptr;
 
 	// source id
 	int src_id = va_arg(ap, int);
 	map_session_data* sd = map_id2sd(src_id);
 	nullpo_ret(sd);
 
 	//target id
 	map_session_data* tsd = map_id2sd(bl->id);
 	nullpo_ret(tsd);
 
 	int i = 0;
 
 	// Check if the target is dead
 	if (pc_isdead(tsd)) {
 		// Check if the target is in the party
 		struct party_data* p = party_search(sd->status.party_id);
 
 		for (int i = 0; i < MAX_PARTY; i++) {
 			if (p->party.member[i].account_id == 0 || p->party.member[i].char_id == 0)
 				continue;
 
 			party_sd = map_charid2sd(p->party.member[i].char_id);
 
 			if (party_sd == nullptr)
 				continue;
 
 			if (party_sd == tsd)
 				break;
 		}
 
 		if (i < MAX_PARTY && party_sd == tsd) {
 			*tbl_dead_id = bl->id;
 			return 1;
 		}
 	}
 
 	return 0;
 }
 
 void ab_save(map_session_data* sd) {
 
 	// Insert ab_common_config
 	if (SQL_ERROR == Sql_Query(mmysql_handle,
 		"INSERT INTO `ab_common_config` (`char_id`,`following_player`,`dist_to_leader`,`autobuff_resurection`,`skill_cd`,`allow_pm_cmd`,`autobuff_potions`,`return_to_savepoint`,`autobuff_token_siegfried`,`autobuff_disable_alone`,`priorize_buff`,`autositregen_conf`,`autositregen_maxhp`,`autositregen_minhp`,`autositregen_maxsp`,`autositregen_minsp`,`tp_cap_dmg`,`tp_min_hp`) "
 		"VALUES (%u, %u, %" PRIu16 ", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d) "
 		"ON DUPLICATE KEY UPDATE `following_player` = %u, `dist_to_leader` = %" PRIu16 ", `autobuff_resurection` = %d, `skill_cd` = %" PRId64 ", `allow_pm_cmd` = %d, `autobuff_potions` = %d, `return_to_savepoint` = %d, `autobuff_token_siegfried` = %d, `autobuff_disable_alone` = %d, `priorize_buff` = %d, `autositregen_conf` = %d, `autositregen_maxhp` = %d, `autositregen_minhp` = %d, `autositregen_maxsp` = %d, `autositregen_minsp` = %d, `tp_cap_dmg` = %d, `tp_min_hp` = %d",
 		sd->status.char_id,
 		sd->ab.following_player,
 		sd->ab.dist_to_leader,
 		sd->ab.autobuff_resurection,
 		1,
 		sd->ab.allow_pm_cmd,
 		sd->ab.state_autobuff_potions,
 		sd->ab.return_to_savepoint,
 		sd->ab.autobuff_token_siegfried,
 		sd->ab.autobuff_disable_alone,
 		sd->ab.priorize_buff,
 		sd->ab.autositregen.is_active,
 		sd->ab.autositregen.max_hp,
 		sd->ab.autositregen.min_hp,
 		sd->ab.autositregen.max_sp,
 		sd->ab.autositregen.min_sp,
 		sd->ab.emergency_tp_damage,
 		sd->ab.emergency_tp_perc,
 		sd->ab.following_player,
 		sd->ab.dist_to_leader,
 		sd->ab.autobuff_resurection,
 		1,
 		sd->ab.allow_pm_cmd,
 		sd->ab.state_autobuff_potions,
 		sd->ab.return_to_savepoint,
 		sd->ab.autobuff_token_siegfried,
 		sd->ab.autobuff_disable_alone,
 		sd->ab.priorize_buff,
 		sd->ab.autositregen.is_active,
 		sd->ab.autositregen.max_hp,
 		sd->ab.autositregen.min_hp,
 		sd->ab.autositregen.max_sp,
 		sd->ab.autositregen.min_sp,
 		sd->ab.emergency_tp_damage,
 		sd->ab.emergency_tp_perc)
 		) {
 		Sql_ShowDebug(mmysql_handle);
 	}
 
 	// Clean ab_items
 	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `ab_items` WHERE `char_id` = %u", sd->status.char_id)) {
 		Sql_ShowDebug(mmysql_handle);
 	}
 
 	// Insert ab_items - 0 - autobuffitems
 	if (!sd->ab.autobuff_buffitems.empty()) {
 		for (auto& itAutobuffitem : sd->ab.autobuff_buffitems) {
 			if (SQL_ERROR == Sql_Query(mmysql_handle,
 				"INSERT INTO `ab_items` (`char_id`, `type`, `item_id`, `status`) "
 				"VALUES (%u, 0, %" PRIu16 ", %d)",
 				sd->status.char_id, itAutobuffitem.item_id, itAutobuffitem.status))
 			{
 				Sql_ShowDebug(mmysql_handle);
 			}
 		}
 	}
 
 	// Insert ab_items - 1 - autopotions
 	if (!sd->ab.autobuff_potions.empty()) {
 		for (auto& itAutopotion : sd->ab.autobuff_potions) {
 			if (SQL_ERROR == Sql_Query(mmysql_handle,
 				"INSERT INTO `ab_items` (`char_id`,`type`,`item_id`,`min_hp`,`min_sp`) VALUES (%u, 1, %u, %" PRIu16 ", %" PRIu16 ")",
 				sd->status.char_id, itAutopotion.item_id, itAutopotion.min_hp, itAutopotion.min_sp)
 				) {
 				Sql_ShowDebug(mmysql_handle);
 			}
 		}
 	}
 
 	// Clean ab_skills_char_ids
 	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `ab_skills_char_ids` WHERE `ab_skills_id` IN (SELECT `ab_skills_id` FROM `ab_skills` WHERE `char_id` = %u)", sd->status.char_id)) {
 		Sql_ShowDebug(mmysql_handle);
 	}
 
 	// Clean ab_skills
 	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `ab_skills` WHERE `char_id` = %u", sd->status.char_id)) {
 		Sql_ShowDebug(mmysql_handle);
 	}
 
 	// Insert autobuff_heal
 	if (!sd->ab.autobuff_heal.empty()) {
 		for (auto& itAutoheal : sd->ab.autobuff_heal) {
 			uint64 ab_skills_id = 0;
 			if (SQL_ERROR == Sql_Query(mmysql_handle,
 				"INSERT INTO `ab_skills` (`char_id`,`type`,`skill_id`,`skill_lv`,`min_hp`,`last_use`,`priorize_type`) VALUES (%u, 0, %" PRIu16 ", %" PRIu16 ", %" PRIu16 ", %" PRId64 ", %d)",
 				sd->status.char_id, itAutoheal.skill_id, itAutoheal.skill_lv, itAutoheal.min_hp, itAutoheal.last_use, itAutoheal.priorize_heal)
 				) {
 				Sql_ShowDebug(mmysql_handle);
 			}
 
 			ab_skills_id = Sql_LastInsertId(mmysql_handle);
 
 			if (ab_skills_id > 0) {
 				for (auto& char_id : itAutoheal.priorize_heal_char_id) {
 					if (SQL_ERROR == Sql_Query(mmysql_handle,
 						"INSERT INTO `ab_skills_char_ids` (`ab_skills_id`,`ab_char_id`) VALUES (%" PRIu64 ", %u)",
 						ab_skills_id, char_id)
 						) {
 						Sql_ShowDebug(mmysql_handle);
 					}
 				}
 			}
 		}
 	}
 
 	// Insert autobuff_buffskills
 	if (!sd->ab.autobuff_buffskills.empty()) {
 		for (auto& itAutobuffskills : sd->ab.autobuff_buffskills) {
 			uint64 ab_skills_id = 0;
 			if (SQL_ERROR == Sql_Query(mmysql_handle,
 				"INSERT INTO `ab_skills` (`char_id`,`type`,`skill_id`,`skill_lv`,`last_use`,`priorize_type`) VALUES (%u, 1, %" PRIu16 ", %" PRIu16 ", %" PRId64 ", %d)",
 				sd->status.char_id, itAutobuffskills.skill_id, itAutobuffskills.skill_lv, itAutobuffskills.last_use, itAutobuffskills.priorize_buff)
 				) {
 				Sql_ShowDebug(mmysql_handle);
 			}
 
 			ab_skills_id = Sql_LastInsertId(mmysql_handle);
 
 			if (ab_skills_id > 0) {
 				for (auto& char_id : itAutobuffskills.priorize_buff_char_id) {
 					if (SQL_ERROR == Sql_Query(mmysql_handle,
 						"INSERT INTO `ab_skills_char_ids` (`ab_skills_id`,`ab_char_id`) VALUES (%" PRIu64 ", %u)",
 						ab_skills_id, char_id)
 						) {
 						Sql_ShowDebug(mmysql_handle);
 					}
 				}
 			}
 		}
 	}
 
 	// Handle authorized pm player
 	// Clean ab_pm_char_ids
 	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `ab_pm_char_ids` WHERE `char_id` = %u", sd->status.char_id)) {
 		Sql_ShowDebug(mmysql_handle);
 	}
 
 	// Insert ab_pm_char_ids
 	if (!sd->ab.allow_pm_char_id.empty()) {
 		for (auto& pmCharId : sd->ab.allow_pm_char_id) {
 			if (SQL_ERROR == Sql_Query(mmysql_handle,
 				"INSERT INTO `ab_pm_char_ids` (`char_id`,`ab_char_id`) VALUES (%u, %u)",
 				sd->status.char_id, pmCharId)
 				) {
 				Sql_ShowDebug(mmysql_handle);
 			}
 		}
 	}
 	//autobuyitems
 	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `ab_autobuyitems` WHERE `char_id` = %d", sd->status.char_id)) {
 		Sql_ShowDebug(mmysql_handle);
 	}
 	if (sd->ab.autobuyitems.size()) {
 		for (const auto& it : sd->ab.autobuyitems) {
 			if (SQL_ERROR == Sql_Query(mmysql_handle,
 				"INSERT INTO `ab_autobuyitems` (`char_id`, `item_id`, `quantity`, `min_trigger`) "
 				"VALUES (%d, %d, %d, %d)",
 				sd->status.char_id, it.item_id, it.quantity, it.min_trigger))
 			{
 				Sql_ShowDebug(mmysql_handle);
 			}
 		}
 	}
 	// autosellitems
 	if (SQL_ERROR == Sql_Query(mmysql_handle, "DELETE FROM `ab_autosellitems` WHERE `char_id` = %d", sd->status.char_id)) {
 		Sql_ShowDebug(mmysql_handle);
 	}
 	if (!sd->ab.autosellitems.empty()) {
 		for (const auto& it : sd->ab.autosellitems) {
 			if (SQL_ERROR == Sql_Query(mmysql_handle,
 				"INSERT INTO `ab_autosellitems` (`char_id`, `item_id`) "
 				"VALUES (%d, %d)",
 				sd->status.char_id, it.item_id))
 			{
 				Sql_ShowDebug(mmysql_handle);
 			}
 		}
 	}
 }
 
 void ab_load(map_session_data* sd) {
 	int type;
 	struct party_data* p = nullptr;
 
 	if (!sd)
 		return;
 
 	if (sd->sc.getSCE(SC_AUTOBUFF))
 		status_change_end(&sd->bl, SC_AUTOBUFF);
 
 	// Check if the player is in a party
 	if (sd->status.party_id)
 		p = party_search(sd->status.party_id);
 
 	// ab_common_config
 	if (Sql_Query(mmysql_handle,
 		"SELECT `following_player`,`dist_to_leader`,`autobuff_resurection`,`skill_cd`,`allow_pm_cmd`,`autobuff_potions`,`return_to_savepoint`,`autobuff_token_siegfried`,`autobuff_disable_alone`,`priorize_buff`,`autositregen_conf`,`autositregen_maxhp`,`autositregen_minhp`,`autositregen_maxsp`,`autositregen_minsp`,`tp_cap_dmg`,`tp_min_hp` "
 		"FROM `ab_common_config` "
 		"WHERE `char_id` = %u",
 		sd->status.char_id) != SQL_SUCCESS) {
 		Sql_ShowDebug(mmysql_handle);
 		return;
 	}
 
 	if (Sql_NumRows(mmysql_handle) > 0) {
 		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
 			char* data;
 			Sql_GetData(mmysql_handle, 0, &data, NULL); sd->ab.following_player = static_cast<uint32>(atoi(data));
 			Sql_GetData(mmysql_handle, 1, &data, NULL); sd->ab.dist_to_leader = static_cast<uint16>(atoi(data));
 			Sql_GetData(mmysql_handle, 2, &data, NULL); sd->ab.autobuff_resurection = static_cast<bool>(atoi(data));
 			Sql_GetData(mmysql_handle, 3, &data, NULL); sd->ab.skill_cd = gettick();
 			Sql_GetData(mmysql_handle, 4, &data, NULL); sd->ab.allow_pm_cmd = static_cast<ab_pmcmdconfig>(atoi(data)); // Cast vers enum
 			Sql_GetData(mmysql_handle, 5, &data, NULL); sd->ab.state_autobuff_potions = static_cast<bool>(atoi(data));
 			Sql_GetData(mmysql_handle, 6, &data, NULL); sd->ab.return_to_savepoint = static_cast<bool>(atoi(data));
 			Sql_GetData(mmysql_handle, 7, &data, NULL); sd->ab.autobuff_token_siegfried = static_cast<bool>(atoi(data));
 			Sql_GetData(mmysql_handle, 8, &data, NULL); sd->ab.autobuff_disable_alone = static_cast<bool>(atoi(data));
 			Sql_GetData(mmysql_handle, 9, &data, NULL); sd->ab.priorize_buff = static_cast<bool>(atoi(data));
 			Sql_GetData(mmysql_handle, 10, &data, NULL); sd->ab.autositregen.is_active = atoi(data);
 			Sql_GetData(mmysql_handle, 11, &data, NULL); sd->ab.autositregen.max_hp = atoi(data);
 			Sql_GetData(mmysql_handle, 12, &data, NULL); sd->ab.autositregen.min_hp = atoi(data);
 			Sql_GetData(mmysql_handle, 13, &data, NULL); sd->ab.autositregen.max_sp = atoi(data);
 			Sql_GetData(mmysql_handle, 14, &data, NULL); sd->ab.autositregen.min_sp = atoi(data);
 			Sql_GetData(mmysql_handle, 15, &data, NULL); sd->ab.emergency_tp_damage = atoi(data);
 			Sql_GetData(mmysql_handle, 16, &data, NULL); sd->ab.emergency_tp_perc = atoi(data);
 			sd->ab.autositregen.status = 0;
 		}
 	}
 	else {
 		sd->ab.following_player = 0;
 		sd->ab.dist_to_leader = 2;
 		sd->ab.autobuff_resurection = true;
 		sd->ab.skill_cd = gettick();
 		sd->ab.allow_pm_cmd = AB_PM_ALLOWPM;
 		sd->ab.state_autobuff_potions = true;
 		sd->ab.return_to_savepoint = false;
 		sd->ab.autobuff_token_siegfried = false;
 		sd->ab.autobuff_disable_alone = true;
 		sd->ab.priorize_buff = false;
 		sd->ab.duration_ = 0;
 		sd->ab.autositregen.is_active = 0;
 		sd->ab.autositregen.max_hp = 0;
 		sd->ab.autositregen.min_hp = 0;
 		sd->ab.autositregen.max_sp = 0;
 		sd->ab.autositregen.min_sp = 0;
 		sd->ab.autositregen.status = 0;
 		sd->ab.emergency_tp_damage = 0;
 		sd->ab.emergency_tp_perc = 0;
 	}
 	Sql_FreeResult(mmysql_handle);
 
 
 	//Auto Buy Item
 	sd->ab.autobuyitems.clear();
 	
 	if (Sql_Query(mmysql_handle, "SELECT `item_id`, `quantity`, `min_trigger` FROM `ab_autobuyitems` WHERE `char_id` = %d", sd->status.char_id) != SQL_SUCCESS) {
 		Sql_ShowDebug(mmysql_handle);
 		return;
 	}
 	
 	if (Sql_NumRows(mmysql_handle)) {
 		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
 			char* data;
 			s_autobuyitems_2 entry = {};
 	
 			Sql_GetData(mmysql_handle, 0, &data, nullptr); entry.item_id = atoi(data);
 			Sql_GetData(mmysql_handle, 1, &data, nullptr); entry.quantity = atoi(data);
 			Sql_GetData(mmysql_handle, 2, &data, nullptr); entry.min_trigger = atoi(data);
 	
 			sd->ab.autobuyitems.push_back(entry);
 		}
 	}
 	
 	Sql_FreeResult(mmysql_handle);
 
 	// Auto Sell Item
 	sd->ab.autosellitems.clear();
 	
 	if (Sql_Query(mmysql_handle, "SELECT `item_id` FROM `ab_autosellitems` WHERE `char_id` = %d", sd->status.char_id) != SQL_SUCCESS) {
 		Sql_ShowDebug(mmysql_handle);
 		return;
 	}
 	
 	if (Sql_NumRows(mmysql_handle)) {
 		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
 			char* data;
 			s_autosellitems_2 entry = {};
 	
 			Sql_GetData(mmysql_handle, 0, &data, nullptr);
 			entry.item_id = atoi(data);
 	
 			sd->ab.autosellitems.push_back(entry);
 		}
 	}
 	
 	Sql_FreeResult(mmysql_handle);
 
 	// ab_items
 	if (Sql_Query(mmysql_handle,
 		"SELECT `type`,`item_id`,`min_hp`,`min_sp`,`status` "
 		"FROM `ab_items` "
 		"WHERE `char_id` = %u",
 		sd->status.char_id) != SQL_SUCCESS) {
 		Sql_ShowDebug(mmysql_handle);
 		return;
 	}
 
 	if (Sql_NumRows(mmysql_handle) > 0) {
 		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
 			char* data;
 			Sql_GetData(mmysql_handle, 0, &data, NULL); type = atoi(data);
 
 			switch (type) {
 			case 0: {
 				// Autobuff item
 				s_autobuff_buffitems autobuffitem;
 				Sql_GetData(mmysql_handle, 1, &data, NULL); autobuffitem.item_id = static_cast<uint32>(atoi(data));
 				Sql_GetData(mmysql_handle, 4, &data, NULL); autobuffitem.status = atoi(data);
 				sd->ab.autobuff_buffitems.push_back(autobuffitem);
 				break;
 			}
 			case 1: {
 				// Autopotion
 				s_autobuff_potions autopotion;
 				Sql_GetData(mmysql_handle, 1, &data, NULL); autopotion.item_id = static_cast<uint32>(atoi(data));
 				Sql_GetData(mmysql_handle, 2, &data, NULL); autopotion.min_hp = static_cast<uint16>(atoi(data));
 				Sql_GetData(mmysql_handle, 3, &data, NULL); autopotion.min_sp = static_cast<uint16>(atoi(data));
 				sd->ab.autobuff_potions.push_back(autopotion);
 				break;
 			}
 			}
 		}
 	}
 	Sql_FreeResult(mmysql_handle);
 
 	// ab_skills
 	if (Sql_Query(mmysql_handle,
 		"SELECT `type`,`skill_id`,`skill_lv`,`min_hp`,`last_use`,`priorize_type`,`ab_skills_id` "
 		"FROM `ab_skills` "
 		"WHERE `char_id` = %u",
 		sd->status.char_id) != SQL_SUCCESS) {
 		Sql_ShowDebug(mmysql_handle);
 		return;
 	}
 
 	if (Sql_NumRows(mmysql_handle) > 0) {
 		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
 			uint64 ab_skills_id = 0;
 			char* data;
 			int player_pos = 0;
 
 			Sql_GetData(mmysql_handle, 0, &data, NULL); type = atoi(data);
 
 			switch (type) {
 			case 0: {
 				// Heal skill
 				s_autobuff_heal autoheal;
 				Sql_GetData(mmysql_handle, 1, &data, NULL); autoheal.skill_id = static_cast<uint16>(atoi(data));
 				Sql_GetData(mmysql_handle, 2, &data, NULL); autoheal.skill_lv = static_cast<uint16>(atoi(data));
 				Sql_GetData(mmysql_handle, 3, &data, NULL); autoheal.min_hp = static_cast<uint16>(atoi(data));
 				Sql_GetData(mmysql_handle, 4, &data, NULL); autoheal.last_use = 0;
 				Sql_GetData(mmysql_handle, 5, &data, NULL); autoheal.priorize_heal = static_cast<ab_targettype>(atoi(data));
 				Sql_GetData(mmysql_handle, 6, &data, NULL); ab_skills_id = static_cast<uint64>(atoll(data));
 				// Load members
 				if (Sql_Query(qsmysql_handle,
 					"SELECT `ab_char_id` FROM `ab_skills_char_ids` WHERE `ab_skills_id` = %" PRIu64, ab_skills_id) == SQL_SUCCESS) {
 					while (SQL_SUCCESS == Sql_NextRow(qsmysql_handle)) {
 						Sql_GetData(qsmysql_handle, 0, &data, NULL);
 						uint32 priorize_heal_char_id = static_cast<uint32>(atoi(data));
 						if (p != nullptr) {
 							ARR_FIND(0, MAX_PARTY, player_pos, p->party.member[player_pos].char_id == priorize_heal_char_id);
 							if (player_pos < MAX_PARTY)
 								autoheal.priorize_heal_char_id.insert(priorize_heal_char_id);
 						}
 					}
 				}
 				Sql_FreeResult(qsmysql_handle);
 				sd->ab.autobuff_heal.push_back(autoheal);
 				break;
 			}
 			case 1: {
 				// Buff skill
 				s_autobuff_buffskills autobuffskill;
 				Sql_GetData(mmysql_handle, 1, &data, NULL); autobuffskill.skill_id = static_cast<uint16>(atoi(data));
 				Sql_GetData(mmysql_handle, 2, &data, NULL); autobuffskill.skill_lv = static_cast<uint16>(atoi(data));
 				Sql_GetData(mmysql_handle, 4, &data, NULL); autobuffskill.last_use = static_cast<sint64>(atoll(data));
 				Sql_GetData(mmysql_handle, 5, &data, NULL); autobuffskill.priorize_buff = static_cast<ab_targettype>(atoi(data));
 				Sql_GetData(mmysql_handle, 6, &data, NULL); ab_skills_id = static_cast<uint64>(atoll(data));
 
 				// Charger les prioritโ”ยฎs associโ”ยฎes
 				if (Sql_Query(qsmysql_handle,
 					"SELECT `ab_char_id` FROM `ab_skills_char_ids` WHERE `ab_skills_id` = %" PRIu64, ab_skills_id) == SQL_SUCCESS) {
 					while (SQL_SUCCESS == Sql_NextRow(qsmysql_handle)) {
 						Sql_GetData(qsmysql_handle, 0, &data, NULL);
 						uint32 priorize_buff_char_id = static_cast<uint32>(atoi(data));
 						autobuffskill.priorize_buff_char_id.insert(static_cast<uint32>(atoi(data)));
 
 						if (p == nullptr) {
 							// nothing party not found
 						}
 						else {
 							ARR_FIND(0, MAX_PARTY, player_pos, p->party.member[player_pos].char_id == priorize_buff_char_id);
 							if (player_pos < MAX_PARTY)
 								autobuffskill.priorize_buff_char_id.insert(priorize_buff_char_id);
 						}
 					}
 				}
 				Sql_FreeResult(qsmysql_handle);
 				sd->ab.autobuff_buffskills.push_back(autobuffskill);
 				break;
 			}
 			}
 		}
 	}
 	Sql_FreeResult(mmysql_handle);
 
 	// ab_pm_char_ids
 	if (Sql_Query(mmysql_handle,
 		"SELECT `ab_char_id` FROM `ab_pm_char_ids` WHERE `char_id` = %u",
 		sd->status.char_id) != SQL_SUCCESS) {
 		Sql_ShowDebug(mmysql_handle);
 		return;
 	}
 
 	if (Sql_NumRows(mmysql_handle) > 0) {
 		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
 			char* data;
 			Sql_GetData(mmysql_handle, 0, &data, NULL);
 			sd->ab.allow_pm_char_id.insert(static_cast<uint32>(atoi(data)));
 		}
 	}
 	Sql_FreeResult(mmysql_handle);
 
 	return;
 }
 
 ab_commandHandler::ab_commandHandler() {
 	command_map = {
 		{"clear", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_clear_command(sd, ssd, cmd); }},
 		{"stop", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_stop_command(sd, ssd, cmd); }},
 		{"start", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_start_command(sd, ssd, cmd); }},
 		{"stay", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_stay_command(sd, ssd, cmd); }},
 		{"move", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_move_command(sd, ssd, cmd); }},
 		{"sit", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_sit_command(sd, ssd, cmd); }},
 		{"stand", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_stand_command(sd, ssd, cmd); }},
 		{"logout", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_logout_command(sd, ssd, cmd); }},
 		{"go", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_go_command(sd, ssd, cmd); }},
 		{"heal", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"bless", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"agi", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"ress", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"kyrie", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"pneuma", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"safety", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"impo", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"suffra", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_buff_command(sd, ssd, cmd); }},
 		{"follow", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_follow_command(sd, ssd, cmd); }},
 		{"warp", [this](map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) { return ab_warp_command(sd, ssd, cmd); }},
 	};
 }
 
 // Autobuff command msg functions
 bool ab_commandHandler::ab_clear_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	sd->ab.order_msg.clear();
 	return true;
 }
 
 bool ab_commandHandler::ab_stop_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	sd->state.ab_stop = true;
 	return true;
 }
 
 bool ab_commandHandler::ab_start_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	sd->state.ab_stop = false;
 	return true;
 }
 
 bool ab_commandHandler::ab_stay_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	sd->state.ab_stay = true;
 	return true;
 }
 
 bool ab_commandHandler::ab_move_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	sd->state.ab_stay = false;
 	return true;
 }
 
 bool ab_commandHandler::ab_sit_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	if (!pc_issit(sd)) {
 		pc_setsit(sd);
 		skill_sit(sd, true);
 		clif_sitting(&sd->bl);
 	}
 	return true;
 }
 
 bool ab_commandHandler::ab_stand_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	if (pc_issit(sd)) {
 		pc_setstand(sd, true);
 		skill_sit(sd, false);
 		clif_standing(&sd->bl);
 	}
 	return true;
 }
 
 bool ab_commandHandler::ab_logout_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	if (session_isActive(sd->fd))
 		clif_parse_QuitGame(sd->fd, sd);
 	else
 		map_quit(sd);
 	return true;
 }
 
 bool ab_commandHandler::ab_go_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	int dest_x, dest_y;
 	bool walk_result = false;
 
 	//Not enough args
 	if (cmd.size() != 3) { //todoab tell player right command arg
 		std::string msg = "Autobuff : send the following private message : go <x> <y>";
 		ab_partymessage(sd, "goCmd", msg.data(), 5);
 		return false;
 	}
 
 	try {
 		dest_x = std::stoi(cmd[1]);
 	}
 	catch (const std::invalid_argument&) {
 		std::string msg = "Autobuff : send the following private message : go <x> <y>";
 		ab_partymessage(sd, "goCmd", msg.data(), 5);
 		return false;
 	}
 	catch (const std::out_of_range&) {
 		std::string msg = "Autobuff : send the following private message : go <x> <y>";
 		ab_partymessage(sd, "goCmd", msg.data(), 5);
 		return false;
 	}
 
 	try {
 		dest_y = std::stoi(cmd[2]);
 	}
 	catch (const std::invalid_argument&) {
 		std::string msg = "Autobuff : send the following private message : go <x> <y>";
 		ab_partymessage(sd, "goCmd", msg.data(), 5);
 		return false;
 	}
 	catch (const std::out_of_range&) {
 		std::string msg = "Autobuff : send the following private message : go <x> <y>";
 		ab_partymessage(sd, "goCmd", msg.data(), 5);
 		return false;
 	}
 
 	if ((abs(dest_x - sd->bl.x) > 2) || (abs(dest_y - sd->bl.y) > 2)) {
 		walk_result = ab_walk(&sd->bl, dest_x, dest_y, 8);
 	}
 	if (!walk_result) {
 		if ((abs(sd->bl.x - dest_x) > 6) || (abs(sd->bl.y - dest_y) > 6)
 			|| !(path_search_long(NULL, sd->bl.m, sd->bl.x, sd->bl.y, dest_x, dest_y, CELL_CHKNOPASS))
 			) {
 			struct walkpath_data wpd1;
 			if (path_search(&wpd1, sd->bl.m, sd->bl.x, sd->bl.y, dest_x, dest_y, 0, CELL_CHKNOPASS))
 				walk_result = ab_walk(&sd->bl, sd->bl.x + dirx[wpd1.path[0]], sd->bl.y + diry[wpd1.path[0]], 8);
 		}
 	}
 
 	if (!walk_result) {
 		std::string msg = "Autobuff : I can't go to the coordinates that were sent";
 		ab_partymessage(sd, "goCmd", msg.data(), 5);
 	}
 
 	//Stay at the indicated location until move cmd
 	if (walk_result)
 		sd->state.ab_stay = true;
 
 	return walk_result;
 }
 
 int ab_commandHandler::ab_buff_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	uint16 skill_id, skill_lv;
 	bool buff_used = false;
 
 	map_session_data* pl_sd;
 
 	//Not enough args
 	if (cmd.size() != 2) {
 		std::string msg = "Autobuff : send the following private message : bless, agi, kyrie, heal, pneuma, safety <me,you,player name>";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	// Check if cmd target is right
 	if (cmd[1] != "me" && cmd[1] != "you") {
 		if ((pl_sd = map_nick2sd(cmd[1].c_str(), true)) == nullptr)
 		{
 			std::string msg = "Autobuff : the player isn't online or doesn't exist";
 			ab_partymessage(sd, "buffCmd", msg.data(), 5);
 			return false;
 		}
 		else { // Check if on the same map
 			if (pl_sd->bl.m != sd->bl.m) {
 				std::string msg = "Autobuff : I can't buff, the player is not on the same map";
 				ab_partymessage(sd, "buffCmd", msg.data(), 5);
 				return false;
 			}
 		}
 	}
 	else if (cmd[1] == "me" && ssd->bl.m != sd->bl.m) { // Check if on the same map
 		std::string msg = "Autobuff : I can't buff, we're not on the same map";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	if (gettick() < sd->ab.pm_skill_cd) {
 		std::string msg = "Autobuff : I can't buff because of skill delay";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	//check if the buff have the skill
 	if (cmd[0] == "bless") {
 		skill_id = AL_BLESSING;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "agi") {
 		skill_id = AL_INCAGI;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "kyrie") {
 		skill_id = PR_KYRIE;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "heal") {
 		skill_id = AL_HEAL;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "ress") {
 		skill_id = ALL_RESURRECTION;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "safety") {
 		skill_id = MG_SAFETYWALL;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "pneuma") {
 		skill_id = AL_PNEUMA;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "impo") {
 		skill_id = PR_IMPOSITIO;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else if (cmd[0] == "suffra") {
 		skill_id = PR_SUFFRAGIUM;
 		skill_lv = pc_checkskill(sd, skill_id);
 	}
 	else {
 		std::string msg = "Autobuff : unknown skill, you can only use bless, agi, kyrie, heal";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	if (skill_lv <= 0) {
 		std::string msg = "Autobuff : I don't have that skill";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	if (!ab_canuseskill(sd, skill_id, skill_lv)) {
 		std::string msg = "Autobuff : I can't use a skill right now";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return 2;
 	}
 
 	if (skill_id == ALL_RESURRECTION) {
 		if (cmd[1] == "you") {
 			std::string msg = "Autobuff : I can't ressurect myself";
 			ab_partymessage(sd, "buffCmd", msg.data(), 5);
 			return false;
 		}
 		else if (cmd[1] == "me" && !status_isdead(&ssd->bl)) {
 			std::string msg = "Autobuff : I can't ressurect, you aren't dead";
 			ab_partymessage(sd, "buffCmd", msg.data(), 5);
 			return false;
 		}
 		else if (pl_sd && !status_isdead(&pl_sd->bl)) {
 			std::string msg = "Autobuff : I can't ressurect, player isn't't dead";
 			ab_partymessage(sd, "buffCmd", msg.data(), 5);
 			return false;
 		}
 	}
 
 	if ((skill_id == AL_PNEUMA || skill_id == MG_SAFETYWALL) && (sd->sc.getSCE(SC_PNEUMA) || sd->sc.getSCE(SC_SAFETYWALL))) {
 		std::string msg = "Autobuff : I can't use pneuma or safety wall if you are already under a pneuma or on a safety wall";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	if (cmd[1] == "you") {
 		buff_used = unit_skilluse_id(&sd->bl, sd->bl.id, skill_id, skill_lv);
 	}
 #ifdef RENEWAL
 	else if (cmd[1] == "me") {
 		if (skill_id == PR_SUFFRAGIUM || skill_id == PR_IMPOSITIO)
 			buff_used = unit_skilluse_id(&sd->bl, sd->bl.id, skill_id, skill_lv);
 		else
 			buff_used = unit_skilluse_id(&sd->bl, ssd->bl.id, skill_id, skill_lv);
 	}
 #else
 	else if (cmd[1] == "me") {
 		buff_used = unit_skilluse_id(&sd->bl, ssd->bl.id, skill_id, skill_lv);
 	}
 #endif
 	else {
 		buff_used = unit_skilluse_id(&sd->bl, pl_sd->bl.id, skill_id, skill_lv);
 	}
 
 	if (!buff_used) {
 		std::string msg = "Autobuff : I can't buff for some reasons";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	sd->idletime = time(NULL);
 
 	// Skill consumption
 	skill_consume_requirement(sd, skill_id, skill_lv, 2);
 
 	// General skill delay with min buff skill delay
 	sd->ab.pm_skill_cd = std::max(sd->ab.pm_skill_cd, gettick() + battle_config.feature_autobuff_bskill_delay);
 
 	return buff_used;
 }
 
 bool ab_commandHandler::ab_warp_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	bool buff_used = false;
 	uint16 skill_id, skill_lv;
 	int16 x, y;
 	int warp_slot, i_warp_slot;
 	std::shared_ptr<s_skill_unit_group> group;
 
 	//Not enough args
 	if (cmd.size() != 2) {
 		std::string msg = "Autobuff : send the following private message : warp <warp number>";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	if (gettick() < sd->ab.pm_skill_cd) {
 		std::string msg = "Autobuff : I can't buff because of skill delay";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	skill_id = AL_WARP;
 	skill_lv = pc_checkskill(sd, skill_id);
 
 	if (skill_lv <= 0) {
 		std::string msg = "Autobuff : I don't have that skill";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	int gemstone_index = pc_search_inventory(sd, ITEMID_BLUE_GEMSTONE);
 	int gemstone_amount = (gemstone_index >= 0) ? sd->inventory.u.items_inventory[gemstone_index].amount : 0;
 
 	if (gemstone_amount == 0) {
 		std::string out_gemstone_msg = "I'm out of Blue Gemstones!";
 		ab_partymessage(sd, "OutGemstone", out_gemstone_msg.data(), 600);
 		return false;
 	}
 
 
 	i_warp_slot = std::stoi(cmd[1]);
 	warp_slot = i_warp_slot - 1;
 
 	// Check if cmd target is right
 	if (i_warp_slot < 0 || warp_slot > skill_lv + 1) {
 		std::string msg = "Autobuff : You need to send a correct warp number that exist on your char";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	if (!ab_canuseskill(sd, skill_id, skill_lv)) {
 		std::string msg = "Autobuff : I can't use a skill right now";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	if (i_warp_slot && strcmp(sd->status.memo_point[warp_slot].map, "") == 0) {
 		std::string msg = "Autobuff : You don't have a memo on this warp slot yet";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;
 	}
 
 	int range = (int)sqrt((float)1) + 2; // calculation of an odd number (+ 4 area around)
 	if (!map_search_freecell(&sd->bl, 0, &x, &y, range, range, 0)) {
 		std::string msg = "Autobuff : Skill failed";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;// failed
 	}
 
 	buff_used = unit_skilluse_pos(&sd->bl, x, y, skill_id, skill_lv);
 
 	if (!buff_used) {
 		std::string msg = "Autobuff : Skill failed";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;// failed
 	}
 
 	sd->menuskill_id = skill_id;
 	sd->menuskill_val = (x << 16) | sd->ud.skilly; //Store warp position here.
 
 	//It is possible to use teleport with the storage window open bugreport:8027
 	if (pc_cant_act(sd) && !sd->state.storage_flag && skill_id != AL_TELEPORT) {
 		clif_menuskill_clear(sd);
 		std::string msg = "Autobuff : Skill failed";
 		ab_partymessage(sd, "buffCmd", msg.data(), 5);
 		return false;// failed
 	}
 
 	if (!i_warp_slot)
 		buff_used = skill_castend_map(sd, skill_id, sd->status.save_point.map);
 	else
 		buff_used = skill_castend_map(sd, skill_id, sd->status.memo_point[warp_slot].map);
 
 	sd->idletime = time(NULL);
 
 	// Reset values
 	sd->skillitem = sd->skillitemlv = sd->skillitem_keep_requirement = 0;
 
 	// General skill delay with min buff skill delay
 	sd->ab.pm_skill_cd = std::max(sd->ab.pm_skill_cd, gettick() + battle_config.feature_autobuff_bskill_delay);
 
 	return buff_used;
 }
 
 bool ab_teleport(map_session_data* sd) {
 	// Early exit if teleportation is disabled globally or via configuration
 	if (!sd->state.autobuff)
 		return false;
 
 	map_data* mapdata = map_getmapdata(sd->bl.m);
 	if (mapdata->getMapFlag(MF_NOTELEPORT))
 		return false;
 
 	bool flywing_used = false;
 
 	// Check if teleport skill can be used
 	if (sd->status.sp > 20 && !mapdata->getMapFlag(MF_NOSKILL)) {
 		if (pc_checkskill(sd, AL_TELEPORT) > 0) {
 			skill_consume_requirement(sd, AL_TELEPORT, 1, 2);
 			pc_randomwarp(sd, CLR_TELEPORT);
 			status_heal(&sd->bl, 0, -skill_get_sp(AL_TELEPORT, 1), 1);
 			flywing_used = true;
 		}
 	}
 
 	// Check for Fly Wing usage if teleport was not used
 	if (!flywing_used) {
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
 			if (requires_consumption) {
 				pc_delitem(sd, inventory_index, 1, 0, 0, LOG_TYPE_OTHER);
 			}
 			pc_randomwarp(sd, CLR_TELEPORT);
 			flywing_used = true;
 		}
 	}
 
 	// Finalize teleportation actions
 	if (flywing_used) {
 		sd->ab.emergency_timer = gettick() + 500;
 		sd->ab.emergency_tp = true;
 
 		// Action after tp
 		pc_delinvincibletimer(sd);
 		clif_parse_LoadEndAck(0, sd);
 	}
 
 	return flywing_used;
 }
 
 bool ab_commandHandler::ab_follow_command(map_session_data* sd, map_session_data* ssd, std::vector<std::string> cmd) {
 	map_session_data* pl_sd;
 	map_session_data* party_sd = nullptr;
 	struct party_data* p = party_search(sd->status.party_id);
 	int player_pos = 0;
 
 	//Not enough args
 	if (cmd.size() != 2) { //todoab tell player right command arg
 		std::string msg = "Autobuff : send the following private message : follow <me,player name>";
 		ab_partymessage(sd, "followCmd", msg.data(), 5);
 		return false;
 	}
 
 	// Check if cmd target is right
 	if (cmd[1] == "you") {
 		std::string msg = "Autobuff : I can't follow myself";
 		ab_partymessage(sd, "followCmd", msg.data(), 5);
 		return false;
 	}
 	else if (cmd[1] == "me") {
 		for (player_pos = 0; player_pos < MAX_PARTY; player_pos++) {
 			if (p->party.member[player_pos].account_id == 0 || p->party.member[player_pos].char_id == 0)
 				continue;
 
 			party_sd = map_charid2sd(p->party.member[player_pos].char_id);
 
 			if (party_sd == nullptr)
 				continue;
 
 			if (party_sd == ssd)
 				break;
 		}
 
 		if (player_pos >= MAX_PARTY) {
 			std::string msg = "Autobuff : I can't follow a player that is not in my party";
 			ab_partymessage(sd, "followCmd", msg.data(), 5);
 			return false;
 		}
 
 		sd->ab.following_player = ssd->status.char_id;
 	}
 	else {
 		if ((pl_sd = map_nick2sd(cmd[1].c_str(), true)) == nullptr)
 		{
 			std::string msg = "Autobuff : the player isn't online or doesn't exist";
 			ab_partymessage(sd, "followCmd", msg.data(), 5);
 			return false;
 		}
 
 		for (player_pos = 0; player_pos < MAX_PARTY; player_pos++) {
 			if (p->party.member[player_pos].account_id == 0 || p->party.member[player_pos].char_id == 0)
 				continue;
 
 			party_sd = map_charid2sd(p->party.member[player_pos].char_id);
 
 			if (party_sd == nullptr)
 				continue;
 
 			if (party_sd == pl_sd)
 				break;
 		}
 
 		if (player_pos >= MAX_PARTY) {
 			std::string msg = "Autobuff : I can't follow a player that is not in my party";
 			ab_partymessage(sd, "followCmd", msg.data(), 5);
 			return false;
 		}
 
 		sd->ab.following_player = pl_sd->status.char_id;
 	}
 
 	sd->state.ab_stay = false;
 	return true;
 }
 
 // 0 = init, 1 = start, 2 = stop
 bool ab_changestate_autobuff(map_session_data* sd, int flag) {
 	map_data* mapdata;
 	struct unit_data* ud;
 	map_session_data* pl_sd;
 	struct s_mapiterator* iter;
 	int ip_limitation = 0, gepard_limitation = 0;
 
 	switch (flag) {
 	case 1:
 		if (battle_config.feature_autobuff_iplimit) {
 			iter = mapit_getallusers();
 			for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter))
 			{
 				if (pl_sd->bl.id != sd->bl.id && session[sd->fd]->client_addr == pl_sd->ab.client_addr && pl_sd->state.autobuff)
 					ip_limitation++;
 
 				if (ip_limitation >= battle_config.feature_autobuff_iplimit) {
 					std::string msg = "There is already an account using autobuff";
 					ab_partymessage(sd, "FlagOff", msg.data(), 5);
 					mapit_free(iter);
 					return false;
 				}
 			}
 			mapit_free(iter);
 		}
 
 		if (battle_config.feature_autobuff_gepardlimit) {
 			iter = mapit_getallusers();
 			for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter))
 			{
 				//Uncomment this if you have gepard and want to set a limit
 				//if (pl_sd->bl.id != sd->bl.id && session[sd->fd]->gepard_info.unique_id == pl_sd->ab.unique_id && pl_sd->state.autobuff)
 				//	gepard_limitation++;
 
 				if (gepard_limitation >= battle_config.feature_autobuff_gepardlimit) {
 					std::string msg = "There is already an account using autobuff";
 					ab_partymessage(sd, "FlagOff", msg.data(), 5);
 					mapit_free(iter);
 					return false;
 				}
 			}
 			mapit_free(iter);
 		}
 
 		mapdata = map_getmapdata(sd->bl.m);
 		if (!battle_config.feature_autobuff_allow_town && mapdata->getMapFlag(MF_TOWN)) {
 			std::string msg = "Automessage - autobuff is not allowed in town!";
 			ab_partymessage(sd, "FlagOff", msg.data(), 5);
 			return false;
 		}
 
 		if (!battle_config.feature_autobuff_allow_pvp && mapdata->getMapFlag(MF_PVP)) {
 			std::string msg = "Automessage - autobuff is not allowed in pvp map!";
 			ab_partymessage(sd, "FlagOff", msg.data(), 5);
 			return false;
 		}
 
 		if (!battle_config.feature_autobuff_allow_gvg && mapdata_flag_gvg2(mapdata)) {
 			std::string msg = "Automessage - autobuff is not allowed in woe!";
 			ab_partymessage(sd, "FlagOff", msg.data(), 5);
 			return false;
 		}
 
 		if (!battle_config.feature_autobuff_allow_bg && mapdata->getMapFlag(MF_BATTLEGROUND)) {
 			std::string msg = "Automessage - autobuff is not allowed in battleground map!";
 			ab_partymessage(sd, "FlagOff", msg.data(), 5);
 			return false;
 		}
 
 		if (battle_config.feature_autobuff_hateffect) {
 			for (const auto& effectID : AB_HATEFFECTS) {
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
 
 		if (battle_config.feature_autobuff_prefixname) {
 			char temp_name[NAME_LENGTH];
 			safestrncpy(temp_name, AB_PREFIX_NAME, sizeof(temp_name));
 			strncat(temp_name, sd->status.name, sizeof(temp_name) - strlen(temp_name) - 1);
 			safestrncpy(sd->fakename, temp_name, sizeof(sd->fakename));
 
 			clif_name_area(&sd->bl);
 			if (sd->disguise) // Another packet should be sent so the client updates the name for sd
 				clif_name_self(&sd->bl);
 		}
 
 		sd->state.autobuff = true;
 	case 0:
 		sd->ab.client_addr = session[sd->fd]->client_addr;
 		// Uncomment this if you have gepard and want to set a limit
 		//sd->ab.unique_id = session[sd->fd]->gepard_info.unique_id;
 		break;
 
 	case 2:
 		if (battle_config.feature_autobuff_prefixname && sd->fakename[0]) {
 			sd->fakename[0] = '\0';
 			clif_name_area(&sd->bl);
 			if (sd->disguise)
 				clif_name_self(&sd->bl);
 		}
 
 		if (battle_config.feature_autobuff_hateffect) {
 			for (const auto& effectID : AB_HATEFFECTS) {
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
 		sd->state.autobuff = false;
 		break;
 	}
 
 	return true;
 }
