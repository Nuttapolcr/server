ALTER TABLE `aa_common_config`
  ADD COLUMN IF NOT EXISTS `prio_item_config` int(10) unsigned NOT NULL DEFAULT 0 AFTER `pickup_item_config`,
  ADD COLUMN IF NOT EXISTS `tp_mvp` tinyint(1) NOT NULL DEFAULT 0 AFTER `tp_delay_nomobmeet`,
  ADD COLUMN IF NOT EXISTS `tp_miniboss` tinyint(1) NOT NULL DEFAULT 0 AFTER `tp_mvp`,
  ADD COLUMN IF NOT EXISTS `accept_party_request` tinyint(1) NOT NULL DEFAULT 1 AFTER `tp_miniboss`,
  ADD COLUMN IF NOT EXISTS `token_siegfried` tinyint(1) NOT NULL DEFAULT 1 AFTER `accept_party_request`,
  ADD COLUMN IF NOT EXISTS `return_to_savepoint` tinyint(1) NOT NULL DEFAULT 1 AFTER `token_siegfried`,
  ADD COLUMN IF NOT EXISTS `map_mob_selection` int(11) NOT NULL DEFAULT 0 AFTER `return_to_savepoint`,
  ADD COLUMN IF NOT EXISTS `action_on_end` int(11) NOT NULL DEFAULT 0 AFTER `map_mob_selection`,
  ADD COLUMN IF NOT EXISTS `monster_surround` int(11) NOT NULL DEFAULT 0 AFTER `action_on_end`,
  ADD COLUMN IF NOT EXISTS `monk_combo` tinyint(1) NOT NULL DEFAULT 0 AFTER `monster_surround`;

ALTER TABLE `aa_items`
  ADD COLUMN IF NOT EXISTS `status` int(11) NOT NULL DEFAULT 0 AFTER `min_sp`;

CREATE TABLE IF NOT EXISTS `aa_autobuyitems` (
  `char_id` int(10) unsigned NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  `quantity` int(11) NOT NULL DEFAULT 0,
  `min_trigger` int(11) NOT NULL DEFAULT 0,
  UNIQUE KEY `char_id` (`char_id`,`item_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620;

CREATE TABLE IF NOT EXISTS `aa_autosellitems` (
  `char_id` int(10) unsigned NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  UNIQUE KEY `char_id` (`char_id`,`item_id`)
) ENGINE=MyISAM DEFAULT CHARSET=tis620;
