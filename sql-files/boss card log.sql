
CREATE TABLE `mini_boss_card_log` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `account_id` INT(11) NOT NULL COMMENT 'Account ID of the player',
  `char_id` INT(11) NOT NULL COMMENT 'Character ID of the player',
  `char_name` VARCHAR(30) NOT NULL COMMENT 'Character name',
  `mob_id` INT(11) NOT NULL COMMENT 'Monster ID of the miniboss',
  `mob_name` VARCHAR(50) NOT NULL COMMENT 'Sprite name of the miniboss',
  `item_id` INT(11) NOT NULL COMMENT 'Item ID of the card',
  `item_name` VARCHAR(50) NOT NULL COMMENT 'Name of the card',
  `drop_rate` INT(11) NOT NULL COMMENT 'Drop rate in permillage (e.g., 100 = 1%)',
  `map` VARCHAR(11) NOT NULL COMMENT 'Map where the card was dropped',
  `drop_time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Timestamp of the drop',
  PRIMARY KEY (`id`),
  INDEX `account_id` (`account_id`),
  INDEX `char_id` (`char_id`),
  INDEX `mob_id` (`mob_id`),
  INDEX `item_id` (`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Log of miniboss card drops';