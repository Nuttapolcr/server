CREATE TABLE IF NOT EXISTS `vip_rank_account` (
  `account_id` INT(11) UNSIGNED NOT NULL,
  `donation_total` BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,
  `rank` SMALLINT(5) UNSIGNED NOT NULL DEFAULT 0,
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`account_id`),
  KEY `rank` (`rank`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `vip_rank_transaction` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` INT(11) UNSIGNED NOT NULL,
  `operation` ENUM('set','add','refund') NOT NULL,
  `amount` BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,
  `total_before` BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,
  `total_after` BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,
  `rank_before` SMALLINT(5) UNSIGNED NOT NULL DEFAULT 0,
  `rank_after` SMALLINT(5) UNSIGNED NOT NULL DEFAULT 0,
  `reference` VARCHAR(64) NULL DEFAULT NULL,
  `actor_account_id` INT(11) UNSIGNED NOT NULL DEFAULT 0,
  `reason` VARCHAR(255) NOT NULL DEFAULT '',
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `account_created` (`account_id`, `created_at`),
  UNIQUE KEY `reference` (`reference`)
) ENGINE=InnoDB;
