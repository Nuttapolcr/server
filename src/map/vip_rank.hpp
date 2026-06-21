// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL

#ifndef VIP_RANK_HPP
#define VIP_RANK_HPP

#include <memory>
#include <string>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>

struct map_session_data;
struct script_code;

struct s_vip_rank {
	uint16 rank;
	std::string name;
	int64 threshold;
	uint16 box_speed;
	int32 icon = -1; // EFST_BLANK
	script_code* script = nullptr;
	script_code* pve_script = nullptr;
	script_code* woe_script = nullptr;

	~s_vip_rank();
};

class VipRankDatabase : public TypesafeYamlDatabase<uint16, s_vip_rank> {
public:
	VipRankDatabase() : TypesafeYamlDatabase("VIP_RANK_DB", 1) {
	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

extern VipRankDatabase vip_rank_db;

enum e_vip_rank_operation : uint8 {
	VIP_RANK_OP_SET = 0,
	VIP_RANK_OP_ADD,
	VIP_RANK_OP_REFUND,
};

uint16 vip_rank_from_total(int64 donation_total);
bool vip_rank_load(map_session_data* sd, bool notify = false);
bool vip_rank_load_account(uint32 account_id, int64& donation_total, uint16& rank);
bool vip_rank_change_total(uint32 account_id, int64 value, e_vip_rank_operation operation, uint32 actor_account_id, const char* reference, const char* reason);
void vip_rank_refresh_online(uint32 account_id, int64 donation_total, uint16 rank, bool notify);
void vip_rank_refresh_icon(map_session_data* sd, bool force = false);
void vip_rank_apply(map_session_data* sd);
int vip_rank_box_time(map_session_data* sd, int base_milliseconds, int minimum_milliseconds);
void vip_rank_reload();

#endif
