-- Separate HW rankings for MA 9/9/69; preserves previous-season rows.
CREATE TABLE IF NOT EXISTS `cabinet_hw_zeny_ranking_090969` (`char_id` INT NOT NULL, `player_name` VARCHAR(30) NOT NULL, `spins` INT NOT NULL DEFAULT 0, PRIMARY KEY (`char_id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE IF NOT EXISTS `cabinet_hw_cash_ranking_090969` (`char_id` INT NOT NULL, `player_name` VARCHAR(30) NOT NULL, `spins` INT NOT NULL DEFAULT 0, PRIMARY KEY (`char_id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8;
