ALTER TABLE `sales` DROP PRIMARY KEY;
ALTER TABLE `sales` ADD `id` INT NOT NULL FIRST;
ALTER TABLE `sales` ADD PRIMARY KEY (`id`);
ALTER TABLE `sales` CHANGE `id` `id` INT NOT NULL AUTO_INCREMENT;

CREATE TABLE `sales_limited_acc` (
  `sales_id` int NOT NULL,
  `account_id` int NOT NULL,
  `amount` int NOT NULL
) ENGINE=MyISAM;

ALTER TABLE `sales_limited_acc`
  ADD PRIMARY KEY (`sales_id`,`account_id`);

ALTER TABLE `sales`
	ADD COLUMN `rental` int(20) NOT NULL default '0' AFTER `amount`;
