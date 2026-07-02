CREATE TABLE IF NOT EXISTS `ab_common_config` (
  `char_id` int(10) unsigned NOT NULL,
  `following_player` int(10) unsigned NOT NULL DEFAULT 0,
  `dist_to_leader` smallint(5) unsigned NOT NULL DEFAULT 2,
  `autobuff_resurection` tinyint(1) NOT NULL DEFAULT 1,
  `skill_cd` bigint(20) NOT NULL DEFAULT 0,
  `allow_pm_cmd` smallint(5) unsigned NOT NULL DEFAULT 0,
  `autobuff_potions` tinyint(1) NOT NULL DEFAULT 1,
  `return_to_savepoint` tinyint(1) NOT NULL DEFAULT 0,
  `autobuff_token_siegfried` tinyint(1) NOT NULL DEFAULT 0,
  `autobuff_disable_alone` tinyint(1) NOT NULL DEFAULT 1,
  `priorize_buff` tinyint(1) NOT NULL DEFAULT 0,
  `autositregen_conf` tinyint(1) NOT NULL DEFAULT 0,
  `autositregen_maxhp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `autositregen_minhp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `autositregen_maxsp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `autositregen_minsp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `tp_cap_dmg` int(10) unsigned NOT NULL DEFAULT 0,
  `tp_min_hp` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620 COLLATE=tis620_thai_ci;

CREATE TABLE IF NOT EXISTS `ab_items` (
  `char_id` int(10) unsigned NOT NULL,
  `type` smallint(5) unsigned NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  `min_hp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `min_sp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `status` int(11) NOT NULL DEFAULT 0,
  UNIQUE KEY `char_type_item` (`char_id`,`type`,`item_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620 COLLATE=tis620_thai_ci;

CREATE TABLE IF NOT EXISTS `ab_skills` (
  `ab_skills_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(10) unsigned NOT NULL,
  `type` smallint(5) unsigned NOT NULL,
  `skill_id` smallint(5) unsigned NOT NULL,
  `skill_lv` smallint(5) unsigned NOT NULL DEFAULT 0,
  `min_hp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `last_use` bigint(20) NOT NULL DEFAULT 0,
  `priorize_type` smallint(5) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`ab_skills_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620 COLLATE=tis620_thai_ci;

CREATE TABLE IF NOT EXISTS `ab_skills_char_ids` (
  `ab_skills_id` bigint(20) unsigned NOT NULL,
  `ab_char_id` int(10) unsigned NOT NULL,
  UNIQUE KEY `skill_char` (`ab_skills_id`,`ab_char_id`),
  KEY `ab_char_id` (`ab_char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620 COLLATE=tis620_thai_ci;

CREATE TABLE IF NOT EXISTS `ab_pm_char_ids` (
  `char_id` int(10) unsigned NOT NULL,
  `ab_char_id` int(10) unsigned NOT NULL,
  UNIQUE KEY `char_ab_char` (`char_id`,`ab_char_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620 COLLATE=tis620_thai_ci;

CREATE TABLE IF NOT EXISTS `ab_autobuyitems` (
  `char_id` int(10) unsigned NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  `quantity` int(11) NOT NULL DEFAULT 0,
  `min_trigger` int(11) NOT NULL DEFAULT 0,
  UNIQUE KEY `char_item` (`char_id`,`item_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620 COLLATE=tis620_thai_ci;

CREATE TABLE IF NOT EXISTS `ab_autosellitems` (
  `char_id` int(10) unsigned NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  UNIQUE KEY `char_item` (`char_id`,`item_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620 COLLATE=tis620_thai_ci;
