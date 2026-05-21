CREATE TABLE `rank_title` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `zeny` int(10) UNSIGNED NOT NULL,
  `elunium` int(10) UNSIGNED NOT NULL,
  `oridecon` int(10) UNSIGNED NOT NULL,
  `emperium` int(10) UNSIGNED NOT NULL,
  PRIMARY KEY (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

# AUTO Ranking v1
  ALTER TABLE  `char` ADD  `rank` INT NOT NULL DEFAULT  '0';