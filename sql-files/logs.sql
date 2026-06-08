USE `oasis_db`;

CREATE TABLE IF NOT EXISTS `account_log` (
  `log_id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `userid` varchar(23) NOT NULL DEFAULT '',
  `ip` varchar(45) NOT NULL DEFAULT '',
  `login_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `event_type` varchar(32) NOT NULL DEFAULT '',
  `message` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`log_id`),
  KEY `account_id` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `char_log` (
  `log_id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `char_name` varchar(24) NOT NULL DEFAULT '',
  `log_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `log_type` varchar(32) NOT NULL DEFAULT '',
  `details` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`log_id`),
  KEY `account_id` (`account_id`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `map_log` (
  `log_id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `map_id` int unsigned NOT NULL DEFAULT 0,
  `x` float NOT NULL DEFAULT 0.0,
  `y` float NOT NULL DEFAULT 0.0,
  `log_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `event_type` varchar(32) NOT NULL DEFAULT '',
  `details` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`log_id`),
  KEY `account_id` (`account_id`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
