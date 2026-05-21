-- phpMyAdmin SQL Dump
-- version 4.9.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Dec 25, 2024 at 02:26 PM
-- Server version: 8.0.17
-- PHP Version: 7.3.10

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `100main`
--

-- --------------------------------------------------------

--
-- Table structure for table `aa_common_config`
--

--
-- Table structure for table `vip_storage`
--

--
-- Table structure for table `vip_storage`
--

--
-- Table structure for table `stalls`
--

CREATE TABLE IF NOT EXISTS `stalls` (
  `id` int UNSIGNED NOT NULL,
  `char_id` int UNSIGNED NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `class` smallint UNSIGNED NOT NULL DEFAULT '0',
  `map` varchar(20) NOT NULL,
  `x` smallint UNSIGNED NOT NULL,
  `y` smallint UNSIGNED NOT NULL,
  `sex` enum('F','M') NOT NULL DEFAULT 'M',
  `hair` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `weapon` smallint UNSIGNED NOT NULL DEFAULT '0',
  `shield` smallint UNSIGNED NOT NULL DEFAULT '0',
  `head_mid` smallint UNSIGNED NOT NULL DEFAULT '0',
  `head_top` smallint UNSIGNED NOT NULL DEFAULT '0',
  `head_bottom` smallint UNSIGNED NOT NULL DEFAULT '0',
  `hair_color` smallint UNSIGNED NOT NULL DEFAULT '0',
  `clothes_color` smallint UNSIGNED NOT NULL DEFAULT '0',
  `body` smallint UNSIGNED NOT NULL DEFAULT '0',
  `name` varchar(30) NOT NULL DEFAULT '',
  `title` varchar(80) NOT NULL,
  `expire_time` int UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `stalls_vending_items`
--
CREATE TABLE IF NOT EXISTS  `stalls_vending_items` (
  `stalls_id` int(11) unsigned NOT NULL default '0',
  `index` smallint(5) unsigned NOT NULL,
  `nameid` int(10) unsigned NOT NULL default '0',
  `amount` int(11) unsigned NOT NULL default '0',
  `identify` smallint(6) NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) unsigned NOT NULL default '0',
  `card0` int(10) unsigned NOT NULL default '0',
  `card1` int(10) unsigned NOT NULL default '0',
  `card2` int(10) unsigned NOT NULL default '0',
  `card3` int(10) unsigned NOT NULL default '0',
  `option_id0` smallint(5) NOT NULL default '0',
  `option_val0` smallint(5) NOT NULL default '0',
  `option_parm0` tinyint(3) NOT NULL default '0',
  `option_id1` smallint(5) NOT NULL default '0',
  `option_val1` smallint(5) NOT NULL default '0',
  `option_parm1` tinyint(3) NOT NULL default '0',
  `option_id2` smallint(5) NOT NULL default '0',
  `option_val2` smallint(5) NOT NULL default '0',
  `option_parm2` tinyint(3) NOT NULL default '0',
  `option_id3` smallint(5) NOT NULL default '0',
  `option_val3` smallint(5) NOT NULL default '0',
  `option_parm3` tinyint(3) NOT NULL default '0',
  `option_id4` smallint(5) NOT NULL default '0',
  `option_val4` smallint(5) NOT NULL default '0',
  `option_parm4` tinyint(3) NOT NULL default '0',
  `expire_time` int(11) unsigned NOT NULL default '0',
  `bound` tinyint(3) unsigned NOT NULL default '0',
  `unique_id` bigint(20) unsigned NOT NULL default '0',
  `enchantgrade` tinyint unsigned NOT NULL default '0',
  `price` int(10) unsigned NOT NULL default '0',
   PRIMARY KEY (`stalls_id`, `index`)
) ENGINE=MyISAM;

--
-- Table structure for table `stalls_buying_items`
--
CREATE TABLE IF NOT EXISTS  `stalls_buying_items` (
  `stalls_id` int(11) unsigned NOT NULL default '0',
  `nameid` int(10) unsigned NOT NULL default '0',
  `amount` int(11) unsigned NOT NULL default '0',
  `price` int(10) unsigned NOT NULL default '0',
   PRIMARY KEY (`stalls_id`, `nameid`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `storage2` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `account_id` int(11) unsigned NOT NULL default '0',
  `nameid` int(10) unsigned NOT NULL default '0',
  `amount` smallint(11) unsigned NOT NULL default '0',
  `equip` int(11) unsigned NOT NULL default '0',
  `identify` smallint(6) unsigned NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) unsigned NOT NULL default '0',
  `card0` int(10) unsigned NOT NULL default '0',
  `card1` int(10) unsigned NOT NULL default '0',
  `card2` int(10) unsigned NOT NULL default '0',
  `card3` int(10) unsigned NOT NULL default '0',
  `option_id0` smallint(5) unsigned NOT NULL default '0',
  `option_val0` smallint(5) unsigned NOT NULL default '0',
  `option_parm0` tinyint(3) unsigned NOT NULL default '0',
  `option_id1` smallint(5) unsigned NOT NULL default '0',
  `option_val1` smallint(5) unsigned NOT NULL default '0',
  `option_parm1` tinyint(3) unsigned NOT NULL default '0',
  `option_id2` smallint(5) unsigned NOT NULL default '0',
  `option_val2` smallint(5) unsigned NOT NULL default '0',
  `option_parm2` tinyint(3) unsigned NOT NULL default '0',
  `option_id3` smallint(5) unsigned NOT NULL default '0',
  `option_val3` smallint(5) unsigned NOT NULL default '0',
  `option_parm3` tinyint(3) unsigned NOT NULL default '0',
  `option_id4` smallint(5) unsigned NOT NULL default '0',
  `option_val4` smallint(5) unsigned NOT NULL default '0',
  `option_parm4` tinyint(3) unsigned NOT NULL default '0',
  `expire_time` int(11) unsigned NOT NULL default '0',
  `bound` tinyint(3) unsigned NOT NULL default '0',
  `unique_id` bigint(20) unsigned NOT NULL default '0',
  `enchantgrade` tinyint unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `storage1` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `account_id` int(11) unsigned NOT NULL default '0',
  `nameid` int(10) unsigned NOT NULL default '0',
  `amount` smallint(11) unsigned NOT NULL default '0',
  `equip` int(11) unsigned NOT NULL default '0',
  `identify` smallint(6) unsigned NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) unsigned NOT NULL default '0',
  `card0` int(10) unsigned NOT NULL default '0',
  `card1` int(10) unsigned NOT NULL default '0',
  `card2` int(10) unsigned NOT NULL default '0',
  `card3` int(10) unsigned NOT NULL default '0',
  `option_id0` smallint(5) unsigned NOT NULL default '0',
  `option_val0` smallint(5) unsigned NOT NULL default '0',
  `option_parm0` tinyint(3) unsigned NOT NULL default '0',
  `option_id1` smallint(5) unsigned NOT NULL default '0',
  `option_val1` smallint(5) unsigned NOT NULL default '0',
  `option_parm1` tinyint(3) unsigned NOT NULL default '0',
  `option_id2` smallint(5) unsigned NOT NULL default '0',
  `option_val2` smallint(5) unsigned NOT NULL default '0',
  `option_parm2` tinyint(3) unsigned NOT NULL default '0',
  `option_id3` smallint(5) unsigned NOT NULL default '0',
  `option_val3` smallint(5) unsigned NOT NULL default '0',
  `option_parm3` tinyint(3) unsigned NOT NULL default '0',
  `option_id4` smallint(5) unsigned NOT NULL default '0',
  `option_val4` smallint(5) unsigned NOT NULL default '0',
  `option_parm4` tinyint(3) unsigned NOT NULL default '0',
  `expire_time` int(11) unsigned NOT NULL default '0',
  `bound` tinyint(3) unsigned NOT NULL default '0',
  `unique_id` bigint(20) unsigned NOT NULL default '0',
  `enchantgrade` tinyint unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;




CREATE TABLE `aa_common_config` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `stopmelee` tinyint(1) NOT NULL DEFAULT '0',
  `pickup_item_config` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `aggressive_behavior` tinyint(1) NOT NULL DEFAULT '0',
  `autositregen_conf` tinyint(1) NOT NULL DEFAULT '0',
  `autositregen_maxhp` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `autositregen_minhp` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `autositregen_maxsp` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `autositregen_minsp` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `tp_use_teleport` tinyint(1) NOT NULL DEFAULT '0',
  `tp_use_flywing` tinyint(1) NOT NULL DEFAULT '0',
  `tp_min_hp` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `tp_delay_nomobmeet` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `skill_rate` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `teleport_boss` tinyint(1) NOT NULL DEFAULT '0',
  `focus_mob` tinyint(1) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=tis620;

-- --------------------------------------------------------

--
-- Table structure for table `aa_flee_mobs`
--

CREATE TABLE `aa_flee_mobs` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `mob_id` int(10) UNSIGNED NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=tis620;

-- --------------------------------------------------------

--
-- Table structure for table `aa_items`
--

CREATE TABLE `aa_items` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `type` smallint(5) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL,
  `min_hp` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `min_sp` smallint(5) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=tis620;

-- --------------------------------------------------------

--
-- Table structure for table `aa_mobs`
--

CREATE TABLE `aa_mobs` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `mob_id` int(10) UNSIGNED NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=tis620;

-- --------------------------------------------------------

--
-- Table structure for table `aa_skills`
--

CREATE TABLE `aa_skills` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `type` smallint(5) UNSIGNED NOT NULL,
  `skill_id` smallint(5) UNSIGNED NOT NULL,
  `skill_lv` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `min_hp` smallint(5) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=tis620;

-- --------------------------------------------------------

--
-- Table structure for table `account_emote`
--

CREATE TABLE `account_emote` (
  `id` int(11) NOT NULL,
  `account_id` int(11) NOT NULL,
  `pack_id` int(11) NOT NULL,
  `expire_time` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `acc_reg_num`
--

CREATE TABLE `acc_reg_num` (
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '',
  `index` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` bigint(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `acc_reg_str`
--

CREATE TABLE `acc_reg_str` (
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '',
  `index` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` varchar(254) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `achievement`
--

CREATE TABLE `achievement` (
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `id` bigint(11) UNSIGNED NOT NULL,
  `count1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count4` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count5` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count6` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count7` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count8` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count9` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count10` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `completed` datetime DEFAULT NULL,
  `rewarded` datetime DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------


-- --------------------------------------------------------

--
-- Table structure for table `auction`
--

CREATE TABLE `auction` (
  `auction_id` bigint(20) UNSIGNED NOT NULL,
  `seller_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `seller_name` varchar(30) NOT NULL DEFAULT '',
  `buyer_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `buyer_name` varchar(30) NOT NULL DEFAULT '',
  `price` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `buynow` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `hours` smallint(6) NOT NULL DEFAULT '0',
  `timestamp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `item_name` varchar(50) NOT NULL DEFAULT '',
  `type` smallint(6) NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `barter`
--

CREATE TABLE `barter` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `index` smallint(5) UNSIGNED NOT NULL,
  `amount` smallint(5) UNSIGNED NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `bonus_script`
--

CREATE TABLE `bonus_script` (
  `char_id` int(11) UNSIGNED NOT NULL,
  `script` text NOT NULL,
  `tick` bigint(20) NOT NULL DEFAULT '0',
  `flag` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `type` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `icon` smallint(3) NOT NULL DEFAULT '-1'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `boss_card_log`
--

CREATE TABLE `boss_card_log` (
  `id` int(11) NOT NULL,
  `account_id` int(11) NOT NULL,
  `char_name` varchar(32) NOT NULL DEFAULT '',
  `mvp_id` int(11) NOT NULL,
  `mvp_name` varchar(32) NOT NULL DEFAULT '',
  `card_id` int(11) NOT NULL,
  `card_name` varchar(50) NOT NULL DEFAULT '',
  `drop_map` varchar(32) NOT NULL DEFAULT '',
  `drop_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------


-- --------------------------------------------------------

--
-- Table structure for table `buffet_time`
--

CREATE TABLE `buffet_time` (
  `account_id` int(10) NOT NULL,
  `expire` timestamp NULL DEFAULT NULL,
  `type` tinyint(4) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `buyingstores`
--

CREATE TABLE `buyingstores` (
  `id` int(10) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL,
  `char_id` int(10) UNSIGNED NOT NULL,
  `sex` enum('F','M') NOT NULL DEFAULT 'M',
  `map` varchar(20) NOT NULL,
  `x` smallint(5) UNSIGNED NOT NULL,
  `y` smallint(5) UNSIGNED NOT NULL,
  `title` varchar(80) NOT NULL,
  `limit` int(10) UNSIGNED NOT NULL,
  `body_direction` char(1) NOT NULL DEFAULT '4',
  `head_direction` char(1) NOT NULL DEFAULT '0',
  `sit` char(1) NOT NULL DEFAULT '1',
  `autotrade` tinyint(4) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `buyingstore_items`
--

CREATE TABLE `buyingstore_items` (
  `buyingstore_id` int(10) UNSIGNED NOT NULL,
  `index` smallint(5) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL,
  `amount` smallint(5) UNSIGNED NOT NULL,
  `price` int(10) UNSIGNED NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `cart_inventory`
--

CREATE TABLE `cart_inventory` (
  `id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` int(11) NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------


-- --------------------------------------------------------

--
-- Table structure for table `char`
--

CREATE TABLE `char` (
  `char_id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_num` tinyint(1) NOT NULL DEFAULT '0',
  `name` varchar(30) NOT NULL DEFAULT '',
  `class` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `base_level` smallint(6) UNSIGNED NOT NULL DEFAULT '1',
  `job_level` smallint(6) UNSIGNED NOT NULL DEFAULT '1',
  `base_exp` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `job_exp` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `zeny` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `str` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `agi` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `vit` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `int` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `dex` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `luk` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `pow` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `sta` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `wis` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `spl` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `con` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `crt` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `max_hp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `hp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `max_sp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `sp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `max_ap` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `ap` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `status_point` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `skill_point` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `trait_point` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `option` int(11) NOT NULL DEFAULT '0',
  `karma` tinyint(3) NOT NULL DEFAULT '0',
  `manner` smallint(6) NOT NULL DEFAULT '0',
  `party_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `pet_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `homun_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `elemental_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `hair` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `hair_color` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `clothes_color` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `body` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `weapon` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `shield` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `head_top` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `head_mid` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `head_bottom` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `robe` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `last_map` varchar(11) NOT NULL DEFAULT '',
  `last_x` smallint(4) UNSIGNED NOT NULL DEFAULT '53',
  `last_y` smallint(4) UNSIGNED NOT NULL DEFAULT '111',
  `last_instanceid` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `save_map` varchar(11) NOT NULL DEFAULT '',
  `save_x` smallint(4) UNSIGNED NOT NULL DEFAULT '53',
  `save_y` smallint(4) UNSIGNED NOT NULL DEFAULT '111',
  `partner_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `online` tinyint(2) NOT NULL DEFAULT '0',
  `father` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `mother` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `child` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `fame` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `rename` smallint(3) UNSIGNED NOT NULL DEFAULT '0',
  `delete_date` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `moves` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `unban_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `font` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `uniqueitem_counter` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `sex` enum('M','F') NOT NULL,
  `hotkey_rowshift` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `hotkey_rowshift2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `clan_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `last_login` datetime DEFAULT NULL,
  `title_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `show_equip` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `inventory_slots` smallint(6) NOT NULL DEFAULT '200',
  `body_direction` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `disable_call` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `charlog`
--

CREATE TABLE `charlog` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `time` datetime NOT NULL,
  `char_msg` varchar(255) NOT NULL DEFAULT 'char select',
  `account_id` int(11) NOT NULL DEFAULT '0',
  `char_num` tinyint(4) NOT NULL DEFAULT '0',
  `name` varchar(23) NOT NULL DEFAULT '',
  `str` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `agi` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `vit` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `int` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `dex` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `luk` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `hair` tinyint(4) NOT NULL DEFAULT '0',
  `hair_color` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `char_configs`
--

CREATE TABLE IF NOT EXISTS `user_configs` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `data` longtext NOT NULL,
  PRIMARY KEY (`world_name`, `account_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `char_configs` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(11) unsigned NOT NULL,
  `data` longtext NOT NULL,
  PRIMARY KEY (`world_name`, `account_id`, `char_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `merchant_configs` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `char_id` INT(11) UNSIGNED NOT NULL,
  `store_type` tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
  `data` longtext NOT NULL,
  PRIMARY KEY (`world_name`, `account_id`, `char_id`, `store_type`)
) ENGINE=MyISAM;


-- --------------------------------------------------------

--
-- Table structure for table `char_emote`
--

CREATE TABLE `char_emote` (
  `id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `pack_id` int(11) NOT NULL,
  `expire_time` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `char_reg_num`
--

CREATE TABLE `char_reg_num` (
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '',
  `index` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` bigint(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `char_reg_str`
--

CREATE TABLE `char_reg_str` (
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '',
  `index` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` varchar(254) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------



-- --------------------------------------------------------

--
-- Table structure for table `clan`
--

CREATE TABLE `clan` (
  `clan_id` int(11) UNSIGNED NOT NULL,
  `name` varchar(24) NOT NULL DEFAULT '',
  `master` varchar(24) NOT NULL DEFAULT '',
  `mapname` varchar(24) NOT NULL DEFAULT '',
  `max_member` smallint(6) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `clan`
--

INSERT INTO `clan` (`clan_id`, `name`, `master`, `mapname`, `max_member`) VALUES
(1, 'Swordman Clan', 'Raffam Oranpere', 'prontera', 500),
(2, 'Arcwand Clan', 'Devon Aire', 'geffen', 500),
(3, 'Golden Mace Clan', 'Berman Aire', 'prontera', 500),
(4, 'Crossbow Clan', 'Shaam Rumi', 'payon', 500);

-- --------------------------------------------------------

--
-- Table structure for table `clan_alliance`
--

CREATE TABLE `clan_alliance` (
  `clan_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `opposition` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `alliance_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `clan_alliance`
--

INSERT INTO `clan_alliance` (`clan_id`, `opposition`, `alliance_id`, `name`) VALUES
(1, 0, 3, 'Golden Mace Clan'),
(2, 0, 3, 'Golden Mace Clan'),
(2, 1, 4, 'Crossbow Clan'),
(3, 0, 1, 'Swordman Clan'),
(3, 0, 2, 'Arcwand Clan'),
(3, 0, 4, 'Crossbow Clan'),
(4, 0, 3, 'Golden Mace Clan'),
(4, 1, 2, 'Arcwand Clan');

-- --------------------------------------------------------

--
-- Table structure for table `collection_storage`
--

CREATE TABLE `collection_storage` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `db_roulette`
--

CREATE TABLE `db_roulette` (
  `index` int(11) NOT NULL DEFAULT '0',
  `level` smallint(5) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL,
  `amount` smallint(5) UNSIGNED NOT NULL DEFAULT '1',
  `flag` smallint(5) UNSIGNED NOT NULL DEFAULT '1'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `db_roulette`
--

INSERT INTO `db_roulette` (`index`, `level`, `item_id`, `amount`, `flag`) VALUES
(0, 1, 675, 1, 1),
(1, 1, 671, 1, 0),
(2, 1, 678, 1, 0),
(3, 1, 604, 1, 0),
(4, 1, 522, 1, 0),
(5, 1, 12609, 1, 0),
(6, 1, 12523, 1, 0),
(7, 1, 985, 1, 0),
(8, 1, 984, 1, 0),
(9, 2, 675, 1, 1),
(10, 2, 671, 1, 0),
(11, 2, 603, 1, 0),
(12, 2, 608, 1, 0),
(13, 2, 607, 1, 0),
(14, 2, 12522, 1, 0),
(15, 2, 6223, 1, 0),
(16, 2, 6224, 1, 0),
(17, 3, 675, 1, 1),
(18, 3, 671, 1, 0),
(19, 3, 12108, 1, 0),
(20, 3, 617, 1, 0),
(21, 3, 12514, 1, 0),
(22, 3, 7444, 1, 0),
(23, 3, 969, 1, 0),
(24, 4, 675, 1, 1),
(25, 4, 671, 1, 0),
(26, 4, 616, 1, 0),
(27, 4, 12516, 1, 0),
(28, 4, 22777, 1, 0),
(29, 4, 6231, 1, 0),
(30, 5, 671, 1, 1),
(31, 5, 12246, 1, 0),
(32, 5, 12263, 1, 0),
(33, 5, 12831, 1, 0),
(34, 5, 6235, 1, 0),
(35, 6, 671, 1, 1),
(36, 6, 12766, 1, 0),
(37, 6, 6234, 1, 0),
(38, 6, 6233, 1, 0),
(39, 7, 671, 1, 1),
(40, 7, 6233, 1, 0),
(41, 7, 6233, 1, 0);

-- --------------------------------------------------------

--
-- Table structure for table `elemental`
--

CREATE TABLE `elemental` (
  `ele_id` int(11) UNSIGNED NOT NULL,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) UNSIGNED NOT NULL DEFAULT '0',
  `mode` int(11) UNSIGNED NOT NULL DEFAULT '1',
  `hp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `sp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `max_hp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `max_sp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `atk1` mediumint(6) UNSIGNED NOT NULL DEFAULT '0',
  `atk2` mediumint(6) UNSIGNED NOT NULL DEFAULT '0',
  `matk` mediumint(6) UNSIGNED NOT NULL DEFAULT '0',
  `aspd` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `def` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `mdef` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `flee` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `hit` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `life_time` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------


-- --------------------------------------------------------

--
-- Table structure for table `friends`
--

CREATE TABLE `friends` (
  `char_id` int(11) NOT NULL DEFAULT '0',
  `friend_id` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `global_acc_reg_num`
--

CREATE TABLE `global_acc_reg_num` (
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '',
  `index` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` bigint(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `global_acc_reg_str`
--

CREATE TABLE `global_acc_reg_str` (
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '',
  `index` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` varchar(254) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild`
--

CREATE TABLE `guild` (
  `guild_id` int(11) UNSIGNED NOT NULL,
  `name` varchar(24) NOT NULL DEFAULT '',
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `master` varchar(24) NOT NULL DEFAULT '',
  `guild_lv` tinyint(6) UNSIGNED NOT NULL DEFAULT '0',
  `connect_member` tinyint(6) UNSIGNED NOT NULL DEFAULT '0',
  `max_member` tinyint(6) UNSIGNED NOT NULL DEFAULT '0',
  `average_lv` smallint(6) UNSIGNED NOT NULL DEFAULT '1',
  `exp` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `next_exp` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `skill_point` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',
  `mes1` varchar(60) NOT NULL DEFAULT '',
  `mes2` varchar(120) NOT NULL DEFAULT '',
  `emblem_len` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `emblem_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `emblem_data` blob,
  `last_master_change` datetime DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_alliance`
--

CREATE TABLE `guild_alliance` (
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `opposition` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `alliance_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_castle`
--

CREATE TABLE `guild_castle` (
  `castle_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `economy` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `defense` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `triggerE` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `triggerD` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nextTime` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `payTime` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `createTime` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleC` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG0` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG1` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG2` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG3` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG4` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG5` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG6` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG7` int(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_emblems`
--

CREATE TABLE IF NOT EXISTS `guild_emblems` (
  `world_name` varchar(32) NOT NULL,
  `guild_id` int(11) unsigned NOT NULL,
  `file_type` varchar(255) NOT NULL,
  `file_data` blob,
  `version` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY (`world_name`, `guild_id`)
) ENGINE=MyISAM;
-- --------------------------------------------------------

--
-- Table structure for table `guild_expulsion`
--

CREATE TABLE `guild_expulsion` (
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT '',
  `mes` varchar(40) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_member`
--

CREATE TABLE `guild_member` (
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `exp` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `position` tinyint(6) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_position`
--

CREATE TABLE `guild_position` (
  `guild_id` int(9) UNSIGNED NOT NULL DEFAULT '0',
  `position` tinyint(6) UNSIGNED NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT '',
  `mode` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `exp_mode` tinyint(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_skill`
--

CREATE TABLE `guild_skill` (
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `lv` tinyint(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_storage`
--

CREATE TABLE `guild_storage` (
  `id` int(10) UNSIGNED NOT NULL,
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `guild_storage_log`
--

CREATE TABLE `guild_storage_log` (
  `id` int(11) NOT NULL,
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `time` datetime NOT NULL,
  `char_id` int(11) NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT '',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` int(11) NOT NULL DEFAULT '1',
  `identify` smallint(6) NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `homunculus`
--

CREATE TABLE `homunculus` (
  `homun_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) UNSIGNED NOT NULL DEFAULT '0',
  `prev_class` mediumint(9) NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT '',
  `level` smallint(4) NOT NULL DEFAULT '0',
  `exp` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `intimacy` int(12) NOT NULL DEFAULT '0',
  `hunger` smallint(4) NOT NULL DEFAULT '0',
  `str` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `agi` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `vit` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `int` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `dex` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `luk` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `hp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `max_hp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `sp` int(11) NOT NULL DEFAULT '0',
  `max_sp` int(11) NOT NULL DEFAULT '0',
  `skill_point` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `alive` tinyint(2) NOT NULL DEFAULT '1',
  `rename_flag` tinyint(2) NOT NULL DEFAULT '0',
  `vaporize` tinyint(2) NOT NULL DEFAULT '0',
  `autofeed` tinyint(2) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `hotkey`
--

CREATE TABLE `hotkey` (
  `char_id` int(11) NOT NULL,
  `hotkey` tinyint(2) UNSIGNED NOT NULL,
  `type` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `itemskill_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `skill_lvl` tinyint(4) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `interlog`
--

CREATE TABLE `interlog` (
  `id` int(11) NOT NULL,
  `time` datetime NOT NULL,
  `log` varchar(255) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `inventory`
--

CREATE TABLE `inventory` (
  `id` int(11) UNSIGNED NOT NULL,
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `favorite` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `equip_switch` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `ipbanlist`
--

CREATE TABLE `ipbanlist` (
  `list` varchar(15) NOT NULL DEFAULT '',
  `btime` datetime NOT NULL,
  `rtime` datetime NOT NULL,
  `reason` varchar(255) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `login`
--

CREATE TABLE `login` (
  `account_id` int(11) UNSIGNED NOT NULL,
  `userid` varchar(23) NOT NULL DEFAULT '',
  `user_pass` varchar(32) NOT NULL DEFAULT '',
  `sex` enum('M','F','S') NOT NULL DEFAULT 'M',
  `email` varchar(39) NOT NULL DEFAULT '',
  `group_id` tinyint(3) NOT NULL DEFAULT '0',
  `state` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `unban_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `expiration_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `logincount` mediumint(9) UNSIGNED NOT NULL DEFAULT '0',
  `lastlogin` datetime DEFAULT NULL,
  `last_ip` varchar(100) NOT NULL DEFAULT '',
  `birthdate` date DEFAULT NULL,
  `character_slots` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `pincode` varchar(4) NOT NULL DEFAULT '',
  `pincode_change` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `vip_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `old_group` tinyint(3) NOT NULL DEFAULT '0',
  `web_auth_token` varchar(17) DEFAULT NULL,
  `web_auth_token_enabled` tinyint(2) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `login`
--

INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `group_id`, `state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, `last_ip`, `birthdate`, `character_slots`, `pincode`, `pincode_change`, `vip_time`, `old_group`, `web_auth_token`, `web_auth_token_enabled`) VALUES
(1, 's1', 'p1', 'S', 'athena@athena.com', 0, 0, 0, 0, 5, '2024-12-25 21:25:58', '127.0.0.1', NULL, 0, '', 0, 0, 0, NULL, 0);

-- --------------------------------------------------------


--
-- Table structure for table `mail`
--

CREATE TABLE `mail` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `send_name` varchar(30) NOT NULL DEFAULT '',
  `send_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `dest_name` varchar(30) NOT NULL DEFAULT '',
  `dest_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `title` varchar(45) NOT NULL DEFAULT '',
  `message` varchar(500) NOT NULL DEFAULT '',
  `time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(2) NOT NULL DEFAULT '0',
  `zeny` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `type` smallint(5) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `mail_attachments`
--

CREATE TABLE `mail_attachments` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `index` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `mapreg`
--

CREATE TABLE `mapreg` (
  `varname` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `index` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` varchar(255) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `mapreg`
--

INSERT INTO `mapreg` (`varname`, `index`, `value`) VALUES
('$spiner_zeny_05', 4, '716'),
('$spiner_zeny_05', 3, '715'),
('$spiner_zeny_05', 2, '644'),
('$spiner_zeny_05', 1, '756'),
('$spiner_zeny_05', 0, '757'),
('$spiner_zeny_04', 13, '12216'),
('$spiner_zeny_04', 12, '12215'),
('$spiner_zeny_04', 11, '12217'),
('$spiner_zeny_04', 10, '12117'),
('$spiner_zeny_04', 9, '12116'),
('$spiner_zeny_04', 8, '12115'),
('$spiner_zeny_01', 0, '5967'),
('$spiner_zeny_01', 1, '28321'),
('$spiner_zeny_01', 2, '28302'),
('$spiner_zeny_02', 0, '100000'),
('$spiner_zeny_02', 1, '12103'),
('$spiner_zeny_02', 2, '616'),
('$spiner_zeny_03', 0, '100002'),
('$spiner_zeny_03', 1, '7620'),
('$spiner_zeny_03', 2, '7619'),
('$spiner_zeny_03', 3, '14192'),
('$spiner_zeny_03', 4, '70002'),
('$spiner_zeny_04', 0, '30008'),
('$spiner_zeny_04', 1, '603'),
('$spiner_cash_05', 4, '716'),
('$spiner_cash_05', 3, '715'),
('$spiner_cash_05', 2, '644'),
('$spiner_cash_05', 1, '756'),
('$spiner_cash_05', 0, '757'),
('$spiner_cash_04', 13, '12216'),
('$spiner_cash_04', 12, '12215'),
('$spiner_cash_04', 11, '12217'),
('$spiner_cash_04', 10, '12117'),
('$spiner_cash_04', 9, '12116'),
('$spiner_cash_04', 8, '12115'),
('$spiner_cash_01', 0, '5967'),
('$spiner_cash_01', 1, '28321'),
('$spiner_cash_01', 2, '28302'),
('$spiner_cash_02', 0, '100000'),
('$spiner_cash_02', 1, '12103'),
('$spiner_cash_02', 2, '616'),
('$spiner_cash_03', 0, '100002'),
('$spiner_cash_03', 1, '7620'),
('$spiner_cash_03', 2, '7619'),
('$spiner_cash_03', 3, '14192'),
('$spiner_cash_03', 4, '70002'),
('$spiner_cash_04', 0, '30008'),
('$spiner_cash_04', 1, '603'),
('$PoringSlotsX', 0, '198'),
('$PoringSlotsX', 1, '198'),
('$PoringSlotsX', 2, '198'),
('$PoringSlotsX', 3, '198'),
('$PoringSlotsX', 4, '198'),
('$PoringSlotsX', 5, '198'),
('$PoringSlotsX', 6, '200'),
('$PoringSlotsX', 7, '202'),
('$PoringSlotsX', 8, '204'),
('$PoringSlotsX', 9, '206'),
('$PoringSlotsX', 10, '208'),
('$PoringSlotsX', 11, '210'),
('$PoringSlotsX', 12, '212'),
('$PoringSlotsX', 13, '214'),
('$PoringSlotsX', 14, '216'),
('$PoringSlotsX', 15, '216'),
('$PoringSlotsX', 16, '216'),
('$PoringSlotsX', 17, '216'),
('$PoringSlotsX', 18, '216'),
('$PoringSlotsX', 19, '216'),
('$PoringSlotsX', 20, '214'),
('$PoringSlotsX', 21, '212'),
('$PoringSlotsX', 22, '210'),
('$PoringSlotsX', 23, '208'),
('$PoringSlotsX', 24, '206'),
('$PoringSlotsX', 25, '204'),
('$PoringSlotsX', 26, '202'),
('$PoringSlotsX', 27, '200'),
('$PoringSlotsY', 0, '45'),
('$PoringSlotsY', 1, '43'),
('$PoringSlotsY', 2, '41'),
('$PoringSlotsY', 3, '39'),
('$PoringSlotsY', 4, '37'),
('$PoringSlotsY', 5, '35'),
('$PoringSlotsY', 6, '35'),
('$PoringSlotsY', 7, '35'),
('$PoringSlotsY', 8, '35'),
('$PoringSlotsY', 9, '35'),
('$PoringSlotsY', 10, '35'),
('$PoringSlotsY', 11, '35'),
('$PoringSlotsY', 12, '35'),
('$PoringSlotsY', 13, '35'),
('$PoringSlotsY', 14, '35'),
('$PoringSlotsY', 15, '37'),
('$PoringSlotsY', 16, '39'),
('$PoringSlotsY', 17, '41'),
('$PoringSlotsY', 18, '43'),
('$PoringSlotsY', 19, '45'),
('$PoringSlotsY', 20, '45'),
('$PoringSlotsY', 21, '45'),
('$PoringSlotsY', 22, '45'),
('$PoringSlotsY', 23, '45'),
('$PoringSlotsY', 24, '45'),
('$PoringSlotsY', 25, '45'),
('$PoringSlotsY', 26, '45'),
('$PoringSlotsY', 27, '45'),
('$spiner_zeny_04', 7, '12114'),
('$spiner_zeny_04', 6, '12221'),
('$spiner_zeny_04', 5, '12214'),
('$spiner_zeny_04', 4, '6635'),
('$spiner_zeny_04', 3, '985'),
('$spiner_cash_04', 7, '12114'),
('$spiner_cash_04', 6, '12221'),
('$spiner_cash_04', 5, '12214'),
('$spiner_cash_04', 4, '6635'),
('$spiner_cash_04', 3, '985'),
('$spiner_zeny_05', 9, '11503'),
('$spiner_zeny_05', 8, '12210'),
('$spiner_zeny_05', 7, '12209'),
('$spiner_zeny_05', 6, '12208'),
('$spiner_zeny_05', 5, '717'),
('$spiner_cash_05', 9, '11503'),
('$spiner_cash_05', 8, '12210'),
('$spiner_cash_05', 7, '12209'),
('$spiner_cash_05', 6, '12208'),
('$spiner_cash_05', 5, '717'),
('$spiner_zeny_04', 2, '984'),
('$spiner_cash_04', 2, '984'),
('$spiner_zeny_05', 10, '11504'),
('$spiner_cash_05', 10, '11504');

-- --------------------------------------------------------

--
-- Table structure for table `market`
--

CREATE TABLE `market` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `nameid` int(10) UNSIGNED NOT NULL,
  `price` int(11) UNSIGNED NOT NULL,
  `amount` int(11) NOT NULL,
  `flag` tinyint(2) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `memo`
--

CREATE TABLE `memo` (
  `memo_id` int(11) UNSIGNED NOT NULL,
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `map` varchar(11) NOT NULL DEFAULT '',
  `x` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `y` smallint(4) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `mercenary`
--

CREATE TABLE `mercenary` (
  `mer_id` int(11) UNSIGNED NOT NULL,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) UNSIGNED NOT NULL DEFAULT '0',
  `hp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `sp` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `kill_counter` int(11) NOT NULL,
  `life_time` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `mercenary_owner`
--

CREATE TABLE `mercenary_owner` (
  `char_id` int(11) NOT NULL,
  `merc_id` int(11) NOT NULL DEFAULT '0',
  `arch_calls` int(11) NOT NULL DEFAULT '0',
  `arch_faith` int(11) NOT NULL DEFAULT '0',
  `spear_calls` int(11) NOT NULL DEFAULT '0',
  `spear_faith` int(11) NOT NULL DEFAULT '0',
  `sword_calls` int(11) NOT NULL DEFAULT '0',
  `sword_faith` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `party`
--

CREATE TABLE `party` (
  `party_id` int(11) UNSIGNED NOT NULL,
  `name` varchar(24) NOT NULL DEFAULT '',
  `exp` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',
  `item` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',
  `leader_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `leader_char` int(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `party_bookings`
--
CREATE TABLE party_bookings (
  world_name varchar(32) NOT NULL,
  account_id int(11) UNSIGNED NOT NULL,
  char_id int(11) UNSIGNED NOT NULL,
  char_name varchar(23) NOT NULL,
  purpose smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  assist tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  damagedealer tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  healer tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  tanker tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  minimum_level smallint(5) UNSIGNED NOT NULL,
  maximum_level smallint(5) UNSIGNED NOT NULL,
  comment varchar(255) NOT NULL DEFAULT '',
  created timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) DEFAULT CHARSET=utf8mb4;
-- --------------------------------------------------------

--
-- Table structure for table `pet`
--

CREATE TABLE `pet` (
  `pet_id` int(11) UNSIGNED NOT NULL,
  `class` mediumint(9) UNSIGNED NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT '',
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `level` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `egg_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `intimate` smallint(9) UNSIGNED NOT NULL DEFAULT '0',
  `hungry` smallint(9) UNSIGNED NOT NULL DEFAULT '0',
  `rename_flag` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `incubate` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `autofeed` tinyint(2) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `quest`
--

CREATE TABLE `quest` (
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `quest_id` int(10) UNSIGNED NOT NULL,
  `state` enum('0','1','2') NOT NULL DEFAULT '0',
  `time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `count1` mediumint(8) UNSIGNED NOT NULL DEFAULT '0',
  `count2` mediumint(8) UNSIGNED NOT NULL DEFAULT '0',
  `count3` mediumint(8) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `sales`
--

CREATE TABLE `sales` (
  `id` int(11) NOT NULL,
  `nameid` int(10) UNSIGNED NOT NULL,
  `start` datetime NOT NULL,
  `end` datetime NOT NULL,
  `amount` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `sales_limited_acc`
--

CREATE TABLE `sales_limited_acc` (
  `sales_id` int(11) NOT NULL,
  `account_id` int(11) NOT NULL,
  `amount` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `sc_data`
--

CREATE TABLE `sc_data` (
  `account_id` int(11) UNSIGNED NOT NULL,
  `char_id` int(11) UNSIGNED NOT NULL,
  `type` smallint(11) UNSIGNED NOT NULL,
  `tick` bigint(20) NOT NULL,
  `val1` int(11) NOT NULL DEFAULT '0',
  `val2` int(11) NOT NULL DEFAULT '0',
  `val3` int(11) NOT NULL DEFAULT '0',
  `val4` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `skill`
--

CREATE TABLE `skill` (
  `char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `lv` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `flag` tinyint(1) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `skillcooldown`
--

CREATE TABLE `skillcooldown` (
  `account_id` int(11) UNSIGNED NOT NULL,
  `char_id` int(11) UNSIGNED NOT NULL,
  `skill` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `tick` bigint(20) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `skill_homunculus`
--

CREATE TABLE `skill_homunculus` (
  `homun_id` int(11) NOT NULL,
  `id` int(11) NOT NULL,
  `lv` smallint(6) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `spiner_cash_player_logs`
--

CREATE TABLE `spiner_cash_player_logs` (
  `id` int(7) NOT NULL,
  `account_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `player_name` varchar(255) NOT NULL,
  `id_item` varchar(255) NOT NULL,
  `play_time` datetime NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `spiner_zeny_player_logs`
--

CREATE TABLE `spiner_zeny_player_logs` (
  `id` int(7) NOT NULL,
  `account_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `player_name` varchar(255) NOT NULL,
  `id_item` varchar(255) NOT NULL,
  `play_time` datetime NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `storage`
--

CREATE TABLE `storage` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) NOT NULL DEFAULT '0',
  `option_val0` smallint(5) NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) NOT NULL DEFAULT '0',
  `option_id1` smallint(5) NOT NULL DEFAULT '0',
  `option_val1` smallint(5) NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) NOT NULL DEFAULT '0',
  `option_id2` smallint(5) NOT NULL DEFAULT '0',
  `option_val2` smallint(5) NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) NOT NULL DEFAULT '0',
  `option_id3` smallint(5) NOT NULL DEFAULT '0',
  `option_val3` smallint(5) NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) NOT NULL DEFAULT '0',
  `option_id4` smallint(5) NOT NULL DEFAULT '0',
  `option_val4` smallint(5) NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `storage_1`
--

CREATE TABLE `storage_1` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `storage_2`
--

CREATE TABLE `storage_2` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `vendings`
--

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

-- --------------------------------------------------------

--
-- Table structure for table `vending_items`
--

CREATE TABLE `vending_items` (
  `vending_id` int(10) UNSIGNED NOT NULL,
  `index` smallint(5) UNSIGNED NOT NULL,
  `cartinventory_id` int(10) UNSIGNED NOT NULL,
  `amount` smallint(5) UNSIGNED NOT NULL,
  `price` int(10) UNSIGNED NOT NULL,
  `vending_item` int(10) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `vip_storage`
--

CREATE TABLE `vip_storage` (
  `id` int(11) UNSIGNED NOT NULL,
  `account_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `amount` smallint(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` smallint(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` tinyint(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card1` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card2` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `card3` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `option_id0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val0` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val1` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val2` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val3` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `option_id4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_val4` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `enchantgrade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `aa_common_config`
--
ALTER TABLE `aa_common_config`
  ADD PRIMARY KEY (`char_id`),
  ADD UNIQUE KEY `char_id` (`char_id`);

--
-- Indexes for table `aa_flee_mobs`
--
ALTER TABLE `aa_flee_mobs`
  ADD UNIQUE KEY `char_id` (`char_id`,`mob_id`);

--
-- Indexes for table `aa_items`
--
ALTER TABLE `aa_items`
  ADD UNIQUE KEY `char_id` (`char_id`,`type`,`item_id`);

--
-- Indexes for table `aa_mobs`
--
ALTER TABLE `aa_mobs`
  ADD UNIQUE KEY `char_id` (`char_id`,`mob_id`);

--
-- Indexes for table `aa_skills`
--
ALTER TABLE `aa_skills`
  ADD UNIQUE KEY `char_id` (`char_id`,`type`,`skill_id`);

--
-- Indexes for table `acc_reg_num`
--
ALTER TABLE `acc_reg_num`
  ADD PRIMARY KEY (`account_id`,`key`,`index`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `acc_reg_str`
--
ALTER TABLE `acc_reg_str`
  ADD PRIMARY KEY (`account_id`,`key`,`index`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `achievement`
--
ALTER TABLE `achievement`
  ADD PRIMARY KEY (`char_id`,`id`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `auction`
--
ALTER TABLE `auction`
  ADD PRIMARY KEY (`auction_id`);

--
-- Indexes for table `barter`
--
ALTER TABLE `barter`
  ADD PRIMARY KEY (`name`,`index`);

--
-- Indexes for table `bonus_script`
--
ALTER TABLE `bonus_script`
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `buffet_time`
--
ALTER TABLE `buffet_time`
  ADD UNIQUE KEY `account_id` (`account_id`);

--
-- Indexes for table `buyingstores`
--
ALTER TABLE `buyingstores`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `buyingstore_items`
--
ALTER TABLE `buyingstore_items`
  ADD PRIMARY KEY (`buyingstore_id`,`index`);

--
-- Indexes for table `cart_inventory`
--
ALTER TABLE `cart_inventory`
  ADD PRIMARY KEY (`id`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `char`
--
ALTER TABLE `char`
  ADD PRIMARY KEY (`char_id`),
  ADD UNIQUE KEY `name_key` (`name`),
  ADD KEY `account_id` (`account_id`),
  ADD KEY `party_id` (`party_id`),
  ADD KEY `guild_id` (`guild_id`),
  ADD KEY `online` (`online`);

--
-- Indexes for table `charlog`
--
ALTER TABLE `charlog`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `char_reg_num`
--
ALTER TABLE `char_reg_num`
  ADD PRIMARY KEY (`char_id`,`key`,`index`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `char_reg_str`
--
ALTER TABLE `char_reg_str`
  ADD PRIMARY KEY (`char_id`,`key`,`index`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `clan`
--
ALTER TABLE `clan`
  ADD PRIMARY KEY (`clan_id`);

--
-- Indexes for table `clan_alliance`
--
ALTER TABLE `clan_alliance`
  ADD PRIMARY KEY (`clan_id`,`alliance_id`),
  ADD KEY `alliance_id` (`alliance_id`);

--
-- Indexes for table `collection_storage`
--
ALTER TABLE `collection_storage`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `db_roulette`
--
ALTER TABLE `db_roulette`
  ADD PRIMARY KEY (`index`);

--
-- Indexes for table `elemental`
--
ALTER TABLE `elemental`
  ADD PRIMARY KEY (`ele_id`);

--
-- Indexes for table `friends`
--
ALTER TABLE `friends`
  ADD PRIMARY KEY (`char_id`,`friend_id`);

--
-- Indexes for table `global_acc_reg_num`
--
ALTER TABLE `global_acc_reg_num`
  ADD PRIMARY KEY (`account_id`,`key`,`index`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `global_acc_reg_str`
--
ALTER TABLE `global_acc_reg_str`
  ADD PRIMARY KEY (`account_id`,`key`,`index`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `guild`
--
ALTER TABLE `guild`
  ADD PRIMARY KEY (`guild_id`,`char_id`),
  ADD UNIQUE KEY `guild_id` (`guild_id`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `guild_alliance`
--
ALTER TABLE `guild_alliance`
  ADD PRIMARY KEY (`guild_id`,`alliance_id`),
  ADD KEY `alliance_id` (`alliance_id`);

--
-- Indexes for table `guild_castle`
--
ALTER TABLE `guild_castle`
  ADD PRIMARY KEY (`castle_id`),
  ADD KEY `guild_id` (`guild_id`);

--
-- Indexes for table `guild_expulsion`
--
ALTER TABLE `guild_expulsion`
  ADD PRIMARY KEY (`guild_id`,`name`);

--
-- Indexes for table `guild_member`
--
ALTER TABLE `guild_member`
  ADD PRIMARY KEY (`guild_id`,`char_id`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `guild_position`
--
ALTER TABLE `guild_position`
  ADD PRIMARY KEY (`guild_id`,`position`);

--
-- Indexes for table `guild_skill`
--
ALTER TABLE `guild_skill`
  ADD PRIMARY KEY (`guild_id`,`id`);

--
-- Indexes for table `guild_storage`
--
ALTER TABLE `guild_storage`
  ADD PRIMARY KEY (`id`),
  ADD KEY `guild_id` (`guild_id`);

--
-- Indexes for table `guild_storage_log`
--
ALTER TABLE `guild_storage_log`
  ADD PRIMARY KEY (`id`),
  ADD KEY `guild_id` (`guild_id`);

--
-- Indexes for table `homunculus`
--
ALTER TABLE `homunculus`
  ADD PRIMARY KEY (`homun_id`);

--
-- Indexes for table `hotkey`
--
ALTER TABLE `hotkey`
  ADD PRIMARY KEY (`char_id`,`hotkey`);

--
-- Indexes for table `interlog`
--
ALTER TABLE `interlog`
  ADD PRIMARY KEY (`id`),
  ADD KEY `time` (`time`);

--
-- Indexes for table `inventory`
--
ALTER TABLE `inventory`
  ADD PRIMARY KEY (`id`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `ipbanlist`
--
ALTER TABLE `ipbanlist`
  ADD PRIMARY KEY (`list`,`btime`);

--
-- Indexes for table `login`
--
ALTER TABLE `login`
  ADD PRIMARY KEY (`account_id`),
  ADD UNIQUE KEY `web_auth_token_key` (`web_auth_token`),
  ADD KEY `name` (`userid`);

--
-- Indexes for table `mail`
--
ALTER TABLE `mail`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `mail_attachments`
--
ALTER TABLE `mail_attachments`
  ADD PRIMARY KEY (`id`,`index`);

--
-- Indexes for table `mapreg`
--
ALTER TABLE `mapreg`
  ADD PRIMARY KEY (`varname`,`index`);

--
-- Indexes for table `market`
--
ALTER TABLE `market`
  ADD PRIMARY KEY (`name`,`nameid`);

--
-- Indexes for table `memo`
--
ALTER TABLE `memo`
  ADD PRIMARY KEY (`memo_id`),
  ADD KEY `char_id` (`char_id`);

--
-- Indexes for table `mercenary`
--
ALTER TABLE `mercenary`
  ADD PRIMARY KEY (`mer_id`);

--
-- Indexes for table `mercenary_owner`
--
ALTER TABLE `mercenary_owner`
  ADD PRIMARY KEY (`char_id`);

--
-- Indexes for table `party`
--
ALTER TABLE `party`
  ADD PRIMARY KEY (`party_id`);

--
-- Indexes for table `party_bookings`
--
ALTER TABLE `party_bookings`
  ADD PRIMARY KEY (`world_name`,`account_id`,`char_id`);

--
-- Indexes for table `pet`
--
ALTER TABLE `pet`
  ADD PRIMARY KEY (`pet_id`);

--
-- Indexes for table `quest`
--
ALTER TABLE `quest`
  ADD PRIMARY KEY (`char_id`,`quest_id`);

--
-- Indexes for table `sales`
--
ALTER TABLE `sales`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `sc_data`
--
ALTER TABLE `sc_data`
  ADD PRIMARY KEY (`char_id`,`type`);

--
-- Indexes for table `skill`
--
ALTER TABLE `skill`
  ADD PRIMARY KEY (`char_id`,`id`);

--
-- Indexes for table `skillcooldown`
--
ALTER TABLE `skillcooldown`
  ADD PRIMARY KEY (`char_id`,`skill`);

--
-- Indexes for table `skill_homunculus`
--
ALTER TABLE `skill_homunculus`
  ADD PRIMARY KEY (`homun_id`,`id`);

--
-- Indexes for table `spiner_cash_player_logs`
--
ALTER TABLE `spiner_cash_player_logs`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `spiner_zeny_player_logs`
--
ALTER TABLE `spiner_zeny_player_logs`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `storage`
--
ALTER TABLE `storage`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `storage_1`
--
ALTER TABLE `storage_1`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `storage_2`
--
ALTER TABLE `storage_2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `vending_items`
--
ALTER TABLE `vending_items`
  ADD PRIMARY KEY (`vending_id`,`index`);

--
-- Indexes for table `vip_storage`
--
ALTER TABLE `vip_storage`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `auction`
--
ALTER TABLE `auction`
  MODIFY `auction_id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `cart_inventory`
--
ALTER TABLE `cart_inventory`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `char`
--
ALTER TABLE `char`
  MODIFY `char_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=150000;

--
-- AUTO_INCREMENT for table `charlog`
--
ALTER TABLE `charlog`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `clan`
--
ALTER TABLE `clan`
  MODIFY `clan_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- AUTO_INCREMENT for table `collection_storage`
--
ALTER TABLE `collection_storage`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `elemental`
--
ALTER TABLE `elemental`
  MODIFY `ele_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `guild`
--
ALTER TABLE `guild`
  MODIFY `guild_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `guild_storage`
--
ALTER TABLE `guild_storage`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `guild_storage_log`
--
ALTER TABLE `guild_storage_log`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `homunculus`
--
ALTER TABLE `homunculus`
  MODIFY `homun_id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `interlog`
--
ALTER TABLE `interlog`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `inventory`
--
ALTER TABLE `inventory`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `login`
--
ALTER TABLE `login`
  MODIFY `account_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2000000;

--
-- AUTO_INCREMENT for table `mail`
--
ALTER TABLE `mail`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `mail_attachments`
--
ALTER TABLE `mail_attachments`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `memo`
--
ALTER TABLE `memo`
  MODIFY `memo_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `mercenary`
--
ALTER TABLE `mercenary`
  MODIFY `mer_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `party`
--
ALTER TABLE `party`
  MODIFY `party_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `pet`
--
ALTER TABLE `pet`
  MODIFY `pet_id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `sales`
--
ALTER TABLE `sales`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `spiner_cash_player_logs`
--
ALTER TABLE `spiner_cash_player_logs`
  MODIFY `id` int(7) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `spiner_zeny_player_logs`
--
ALTER TABLE `spiner_zeny_player_logs`
  MODIFY `id` int(7) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `storage`
--
ALTER TABLE `storage`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `storage_1`
--
ALTER TABLE `storage_1`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `storage_2`
--
ALTER TABLE `storage_2`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `vip_storage`
--
ALTER TABLE `vip_storage`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;



--
-- Table structure for table `guild_emblems`
--

CREATE TABLE IF NOT EXISTS `guild_emblems` (
  `world_name` varchar(32) NOT NULL,
  `guild_id` int(11) unsigned NOT NULL,
  `file_type` varchar(255) NOT NULL,
  `file_data` blob,
  `version` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY (`world_name`, `guild_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `user_configs`
--

CREATE TABLE IF NOT EXISTS `user_configs` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `data` longtext NOT NULL,
  PRIMARY KEY (`world_name`, `account_id`)
) ENGINE=MyISAM;


--
-- Table structure for table `char_configs`
--

CREATE TABLE IF NOT EXISTS `char_configs` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(11) unsigned NOT NULL,
  `data` longtext NOT NULL,
  PRIMARY KEY (`world_name`, `account_id`, `char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `merchant_configs`
--

CREATE TABLE IF NOT EXISTS `merchant_configs` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int(11) unsigned NOT NULL,
  `char_id` INT(11) UNSIGNED NOT NULL,
  `store_type` tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
  `data` longtext NOT NULL,
  PRIMARY KEY (`world_name`, `account_id`, `char_id`, `store_type`)
) ENGINE=MyISAM;

CREATE TABLE `runes` (
  `char_id` int UNSIGNED NOT NULL,
  `rune_id` int UNSIGNED NOT NULL,
  `set_id` int UNSIGNED NOT NULL,
  `selected` tinyint NOT NULL DEFAULT '0',
  `upgrade` smallint UNSIGNED NOT NULL DEFAULT '0',
  `failcount` tinyint UNSIGNED NOT NULL DEFAULT '0',
  UNIQUE KEY `char_id` (`char_id`,`rune_id`,`set_id`)
) ENGINE=MyISAM;

-- Table 'runes_book'
CREATE TABLE `runes_book` (
  `char_id` int UNSIGNED NOT NULL,
  `rune_id` int UNSIGNED NOT NULL,
  `book_id` int UNSIGNED NOT NULL,
  UNIQUE KEY `char_id` (`char_id`,`rune_id`,`book_id`)
) ENGINE=MyISAM;

ALTER TABLE  `char` ADD  `point_pvp` INT NOT NULL DEFAULT  '0',
  ADD  `point_mvp` INT NOT NULL DEFAULT  '0',
  ADD  `point_emp` INT NOT NULL DEFAULT  '0',
  ADD  `point_machine1` INT NOT NULL DEFAULT  '0',
  ADD  `point_machine2` INT NOT NULL DEFAULT  '0',
  ADD  `event_01` INT NOT NULL DEFAULT  '0'
  
  
  