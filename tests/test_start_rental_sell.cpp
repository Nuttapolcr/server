#include <cassert>
#include <cstdio>
#include <ctime>

#include "map/battle.hpp"
#include "map/itemdb.hpp"
#include "map/npc.hpp"
#include "map/pc.hpp"
#include "map/pc_groups.hpp"

// Link with the map-server objects and -Wl,--wrap=main to avoid server startup.
extern "C" int __wrap_main(int, char**) {
	battle_config_read("conf/battle_athena.conf");
	assert(battle_config.rental_transaction);
	assert(battle_config.allow_bound_sell == 0);

	const t_itemid starter_ids[] = {
		1292, 1394, 1491, 1585, 2015, 13071, 13115, 21000, 16019,
		18112, 13316, 1030046, 1030047, 1030048, 15015, 2149, 2460, 2562, 28311,
	};
	auto sd = std::make_unique<map_session_data>();
	sd->group = std::make_shared<s_player_group>();
	sd->group->level = 0;
	sd->permissions.set(PC_PERM_TRADE);

	for (t_itemid id : starter_ids) {
		auto data = std::make_shared<item_data>();
		data->nameid = id;
		data->slots = 1;
		data->gm_lv_trade_override = 100;
		item_db.put(id, data);

		item rental{};
		rental.nameid = id;
		rental.amount = 1;
		rental.bound = BOUND_CHAR;
		rental.expire_time = static_cast<uint32>(time(nullptr) + 7 * 24 * 60 * 60);
		const item original = rental;
		assert(pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		assert(rental.bound == original.bound && rental.expire_time == original.expire_time);
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_ITEMSHOP));
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_CASHSHOP));

		sd->inventory.u.items_inventory[0] = rental;
		assert(!pc_can_trade_item(sd.get(), 0));

		for (auto bound : {BOUND_ACCOUNT, BOUND_GUILD, BOUND_PARTY}) {
			rental.bound = bound;
			assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		}
		rental = original;
		rental.expire_time = 0;
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		rental = original;
		rental.equip = EQP_HAND_R;
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		rental = original;
		rental.equipSwitch = EQP_HAND_R;
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		rental = original;
		rental.favorite = 1;
		battle_config.hide_fav_sell = 1;
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		rental = original;
		battle_config.rental_transaction = 0;
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		battle_config.rental_transaction = 1;

		data->flag.trade_restriction.sell = true;
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		data->flag.trade_restriction.sell = false;
		auto card = std::make_shared<item_data>();
		card->nameid = 4001;
		card->gm_lv_trade_override = 100;
		card->flag.trade_restriction.sell = true;
		item_db.put(card->nameid, card);
		rental.card[0] = card->nameid;
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		rental = original;
		sd->permissions.reset(PC_PERM_TRADE);
		assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
		sd->permissions.set(PC_PERM_TRADE);
	}

	auto other = std::make_shared<item_data>();
	other->nameid = 1201;
	other->gm_lv_trade_override = 100;
	item_db.put(other->nameid, other);
	item rental{};
	rental.nameid = other->nameid;
	rental.amount = 1;
	rental.bound = BOUND_CHAR;
	rental.expire_time = static_cast<uint32>(time(nullptr) + 3600);
	assert(!pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
	rental.bound = BOUND_NONE;
	assert(pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
	rental.expire_time = 0;
	assert(pc_can_sell_item(sd.get(), &rental, NPCTYPE_SHOP));
	assert(!pc_can_sell_item(nullptr, &rental, NPCTYPE_SHOP));
	assert(!pc_can_sell_item(sd.get(), nullptr, NPCTYPE_SHOP));

	std::puts("PASS: all 19 starter rentals sell to NPC shops; bound, rental, trade, itemshop, equipment, favorite, and item/card sale restrictions remain enforced.");
	return 0;
}
