Premium Service
===================================================================
Index: map/script_constants.hpp
===================================================================
Find: 
	export_constant(SC_EP16_2_BUFF_AC);

Add:
+	// TAURUS PREMIUM SERVICE
+	export_constant(SC_PREMIUMSERVICE_EXPBOOST_A);
+	export_constant(SC_PREMIUMSERVICE_EXPBOOST_S);
+	export_constant(SC_PREMIUMSERVICE_JEXPBOOST_A);
+	export_constant(SC_PREMIUMSERVICE_JEXPBOOST_S);
+	export_constant(SC_PREMIUMSERVICE_ITEMBOOST_A);
+	export_constant(SC_PREMIUMSERVICE_ITEMBOOST_S);
+	export_constant(SC_PREMIUMSERVICE_STORAGE);
+	export_constant(SC_PREMIUMSERVICE_LIFEINSURANCE);
+	export_constant(SC_PREMIUMSERVICE);
	
Find:
	/// @APIHOOK_END
	
Add:
+	export_constant(EFST_PREMIUMSERVICE); // Premium Service Icon.
	export_constant(EFST_MAX);


===================================================================
Index: map/status.hpp
===================================================================
Find:
	SC_EP16_2_BUFF_AC,
	
Add:
+	// TAURUS PREMIUM SERVICE
+	SC_PREMIUMSERVICE_EXPBOOST_A,
+	SC_PREMIUMSERVICE_EXPBOOST_S,
+	SC_PREMIUMSERVICE_JEXPBOOST_A,
+	SC_PREMIUMSERVICE_JEXPBOOST_S,
+	SC_PREMIUMSERVICE_ITEMBOOST_A,
+	SC_PREMIUMSERVICE_ITEMBOOST_S,
+	SC_PREMIUMSERVICE_STORAGE,
+	SC_PREMIUMSERVICE_LIFEINSURANCE,
+	SC_PREMIUMSERVICE,

Find:
	/// @APIHOOK_END
	
Add:
+	EFST_PREMIUMSERVICE = 1501,

===================================================================
Index: map/status.cpp
===================================================================
Find:
	StatusIconChangeTable[SC_EP16_2_BUFF_AC] = EFST_EP16_2_BUFF_AC;
	
Add:
+	// TAURUS PREMIUM SERVICE
+	StatusIconChangeTable[SC_PREMIUMSERVICE_EXPBOOST_A] = EFST_AID_PERIOD_PLUSEXP_2ND;
+	StatusIconChangeTable[SC_PREMIUMSERVICE_EXPBOOST_S] = EFST_AID_PERIOD_PLUSEXP;
+	StatusIconChangeTable[SC_PREMIUMSERVICE_JEXPBOOST_A] = EFST_AID_PERIOD_PLUSJOBEXP_2ND;
+	StatusIconChangeTable[SC_PREMIUMSERVICE_JEXPBOOST_S] = EFST_AID_PERIOD_PLUSJOBEXP;
+	StatusIconChangeTable[SC_PREMIUMSERVICE_ITEMBOOST_A] = EFST_AID_PERIOD_RECEIVEITEM_2ND;
+	StatusIconChangeTable[SC_PREMIUMSERVICE_ITEMBOOST_S] = EFST_AID_PERIOD_RECEIVEITEM;
+	StatusIconChangeTable[SC_PREMIUMSERVICE_STORAGE] = EFST_AID_PERIOD_ADDSTOREITEMCOUNT;
+	StatusIconChangeTable[SC_PREMIUMSERVICE_LIFEINSURANCE] = EFST_AID_PERIOD_DEADPENALTY;
+	StatusIconChangeTable[SC_PREMIUMSERVICE] = EFST_PREMIUMSERVICE;


Find:
	case SC_JP_EVENT04:
	
Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:
+	case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:
+	case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE:


Find:
	case SC_MADOGEAR:
	
Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:
+	case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:
+	case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE_ITEMBOOST_A:
+	case SC_PREMIUMSERVICE_ITEMBOOST_S:
+	case SC_PREMIUMSERVICE_STORAGE:
+	case SC_PREMIUMSERVICE_LIFEINSURANCE:
+	case SC_PREMIUMSERVICE:


Find:
	case SC_DAILYSENDMAILCNT:
	
Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:
+	case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:
+	case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE_ITEMBOOST_A:
+	case SC_PREMIUMSERVICE_ITEMBOOST_S:
+	case SC_PREMIUMSERVICE_STORAGE:
+	case SC_PREMIUMSERVICE_LIFEINSURANCE:
+	case SC_PREMIUMSERVICE:


Find:
	case SC_DAILYSENDMAILCNT:
	
Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:
+	case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:
+	case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE_ITEMBOOST_A:
+	case SC_PREMIUMSERVICE_ITEMBOOST_S:
+	case SC_PREMIUMSERVICE_STORAGE:
+	case SC_PREMIUMSERVICE_LIFEINSURANCE:
+	case SC_PREMIUMSERVICE:
	
Find:
	case SC_MAGICAL_FEATHER:
	
Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:
+	case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:
+	case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE_ITEMBOOST_A:
+	case SC_PREMIUMSERVICE_ITEMBOOST_S:
+	case SC_PREMIUMSERVICE_STORAGE:
+	case SC_PREMIUMSERVICE_LIFEINSURANCE:
+	case SC_PREMIUMSERVICE:


===================================================================
Index: map/pc.cpp
===================================================================
Find:
	// Give EXPBOOST for quests even if src is NULL.
	if (sd->sc.data[SC_EXPBOOST]) {
		bonus += sd->sc.data[SC_EXPBOOST]->val1;
		if (battle_config.vip_bm_increase && pc_isvip(sd)) // Increase Battle Manual EXP rate for VIP
			bonus += (sd->sc.data[SC_EXPBOOST]->val1 / battle_config.vip_bm_increase);
	}
	
Add:
+	// TAURUS PREMIUM SERVICE
+	if (sd->sc.data[SC_PREMIUMSERVICE_EXPBOOST_A]) {
+		bonus += sd->sc.data[SC_PREMIUMSERVICE_EXPBOOST_A]->val1;
+		if (battle_config.vip_bm_increase && pc_isvip(sd)) // Increase Battle Manual EXP rate for VIP
+			bonus += (sd->sc.data[SC_PREMIUMSERVICE_EXPBOOST_A]->val1 / battle_config.vip_bm_increase);
+	}
+	if (sd->sc.data[SC_PREMIUMSERVICE_EXPBOOST_S]) {
+		bonus += sd->sc.data[SC_PREMIUMSERVICE_EXPBOOST_S]->val1;
+		if (battle_config.vip_bm_increase && pc_isvip(sd)) // Increase Battle Manual EXP rate for VIP
+			bonus += (sd->sc.data[SC_PREMIUMSERVICE_EXPBOOST_S]->val1 / battle_config.vip_bm_increase);
+	}

Find:
	// Give JEXPBOOST for quests even if src is NULL.
	if (sd->sc.data[SC_JEXPBOOST])
		bonus += sd->sc.data[SC_JEXPBOOST]->val1;
		
Add:
+	// TAURUS PREMIUM SERVICE
+	if (sd->sc.data[SC_PREMIUMSERVICE_JEXPBOOST_A])
+		bonus += sd->sc.data[SC_PREMIUMSERVICE_JEXPBOOST_A]->val1;
+	if (sd->sc.data[SC_PREMIUMSERVICE_JEXPBOOST_S])
+		bonus += sd->sc.data[SC_PREMIUMSERVICE_JEXPBOOST_S]->val1;

Find:
	if ((battle_config.death_penalty_maxlv&1 || !pc_is_maxbaselv(sd)) && base_penalty > 0) {
		switch (battle_config.death_penalty_type) {
			case 1: base_penalty = (t_exp) ( pc_nextbaseexp(sd) * ( base_penalty / 10000. ) ); break;
			case 2: base_penalty = (t_exp) ( sd->status.base_exp * ( base_penalty / 10000. ) ); break;
		}
			
Add:
+	// TAURUS PREMIUM SERVICE
+	t_exp a_base = 0;
+	a_base = base_penalty;
+	if (sd->sc.data[SC_PREMIUMSERVICE_LIFEINSURANCE])
+		base_penalty -= (uint32)(a_base * (sd->sc.data[SC_PREMIUMSERVICE_LIFEINSURANCE]->val1 / 100.));

Find:
	if ((battle_config.death_penalty_maxlv&2 || !pc_is_maxjoblv(sd)) && job_penalty > 0) {
		switch (battle_config.death_penalty_type) {
			case 1: job_penalty = (uint32) ( pc_nextjobexp(sd) * ( job_penalty / 10000. ) ); break;
			case 2: job_penalty = (uint32) ( sd->status.job_exp * ( job_penalty /10000. ) ); break;
		}
		
Add:
+	// TAURUS PREMIUM SERVICE
+	t_exp a_job = 0;
+	a_job = job_penalty;
+	if (sd->sc.data[SC_PREMIUMSERVICE_LIFEINSURANCE])
+		job_penalty -= (uint32)(a_job * (sd->sc.data[SC_PREMIUMSERVICE_LIFEINSURANCE]->val1 / 100.));

===================================================================
Index: map/skill.cpp
===================================================================
Find:
	case SC_USE_SKILL_SP_SHA:	case SC_SP_SHA:
	
Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:						case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:						case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE_ITEMBOOST_A:						case SC_PREMIUMSERVICE_ITEMBOOST_S:
+	case SC_PREMIUMSERVICE_STORAGE:							case SC_PREMIUMSERVICE_LIFEINSURANCE:
+	case SC_PREMIUMSERVICE:

Find:
	case SC_USE_SKILL_SP_SHA:	case SC_SP_SHA:

Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:						case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:						case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE_ITEMBOOST_A:						case SC_PREMIUMSERVICE_ITEMBOOST_S:
+	case SC_PREMIUMSERVICE_STORAGE:							case SC_PREMIUMSERVICE_LIFEINSURANCE:
+	case SC_PREMIUMSERVICE:

Find:
	case SC_SPORE_EXPLOSION:
	
Add:
+	// TAURUS PREMIUM SERVICE
+	case SC_PREMIUMSERVICE_EXPBOOST_A:						case SC_PREMIUMSERVICE_EXPBOOST_S:
+	case SC_PREMIUMSERVICE_JEXPBOOST_A:						case SC_PREMIUMSERVICE_JEXPBOOST_S:
+	case SC_PREMIUMSERVICE_ITEMBOOST_A:						case SC_PREMIUMSERVICE_ITEMBOOST_S:
+	case SC_PREMIUMSERVICE_STORAGE:							case SC_PREMIUMSERVICE_LIFEINSURANCE:
+	case SC_PREMIUMSERVICE:


===================================================================
Index: map/mob.cpp
===================================================================
Find:
	// Increase drop rate if user has SC_ITEMBOOST
	if (sd->sc.data[SC_ITEMBOOST])
		drop_rate_bonus += sd->sc.data[SC_ITEMBOOST]->val1;

Add:
+	// TAURUS PREMIUM SERVICE
+	if (sd->sc.data[SC_PREMIUMSERVICE_ITEMBOOST_A])
+		drop_rate_bonus += sd->sc.data[SC_PREMIUMSERVICE_ITEMBOOST_A]->val1;
+	if (sd->sc.data[SC_PREMIUMSERVICE_ITEMBOOST_S])
+		drop_rate_bonus += sd->sc.data[SC_PREMIUMSERVICE_ITEMBOOST_S]->val1;

===================================================================
Index: data/texture/effect
===================================================================
Add:
+	premiumservice.tga
	
===================================================================
Index: data/luafiles514/lua files/stateicon/efstids.lub
===================================================================
Add:
+	EFST_PREMIUMSERVICE = 1501,
	
===================================================================
Index: data/luafiles514/lua files/stateicon/stateiconimginfo.lub
===================================================================
Find:
	[EFST_IDs.EFST_AID_PERIOD_PLUSJOBEXP_2ND] = "JOB_W.tga",
	
Add:
+	[EFST_IDs.EFST_PREMIUMSERVICE] = "PREMIUMSERVICE.tga",
	
===================================================================
Index: data/luafiles514/lua files/stateicon/stateiconinfo.lub
===================================================================
Find & Edit:
StateIconList[EFST_IDs.EFST_AID_PERIOD_PLUSEXP] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "ปรับค่าประสบการณ์ ", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "เพิ่มอัตราการรับค่าประสบการณ์." }
	}
}
StateIconList[EFST_IDs.EFST_AID_PERIOD_PLUSJOBEXP] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "ปรับค่าประสบการณ์ Job", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "เพิ่มอัตรารับค่าประสบการณ์ Job." }
	}
}
StateIconList[EFST_IDs.EFST_AID_PERIOD_DEADPENALTY] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "ปรับลดบทลงโทษตอนตาย", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "ลดบทลงโทษตอนตาย." }
	}
}
StateIconList[EFST_IDs.EFST_AID_PERIOD_ADDSTOREITEMCOUNT] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "Premium Storage", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "คลังเก็บของ Premium 300 ช่อง." }
	}
}
StateIconList[EFST_IDs.EFST_AID_PERIOD_RECEIVEITEM_2ND] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "ปรับอัตราการดรอป", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "เพิ่มอัตราการดรอปไอเท็ม." }
	}
}
StateIconList[EFST_IDs.EFST_AID_PERIOD_PLUSEXP_2ND] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "ปรับค่าประสบการณ์", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "เพิ่มอัตราการรับค่าประสบการณ์." }
	}
}
StateIconList[EFST_IDs.EFST_AID_PERIOD_PLUSJOBEXP_2ND] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "ปรับค่าประสบการณ์ Job", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "เพิ่มอัตราการรับค่าประสบการณ์ Job." }
	}
}
	

Add:
StateIconList[EFST_IDs.EFST_PREMIUMSERVICE] = {
	haveTimeLimit = 1,
	posTimeLimitStr = 2,
	descript = {
		{ "Premium Service", COLOR_TITLE_BUFF },
		{ "%s", COLOR_TIME },
		{ "มอบสิทธิพิเศษต่างๆให้แก่ผู้เล่น" }
	}
}