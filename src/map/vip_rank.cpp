// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL

#include "vip_rank.hpp"

#include <algorithm>
#include <cinttypes>
#include <limits>

#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>

#include "clif.hpp"
#include "map.hpp"
#include "pc.hpp"
#include "script.hpp"
#include "status.hpp"

VipRankDatabase vip_rank_db;

s_vip_rank::~s_vip_rank() {
	if (script != nullptr) {
		script_free_code(script);
		script = nullptr;
	}
	if (pve_script != nullptr) {
		script_free_code(pve_script);
		pve_script = nullptr;
	}
	if (woe_script != nullptr) {
		script_free_code(woe_script);
		woe_script = nullptr;
	}
}

const std::string VipRankDatabase::getDefaultLocation() {
	return std::string(db_path) + "/custom/vip_rank.yml";
}

uint64 VipRankDatabase::parseBodyNode(const ryml::NodeRef& node) {
	if (!this->nodesExist(node, { "Rank", "Name", "Threshold", "BoxSpeed", "Icon", "Script" }))
		return 0;

	uint16 rank;
	if (!this->asUInt16(node, "Rank", rank) || rank == 0)
		return 0;

	std::shared_ptr<s_vip_rank> entry = this->find(rank);
	bool exists = entry != nullptr;
	if (!exists) {
		entry = std::make_shared<s_vip_rank>();
		entry->rank = rank;
	}

	if (!this->asString(node, "Name", entry->name) ||
		!this->asInt64(node, "Threshold", entry->threshold) ||
		!this->asUInt16(node, "BoxSpeed", entry->box_speed))
		return 0;

	if (entry->threshold < 0) {
		this->invalidWarning(node["Threshold"], "Threshold must not be negative.\n");
		return 0;
	}

	if (entry->box_speed > 90) {
		this->invalidWarning(node["BoxSpeed"], "BoxSpeed must be between 0 and 90.\n");
		return 0;
	}

	std::string icon_name;
	if (!this->asString(node, "Icon", icon_name))
		return 0;

	int64 icon_constant;
	if (!script_get_constant(icon_name.c_str(), &icon_constant)) {
		this->invalidWarning(node["Icon"], "Icon %s is invalid, defaulting to EFST_BLANK.\n", icon_name.c_str());
		icon_constant = EFST_BLANK;
	}
	if (icon_constant < EFST_BLANK || icon_constant >= EFST_MAX) {
		this->invalidWarning(node["Icon"], "Icon %s is out of bounds, defaulting to EFST_BLANK.\n", icon_name.c_str());
		icon_constant = EFST_BLANK;
	}
	entry->icon = static_cast<int32>(icon_constant);

	std::string script_source;
	if (!this->asString(node, "Script", script_source))
		return 0;
	if (entry->script != nullptr) {
		script_free_code(entry->script);
		entry->script = nullptr;
	}
	entry->script = parse_script(script_source.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["Script"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);

	if (this->nodeExists(node, "PveScript")) {
		std::string pve_script_source;
		if (!this->asString(node, "PveScript", pve_script_source))
			return 0;
		if (entry->pve_script != nullptr) {
			script_free_code(entry->pve_script);
			entry->pve_script = nullptr;
		}
		entry->pve_script = parse_script(pve_script_source.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["PveScript"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else if (entry->pve_script != nullptr) {
		script_free_code(entry->pve_script);
		entry->pve_script = nullptr;
	}

	if (this->nodeExists(node, "WoeScript")) {
		std::string woe_script_source;
		if (!this->asString(node, "WoeScript", woe_script_source))
			return 0;
		if (entry->woe_script != nullptr) {
			script_free_code(entry->woe_script);
			entry->woe_script = nullptr;
		}
		entry->woe_script = parse_script(woe_script_source.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["WoeScript"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	} else if (entry->woe_script != nullptr) {
		script_free_code(entry->woe_script);
		entry->woe_script = nullptr;
	}

	if (!exists)
		this->put(rank, entry);

	return 1;
}

uint16 vip_rank_from_total(int64 donation_total) {
	uint16 result = 0;
	int64 highest_threshold = -1;

	for (const auto& pair : vip_rank_db) {
		const std::shared_ptr<s_vip_rank>& entry = pair.second;
		if (entry != nullptr && donation_total >= entry->threshold && entry->threshold >= highest_threshold) {
			result = entry->rank;
			highest_threshold = entry->threshold;
		}
	}

	return result;
}

bool vip_rank_load_account(uint32 account_id, int64& donation_total, uint16& rank) {
	char* data = nullptr;

	donation_total = 0;
	rank = 0;

	if (Sql_Query(mmysql_handle,
		"SELECT `donation_total`, `rank` FROM `vip_rank_account` WHERE `account_id` = %u",
		account_id) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}

	if (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
		Sql_GetData(mmysql_handle, 0, &data, nullptr);
		donation_total = data != nullptr ? strtoll(data, nullptr, 10) : 0;
		Sql_GetData(mmysql_handle, 1, &data, nullptr);
		rank = data != nullptr ? static_cast<uint16>(strtoul(data, nullptr, 10)) : 0;
	}
	Sql_FreeResult(mmysql_handle);

	uint16 calculated_rank = vip_rank_from_total(donation_total);
	if (rank != calculated_rank) {
		rank = calculated_rank;
		if (Sql_Query(mmysql_handle,
			"INSERT INTO `vip_rank_account` (`account_id`, `donation_total`, `rank`) VALUES (%u, %" PRId64 ", %u) "
			"ON DUPLICATE KEY UPDATE `rank` = VALUES(`rank`)",
			account_id, donation_total, rank) != SQL_SUCCESS) {
			Sql_ShowDebug(mmysql_handle);
			return false;
		}
	}

	return true;
}

bool vip_rank_load(map_session_data* sd, bool notify) {
	nullpo_retr(false, sd);

	int64 donation_total;
	uint16 rank;
	if (!vip_rank_load_account(sd->status.account_id, donation_total, rank)) {
		sd->vip_rank_loaded = false;
		return false;
	}

	uint16 old_rank = sd->vip_rank;
	sd->vip_rank_donation_total = donation_total;
	sd->vip_rank = rank;
	sd->vip_rank_loaded = true;

	if (notify && old_rank != rank) {
		char output[128];
		safesnprintf(output, sizeof(output), "VIP Rank updated: %u -> %u (Donate: %" PRId64 ")", old_rank, rank, donation_total);
		clif_displaymessage(sd->fd, output);
	}

	return true;
}

void vip_rank_refresh_icon(map_session_data* sd, bool force) {
	if (sd == nullptr || sd->fd <= 0)
		return;

	int32 new_icon = EFST_BLANK;
	if (sd->vip_rank_loaded && sd->vip_rank > 0) {
		std::shared_ptr<s_vip_rank> entry = vip_rank_db.find(sd->vip_rank);
		if (entry != nullptr)
			new_icon = entry->icon;
	}

	if (!force && sd->vip_rank_icon == new_icon)
		return;

	// Clear the icon that this session previously displayed. The MEMBER range
	// is also cleared during a forced refresh to migrate from the old fixed mapping.
	if (sd->vip_rank_icon > EFST_BLANK && sd->vip_rank_icon < EFST_MAX)
		clif_status_load(&sd->bl, static_cast<efst_type>(sd->vip_rank_icon), 0);
	if (force) {
		for (uint16 rank = 0; rank <= 10; ++rank)
			clif_status_load(&sd->bl, static_cast<efst_type>(EFST_MEMBER0 + rank), 0);
	}

	if (new_icon > EFST_BLANK)
		clif_status_load(&sd->bl, static_cast<efst_type>(new_icon), 1);

	sd->vip_rank_icon = new_icon;
}

void vip_rank_refresh_online(uint32 account_id, int64 donation_total, uint16 rank, bool notify) {
	s_mapiterator* iter = mapit_getallusers();
	for (map_session_data* sd = reinterpret_cast<map_session_data*>(mapit_first(iter));
		mapit_exists(iter);
		sd = reinterpret_cast<map_session_data*>(mapit_next(iter))) {
		if (sd == nullptr || sd->status.account_id != account_id)
			continue;

		uint16 old_rank = sd->vip_rank;
		sd->vip_rank_donation_total = donation_total;
		sd->vip_rank = rank;
		sd->vip_rank_loaded = true;
		status_calc_pc(sd, SCO_FORCE);
		vip_rank_refresh_icon(sd);

		if (notify && old_rank != rank) {
			char output[128];
			safesnprintf(output, sizeof(output), "VIP Rank updated: %u -> %u (Donate: %" PRId64 ")", old_rank, rank, donation_total);
			clif_displaymessage(sd->fd, output);
		}
	}
	mapit_free(iter);
}

bool vip_rank_change_total(uint32 account_id, int64 value, e_vip_rank_operation operation, uint32 actor_account_id, const char* reference, const char* reason) {
	char escaped_reference[129] = {};
	char escaped_reason[513] = {};
	int64 old_total = 0;
	int64 new_total = 0;
	uint16 old_rank = 0;
	uint16 new_rank = 0;
	char* data = nullptr;

	if (value < 0)
		return false;

	const char* source_reference = reference != nullptr ? reference : "";
	const char* source_reason = reason != nullptr ? reason : "";
	Sql_EscapeStringLen(mmysql_handle, escaped_reference, source_reference, strnlen(source_reference, 64));
	Sql_EscapeStringLen(mmysql_handle, escaped_reason, source_reason, strnlen(source_reason, 255));

	if (Sql_QueryStr(mmysql_handle, "START TRANSACTION") != SQL_SUCCESS)
		return false;

	if (Sql_Query(mmysql_handle,
		"INSERT IGNORE INTO `vip_rank_account` (`account_id`, `donation_total`, `rank`) VALUES (%u, 0, 0)",
		account_id) != SQL_SUCCESS ||
		Sql_Query(mmysql_handle,
		"SELECT `donation_total`, `rank` FROM `vip_rank_account` WHERE `account_id` = %u FOR UPDATE",
		account_id) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
		Sql_QueryStr(mmysql_handle, "ROLLBACK");
		return false;
	}

	if (Sql_NextRow(mmysql_handle) != SQL_SUCCESS) {
		Sql_FreeResult(mmysql_handle);
		Sql_QueryStr(mmysql_handle, "ROLLBACK");
		return false;
	}

	Sql_GetData(mmysql_handle, 0, &data, nullptr);
	old_total = data != nullptr ? strtoll(data, nullptr, 10) : 0;
	Sql_GetData(mmysql_handle, 1, &data, nullptr);
	old_rank = data != nullptr ? static_cast<uint16>(strtoul(data, nullptr, 10)) : 0;
	Sql_FreeResult(mmysql_handle);
	old_rank = vip_rank_from_total(old_total);

	switch (operation) {
		case VIP_RANK_OP_SET:
			new_total = value;
			break;
		case VIP_RANK_OP_ADD:
			if (value > std::numeric_limits<int64>::max() - old_total) {
				Sql_QueryStr(mmysql_handle, "ROLLBACK");
				return false;
			}
			new_total = old_total + value;
			break;
		case VIP_RANK_OP_REFUND:
			new_total = std::max<int64>(0, old_total - value);
			break;
		default:
			Sql_QueryStr(mmysql_handle, "ROLLBACK");
			return false;
	}

	new_rank = vip_rank_from_total(new_total);

	if (Sql_Query(mmysql_handle,
		"UPDATE `vip_rank_account` SET `donation_total` = %" PRId64 ", `rank` = %u WHERE `account_id` = %u",
		new_total, new_rank, account_id) != SQL_SUCCESS ||
		Sql_Query(mmysql_handle,
		"INSERT INTO `vip_rank_transaction` "
		"(`account_id`, `operation`, `amount`, `total_before`, `total_after`, `rank_before`, `rank_after`, `reference`, `actor_account_id`, `reason`) "
		"VALUES (%u, '%s', %" PRId64 ", %" PRId64 ", %" PRId64 ", %u, %u, NULLIF('%s',''), %u, '%s')",
		account_id,
		operation == VIP_RANK_OP_SET ? "set" : operation == VIP_RANK_OP_ADD ? "add" : "refund",
		value, old_total, new_total, old_rank, new_rank, escaped_reference, actor_account_id, escaped_reason) != SQL_SUCCESS ||
		Sql_QueryStr(mmysql_handle, "COMMIT") != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
		Sql_QueryStr(mmysql_handle, "ROLLBACK");
		return false;
	}

	vip_rank_refresh_online(account_id, new_total, new_rank, true);
	return true;
}

void vip_rank_apply(map_session_data* sd) {
	if (sd == nullptr || !sd->vip_rank_loaded || sd->vip_rank == 0)
		return;

	std::shared_ptr<s_vip_rank> entry = vip_rank_db.find(sd->vip_rank);
	if (entry == nullptr)
		return;

	if (entry->script != nullptr)
		run_script(entry->script, 0, sd->bl.id, 0);

	// PveScript is intentionally skipped on PvP/GvG/WoE/Battleground maps.
	if (!map_flag_vs2(sd->bl.m) && entry->pve_script != nullptr)
		run_script(entry->pve_script, 0, sd->bl.id, 0);

	// WoeScript applies only to maps tagged GvG/WoE, including TE and castles.
	if (map_flag_gvg2(sd->bl.m) && entry->woe_script != nullptr)
		run_script(entry->woe_script, 0, sd->bl.id, 0);
}

int vip_rank_box_time(map_session_data* sd, int base_milliseconds, int minimum_milliseconds) {
	if (base_milliseconds <= 0)
		return 0;

	int minimum = std::clamp(minimum_milliseconds, 0, base_milliseconds);
	if (sd == nullptr || !sd->vip_rank_loaded || sd->vip_rank == 0)
		return std::max(base_milliseconds, minimum);

	std::shared_ptr<s_vip_rank> entry = vip_rank_db.find(sd->vip_rank);
	if (entry == nullptr)
		return std::max(base_milliseconds, minimum);

	int result = base_milliseconds * (100 - entry->box_speed) / 100;
	return std::max(result, minimum);
}

void vip_rank_reload() {
	vip_rank_db.reload();

	s_mapiterator* iter = mapit_getallusers();
	for (map_session_data* sd = reinterpret_cast<map_session_data*>(mapit_first(iter));
		mapit_exists(iter);
		sd = reinterpret_cast<map_session_data*>(mapit_next(iter))) {
		if (sd != nullptr && vip_rank_load(sd, true))
			status_calc_pc(sd, SCO_FORCE);
		if (sd != nullptr)
			vip_rank_refresh_icon(sd, true);
	}
	mapit_free(iter);
}
