
CREATE TABLE IF NOT EXISTS `aa_common_config` (
  `char_id` int(10) unsigned NOT NULL,
  `stopmelee` tinyint(1) NOT NULL DEFAULT 0,
  `pickup_item_config` int(10) unsigned NOT NULL DEFAULT 0,
  `aggressive_behavior` tinyint(1) NOT NULL DEFAULT 0,
  `autositregen_conf` tinyint(1) NOT NULL DEFAULT 0,
  `autositregen_maxhp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `autositregen_minhp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `autositregen_maxsp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `autositregen_minsp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `tp_use_teleport` tinyint(1) NOT NULL DEFAULT 0,
  `tp_use_flywing` tinyint(1) NOT NULL DEFAULT 0,
  `tp_min_hp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `tp_delay_nomobmeet` int(10) unsigned NOT NULL DEFAULT 0,
  `skill_rate` int(10) unsigned NOT NULL DEFAULT 0,
  `teleport_boss` tinyint(1) NOT NULL DEFAULT 0,
  `focus_mob` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`),
  UNIQUE KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=tis620;

CREATE TABLE IF NOT EXISTS `aa_items` (
  `char_id` int(10) unsigned NOT NULL,
  `type` smallint(5) unsigned NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  `min_hp` smallint(5) unsigned NOT NULL DEFAULT 0,
  `min_sp` smallint(5) unsigned NOT NULL DEFAULT 0,
  UNIQUE KEY `char_id` (`char_id`,`type`,`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=tis620;

CREATE TABLE IF NOT EXISTS `aa_mobs` (
  `char_id` int(10) unsigned NOT NULL,
  `mob_id` int(10) unsigned NOT NULL,
  UNIQUE KEY `char_id` (`char_id`,`mob_id`)
) ENGINE=InnoDB DEFAULT CHARSET=tis620;

CREATE TABLE IF NOT EXISTS `aa_skills` (
  `char_id` int(10) unsigned NOT NULL,
  `type` smallint(5) unsigned NOT NULL,
  `skill_id` smallint(5) unsigned NOT NULL,
  `skill_lv` smallint(5) unsigned NOT NULL DEFAULT 0,
  `min_hp` smallint(5) unsigned NOT NULL DEFAULT 0,
  UNIQUE KEY `char_id` (`char_id`,`type`,`skill_id`)
) ENGINE=InnoDB DEFAULT CHARSET=tis620;

CREATE TABLE IF NOT EXISTS `aa_flee_mobs` (
  `char_id` int(10) unsigned NOT NULL,
  `mob_id` int(10) unsigned NOT NULL,
  UNIQUE KEY `char_id` (`char_id`,`mob_id`)
) ENGINE=InnoDB DEFAULT CHARSET=tis620;
