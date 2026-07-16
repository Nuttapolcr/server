// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vending.hpp"

#include <stdlib.h> // atoi

#include <common/malloc.hpp> // aMalloc, aFree
#include <common/nullpo.hpp>
#include <common/showmsg.hpp> // ShowInfo
#include <common/strlib.hpp>
#include <common/timer.hpp>  // DIFF_TICK

#include "achievement.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "buyingstore.hpp"
#include "buyingstore.hpp" // struct s_autotrade_entry, struct s_autotrader
#include "chrif.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "npc.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"

static uint32 vending_nextid = 0; ///Vending_id counter
static DBMap* vending_db; ///DB holder the vender : charid -> map_session_data

//Autotrader
static DBMap* vending_autotrader_db; /// Holds autotrader info: char_id -> struct s_autotrader
static void vending_autotrader_remove(struct s_autotrader* at, bool remove);
static int vending_autotrader_free(DBKey key, DBData* data, va_list ap);

/**
 * Lookup to get the vending_db outside module
 * @return the vending_db
 */
DBMap* vending_getdb()
{
	return vending_db;
}

/**
 * Create an unique vending shop id.
 * @return the next vending_id
 */
static int vending_getuid(void)
{
	return ++vending_nextid;
}

/**
 * Make a player close his shop
 * @param sd : player session
 */
void vending_closevending(map_session_data* sd)
{
	nullpo_retv(sd);

	if (sd->state.vending) {
		if (Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE vending_id = %d;", vending_items_table, sd->vender_id) != SQL_SUCCESS ||
			Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `id` = %d;", vendings_table, sd->vender_id) != SQL_SUCCESS) {
			Sql_ShowDebug(mmysql_handle);
		}

		sd->state.vending = false;
		sd->state.open_extended_vending = false;
		sd->vender_id = 0;
		clif_closevendingboard(&sd->bl, 0);
		idb_remove(vending_db, sd->status.char_id);
	}
}

/**
 * Player request a shop's item list (a player shop)
 * @param sd : player requestion the list
 * @param id : vender account id (gid)
 */
void vending_vendinglistreq(map_session_data* sd, int id)
{
	map_session_data* vsd;
	nullpo_retv(sd);

	if ((vsd = map_id2sd(id)) == NULL)
		return;
	if (!vsd->state.vending)
		return; // not vending

	if (!pc_can_give_items(sd) || !pc_can_give_items(vsd)) { //check if both GMs are allowed to trade
		clif_displaymessage(sd->fd, msg_txt(sd, 246));
		return;
	}

	sd->vended_id = vsd->vender_id;  // register vending uid

	if (vsd->state.open_extended_vending) {
		char output[CHAT_SIZE_MAX];
		std::shared_ptr<item_data> item = item_db.find(vsd->state.vending_item);
		sprintf(output, msg_txt(sd, 1541), item->ename.c_str());
		clif_broadcast(&sd->bl, output, strlen(output) + 1, BC_BLUE, SELF);
	}

	clif_vendinglist(sd, vsd);
}

/**
 * Calculates taxes for vending
 * @param sd: Vender
 * @param zeny: Total amount to tax
 * @return Total amount after taxes
 */
static double vending_calc_tax(map_session_data* sd, double zeny)
{
	if (battle_config.vending_tax && zeny >= battle_config.vending_tax_min)
		zeny -= zeny * (battle_config.vending_tax / 10000.);

	return zeny;
}

static void vending_add_item(map_session_data* sd, t_itemid nameid, int amount, t_itemid sell_nameid)
{
	nullpo_retv(sd);

	if (nameid == 0)
		return;

	std::shared_ptr<item_data> id = item_db.find(nameid);
	std::shared_ptr<item_data> sid = item_db.find(sell_nameid);

	if (!id || !sid)
		return;

	struct item it;
	memset(&it, 0, sizeof(it));
	it.nameid = nameid;
	it.identify = 1;

	pc_additem(sd, &it, amount, LOG_TYPE_VENDING);

	char output[CHAT_SIZE_MAX];
	sprintf(output, msg_txt(sd, 1548), item_db.create_item_link(id).c_str(), amount, item_db.create_item_link(sid).c_str());
	clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], output, false, SELF);
}



// Custom function to add commas to numbers
void format_number(char* output, int number, size_t size) {
	char temp[32];
	snprintf(temp, sizeof(temp), "%d", number);
	int len = strlen(temp);
	int comma_count = (len - 1) / 3;
	int out_pos = 0;

	for (int i = 0; i < len; i++) {
		if (i > 0 && (len - i) % 3 == 0 && comma_count > 0) {
			output[out_pos++] = ',';
			comma_count--;
		}
		output[out_pos++] = temp[i];
	}
	output[out_pos] = '\0';

	// Ensure we don't exceed buffer size
	if (out_pos >= size) {
		output[size - 1] = '\0';
	}
}
/**
 * Purchase item(s) from a shop
 * @param sd : buyer player session
 * @param aid : account id of vender
 * @param uid : shop unique id
 * @param data : items data who would like to purchase \n
 *	data := {<index>.w <amount>.w }[count]
 * @param count : number of different items he's trying to buy
 */
void vending_purchasereq(map_session_data* sd, int aid, int uid, const uint8* data, int count)
{
	int i, j, cursor, w, new_ = 0, blank, vend_list[MAX_VENDING];
	double z;
	struct s_vending vending[MAX_VENDING]; // against duplicate packets
	map_session_data* vsd = map_id2sd(aid);

	nullpo_retv(sd);
	if (vsd == NULL || !vsd->state.vending || vsd->bl.id == sd->bl.id)
		return; // invalid shop

	if (vsd->vender_id != uid) { // shop has changed
		clif_buyvending(sd, 0, 0, 6);  // store information was incorrect
		return;
	}

	if (!searchstore_queryremote(sd, aid) && (sd->bl.m != vsd->bl.m || !check_distance_bl(&sd->bl, &vsd->bl, AREA_SIZE)))
		return; // shop too far away

	searchstore_clearremote(sd);

	if (count < 1 || count > MAX_VENDING || count > vsd->vend_num)
		return; // invalid amount of purchased items

	blank = pc_inventoryblank(sd); // number of free cells in the buyer's inventory

	// duplicate item in vending to check hacker with multiple packets
	memcpy(&vending, &vsd->vending, sizeof(vsd->vending)); // copy vending list

	int ex_index = -1;
	bool extended = false;
	bool extendediscash = false;
	std::shared_ptr<item_data> id;
	int x = 0;
	int v_index = -1;
	t_itemid vending_item = vsd->state.vending_item;
	char output[CHAT_SIZE_MAX];
	id = item_db.find(vending_item);

	// Determine vending type
	if (vending_item) {
		for (const auto& it : extended_vending_lists) {
			if (it.nameid == vending_item) {
				extended = !it.is_zeny && !it.is_cash; // Item-based (option 0)
				extendediscash = it.is_cash;           // Cash-based (option 2)
				break;
			}
		}
	}

	// Some checks
	z = 0.; // zeny or cash counter
	w = 0;  // weight counter
	for (i = 0; i < count; i++) {
		short amount = *(uint16*)(data + 4 * i + 0);
		short idx = *(uint16*)(data + 4 * i + 2);
		idx -= 2;

		if (amount <= 0)
			return;

		// Check item index in the cart
		if (idx < 0 || idx >= MAX_CART)
			return;

		ARR_FIND(0, vsd->vend_num, j, vsd->vending[j].index == idx);
		if (j == vsd->vend_num)
			return; // picked non-existing item
		else
			vend_list[i] = j;

		// Calculate cost based on vending type
		if (extendediscash) {
			z += ((double)vsd->vending[j].value * (double)amount);
			if (z > sd->cashPoints || z < 0. || z >(double)MAX_ZENY) { // Assuming MAX_CASH exists; adjust if needed
				sprintf(output, msg_txt(sd, 1544), id ? id->ename.c_str() : "Cash");
				clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], "ท่านมี Cash ไม่เพียงพอ", false, SELF);
				return;
			}
		}
		else if (extended) {
			ex_index = pc_search_inventory(sd, vending_item);
			if (ex_index == -1) {
				sprintf(output, msg_txt(sd, 1543), id ? id->ename.c_str() : "Item");
				clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], output, false, SELF);
				return;
			}
			x = vsd->vending[j].value * amount;
			if (x > sd->inventory.u.items_inventory[ex_index].amount) {
				sprintf(output, msg_txt(sd, 1544), id ? id->ename.c_str() : "Item", x);
				clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], output, false, SELF);
				return;
			}
			v_index = idx;
		}
		else {
			z += ((double)vsd->vending[j].value * (double)amount);
			if (z > (double)sd->status.zeny || z < 0. || z >(double)MAX_ZENY) {
				clif_buyvending(sd, idx, amount, 1); // not enough zeny
				return;
			}
			if (z + (double)vsd->status.zeny > (double)MAX_ZENY && !battle_config.vending_over_max) {
				clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // too much zeny = overflow
				return;
			}
		}

		w += itemdb_weight(vsd->cart.u.items_cart[idx].nameid) * amount;
		if (w + sd->weight > sd->max_weight) {
			clif_buyvending(sd, idx, amount, 2); // overweight
			return;
		}

		// Check cart/vend sync
		if (vending[j].amount > vsd->cart.u.items_cart[idx].amount)
			vending[j].amount = vsd->cart.u.items_cart[idx].amount;

		if (vending[j].amount < amount) {
			clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // not enough quantity
			return;
		}

		vending[j].amount -= amount;

		switch (pc_checkadditem(sd, vsd->cart.u.items_cart[idx].nameid, amount)) {
		case CHKADDITEM_EXIST:
			break;
		case CHKADDITEM_NEW:
			new_++;
			if (new_ > blank)
				return; // no space
			break;
		case CHKADDITEM_OVERAMOUNT:
			return; // too many items
		}
	}

	// Execute transaction
	if (extendediscash) {
		pc_paycash(sd, (int)z, 0, LOG_TYPE_VENDING);
		pc_getcash(vsd, (int)z, 0, LOG_TYPE_VENDING);

		// Add mail notification for vendor
		struct mail_message msg = {};
		msg.dest_id = vsd->status.char_id;
		msg.status = MAIL_NEW;
		msg.type = MAIL_INBOX_NORMAL;
		msg.timestamp = time(NULL);

		char send_name[CHAT_SIZE_MAX];
		sprintf(send_name, msg_txt(sd, 1547));
		safestrncpy(msg.send_name, send_name, NAME_LENGTH);

		char msg_title[CHAT_SIZE_MAX];
		sprintf(msg_title, msg_txt(sd, 1545));
		safestrncpy(msg.title, msg_title, MAIL_TITLE_LENGTH);

		char body[512];  // Increased to 512 to support 500 characters
		char buyer_msg[CHAT_SIZE_MAX];
		char item_list[CHAT_SIZE_MAX] = "";       // For short buyer summary
		char item_details[CHAT_SIZE_MAX] = "";    // For vendor mail body

		// Build the list of item names and item details
		for (int i = 0; i < count; i++) {
			int vend_idx = vend_list[i];
			int cart_idx = vsd->vending[vend_idx].index;
			short amount = *(uint16*)(data + 4 * i + 0);  // Get the actual purchased amount from the client data
			int total_item_cost = vsd->vending[vend_idx].value * amount;  // Calculate total cost for this item

			std::shared_ptr<item_data> itm = item_db.find(vsd->cart.u.items_cart[cart_idx].nameid);
			const char* item_name = itm ? itm->ename.c_str() : "an item";

			// Append to buyer message
			if (i > 0)
				strncat(item_list, ", ", CHAT_SIZE_MAX - strlen(item_list) - 1);
			char buyer_item[128];
			snprintf(buyer_item, sizeof(buyer_item), "%d %s", amount, item_name);
			strncat(item_list, buyer_item, CHAT_SIZE_MAX - strlen(item_list) - 1);

			// Append to vendor message with individual item value and amount
			char line[128];
			char value_str[32];
			format_number(value_str, total_item_cost, sizeof(value_str));  // Show total cost instead of unit price
			snprintf(line, sizeof(line), "%d. %d %s for %s cash\r\n", i + 1, amount, item_name, value_str);
			strncat(item_details, line, CHAT_SIZE_MAX - strlen(item_details) - 1);
		}

		// Format vendor's mail body (with detailed item lines)
		char time_str[64];
		char total_str[32];
		time_t now = time(NULL);
		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
		format_number(total_str, (int)z, sizeof(total_str));
		snprintf(body, sizeof(body),
			"Vending Report!\r\n'%s' has bought\r\n\r\nthe following items:\r\n%s\r\nTotal: +%s cash\r\nTime: %s",
			sd->status.name, item_details, total_str, time_str);
		safestrncpy(msg.body, body, MAIL_BODY_LENGTH);  // Now 500 characters after mmo.hpp update

		// Format and send the buyer's message (summary style)
		char buyer_total_str[32];
		format_number(buyer_total_str, (int)z, sizeof(buyer_total_str));
		snprintf(buyer_msg, CHAT_SIZE_MAX,
			"You have bought %s for %s cash from %s's stall",
			item_list, buyer_total_str, vsd->status.name);
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], buyer_msg, false, SELF);

		intif_Mail_send(0, &msg);
		chrif_save(sd, CSAVE_INVENTORY);
	}

	else if (extended) {
		pc_delitem(sd, ex_index, x, 0, 0, LOG_TYPE_VENDING);

		std::shared_ptr<item_data> itm = item_db.find(vsd->cart.u.items_cart[v_index].nameid);
		if (battle_config.extended_vending_mail) {
			struct mail_message msg = {};
			msg.dest_id = vsd->status.char_id;
			msg.item[0].nameid = vending_item;
			msg.item[0].identify = 1;
			msg.item[0].amount = x;
			msg.status = MAIL_NEW;
			msg.type = MAIL_INBOX_NORMAL;
			msg.timestamp = time(NULL);

			char send_name[CHAT_SIZE_MAX];
			sprintf(send_name, msg_txt(sd, 1547));
			safestrncpy(msg.send_name, send_name, NAME_LENGTH);
			char msg_title[CHAT_SIZE_MAX];
			sprintf(msg_title, msg_txt(sd, 1545));
			safestrncpy(msg.title, msg_title, MAIL_TITLE_LENGTH);
			char body[512];  // Increased to 512 to support 500 characters
			sprintf(body, msg_txt(sd, 1546), itm->ename.c_str(), id->ename.c_str(), x);
			safestrncpy(msg.body, body, MAIL_BODY_LENGTH);  // Now 500 characters after mmo.hpp update

			intif_Mail_send(0, &msg);
		}
		else {
			vending_add_item(vsd, vending_item, x, itm->nameid);
		}
	}
	else {
		pc_payzeny(sd, (int)z, LOG_TYPE_VENDING, vsd->status.char_id);
		achievement_update_objective(sd, AG_SPEND_ZENY, 1, (int)z);
		z = vending_calc_tax(sd, z);
		pc_getzeny(vsd, (int)z, LOG_TYPE_VENDING, sd->status.char_id);

		// Add mail notification for vendor
		struct mail_message msg = {};
		msg.dest_id = vsd->status.char_id;
		msg.status = MAIL_NEW;
		msg.type = MAIL_INBOX_NORMAL;
		msg.timestamp = time(NULL);

		char send_name[CHAT_SIZE_MAX];
		sprintf(send_name, msg_txt(sd, 1547));
		safestrncpy(msg.send_name, send_name, NAME_LENGTH);

		char msg_title[CHAT_SIZE_MAX];
		sprintf(msg_title, msg_txt(sd, 1545));
		safestrncpy(msg.title, msg_title, MAIL_TITLE_LENGTH);

		char body[512];  // Increased to 512 to support 500 characters
		char buyer_msg[CHAT_SIZE_MAX];
		char item_list[CHAT_SIZE_MAX] = "";       // For short buyer summary
		char item_details[CHAT_SIZE_MAX] = "";    // For vendor mail body

		// Build the list of item names and item details
		for (int i = 0; i < count; i++) {
			int vend_idx = vend_list[i];
			int cart_idx = vsd->vending[vend_idx].index;
			short amount = *(uint16*)(data + 4 * i + 0);  // Get the actual purchased amount from the client data
			int total_item_cost = vsd->vending[vend_idx].value * amount;  // Calculate total cost for this item

			std::shared_ptr<item_data> itm = item_db.find(vsd->cart.u.items_cart[cart_idx].nameid);
			const char* item_name = itm ? itm->ename.c_str() : "an item";

			// Append to buyer message
			if (i > 0)
				strncat(item_list, ", ", CHAT_SIZE_MAX - strlen(item_list) - 1);
			char buyer_item[128];
			snprintf(buyer_item, sizeof(buyer_item), "%d %s", amount, item_name);
			strncat(item_list, buyer_item, CHAT_SIZE_MAX - strlen(item_list) - 1);

			// Append to vendor message with individual item value and amount
			char line[128];
			char value_str[32];
			format_number(value_str, total_item_cost, sizeof(value_str));  // Show total cost instead of unit price
			snprintf(line, sizeof(line), "%d. %d %s for %s zeny\r\n", i + 1, amount, item_name, value_str);
			strncat(item_details, line, CHAT_SIZE_MAX - strlen(item_details) - 1);
		}

		// Format vendor's mail body (with detailed item lines)
		char time_str[64];
		char total_str[32];
		time_t now = time(NULL);
		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
		format_number(total_str, (int)z, sizeof(total_str));
		snprintf(body, sizeof(body),
			"Vending Report!\r\n'%s' has bought\r\n\r\nthe following items:\r\n%s\r\nTotal: +%s zeny\r\nTime: %s",
			sd->status.name, item_details, total_str, time_str);
		safestrncpy(msg.body, body, MAIL_BODY_LENGTH);  // Now 500 characters after mmo.hpp update

		// Format and send the buyer's message (summary style)
		char buyer_total_str[32];
		format_number(buyer_total_str, (int)z, sizeof(buyer_total_str));
		snprintf(buyer_msg, CHAT_SIZE_MAX,
			"You have bought %s for %s zeny from %s's stall",
			item_list, buyer_total_str, vsd->status.name);
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], buyer_msg, false, SELF);

		intif_Mail_send(0, &msg);
		chrif_save(sd, CSAVE_INVENTORY);
	}




	// Update vending list
	for (i = 0; i < count; i++) {
		short amount = *(uint16*)(data + 4 * i + 0);
		short idx = *(uint16*)(data + 4 * i + 2);
		idx -= 2;

		pc_additem(sd, &vsd->cart.u.items_cart[idx], amount, LOG_TYPE_VENDING);
		vsd->vending[vend_list[i]].amount -= amount;

		if (vsd->vending[vend_list[i]].amount) {
			if (Sql_Query(mmysql_handle, "UPDATE `%s` SET `amount` = %d WHERE `vending_id` = %d AND `cartinventory_id` = %d",
				vending_items_table, vsd->vending[vend_list[i]].amount, vsd->vender_id, vsd->cart.u.items_cart[idx].id) != SQL_SUCCESS) {
				Sql_ShowDebug(mmysql_handle);
			}
		}
		else {
			if (Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `vending_id` = %d AND `cartinventory_id` = %d",
				vending_items_table, vsd->vender_id, vsd->cart.u.items_cart[idx].id) != SQL_SUCCESS) {
				Sql_ShowDebug(mmysql_handle);
			}
		}

		pc_cart_delitem(vsd, idx, amount, 0, LOG_TYPE_VENDING);

		if (extended || extendediscash) {
			clif_vendingreport(vsd, idx, amount, sd->status.char_id, extended ? x : (int)z);
		}
		else {
			z = vending_calc_tax(sd, z);
			clif_vendingreport(vsd, idx, amount, sd->status.char_id, (int)z);
		}

		if (battle_config.buyer_name) {
			char temp[256];
			sprintf(temp, msg_txt(sd, 265), sd->status.name);
			clif_messagecolor(&vsd->bl, color_table[COLOR_LIGHT_GREEN], temp, false, SELF);
		}
	}

	// Compact vending list
	for (i = 0, cursor = 0; i < vsd->vend_num; i++) {
		if (vsd->vending[i].amount == 0)
			continue;

		if (cursor != i) {
			vsd->vending[cursor].index = vsd->vending[i].index;
			vsd->vending[cursor].amount = vsd->vending[i].amount;
			vsd->vending[cursor].value = vsd->vending[i].value;
		}
		cursor++;
	}
	vsd->vend_num = cursor;

	// Save both buyer and seller
	if (save_settings & CHARSAVE_VENDING) {
		chrif_save(sd, CSAVE_INVENTORY | CSAVE_CART);
		chrif_save(vsd, CSAVE_INVENTORY | CSAVE_CART);
	}

	// Check for autotrade
	if (vsd->state.autotrade) {
		ARR_FIND(0, vsd->vend_num, i, vsd->vending[i].amount > 0);
		if (i == vsd->vend_num) {
			vending_closevending(vsd);
			map_quit(vsd);
		}
	}
}

/**
 * Player setup a new shop
 * @param sd : player opening the shop
 * @param message : shop title
 * @param data : itemlist data
 *	data := {<index>.w <amount>.w <value>.l}[count]
 * @param count : number of different items
 * @param at Autotrader info, or NULL if requetsed not from autotrade persistance
 * @return 0 If success, 1 - Cannot open (die, not state.prevend, trading), 2 - No cart, 3 - Count issue, 4 - Cart data isn't saved yet, 5 - No valid item found
 */
int8 vending_openvending(map_session_data* sd, const char* message, const uint8* data, int count, struct s_autotrader* at)
{
	int i, j;
	int vending_skill_lvl;
	char message_sql[MESSAGE_SIZE * 2];
	StringBuf buf;

	nullpo_retr(false, sd);

	if (pc_isdead(sd) || !sd->state.prevend || pc_istrading(sd)) {
		return 1; // can't open vendings lying dead || didn't use via the skill (wpe/hack) || can't have 2 shops at once
	}

	vending_skill_lvl = pc_checkskill(sd, MC_VENDING);

	// skill level and cart check
	if (!vending_skill_lvl || !pc_iscarton(sd)) {
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0);
		return 2;
	}

	// check number of items in shop
	if (count < 1 || count > MAX_VENDING || count > 2 + vending_skill_lvl) { // invalid item count
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0);
		return 3;
	}

	if (save_settings & CHARSAVE_VENDING) // Avoid invalid data from saving
		chrif_save(sd, CSAVE_INVENTORY | CSAVE_CART);

	// filter out invalid items
	i = 0;
	for (j = 0; j < count; j++) {
		short index = *(uint16*)(data + 8 * j + 0);
		short amount = *(uint16*)(data + 8 * j + 2);
		unsigned int value = *(uint32*)(data + 8 * j + 4);

		index -= 2; // offset adjustment (client says that the first cart position is 2)

		if (index < 0 || index >= MAX_CART // invalid position
			|| pc_cartitem_amount(sd, index, amount) < 0 // invalid item or insufficient quantity
			//NOTE: official server does not do any of the following checks!
			|| !sd->cart.u.items_cart[index].identify // unidentified item
			|| sd->cart.u.items_cart[index].attribute == 1 // broken item
			|| sd->cart.u.items_cart[index].expire_time // It should not be in the cart but just in case
			|| (sd->cart.u.items_cart[index].bound && !pc_can_give_bounded_items(sd)) // can't trade account bound items and has no permission
			|| !itemdb_cantrade(&sd->cart.u.items_cart[index], pc_get_group_level(sd), pc_get_group_level(sd))) // untradeable item
			continue;

		sd->vending[i].index = index;
		sd->vending[i].amount = amount;
		sd->vending[i].value = min(value, (unsigned int)battle_config.vending_max_value);
		i++; // item successfully added
	}

	if (i != j) {
		clif_displaymessage(sd->fd, msg_txt(sd, 266)); //"Some of your items cannot be vended and were removed from the shop."
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0); // custom reply packet
		return 5;
	}

	if (i == 0) { // no valid item found
		clif_skill_fail(sd, MC_VENDING, USESKILL_FAIL_LEVEL, 0); // custom reply packet
		return 5;
	}

	sd->state.prevend = 0;
	sd->state.vending = true;
	sd->state.workinprogress = WIP_DISABLE_NONE;
	sd->vender_id = vending_getuid();
	sd->vend_num = i;
	safestrncpy(sd->message, message, MESSAGE_SIZE);

	Sql_EscapeString(mmysql_handle, message_sql, sd->message);

	if (Sql_Query(mmysql_handle, "INSERT INTO `%s`(`id`, `account_id`, `char_id`, `sex`, `map`, `x`, `y`, `title`, `autotrade`, `body_direction`, `head_direction`, `sit`, `vending_item`) "
		"VALUES( %d, %d, %d, '%c', '%s', %d, %d, '%s', %d, '%d', '%d', '%d', '%u' );",
		vendings_table, sd->vender_id, sd->status.account_id, sd->status.char_id, sd->status.sex == SEX_FEMALE ? 'F' : 'M', map_getmapdata(sd->bl.m)->name, sd->bl.x, sd->bl.y, message_sql, sd->state.autotrade, at ? at->dir : sd->ud.dir, at ? at->head_dir : sd->head_dir, at ? at->sit : pc_issit(sd), sd->state.vending_item) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`vending_id`,`index`,`cartinventory_id`,`amount`,`price`) VALUES", vending_items_table);
	for (j = 0; j < i; j++) {
		StringBuf_Printf(&buf, "(%d,%d,%d,%d,%d)", sd->vender_id, j, sd->cart.u.items_cart[sd->vending[j].index].id, sd->vending[j].amount, sd->vending[j].value);
		if (j < i - 1)
			StringBuf_AppendStr(&buf, ",");
	}
	if (SQL_ERROR == Sql_QueryStr(mmysql_handle, StringBuf_Value(&buf)))
		Sql_ShowDebug(mmysql_handle);
	StringBuf_Destroy(&buf);

	clif_openvending(sd, sd->bl.id, sd->vending);
	clif_showvendingboard(*sd);

	if (sd->state.open_extended_vending) {
		char output[CHAT_SIZE_MAX];
		std::shared_ptr<item_data> item = item_db.find(sd->state.vending_item);
		sprintf(output, msg_txt(sd, 1542), item->ename.c_str());
		clif_broadcast(&sd->bl, output, strlen(output) + 1, BC_BLUE, SELF);
	}

	idb_put(vending_db, sd->status.char_id, sd);

	return 0;
}

/**
 * Checks if an item is being sold in given player's vending.
 * @param sd : vender session (player)
 * @param nameid : item id
 * @return 0:not selling it, 1: yes
 */
bool vending_search(map_session_data* sd, t_itemid nameid)
{
	int i;

	if (!sd->state.vending) { // not vending
		return false;
	}

	ARR_FIND(0, sd->vend_num, i, sd->cart.u.items_cart[sd->vending[i].index].nameid == nameid);
	if (i == sd->vend_num) { // not found
		return false;
	}

	return true;
}

/**
 * Searches for all items in a vending, that match given ids, price and possible cards.
 * @param sd : The vender session to search into
 * @param s : parameter of the search (see s_search_store_search)
 * @return Whether or not the search should be continued.
 */
bool vending_searchall(map_session_data* sd, const struct s_search_store_search* s)
{
	int i, c, slot;
	unsigned int idx, cidx;
	struct item* it;

	if (!sd->state.vending) // not vending
		return true;

	for (idx = 0; idx < s->item_count; idx++) {
		ARR_FIND(0, sd->vend_num, i, sd->cart.u.items_cart[sd->vending[i].index].nameid == s->itemlist[idx].itemId);
		if (i == sd->vend_num) { // not found
			continue;
		}
		it = &sd->cart.u.items_cart[sd->vending[i].index];

		if (s->min_price && s->min_price > sd->vending[i].value) { // too low price
			continue;
		}

		if (s->max_price && s->max_price < sd->vending[i].value) { // too high price
			continue;
		}

		if (s->card_count) { // check cards
			if (itemdb_isspecial(it->card[0])) { // something, that is not a carded
				continue;
			}
			slot = itemdb_slots(it->nameid);

			for (c = 0; c < slot && it->card[c]; c++) {
				ARR_FIND(0, s->card_count, cidx, s->cardlist[cidx].itemId == it->card[c]);
				if (cidx != s->card_count) { // found
					break;
				}
			}

			if (c == slot || !it->card[c]) { // no card match
				continue;
			}
		}

		// Check if the result set is full
		if (s->search_sd->searchstore.items.size() >= (unsigned int)battle_config.searchstore_maxresults) {
			return false;
		}

		std::shared_ptr<s_search_store_info_item> ssitem = std::make_shared<s_search_store_info_item>();

		ssitem->store_id = sd->vender_id;
		ssitem->account_id = sd->status.account_id;
		safestrncpy(ssitem->store_name, sd->message, sizeof(ssitem->store_name));
		ssitem->nameid = it->nameid;
		ssitem->amount = sd->vending[i].amount;
		ssitem->price = sd->vending[i].value;
		for (int j = 0; j < MAX_SLOTS; j++) {
			ssitem->card[j] = it->card[j];
		}
		ssitem->refine = it->refine;
		ssitem->enchantgrade = it->enchantgrade;

		s->search_sd->searchstore.items.push_back(ssitem);
	}

	return true;
}

/**
* Open vending for Autotrader
* @param sd Player as autotrader
*/
void vending_reopen(map_session_data* sd)
{
	struct s_autotrader* at = NULL;
	int8 fail = -1;

	nullpo_retv(sd);

	// Open vending for this autotrader
	if ((at = (struct s_autotrader*)uidb_get(vending_autotrader_db, sd->status.char_id)) && at->count && at->entries) {
		uint8* data, * p;
		uint16 j, count;

		// Init vending data for autotrader
		CREATE(data, uint8, at->count * 8);

		for (j = 0, p = data, count = at->count; j < at->count; j++) {
			struct s_autotrade_entry* entry = at->entries[j];
			uint16* index = (uint16*)(p + 0);
			uint16* amount = (uint16*)(p + 2);
			uint32* value = (uint32*)(p + 4);

			// Find item position in cart
			ARR_FIND(0, MAX_CART, entry->index, sd->cart.u.items_cart[entry->index].id == entry->cartinventory_id);

			if (entry->index == MAX_CART) {
				count--;
				continue;
			}

			*index = entry->index + 2;
			*amount = itemdb_isstackable(sd->cart.u.items_cart[entry->index].nameid) ? entry->amount : 1;
			*value = entry->price;

			p += 8;
		}

		sd->state.prevend = 1; // Set him into a hacked prevend state
		sd->state.autotrade = 1;

		// Make sure abort all NPCs
		npc_event_dequeue(sd);
		pc_cleareventtimer(sd);

		// Open the vending again
		if ((fail = vending_openvending(sd, at->title, data, count, at)) == 0) {
			// Make vendor look perfect
			pc_setdir(sd, at->dir, at->head_dir);
			clif_changed_dir(&sd->bl, AREA_WOS);
			if (at->sit) {
				pc_setsit(sd);
				skill_sit(sd, 1);
				clif_sitting(&sd->bl);
			}

			// Immediate save
			chrif_save(sd, CSAVE_AUTOTRADE);

			ShowInfo("Vending loaded for '" CL_WHITE "%s" CL_RESET "' with '" CL_WHITE "%d" CL_RESET "' items at " CL_WHITE "%s (%d,%d)" CL_RESET "\n",
				sd->status.name, count, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y);
		}
		aFree(data);
	}

	if (at) {
		vending_autotrader_remove(at, true);
		if (db_size(vending_autotrader_db) == 0)
			vending_autotrader_db->clear(vending_autotrader_db, vending_autotrader_free);
	}

	if (fail != 0) {
		ShowError("vending_reopen: (Error:%d) Load failed for autotrader '" CL_WHITE "%s" CL_RESET "' (CID=%d/AID=%d)\n", fail, sd->status.name, sd->status.char_id, sd->status.account_id);
		map_quit(sd);
	}
}

/**
* Initializing autotraders from table
*/
void do_init_vending_autotrade(void)
{
	if (battle_config.feature_autotrade) {
		if (Sql_Query(mmysql_handle,
			"SELECT `id`, `account_id`, `char_id`, `sex`, `title`, `body_direction`, `head_direction`, `sit`, `vending_item` "
			"FROM `%s` "
			"WHERE `autotrade` = 1 AND (SELECT COUNT(`vending_id`) FROM `%s` WHERE `vending_id` = `id`) > 0 "
			"ORDER BY `id`;",
			vendings_table, vending_items_table) != SQL_SUCCESS)
		{
			Sql_ShowDebug(mmysql_handle);
			return;
		}

		if (Sql_NumRows(mmysql_handle) > 0) {
			uint16 items = 0;
			DBIterator* iter = NULL;
			struct s_autotrader* at = NULL;

			// Init each autotrader data
			while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
				size_t len;
				char* data;

				t_itemid vending_item = 0;

				at = NULL;
				CREATE(at, struct s_autotrader, 1);
				Sql_GetData(mmysql_handle, 0, &data, NULL); at->id = atoi(data);
				Sql_GetData(mmysql_handle, 1, &data, NULL); at->account_id = atoi(data);
				Sql_GetData(mmysql_handle, 2, &data, NULL); at->char_id = atoi(data);
				Sql_GetData(mmysql_handle, 3, &data, NULL); at->sex = (data[0] == 'F') ? SEX_FEMALE : SEX_MALE;
				Sql_GetData(mmysql_handle, 4, &data, &len); safestrncpy(at->title, data, zmin(len + 1, MESSAGE_SIZE));
				Sql_GetData(mmysql_handle, 5, &data, NULL); at->dir = atoi(data);
				Sql_GetData(mmysql_handle, 6, &data, NULL); at->head_dir = atoi(data);
				Sql_GetData(mmysql_handle, 7, &data, NULL); at->sit = atoi(data);
				Sql_GetData(mmysql_handle, 8, &data, NULL); vending_item = atoi(data);
				at->count = 0;

				if (battle_config.feature_autotrade_direction >= 0)
					at->dir = battle_config.feature_autotrade_direction;
				if (battle_config.feature_autotrade_head_direction >= 0)
					at->head_dir = battle_config.feature_autotrade_head_direction;
				if (battle_config.feature_autotrade_sit >= 0)
					at->sit = battle_config.feature_autotrade_sit;

				// initialize player
				CREATE(at->sd, map_session_data, 1); // TODO: Dont use Memory Manager allocation anymore and rely on the C++ container
				pc_setnewpc(at->sd, at->account_id, at->char_id, 0, gettick(), at->sex, 0);
				at->sd->state.autotrade = 1 | 2;
				at->sd->state.vending_item = vending_item;
				if (battle_config.autotrade_monsterignore)
					at->sd->state.block_action |= PCBLOCK_IMMUNE;
				else
					at->sd->state.block_action &= ~PCBLOCK_IMMUNE;
				chrif_authreq(at->sd, true);
				uidb_put(vending_autotrader_db, at->char_id, at);
			}
			Sql_FreeResult(mmysql_handle);

			// Init items for each autotraders
			iter = db_iterator(vending_autotrader_db);
			for (at = (struct s_autotrader*)dbi_first(iter); dbi_exists(iter); at = (struct s_autotrader*)dbi_next(iter)) {
				uint16 j = 0;

				if (SQL_ERROR == Sql_Query(mmysql_handle,
					"SELECT `cartinventory_id`, `amount`, `price` "
					"FROM `%s` "
					"WHERE `vending_id` = %d "
					"ORDER BY `index` ASC;",
					vending_items_table, at->id))
				{
					Sql_ShowDebug(mmysql_handle);
					continue;
				}

				if (!(at->count = (uint16)Sql_NumRows(mmysql_handle))) {
					map_quit(at->sd);
					vending_autotrader_remove(at, true);
					continue;
				}

				//Init the list
				CREATE(at->entries, struct s_autotrade_entry*, at->count);

				//Add the item into list
				j = 0;
				while (SQL_SUCCESS == Sql_NextRow(mmysql_handle) && j < at->count) {
					char* data;
					CREATE(at->entries[j], struct s_autotrade_entry, 1);
					Sql_GetData(mmysql_handle, 0, &data, NULL); at->entries[j]->cartinventory_id = atoi(data);
					Sql_GetData(mmysql_handle, 1, &data, NULL); at->entries[j]->amount = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, NULL); at->entries[j]->price = atoi(data);
					j++;
				}
				items += j;
				Sql_FreeResult(mmysql_handle);
			}
			dbi_destroy(iter);

			ShowStatus("Done loading '" CL_WHITE "%d" CL_RESET "' vending autotraders with '" CL_WHITE "%d" CL_RESET "' items.\n", db_size(vending_autotrader_db), items);
		}
	}

	// Everything is loaded fine, their entries will be reinserted once they are loaded
	if (Sql_Query(mmysql_handle, "DELETE FROM `%s`;", vendings_table) != SQL_SUCCESS ||
		Sql_Query(mmysql_handle, "DELETE FROM `%s`;", vending_items_table) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
 * Remove an autotrader's data
 * @param at Autotrader
 * @param remove If true will removes from vending_autotrader_db
 **/
static void vending_autotrader_remove(struct s_autotrader* at, bool remove) {
	nullpo_retv(at);
	if (at->count && at->entries) {
		uint16 i = 0;
		for (i = 0; i < at->count; i++) {
			if (at->entries[i])
				aFree(at->entries[i]);
		}
		aFree(at->entries);
	}
	if (remove)
		uidb_remove(vending_autotrader_db, at->char_id);
	aFree(at);
}

/**
* Clear all autotraders
* @author [Cydh]
*/
static int vending_autotrader_free(DBKey key, DBData* data, va_list ap) {
	struct s_autotrader* at = (struct s_autotrader*)db_data2ptr(data);
	if (at)
		vending_autotrader_remove(at, false);
	return 0;
}

/**
* Update vendor location
* @param sd: Player's session data
*/
void vending_update(map_session_data& sd)
{
	if (Sql_Query(mmysql_handle, "UPDATE `%s` SET `map` = '%s', `x` = '%d', `y` = '%d', `body_direction` = '%d', `head_direction` = '%d', `sit` = '%d', `autotrade` = '%d' WHERE `id` = '%d'",
		vendings_table, map_getmapdata(sd.bl.m)->name, sd.bl.x, sd.bl.y, sd.ud.dir, sd.head_dir, pc_issit(&sd), sd.state.autotrade,
		sd.vender_id
	) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
 * Initialise the vending module
 * called in map::do_init
 */
void do_final_vending(void)
{
	db_destroy(vending_db);
	vending_autotrader_db->destroy(vending_autotrader_db, vending_autotrader_free);
}

/**
 * Destory the vending module
 * called in map::do_final
 */
void do_init_vending(void)
{
	vending_db = idb_alloc(DB_OPT_BASE);
	vending_autotrader_db = uidb_alloc(DB_OPT_BASE);
	vending_nextid = 0;
}
