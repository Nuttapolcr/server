// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

/**
 * @hidepet - Hides all pets except the player's pet.
 * @hidepetall - Hides all pets, including the player's pet.
 * [Daegaladh]
 */
static int atcommand_hidepet_sub(block_list* bl, va_list ap)
{
	map_session_data* sd = va_arg(ap, map_session_data*);

	if (!va_arg(ap, int) && sd->pd != nullptr && sd->pd->bl.id == bl->id) {
		if(va_arg(ap, int) == 2)
			clif_spawn(&sd->pd->bl, true);
		return 0;
	}

	clif_clearunit_single(bl->id, CLR_OUTSIGHT, sd->fd);
	return 0;
}
ACMD_FUNC(hidepet)
{
	nullpo_retr(-1,sd);

	parent_cmd = atcommand_alias_db.checkAlias(command + 1);
	bool all = (strcmpi(parent_cmd, "hidepetall") == 0) ? true : false;
	uint8 prev_state = sd->state.hidepet;

	switch(sd->state.hidepet) {
	case 0:
		sd->state.hidepet = all ? 2 : 1;
		break;
	case 1:
		sd->state.hidepet = all ? 2 : 0;
		break;
	case 2:
		sd->state.hidepet = all ? 0 : 1;
		break;
	}

	if(sd->state.hidepet == 0) {
		map_foreachinallrange(clif_insight, &sd->bl, AREA_SIZE, BL_PET, &sd->bl);
		clif_displaymessage(fd, "All pets are visible.");
	}
	else {
		map_foreachinallrange(atcommand_hidepet_sub, &sd->bl, AREA_SIZE, BL_PET, &sd->bl, all, prev_state);
		clif_displaymessage(fd, all ? "All pets are hidden." : "All pets except yours are hidden.");
	}

	return 0;
}