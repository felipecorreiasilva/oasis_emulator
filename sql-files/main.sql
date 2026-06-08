CREATE DATABASE IF NOT EXISTS `oasis_db` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE `oasis_db`;

CREATE TABLE IF NOT EXISTS `login` (
  `account_id` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` varchar(23) NOT NULL DEFAULT '',
  `user_pass` varchar(32) NOT NULL DEFAULT '',
  `sex` enum('M','F','S') NOT NULL DEFAULT 'M',
  `email` varchar(39) NOT NULL DEFAULT '',
  `group_id` tinyint unsigned NOT NULL DEFAULT 0,
  `state` int unsigned NOT NULL DEFAULT 0,
  `unban_time` int unsigned NOT NULL DEFAULT 0,
  `expiration_time` int unsigned NOT NULL DEFAULT 0,
  `logincount` mediumint unsigned NOT NULL DEFAULT 0,
  `lastlogin` datetime DEFAULT NULL,
  `last_ip` varchar(100) NOT NULL DEFAULT '',
  `birthdate` date DEFAULT NULL,
  `character_slots` tinyint unsigned NOT NULL DEFAULT 0,
  `pincode` varchar(4) NOT NULL DEFAULT '',
  `pincode_change` int unsigned NOT NULL DEFAULT 0,
  `vip_time` int unsigned NOT NULL DEFAULT 0,
  `old_group` tinyint unsigned NOT NULL DEFAULT 0,
  `web_auth_token` varchar(17) DEFAULT NULL,
  `web_auth_token_enabled` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`),
  UNIQUE KEY `userid` (`userid`),
  UNIQUE KEY `web_auth_token_key` (`web_auth_token`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `char` (
  `char_id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `name` varchar(24) NOT NULL DEFAULT '',
  `base_level` smallint unsigned NOT NULL DEFAULT 1,
  `job` smallint unsigned NOT NULL DEFAULT 0,
  `map_id` int unsigned NOT NULL DEFAULT 1,
  `x` float NOT NULL DEFAULT 150.0,
  `y` float NOT NULL DEFAULT 120.0,
  `hair` tinyint unsigned NOT NULL DEFAULT 0,
  `sex` enum('M','F','S') NOT NULL DEFAULT 'M',
  `zeny` int unsigned NOT NULL DEFAULT 0,
  `state` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`),
  KEY `account_id` (`account_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Inserindo a conta especial do servidor (Necessária para a comunicação dos componentes)
INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`)
VALUES (1, 's1', 'p1', 'S', 'athena@athena.com');

-- Inserindo uma conta de testes normal para você testar na Unity depois
INSERT INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `group_id`)
VALUES (2000000, 'oasis', '123', 'M', 'player@oasis.com', 0);