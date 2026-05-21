CREATE TABLE `vendings` (
  `id` int(10) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL,
  `char_id` int(10) UNSIGNED NOT NULL,
  `sex` enum('F','M') NOT NULL DEFAULT 'M',
  `map` varchar(20) NOT NULL,
  `x` smallint(5) UNSIGNED NOT NULL,
  `y` smallint(5) UNSIGNED NOT NULL,
  `title` varchar(80) NOT NULL,
  `body_direction` char(1) NOT NULL DEFAULT '4',
  `head_direction` char(1) NOT NULL DEFAULT '0',
  `sit` char(1) NOT NULL DEFAULT '1',
  `vending_item` int(11) NOT NULL,
  `autotrade` tinyint(4) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;