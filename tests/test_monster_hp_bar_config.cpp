#include <cstdio>

#include "map/battle.hpp"
#include "map/map.hpp"

extern "C" int __wrap_main(int, char**) {
	battle_config_read("conf/battle_athena.conf");

	if (battle_config.monster_hp_bars_info != 1 || battle_config.show_mob_info != 0)
		return 1;

	map[0].initMapFlags();
	if (map_flag_hide_mob_hp_bar(0))
		return 2;

	constexpr e_mapflag woe_flags[] = {
		MF_GVG,
		MF_GVG_CASTLE,
		MF_GVG_TE,
		MF_GVG_TE_CASTLE,
	};
	for (e_mapflag flag : woe_flags) {
		map[0].initMapFlags();
		map[0].setMapFlag(flag, true);
		if (!map_flag_hide_mob_hp_bar(0))
			return 3;
	}

	map[0].initMapFlags();
	map[0].setMapFlag(MF_HIDEMOBHPBAR, true);
	if (!map_flag_hide_mob_hp_bar(0))
		return 4;

	std::puts("PASS: monster HP bars enabled outside WoE and hidden on all WoE map flags.");
	return 0;
}
