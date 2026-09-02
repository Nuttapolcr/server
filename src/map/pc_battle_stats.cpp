#include "pc_battle_stats.hpp"

#include "common/nullpo.hpp"
#include "common/socket.hpp"
#include "atcommand.hpp"

#include <cmath>
#include <cstdlib>
#include <set>
#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/cli.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "achievement.hpp"
#include "battle.hpp"
#include "buyingstore.hpp"
#include "channel.hpp"
#include "chat.hpp"
#include "chrif.hpp"
#include "clan.hpp"
#include "clif.hpp"
#include "duel.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "itemdb.hpp" // MAX_ITEMGROUP
#include "cashshop.hpp"
#include "log.hpp"
#include "mail.hpp"
#include "map.hpp"
#include "mapreg.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"
#include "quest.hpp"
#include "script.hpp"
#include "storage.hpp"
#include "trade.hpp"
#include "vending.hpp"




#define CHECK_STATUS_MENU_COUNT 4 // ???????? 3 ???????????????????? Zeny
#define CS_EQUIPMENT 785
#define CS_STATS 786
#define CS_ZENY 787
#define CS_ITEMLIST 788

/// Check Equip
/// 0442 <Length>.W <count>.L <Skill_list>.W (ZC_SKILL_SELECT_REQUEST).
int pcb_display_menu(struct map_session_data* sd) {
	nullpo_ret(sd);
	if (!sd->fd) return 0;

	unsigned short skills[CHECK_STATUS_MENU_COUNT] = { CS_EQUIPMENT, CS_STATS, CS_ZENY, CS_ITEMLIST };
	WFIFOHEAD(sd->fd, 8 + CHECK_STATUS_MENU_COUNT * 2);
	WFIFOW(sd->fd, 0) = 0x442;
	for (int i = 0; i < CHECK_STATUS_MENU_COUNT; i++) {
		WFIFOW(sd->fd, 8 + i * 2) = skills[i];
	}
	WFIFOW(sd->fd, 2) = 8 + CHECK_STATUS_MENU_COUNT * 2;
	WFIFOL(sd->fd, 4) = 3;
	WFIFOSET(sd->fd, WFIFOW(sd->fd, 2));

	sd->menuskill_id = SC_AUTOSHADOWSPELL;
	sd->menuskill_val = 3;
	sd->state.check_equip_skill = true;
	sd->state.workinprogress = WIP_DISABLE_ALL;

	return 1;
}

void pcb_display(int fd, struct map_session_data* sd) {
	status_data* status = status_get_status_data(&sd->bl);
	char output[CHAT_SIZE_MAX];

#define battlestats_msg(__msg__) (__msg__); clif_displaymessage(fd, output)

	clif_displaymessage(fd, "============ Ê¶Ò¹Ð¼ÙéàÅè¹â´ÂÃÇÁ ============");
	battlestats_msg(sprintf(output, "ª×èÍ: %s (%s) Lv. %d", sd->status.name, job_name(sd->status.class_), sd->status.base_level));
	clif_displaymessage(fd, "|--- ÊàµµÑÊ ---|");
	battlestats_msg(sprintf(output, "   -> Hp: %d", sd->status.hp));
	battlestats_msg(sprintf(output, "   -> Maxhp: %d", sd->status.max_hp));
	battlestats_msg(sprintf(output, "   -> Sp: %d", sd->status.sp));
	battlestats_msg(sprintf(output, "   -> Maxsp: %d", sd->status.max_sp));
	battlestats_msg(sprintf(output, "   -> Str: %d + %d", sd->status.str, sd->battle_status.str - sd->status.str));
	battlestats_msg(sprintf(output, "   -> agi: %d + %d", sd->status.agi, sd->battle_status.agi - sd->status.agi));
	battlestats_msg(sprintf(output, "   -> Vit: %d + %d", sd->status.vit, sd->battle_status.vit - sd->status.vit));
	battlestats_msg(sprintf(output, "   -> Int: %d + %d", sd->status.int_, sd->battle_status.int_ - sd->status.int_));
	battlestats_msg(sprintf(output, "   -> Dex: %d + %d", sd->status.dex, sd->battle_status.dex - sd->status.dex));
	battlestats_msg(sprintf(output, "   -> Luk: %d + %d", sd->status.luk, sd->battle_status.luk - sd->status.luk));
	battlestats_msg(sprintf(output, "   -> ATK: %d + %d", pc_leftside_atk(sd), pc_rightside_atk(sd)));
	battlestats_msg(sprintf(output, "   -> MATK: %d + %d", pc_leftside_matk(sd), pc_rightside_matk(sd)));
	battlestats_msg(sprintf(output, "   -> HIT: %d", status->hit));
	battlestats_msg(sprintf(output, "   -> CRIT: %d", status->cri / 10));
	battlestats_msg(sprintf(output, "   -> DEF: %d + %d", pc_leftside_def(sd), pc_rightside_def(sd)));
	battlestats_msg(sprintf(output, "   -> MDEF: %d + %d", pc_leftside_mdef(sd), pc_rightside_mdef(sd)));
	battlestats_msg(sprintf(output, "   -> FLEE: %d + %d", status->flee, status->flee2 / 10));
	battlestats_msg(sprintf(output, "   -> ASPD: %d", (2000 - status->amotion) / 10));
//	battlestats_msg(sprintf(output, "   -> EXP +: %d%", sd->sc.getSCE(SC_EXPBOOST)));
//	battlestats_msg(sprintf(output, "   -> Drop +: %d%", sd->add_drop));

	clif_displaymessage(fd, "|--- âº¹ÑÊâ¨ÁµÕ¡ÒÂÀÒ¾ ---|");
	if (sd->right_weapon.addsize[SZ_SMALL] + sd->left_weapon.addsize[SZ_SMALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ¢¹Ò´àÅç¡ %d%%", sd->right_weapon.addsize[SZ_SMALL] + sd->left_weapon.addsize[SZ_SMALL]));
	}
	if (sd->right_weapon.addsize[SZ_MEDIUM] + sd->left_weapon.addsize[SZ_MEDIUM]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ¢¹Ò´¡ÅÒ§ %d%%", sd->right_weapon.addsize[SZ_MEDIUM] + sd->left_weapon.addsize[SZ_MEDIUM]));
	}
	if (sd->right_weapon.addsize[SZ_BIG] + sd->left_weapon.addsize[SZ_BIG]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ·Ø¡ãË­è %d%%", sd->right_weapon.addsize[SZ_BIG] + sd->left_weapon.addsize[SZ_BIG]));
	}
	if (sd->right_weapon.addsize[SZ_ALL] + sd->left_weapon.addsize[SZ_ALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ·Ø¡¢¹Ò´ %d%%", sd->right_weapon.addsize[SZ_ALL] + sd->left_weapon.addsize[SZ_ALL]));
	}
	if (sd->right_weapon.addrace[RC_PLAYER_HUMAN] + sd->left_weapon.addrace[RC_PLAYER_HUMAN]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ¼ÙéàÅè¹»ÃÐàÀ·Á¹ØÉÂì %d%%", sd->right_weapon.addrace[RC_PLAYER_HUMAN] + sd->left_weapon.addrace[RC_PLAYER_HUMAN]));
	}
	if (sd->right_weapon.addrace[RC_DEMIHUMAN] + sd->left_weapon.addrace[RC_DEMIHUMAN]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·Á¹ØÉÂì %d%%", sd->right_weapon.addrace[RC_DEMIHUMAN] + sd->left_weapon.addrace[RC_DEMIHUMAN]));
	}
	if (sd->right_weapon.addrace[RC_ANGEL] + sd->left_weapon.addrace[RC_ANGEL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·à·Ç·Ùµ %d%%", sd->right_weapon.addrace[RC_ANGEL] + sd->left_weapon.addrace[RC_ANGEL]));
	}
	if (sd->right_weapon.addrace[RC_BRUTE] + sd->left_weapon.addrace[RC_BRUTE]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·ÊÑµÇì %d%%", sd->right_weapon.addrace[RC_BRUTE] + sd->left_weapon.addrace[RC_BRUTE]));
	}
	if (sd->right_weapon.addrace[RC_DEMON] + sd->left_weapon.addrace[RC_DEMON]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·»ÕÈÒ¨ %d%%", sd->right_weapon.addrace[RC_DEMON] + sd->left_weapon.addrace[RC_DEMON]));
	}
	if (sd->right_weapon.addrace[RC_DRAGON] + sd->left_weapon.addrace[RC_DRAGON]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·ÁÑ§¡Ã %d%%", sd->right_weapon.addrace[RC_DRAGON] + sd->left_weapon.addrace[RC_DRAGON]));
	}
	if (sd->right_weapon.addrace[RC_FISH] + sd->left_weapon.addrace[RC_FISH]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·»ÅÒ %d%%", sd->right_weapon.addrace[RC_FISH] + sd->left_weapon.addrace[RC_FISH]));
	}
	if (sd->right_weapon.addrace[RC_FORMLESS] + sd->left_weapon.addrace[RC_FORMLESS]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·äÃéÃÙ»ÃèÒ§ %d%%", sd->right_weapon.addrace[RC_FORMLESS] + sd->left_weapon.addrace[RC_FORMLESS]));
	}
	if (sd->right_weapon.addrace[RC_INSECT] + sd->left_weapon.addrace[RC_INSECT]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·áÁÅ§ %d%%", sd->right_weapon.addrace[RC_INSECT] + sd->left_weapon.addrace[RC_INSECT]));
	}
	if (sd->right_weapon.addrace[RC_PLANT] + sd->left_weapon.addrace[RC_PLANT]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·¾×ª %d%%", sd->right_weapon.addrace[RC_PLANT] + sd->left_weapon.addrace[RC_PLANT]));
	}
	if (sd->right_weapon.addrace[RC_UNDEAD] + sd->left_weapon.addrace[RC_UNDEAD]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·¼Õ´Ôº %d%%", sd->right_weapon.addrace[RC_UNDEAD] + sd->left_weapon.addrace[RC_UNDEAD]));
	}
	if (sd->right_weapon.addrace[RC_PLAYER_DORAM] + sd->left_weapon.addrace[RC_PLAYER_DORAM]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ»ÃÐàÀ·Doram %d%%", sd->right_weapon.addrace[RC_PLAYER_DORAM] + sd->left_weapon.addrace[RC_PLAYER_DORAM]));
	}
	if (sd->right_weapon.addrace[RC_ALL] + sd->left_weapon.addrace[RC_ALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ·Ø¡»ÃÐàÀ· %d%%", sd->right_weapon.addrace[RC_ALL] + sd->left_weapon.addrace[RC_ALL]));
	}
	if (sd->right_weapon.addclass[CLASS_NORMAL] + sd->left_weapon.addclass[CLASS_NORMAL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ¤ÅÒÊ»¡µÔ %d%%", sd->right_weapon.addclass[CLASS_NORMAL] + sd->left_weapon.addclass[CLASS_NORMAL]));
	}
	if (sd->right_weapon.addclass[CLASS_BOSS] + sd->left_weapon.addclass[CLASS_BOSS]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ¤ÅÒÊºÍÊ %d%%", sd->right_weapon.addclass[CLASS_BOSS] + sd->left_weapon.addclass[CLASS_BOSS]));
	}
	//PRE-RE
	if (sd->right_weapon.addclass[CLASS_ALL] + sd->left_weapon.addclass[CLASS_ALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕ·Ø¡¤ÅÒÊ %d%%", sd->right_weapon.addclass[CLASS_ALL] + sd->left_weapon.addclass[CLASS_ALL]));
	}


	clif_displaymessage(fd, "|--- âº¹ÑÊâ¨ÁµÕàÇ·Âì ---|");
	if (sd->indexed_bonus.magic_addsize[SZ_SMALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì¢¹Ò´àÅç¡ %d%%", sd->indexed_bonus.magic_addsize[SZ_SMALL]));
	}
	if (sd->indexed_bonus.magic_addsize[SZ_MEDIUM]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì¢¹Ò´¡ÅÒ§ %d%%", sd->indexed_bonus.magic_addsize[SZ_MEDIUM]));
	}
	if (sd->indexed_bonus.magic_addsize[SZ_BIG]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì¢¹Ò´ãË­è %d%%", sd->indexed_bonus.magic_addsize[SZ_BIG]));
	}
	if (sd->indexed_bonus.magic_addsize[SZ_ALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì·Ø¡¢¹Ò´ %d%%", sd->indexed_bonus.magic_addsize[SZ_ALL]));
	}
	if (sd->indexed_bonus.magic_addrace[RC_PLAYER_HUMAN]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì»ÃÐàÀ·Á¹ØÉÂì %d%%", sd->indexed_bonus.magic_addrace[RC_PLAYER_HUMAN]));
	}
	if (sd->indexed_bonus.magic_addrace[RC_PLAYER_DORAM]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì»ÃÐàÀ·Doram: %d%%", sd->indexed_bonus.magic_addrace[RC_PLAYER_DORAM]));
	}
	if (sd->indexed_bonus.magic_addrace[RC_ALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì·Ø¡»ÃÐàÀ· %d%%", sd->indexed_bonus.magic_addrace[RC_ALL]));
	}
	if (sd->indexed_bonus.magic_addclass[CLASS_NORMAL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì¤ÅÒÊ»¡µÔ %d%%", sd->indexed_bonus.magic_addclass[CLASS_NORMAL]));
	}
	if (sd->indexed_bonus.magic_addclass[CLASS_BOSS]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì¤ÅÒÊºÍÊ %d%%", sd->indexed_bonus.magic_addclass[CLASS_BOSS]));
	}
	if (sd->indexed_bonus.magic_addclass[CLASS_ALL]) {
		battlestats_msg(sprintf(output, "   -> â¨ÁµÕàÇ·Âì·Ø¡»ÃÐàÀ· %d%%", sd->indexed_bonus.magic_addclass[CLASS_ALL]));
	}

	clif_displaymessage(fd, "|--- »éÍ§¡Ñ¹¸ÒµØ ---|");
	struct {
		const char* format;
		int value;
	} ele_resist_info[] = {
		{ "   -> äÃé¸ÒµØ %d%%", sd->indexed_bonus.subele_script[ELE_NEUTRAL] },
		{ "   -> ¹éÓ %d%%", sd->indexed_bonus.subele_script[ELE_WATER] },
		{ "   -> ´Ô¹ %d%%", sd->indexed_bonus.subele_script[ELE_EARTH] },
		{ "   -> ä¿ %d%%", sd->indexed_bonus.subele_script[ELE_FIRE] },
		{ "   -> ÅÁ %d%%", sd->indexed_bonus.subele_script[ELE_WIND] },
		{ "   -> ¾ÔÉ %d%%", sd->indexed_bonus.subele_script[ELE_POISON] },
		{ "   -> ÈÑ¡´ÔìÊÔ·¸Ôì %d%%", sd->indexed_bonus.subele_script[ELE_HOLY] },
		{ "   -> Á×´ %d%%", sd->indexed_bonus.subele_script[ELE_DARK] },
		{ "   -> ¼Õ %d%%", sd->indexed_bonus.subele_script[ELE_GHOST] },
		{ "   -> ¼Õ´Ôº %d%%", sd->indexed_bonus.subele_script[ELE_UNDEAD] },
		{ "   -> ·Ø¡¸ÒµØ %d%%", sd->indexed_bonus.subele_script[ELE_ALL] },
	};
	for (int i = 0; i < ARRAYLENGTH(ele_resist_info); i++) {
		if (ele_resist_info[i].value == 0)
			continue;
		battlestats_msg(sprintf(output, ele_resist_info[i].format, ele_resist_info[i].value));
	}

	clif_displaymessage(fd, "|--- »éÍ§¡Ñ¹»ÃÐàÀ· ---|");
	struct {
		const char* format;
		int value;
	} race_resist_info[] = {
		{ "   -> äÃéÃÙ»ÃèÒ§ %d%%", sd->indexed_bonus.subrace[RC_FORMLESS] },
		{ "   -> ¼Õ´Ôº %d%%", sd->indexed_bonus.subrace[RC_UNDEAD] },
		{ "   -> ÊÑµÇì %d%%", sd->indexed_bonus.subrace[RC_BRUTE] },
		{ "   -> ¾×ª %d%%", sd->indexed_bonus.subrace[RC_PLANT] },
		{ "   -> áÁÅ§ %d%%", sd->indexed_bonus.subrace[RC_INSECT] },
		{ "   -> »ÅÒ %d%%", sd->indexed_bonus.subrace[RC_FISH] },
		{ "   -> »ÕÈÒ¨ %d%%", sd->indexed_bonus.subrace[RC_DEMON] },
		{ "   -> ¡Öè§Á¹ØÉÂì %d%%", sd->indexed_bonus.subrace[RC_DEMIHUMAN] },
		{ "   -> à·Ç·Ùµ %d%%", sd->indexed_bonus.subrace[RC_ANGEL] },
		{ "   -> ÁÑ§¡Ã %d%%", sd->indexed_bonus.subrace[RC_DRAGON] },
		{ "   -> ¼ÙéàÅè¹(Á¹ØÉÂì) %d%%", sd->indexed_bonus.subrace[RC_PLAYER_HUMAN] },
		{ "   -> ¼ÙéàÅè¹(Doram): %d%%", sd->indexed_bonus.subrace[RC_PLAYER_DORAM] },
		{ "   -> ·Ø¡»ÃÐàÀ· %d%%", sd->indexed_bonus.subrace[RC_ALL] },
	};
	for (int i = 0; i < ARRAYLENGTH(race_resist_info); i++) {
		if (race_resist_info[i].value == 0)
			continue;
		battlestats_msg(sprintf(output, race_resist_info[i].format, race_resist_info[i].value));
	}

	clif_displaymessage(fd, "|--- »éÍ§¡Ñ¹¢¹Ò´ ---|");
	struct {
		const char* format;
		int value;
	} Size_resist_info[] = {
		{ "   -> ¢¹Ò´àÅç¡ %d%%", sd->indexed_bonus.subsize[SZ_SMALL] },
		{ "   -> ¢¹Ò´¡ÅÒ§ %d%%", sd->indexed_bonus.subsize[SZ_MEDIUM] },
		{ "   -> ¢¹Ò´ãË­è %d%%", sd->indexed_bonus.subsize[SZ_BIG] },
		{ "   -> ¢¹Ò´·Ñé§ËÁ´ %d%%", sd->indexed_bonus.subsize[SZ_ALL] },
	};
	for (int i = 0; i < ARRAYLENGTH(Size_resist_info); i++) {
		if (Size_resist_info[i].value == 0)
			continue;
		battlestats_msg(sprintf(output, Size_resist_info[i].format, Size_resist_info[i].value));
	}

#undef battlestats_msg
}

bool pcb_process_selection(struct map_session_data *sd, int skill) {
	char output[CHAT_SIZE_MAX];
	struct map_session_data* tsd = map_id2sd(sd->ce_gid);

	sd->state.check_equip_skill = false;
	sd->state.workinprogress = WIP_DISABLE_NONE;
	clif_menuskill_clear(sd);

	if (!tsd) {
		clif_displaymessage(sd->fd, "äÁè¾º¼ÙéàÅè¹.");
		return false;
	}

	if (!(skill >= CS_EQUIPMENT && skill < CS_EQUIPMENT + CHECK_STATUS_MENU_COUNT)) // Fake zeny check packet will fail if disabled here
		return false;

	switch (skill) {
	case CS_EQUIPMENT:
		if (pc_get_group_level(sd) > 0 || tsd->status.show_equip || pc_has_permission(sd, PC_PERM_VIEW_EQUIPMENT))
			clif_viewequip_ack(sd, tsd);
		else
			//clif_msg(sd->fd, "This player doesn't allow equipment view.");
			sprintf(output, "This player doesn't allow equipment view.");
		return true;
	case CS_STATS:
		//pcb_display(sd->fd, tsd);
		//return true;
		clif_displaymessage(sd->fd, "This function is disabled.");
		break;
	case CS_ZENY:
		//clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], "============ Check Zeny Info ============", false, SELF);
		//sprintf(output, "Zeny: %d", tsd->status.zeny);
		//clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], output, false, SELF);
		//return true;
		clif_displaymessage(sd->fd, "This function is disabled.");
		break;
	case CS_ITEMLIST: {
		/// <summary>
		/// 
		/// </summary>
		/// <param name="sd"></param>
		/// <param name="skill"></param>
		/// <returns></returns>
		// Disabled
		clif_displaymessage(sd->fd, "This function is disabled.");
		break;

		///*
		//char command[CHAT_SIZE_MAX];
		//safesnprintf( command, sizeof(command), "%citemlist %s", charcommand_symbol, tsd->status.name);
		//is_atcommand(sd->fd, sd, command, 0);
		//*/
		////char command[50];
		//int i, j, count, counter;
		//const char* location;
		//struct item* items;
		//int size;
		//StringBuf buf;
		////const char* parent_cmd;
		//nullpo_retr(-1, sd);
		///*
		//parent_cmd = atcommand_alias_db.checkAlias(command + 1);
		//*/
		//	location = "inventory";
		//	items = tsd->inventory.u.items_inventory;
		//	size = MAX_INVENTORY;
		//
		//StringBuf_Init(&buf);
		//
		//count = 0; // total slots occupied
		//counter = 0; // total items found
		//for (i = 0; i < size; ++i) {
		//	struct item* it = &items[i];
		//
		//	if (it->nameid == 0)
		//		continue;
		//
		//	std::shared_ptr<item_data> itd = item_db.find(it->nameid);
		//
		//	if (itd == nullptr)
		//		continue;
		//
		//	counter += it->amount;
		//	count++;
		//
		//	if (count == 1) {
		//		StringBuf_Printf(&buf, msg_txt(sd, 1332), location, tsd->status.name); // ------ %s items list of '%s' ------
		//		clif_displaymessage(sd->fd, StringBuf_Value(&buf));
		//		StringBuf_Clear(&buf);
		//	}
		//
		//	StringBuf_Printf(&buf, "%d %s (%s, id: %u)", it->amount, item_db.create_item_link(*it).c_str(), itd->name.c_str(), it->nameid);
		//
		//	if (it->equip) {
		//		char equipstr[CHAT_SIZE_MAX];
		//
		//		strcpy(equipstr, msg_txt(sd, 1333)); // | Equipped:
		//		if (it->equip & EQP_GARMENT)
		//			strcat(equipstr, msg_txt(sd, 1334)); // Robe,
		//		if (it->equip & EQP_ACC_L)
		//			strcat(equipstr, msg_txt(sd, 1335)); // Left Accessory,
		//		if (it->equip & EQP_ARMOR)
		//			strcat(equipstr, msg_txt(sd, 1336)); // Body/Armor,
		//		if ((it->equip & EQP_ARMS) == EQP_HAND_R)
		//			strcat(equipstr, msg_txt(sd, 1337)); // Right Hand,
		//		if ((it->equip & EQP_ARMS) == EQP_HAND_L)
		//			strcat(equipstr, msg_txt(sd, 1338)); // Left Hand,
		//		if ((it->equip & EQP_ARMS) == EQP_ARMS)
		//			strcat(equipstr, msg_txt(sd, 1339)); // Both Hands,
		//		if (it->equip & EQP_SHOES)
		//			strcat(equipstr, msg_txt(sd, 1340)); // Shoes,
		//		if (it->equip & EQP_ACC_R)
		//			strcat(equipstr, msg_txt(sd, 1341)); // Right Accessory,
		//		if ((it->equip & EQP_HELM) == EQP_HEAD_LOW)
		//			strcat(equipstr, msg_txt(sd, 1342)); // Lower Head,
		//		if ((it->equip & EQP_HELM) == EQP_HEAD_TOP)
		//			strcat(equipstr, msg_txt(sd, 1343)); // Top Head,
		//		if ((it->equip & EQP_HELM) == (EQP_HEAD_LOW | EQP_HEAD_TOP))
		//			strcat(equipstr, msg_txt(sd, 1344)); // Top/Lower Head,
		//		if ((it->equip & EQP_HELM) == EQP_HEAD_MID)
		//			strcat(equipstr, msg_txt(sd, 1345)); // Mid Head,
		//		if ((it->equip & EQP_HELM) == (EQP_HEAD_LOW | EQP_HEAD_MID))
		//			strcat(equipstr, msg_txt(sd, 1346)); // Mid/Lower Head,
		//		if ((it->equip & EQP_HELM) == EQP_HELM)
		//			strcat(equipstr, msg_txt(sd, 1347)); // Top/Mid/Lower Head,
		//		if ((it->equip & EQP_COSTUME_HELM) == EQP_COSTUME_HEAD_LOW)
		//			strcat(equipstr, msg_txt(sd, 518)); // Lower Costume Head,
		//		if ((it->equip & EQP_COSTUME_HELM) == EQP_COSTUME_HEAD_TOP)
		//			strcat(equipstr, msg_txt(sd, 519)); // Top Costume Head,
		//		if ((it->equip & EQP_COSTUME_HELM) == (EQP_COSTUME_HEAD_LOW | EQP_COSTUME_HEAD_TOP))
		//			strcat(equipstr, msg_txt(sd, 520)); // Top/Lower Costume Head,
		//		if ((it->equip & EQP_COSTUME_HELM) == EQP_COSTUME_HEAD_MID)
		//			strcat(equipstr, msg_txt(sd, 521)); // Mid Costume Head,
		//		if ((it->equip & EQP_COSTUME_HELM) == (EQP_COSTUME_HEAD_LOW | EQP_COSTUME_HEAD_MID))
		//			strcat(equipstr, msg_txt(sd, 522)); // Mid/Lower Costume Head,
		//		if ((it->equip & EQP_COSTUME_HELM) == EQP_COSTUME_HELM)
		//			strcat(equipstr, msg_txt(sd, 523)); // Top/Mid/Lower Costume Head,
		//		if (it->equip & EQP_COSTUME_GARMENT)
		//			strcat(equipstr, msg_txt(sd, 524)); // Costume Robe,
		//		//if( it->equip&EQP_COSTUME_FLOOR )
		//			//strcat(equipstr, msg_txt(sd,525)); // Costume Floor,
		//		if (it->equip & EQP_AMMO)
		//			strcat(equipstr, msg_txt(sd, 526)); // Ammo,
		//		if (it->equip & EQP_SHADOW_ARMOR)
		//			strcat(equipstr, msg_txt(sd, 527)); // Shadow Body,
		//		if ((it->equip & EQP_SHADOW_ARMS) == EQP_SHADOW_WEAPON)
		//			strcat(equipstr, msg_txt(sd, 528)); // Shadow Right Hand,
		//		if ((it->equip & EQP_SHADOW_ARMS) == EQP_SHADOW_SHIELD)
		//			strcat(equipstr, msg_txt(sd, 529)); // Shadow Left Hand,
		//		if ((it->equip & EQP_SHADOW_ARMS) == EQP_SHADOW_ARMS)
		//			strcat(equipstr, msg_txt(sd, 530)); // Shadow Both Hands,
		//		if (it->equip & EQP_SHADOW_SHOES)
		//			strcat(equipstr, msg_txt(sd, 531)); // Shadow Shoes,
		//		if (it->equip & EQP_SHADOW_ACC_R)
		//			strcat(equipstr, msg_txt(sd, 532)); // Shadow Right Accessory,
		//		if (it->equip & EQP_SHADOW_ACC_L)
		//			strcat(equipstr, msg_txt(sd, 533)); // Shadow Left Accessory,
		//		// remove final ', '
		//		equipstr[strlen(equipstr) - 2] = '\0';
		//		StringBuf_AppendStr(&buf, equipstr);
		//	}
		//
		//	clif_displaymessage(sd->fd, StringBuf_Value(&buf));
		//	StringBuf_Clear(&buf);
		//
		//	if (it->card[0] == CARD0_PET) { // pet egg
		//		if (it->card[3] & 1)
		//			StringBuf_Printf(&buf, msg_txt(sd, 1348), (unsigned int)MakeDWord(it->card[1], it->card[2])); //  -> (pet egg, pet id: %u, named)
		//		else
		//			StringBuf_Printf(&buf, msg_txt(sd, 1349), (unsigned int)MakeDWord(it->card[1], it->card[2])); //  -> (pet egg, pet id: %u, unnamed)
		//	}
		//	else if (it->card[0] == CARD0_FORGE) { // forged item
		//		StringBuf_Printf(&buf, msg_txt(sd, 1350), (unsigned int)MakeDWord(it->card[2], it->card[3]), it->card[1] >> 8, it->card[1] & 0x0f); //  -> (crafted item, creator id: %u, star crumbs %d, element %d)
		//	}
		//	else if (it->card[0] == CARD0_CREATE) { // created item
		//		StringBuf_Printf(&buf, msg_txt(sd, 1351), (unsigned int)MakeDWord(it->card[2], it->card[3])); //  -> (produced item, creator id: %u)
		//	}
		//	else { // normal item
		//		int counter2 = 0;
		//
		//		for (j = 0; j < itd->slots; ++j) {
		//			if (it->card[j] == 0)
		//				continue;
		//
		//			std::shared_ptr<item_data> card = item_db.find(it->card[j]);
		//
		//			if (card == nullptr)
		//				continue;
		//
		//			counter2++;
		//
		//			if (counter2 == 1)
		//				StringBuf_AppendStr(&buf, msg_txt(sd, 1352)); //  -> (card(s):
		//
		//			if (counter2 != 1)
		//				StringBuf_AppendStr(&buf, ", ");
		//
		//			StringBuf_Printf(&buf, "#%d %s (id: %u)", counter2, item_db.create_item_link(card).c_str(), card->nameid);
		//		}
		//
		//		if (counter2 > 0)
		//			StringBuf_AppendStr(&buf, ")");
		//	}
		//
		//	if (StringBuf_Length(&buf) > 0)
		//		clif_displaymessage(sd->fd, StringBuf_Value(&buf));
		//
		//	StringBuf_Clear(&buf);
		//}
		//
		//if (count == 0)
		//	StringBuf_Printf(&buf, msg_txt(sd, 1353), location); // No item found in this player's %s.
		//else
		//	StringBuf_Printf(&buf, msg_txt(sd, 1354), counter, count, location); // %d item(s) found in %d %s slots.
		//
		//clif_displaymessage(sd->fd, StringBuf_Value(&buf));
		//
		//StringBuf_Destroy(&buf);
		//
		//return 0;
		//
		///// <summary>
		///// ////////////////
		///// </summary>
		///// <param name="sd"></param>
		///// <param name="skill"></param>
		///// <returns></returns>
		//return true;

		}
	default:
		return false;
	}
	return false;
}

#undef CHECK_STATUS_MENU_COUNT
