CREATE TABLE IF NOT EXISTS `loginlog` (
  `time` datetime NOT NULL,
  `ip` varchar(15) NOT NULL default '',
  `unique_id` varchar(15) NOT NULL default '',
  `user` varchar(23) NOT NULL default '',
  `rcode` tinyint(4) NOT NULL default '0',
  `log` varchar(255) NOT NULL default '',
  INDEX (`ip`)
) ENGINE=MyISAM ;