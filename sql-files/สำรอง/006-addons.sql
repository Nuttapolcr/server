
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

CREATE TABLE IF NOT EXISTS `boss_card_log` (
`id` int NOT NULL UNIQUE AUTO_INCREMENT,
`account_id` int(11) NOT NULL,
`char_name` varchar(32) NOT NULL default '',
`mvp_id` int(11) NOT NULL,
`mvp_name` varchar(32) NOT NULL default '',
`card_id` int(11) NOT NULL,
`card_name` varchar(50) NOT NULL default '',
`drop_map` varchar(32) NOT NULL default '',
`drop_date` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
PRIMARY KEY (`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `dropped_mini_boss_card_log` [Bad]
--

CREATE TABLE IF NOT EXISTS `mini_boss_card_log` (
`id` int NOT NULL UNIQUE AUTO_INCREMENT,
`account_id` int(11) NOT NULL,
`char_name` varchar(32) NOT NULL default '',
`mini_boss_id` int(11) NOT NULL,
`mini_boss_name` varchar(32) NOT NULL default '',
`card_id` int(11) NOT NULL,
`card_name` varchar(50) NOT NULL default '',
`drop_map` varchar(32) NOT NULL default '',
`drop_date` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
PRIMARY KEY (`id`)
) ENGINE=MyISAM;