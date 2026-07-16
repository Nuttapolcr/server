#ifndef AB_SCRIPT_H
#define AB_SCRIPT_H

#include "party.hpp"
#include "pc.hpp"

#include <unordered_map>
#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <common/db.hpp>
#include <common/database.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

//Get string script
void ab_handle_autobuff_heal(std::ostringstream & os_buf, int index, int extra_index, script_state * st, TBL_PC * sd, struct party_data* p);
void ab_handle_autobuff_potions(std::ostringstream & os_buf, int index, script_state * st, TBL_PC * sd);
void ab_handle_autobuff_buff(std::ostringstream & os_buf, int index, int extra_index, script_state * st, TBL_PC * sd, struct party_data* p);
void ab_handle_autobuff_follow(std::ostringstream & os_buf, int index, int extra_index, script_state * st, TBL_PC * sd, struct party_data* p);
void ab_handle_autobuff_items(std::ostringstream & os_buf, int index, script_state * st, TBL_PC * sd);
void ab_handle_autobuff_pm(std::ostringstream & os_buf, script_state * st, TBL_PC * sd, struct party_data* p);
void ab_handle_resurrection(std::ostringstream & os_buf, TBL_PC * sd);
void ab_handle_potions(std::ostringstream & os_buf, TBL_PC * sd);
void ab_handle_token_of_siegfried(std::ostringstream & os_buf, TBL_PC * sd);
void ab_handle_return_to_savepoint(std::ostringstream & os_buf, TBL_PC * sd);
void ab_handle_party_config(std::ostringstream & os_buf, TBL_PC * sd);
void ab_handle_priorize_buff(std::ostringstream & os_buf, TBL_PC * sd);
void ab_handle_potion_pitcher(std::ostringstream & os_buf, int index, int extra_index, int extra_index2, script_state * st, TBL_PC * sd, struct party_data* p);
void ab_handle_autobuff_sitrest(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd);
void ab_handle_autobuff_teleport(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd);
void ab_handle_autobuff_buy(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd);
void ab_handle_autobuff_sell(std::ostringstream& os_buf, int index, script_state* st, map_session_data* sd);


//Start the ab
bool ab_handleAutobuff_fromitem(TBL_PC * sd, t_itemid item_id, t_tick max_duration);

//Set script
void ab_handleAutoHeal(const std::vector<std::string>&result, TBL_PC * sd, struct party_data* p);
void ab_handleAutoPotion(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handleFollowPlayer(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handleAutoBuffSkills(const std::vector<std::string>&result, TBL_PC * sd, struct party_data* p);
void ab_handleDistanceToLeader(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handleAutoBuffItems(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handleAutoResurrection(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handlePMConfiguration(const std::vector<std::string>&result, TBL_PC * sd, struct party_data* p);
void ab_handleAutoBuffPotionState(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handleReturnToSavepoint(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handleResetAutoBuffConfig(TBL_PC * sd);
void ab_handleTokenOfSiegfried(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handleDisableWhenAlone(const std::vector<std::string>&result, TBL_PC * sd);
void ab_handlePriorizeBuff(const std::vector<std::string>&result, TBL_PC * sd);
int ab_handleGetautobuffint(TBL_PC * sd, const int value);
void ab_handleAutoPotionPitcher(const std::vector<std::string>&result, TBL_PC * sd, struct party_data* p);
void ab_handleSitRestHp(const std::vector<std::string>& result, map_session_data* sd);
void ab_handleSitRestSp(const std::vector<std::string>& result, map_session_data* sd);
void ab_handleTeleportCapDmg(const std::vector<std::string>& result, map_session_data* sd);
void ab_handleTeleportMinHp(const std::vector<std::string>& result, map_session_data* sd);
void ab_handleAutoBuy(const std::vector<std::string>& result, map_session_data* sd);
void ab_handleAutoSell(const std::vector<std::string>& result, map_session_data* sd);

#endif // AB_SCRIPT_H
