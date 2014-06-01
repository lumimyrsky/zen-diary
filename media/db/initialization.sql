CREATE TABLE IF NOT EXISTS `notes` (`id` INTEGER PRIMARY KEY AUTOINCREMENT, `title` TEXT, `note` TEXT, `hash` TEXT, `encrypted` INTEGER DEFAULT 0, `created` TIMESTAMP DEFAULT CURRENT_TIMESTAMP, `updated` TIMESTAMP DEFAULT CURRENT_TIMESTAMP, `deleted` INTEGER DEFAULT 0);