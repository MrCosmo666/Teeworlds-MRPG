-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Хост: localhost:3306
-- Время создания: Май 19 2020 г., 15:03
-- Версия сервера: 5.7.30-0ubuntu0.18.04.1
-- Версия PHP: 7.2.24-0ubuntu0.18.04.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- База данных: `worldmmo`
--

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_CRAFT_TABS`
--

CREATE TABLE `ENUM_CRAFT_TABS` (
  `TabID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_CRAFT_TABS`
--

INSERT INTO `ENUM_CRAFT_TABS` (`TabID`, `Name`) VALUES
(1, 'Basic craft'),
(2, 'Artifact craft'),
(3, 'Modules craft'),
(4, 'Buff craft'),
(5, 'Work craft'),
(6, 'Quest craft');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_EFFECTS_LIST`
--

CREATE TABLE `ENUM_EFFECTS_LIST` (
  `ID` int(11) NOT NULL,
  `Name` varchar(16) CHARACTER SET utf8mb4 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_EFFECTS_LIST`
--

INSERT INTO `ENUM_EFFECTS_LIST` (`ID`, `Name`) VALUES
(3, 'Fire'),
(2, 'Poison'),
(1, 'Slowdown');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_EMOTES`
--

CREATE TABLE `ENUM_EMOTES` (
  `ID` int(11) NOT NULL,
  `Emote` varchar(64) NOT NULL DEFAULT 'nope'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_EMOTES`
--

INSERT INTO `ENUM_EMOTES` (`ID`, `Emote`) VALUES
(0, 'Normal Emote'),
(1, 'Pain Emote'),
(2, 'Happy Emote'),
(3, 'Surprise Emote'),
(4, 'Angry Emote'),
(5, 'Blink Emote');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_ITEMS_FUNCTIONAL`
--

CREATE TABLE `ENUM_ITEMS_FUNCTIONAL` (
  `FunctionID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_ITEMS_FUNCTIONAL`
--

INSERT INTO `ENUM_ITEMS_FUNCTIONAL` (`FunctionID`, `Name`) VALUES
(-1, 'Not have function'),
(0, 'Equip hammer(Only equip type)'),
(1, 'Equip gun(Only equip type)'),
(2, 'Equip shotgun(Only equip type)'),
(3, 'Equip grenade(Only equip type)'),
(4, 'Equip rifle(Only equip type)'),
(5, 'Equip miner(Only equip type)'),
(6, 'Equip wings(Only equip type)'),
(7, 'Equip discord(Only equip type)'),
(8, 'Once use item x1'),
(9, 'Several times use item x99'),
(10, 'Settings(Only settings or modules type)'),
(11, 'Plants item'),
(12, 'Mining item');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_ITEMS_TYPES`
--

CREATE TABLE `ENUM_ITEMS_TYPES` (
  `TypeID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_ITEMS_TYPES`
--

INSERT INTO `ENUM_ITEMS_TYPES` (`TypeID`, `Name`) VALUES
(-1, 'Invisible'),
(1, 'Useds'),
(2, 'Crafts'),
(3, 'Modules'),
(4, 'Others'),
(5, 'Settings'),
(6, 'Equipping'),
(7, 'Decorations'),
(8, 'Potions');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_MMO_PROJ`
--

CREATE TABLE `ENUM_MMO_PROJ` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_MMO_PROJ`
--

INSERT INTO `ENUM_MMO_PROJ` (`ID`, `Name`) VALUES
(0, 'Magitech Gun'),
(1, 'Magitech Shotgun'),
(2, 'Magitech Grenade'),
(-1, 'No Proj'),
(3, 'Heavenly Gun'),
(4, 'Heavenly Shotgun'),
(5, 'Heavenly Grenade'),
(6, 'Goblin Gun'),
(7, 'Goblin Shotgun'),
(8, 'Goblin Grenade');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_QUEST_INTERACTIVE`
--

CREATE TABLE `ENUM_QUEST_INTERACTIVE` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_QUEST_INTERACTIVE`
--

INSERT INTO `ENUM_QUEST_INTERACTIVE` (`ID`, `Name`) VALUES
(1, 'Randomly accept or refuse with the item'),
(2, 'Pick up items that NPC will drop.');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_TALK_STYLES`
--

CREATE TABLE `ENUM_TALK_STYLES` (
  `ID` int(11) NOT NULL,
  `Style` varchar(64) NOT NULL DEFAULT 'nope'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_TALK_STYLES`
--

INSERT INTO `ENUM_TALK_STYLES` (`ID`, `Style`) VALUES
(0, 'Basic Talking'),
(1, 'Aggresive Talking'),
(2, 'Happed Talking');

-- --------------------------------------------------------

--
-- Структура таблицы `ENUM_WORLDS`
--

CREATE TABLE `ENUM_WORLDS` (
  `WorldID` int(11) NOT NULL,
  `Name` varchar(32) CHARACTER SET utf8 NOT NULL,
  `RespawnWorld` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `ENUM_WORLDS`
--

INSERT INTO `ENUM_WORLDS` (`WorldID`, `Name`, `RespawnWorld`) VALUES
(0, 'Pier Elfinia', 0),
(1, 'Way to the Elfinia', 0),
(2, 'Elfinia', 2),
(3, 'Elfinia Deep cave', 2),
(4, 'Elfia home room', 2),
(5, 'Elfinia occupation of goblins', 5),
(6, 'Elfinia Abandoned mine', NULL),
(7, 'Diana home room', 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts`
--

CREATE TABLE `tw_accounts` (
  `ID` int(11) NOT NULL,
  `Username` varchar(64) NOT NULL,
  `Password` varchar(64) NOT NULL,
  `RegisterDate` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts`
--

INSERT INTO `tw_accounts` (`ID`, `Username`, `Password`, `RegisterDate`) VALUES
(1, 'kuro', '123123', '2020-05-19 08:24:28');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_data`
--

CREATE TABLE `tw_accounts_data` (
  `ID` int(11) NOT NULL,
  `Nick` varchar(32) NOT NULL,
  `DiscordID` varchar(64) NOT NULL DEFAULT 'null',
  `WorldID` int(11) DEFAULT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Exp` int(11) NOT NULL DEFAULT '0',
  `GuildID` int(11) DEFAULT NULL,
  `GuildDeposit` int(11) NOT NULL DEFAULT '0',
  `GuildRank` int(11) DEFAULT NULL,
  `Upgrade` int(11) NOT NULL DEFAULT '0',
  `DiscordEquip` int(11) NOT NULL DEFAULT '-1',
  `SpreadShotgun` int(11) NOT NULL DEFAULT '3',
  `SpreadGrenade` int(11) NOT NULL DEFAULT '1',
  `SpreadRifle` int(11) NOT NULL DEFAULT '1',
  `Strength` int(11) NOT NULL DEFAULT '0',
  `Dexterity` int(11) NOT NULL DEFAULT '0',
  `CriticalHit` int(11) NOT NULL DEFAULT '0',
  `DirectCriticalHit` int(11) NOT NULL DEFAULT '0',
  `Hardness` int(11) NOT NULL DEFAULT '0',
  `Tenacity` int(11) NOT NULL DEFAULT '0',
  `Lucky` int(11) NOT NULL DEFAULT '0',
  `Piety` int(11) NOT NULL DEFAULT '0',
  `Vampirism` int(11) NOT NULL DEFAULT '0',
  `AmmoRegen` int(11) NOT NULL DEFAULT '0',
  `Ammo` int(11) NOT NULL DEFAULT '0',
  `Efficiency` int(11) NOT NULL DEFAULT '0',
  `Extraction` int(11) NOT NULL DEFAULT '0',
  `LoginDate` varchar(64) NOT NULL DEFAULT 'First login'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_data`
--

INSERT INTO `tw_accounts_data` (`ID`, `Nick`, `DiscordID`, `WorldID`, `Level`, `Exp`, `GuildID`, `GuildDeposit`, `GuildRank`, `Upgrade`, `DiscordEquip`, `SpreadShotgun`, `SpreadGrenade`, `SpreadRifle`, `Strength`, `Dexterity`, `CriticalHit`, `DirectCriticalHit`, `Hardness`, `Tenacity`, `Lucky`, `Piety`, `Vampirism`, `AmmoRegen`, `Ammo`, `Efficiency`, `Extraction`, `LoginDate`) VALUES
(1, 'Kurosio5', 'null', 2, 3, 140, NULL, 0, NULL, 0, -1, 3, 1, 1, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, '2020-05-19 13:20:27');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_inbox`
--

CREATE TABLE `tw_accounts_inbox` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) DEFAULT NULL,
  `Count` int(11) DEFAULT NULL,
  `Enchant` int(11) DEFAULT NULL,
  `MailName` varchar(64) NOT NULL,
  `MailDesc` varchar(64) NOT NULL,
  `OwnerID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_items`
--

CREATE TABLE `tw_accounts_items` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `Count` int(11) NOT NULL,
  `Settings` int(11) NOT NULL,
  `Enchant` int(11) NOT NULL,
  `Durability` int(11) NOT NULL DEFAULT '100',
  `OwnerID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

--
-- Дамп данных таблицы `tw_accounts_items`
--

INSERT INTO `tw_accounts_items` (`ID`, `ItemID`, `Count`, `Settings`, `Enchant`, `Durability`, `OwnerID`) VALUES
(150, 2, 1, 1, 0, 100, 1),
(153, 3, 1, 1, 0, 100, 1),
(154, 1, 146, 0, 0, 100, 1),
(156, 27, 1, 1, 0, 100, 1),
(157, 36, 1, 1, 0, 100, 1),
(159, 21, 10, 0, 0, 100, 1),
(160, 30, 20, 0, 0, 100, 1),
(161, 32, 1, 1, 0, 100, 1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_locations`
--

CREATE TABLE `tw_accounts_locations` (
  `ID` int(11) NOT NULL,
  `OwnerID` int(11) NOT NULL,
  `TeleportID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_locations`
--

INSERT INTO `tw_accounts_locations` (`ID`, `OwnerID`, `TeleportID`) VALUES
(6, 1, 2),
(7, 1, 1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_miner`
--

CREATE TABLE `tw_accounts_miner` (
  `AccountID` int(11) NOT NULL,
  `MnrLevel` int(11) NOT NULL DEFAULT '1',
  `MnrExp` int(11) NOT NULL DEFAULT '0',
  `MnrUpgrade` int(11) NOT NULL DEFAULT '0',
  `MnrCount` int(11) NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_miner`
--

INSERT INTO `tw_accounts_miner` (`AccountID`, `MnrLevel`, `MnrExp`, `MnrUpgrade`, `MnrCount`) VALUES
(1, 1, 0, 0, 1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_plants`
--

CREATE TABLE `tw_accounts_plants` (
  `AccountID` int(11) NOT NULL,
  `PlLevel` int(11) NOT NULL DEFAULT '1',
  `PlExp` int(11) NOT NULL DEFAULT '0',
  `PlCounts` int(11) NOT NULL DEFAULT '1',
  `PlUpgrade` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_plants`
--

INSERT INTO `tw_accounts_plants` (`AccountID`, `PlLevel`, `PlExp`, `PlCounts`, `PlUpgrade`) VALUES
(1, 1, 0, 1, 0);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_quests`
--

CREATE TABLE `tw_accounts_quests` (
  `ID` int(11) NOT NULL,
  `QuestID` int(11) DEFAULT NULL,
  `OwnerID` int(11) NOT NULL,
  `Progress` int(11) NOT NULL DEFAULT '1',
  `Mob1Progress` int(11) NOT NULL DEFAULT '0',
  `Mob2Progress` int(11) NOT NULL DEFAULT '0',
  `Type` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_quests`
--

INSERT INTO `tw_accounts_quests` (`ID`, `QuestID`, `OwnerID`, `Progress`, `Mob1Progress`, `Mob2Progress`, `Type`) VALUES
(159, 5, 1, 2, 0, 0, 1),
(160, 1, 1, 6, 0, 0, 2),
(161, 2, 1, 4, 12, 0, 2),
(162, 3, 1, 1, 16, 0, 1),
(163, 10, 1, 7, 0, 0, 2),
(164, 6, 1, 1, 0, 0, 2),
(165, 7, 1, 1, 0, 0, 1),
(166, 11, 1, 3, 0, 0, 2),
(167, 12, 1, 2, 0, 0, 2),
(168, 13, 1, 3, 0, 0, 2),
(169, 14, 1, 1, 0, 0, 2),
(170, 15, 1, 2, 0, 0, 2),
(171, 8, 1, 1, 0, 0, 1),
(172, 16, 1, 3, 0, 0, 1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_skills`
--

CREATE TABLE `tw_accounts_skills` (
  `ID` int(11) NOT NULL,
  `SkillID` int(11) NOT NULL,
  `OwnerID` int(11) NOT NULL,
  `SkillLevel` int(11) NOT NULL,
  `SelectedEmoticion` int(11) DEFAULT '-1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_aethers`
--

CREATE TABLE `tw_aethers` (
  `ID` int(11) NOT NULL,
  `TeleName` varchar(64) NOT NULL DEFAULT 'Teleport name',
  `WorldID` int(11) NOT NULL,
  `TeleX` int(11) NOT NULL,
  `TeleY` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_aethers`
--

INSERT INTO `tw_aethers` (`ID`, `TeleName`, `WorldID`, `TeleX`, `TeleY`) VALUES
(1, 'Crossroad', 2, 8033, 7089),
(2, 'Pier', 0, 3680, 1150),
(3, 'Guard post', 5, 1536, 4396);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_attributs`
--

CREATE TABLE `tw_attributs` (
  `ID` int(11) NOT NULL,
  `name` varchar(32) NOT NULL,
  `field_name` varchar(32) NOT NULL DEFAULT 'unfield',
  `price` int(11) NOT NULL,
  `at_type` int(11) NOT NULL COMMENT '0.tank1.healer2.dps3.weapon4.hard'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_attributs`
--

INSERT INTO `tw_attributs` (`ID`, `name`, `field_name`, `price`, `at_type`) VALUES
(1, 'Shotgun Spread', 'SpreadShotgun', 100, 3),
(2, 'Grenade Spread', 'SpreadGrenade', 100, 3),
(3, 'Rifle Spread', 'SpreadRifle', 100, 3),
(4, 'Strength', 'Strength', 1, 2),
(5, 'Dexterity', 'Dexterity', 1, 2),
(6, 'Crit Hit', 'CriticalHit', 1, 2),
(7, 'Direct Crit Hit', 'DirectCriticalHit', 1, 2),
(8, 'Hardness', 'Hardness', 1, 0),
(9, 'Lucky', 'Lucky', 1, 0),
(10, 'Piety', 'Piety', 1, 1),
(11, 'Vampirism', 'Vampirism', 1, 1),
(12, 'Ammo Regen', 'AmmoRegen', 1, 3),
(13, 'Ammo', 'Ammo', 30, 3),
(14, 'Efficiency', 'unfield', -1, -1),
(15, 'Extraction', 'unfield', -1, -1),
(16, 'Hammer Power', 'unfield', -1, 4),
(17, 'Gun Power', 'unfield', -1, 4),
(18, 'Shotgun Power', 'unfield', -1, 4),
(19, 'Grenade Power', 'unfield', -1, 4),
(20, 'Rifle Power', 'unfield', -1, 4);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_mobs`
--

CREATE TABLE `tw_bots_mobs` (
  `ID` int(11) NOT NULL,
  `BotID` int(11) NOT NULL DEFAULT '-1',
  `WorldID` int(11) DEFAULT NULL,
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Power` int(11) NOT NULL DEFAULT '10',
  `Spread` int(11) NOT NULL DEFAULT '0',
  `Count` int(11) NOT NULL DEFAULT '1',
  `Respawn` int(11) NOT NULL DEFAULT '1',
  `Boss` tinyint(1) NOT NULL DEFAULT '0',
  `Effect` varchar(16) DEFAULT NULL,
  `it_drop_0` int(11) DEFAULT NULL,
  `it_drop_1` int(11) DEFAULT NULL,
  `it_drop_2` int(11) DEFAULT NULL,
  `it_drop_3` int(11) DEFAULT NULL,
  `it_drop_4` int(11) DEFAULT NULL,
  `it_drop_5` int(11) DEFAULT NULL,
  `it_count` varchar(64) NOT NULL DEFAULT '|0|0|0|0|0|0|',
  `it_random` varchar(64) NOT NULL DEFAULT '|0|0|0|0|0|0|'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_bots_mobs`
--

INSERT INTO `tw_bots_mobs` (`ID`, `BotID`, `WorldID`, `PositionX`, `PositionY`, `Level`, `Power`, `Spread`, `Count`, `Respawn`, `Boss`, `Effect`, `it_drop_0`, `it_drop_1`, `it_drop_2`, `it_drop_3`, `it_drop_4`, `it_drop_5`, `it_count`, `it_random`) VALUES
(1, 9, 1, 4049, 890, 2, 8, 0, 5, 5, 0, 'Slowdown', 22, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|160|0|0|0|0|0|'),
(2, 23, 3, 3057, 2577, 4, 18, 0, 12, 5, 0, 'Slowdown', 29, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|150|0|0|0|0|0|'),
(3, 9, 3, 1890, 1160, 2, 10, 0, 12, 5, 0, 'Slowdown', 22, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|140|0|0|0|0|0|'),
(4, 28, 3, 3057, 2577, 10, 240, 0, 1, 320, 1, 'Slowdown', 29, 22, NULL, NULL, NULL, NULL, '|2|2|0|0|0|0|', '|0|0|0|0|0|0|'),
(5, 22, 5, 1280, 2860, 8, 15, 1, 10, 5, 0, NULL, 30, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|150|0|0|0|0|0|'),
(7, 29, 6, 2825, 2430, 12, 50, 1, 10, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', '|0|0|0|0|0|0|'),
(8, 30, 6, 4840, 2560, 12, 80, 2, 8, 1, 0, 'Fire', 30, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|30|0|0|0|0|0|'),
(9, 31, 6, 1150, 3700, 12, 50, 1, 10, 1, 0, 'Poison', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', '|0|0|0|0|0|0|'),
(10, 22, 6, 1440, 5100, 12, 60, 1, 10, 1, 0, NULL, 30, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|40|0|0|0|0|0|'),
(11, 32, 6, 3960, 4595, 15, 800, 1, 1, 1, 1, 'Fire', 37, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|5|0|0|0|0|0|'),
(12, 34, 3, 1570, 2915, 6, 27, 0, 8, 5, 0, NULL, 35, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', '|120|0|0|0|0|0|');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_npc`
--

CREATE TABLE `tw_bots_npc` (
  `ID` int(11) NOT NULL,
  `BotID` int(11) NOT NULL DEFAULT '-1',
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Function` int(11) NOT NULL DEFAULT '-1',
  `Static` int(11) NOT NULL,
  `Emote` int(11) NOT NULL DEFAULT '0' COMMENT '1.Pain 2.Happy 3.Surprise 4.Angry 5.Blink	',
  `Count` int(11) NOT NULL DEFAULT '1',
  `WorldID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_bots_npc`
--

INSERT INTO `tw_bots_npc` (`ID`, `BotID`, `PositionX`, `PositionY`, `Function`, `Static`, `Emote`, `Count`, `WorldID`) VALUES
(1, 1, 1985, 977, -1, 1, 2, 1, 0),
(2, 2, 1022, 1073, -1, 0, 5, 2, 0),
(3, 3, 2691, 1009, -1, 1, 0, 1, 0),
(4, 4, 5312, 1073, -1, 1, 0, 1, 0),
(5, 11, 10092, 8561, -1, 1, 0, 1, 2),
(6, 12, 5693, 8369, -1, 0, 0, 1, 2),
(7, 13, 6471, 7569, -1, 1, 0, 1, 2),
(8, 14, 6451, 7345, 0, 1, 2, 1, 2),
(9, 15, 9464, 6833, -1, 1, 4, 1, 2),
(10, 16, 264, 1009, -1, 1, 4, 1, 1),
(11, 2, 1234, 689, -1, 1, 1, 1, 0),
(12, 14, 2321, 977, 0, 1, 2, 1, 0),
(13, 19, 5739, 7473, -1, 1, 0, 1, 2),
(14, 20, 3759, 8209, -1, 1, 0, 1, 2),
(15, 21, 6218, 6417, -1, 1, 0, 1, 2),
(16, 27, 4590, 977, -1, 1, 2, 1, 4),
(17, 14, 1468, 4433, 0, 1, 2, 1, 5);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_quest`
--

CREATE TABLE `tw_bots_quest` (
  `ID` int(11) NOT NULL,
  `BotID` int(11) NOT NULL DEFAULT '-1',
  `QuestID` int(11) NOT NULL DEFAULT '-1',
  `WorldID` int(11) DEFAULT NULL,
  `pos_x` int(11) NOT NULL,
  `pos_y` int(11) NOT NULL,
  `it_need_0` int(11) DEFAULT NULL,
  `it_need_1` int(11) DEFAULT NULL,
  `it_reward_0` int(11) DEFAULT NULL,
  `it_reward_1` int(11) DEFAULT NULL,
  `mob_0` int(11) DEFAULT NULL,
  `mob_1` int(11) DEFAULT NULL,
  `it_count` varchar(64) NOT NULL DEFAULT '|0|0|0|0|0|0|',
  `interactive_type` int(11) DEFAULT NULL,
  `interactive_temp` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_bots_quest`
--

INSERT INTO `tw_bots_quest` (`ID`, `BotID`, `QuestID`, `WorldID`, `pos_x`, `pos_y`, `it_need_0`, `it_need_1`, `it_reward_0`, `it_reward_1`, `mob_0`, `mob_1`, `it_count`, `interactive_type`, `interactive_temp`) VALUES
(1, 5, 1, 0, 3925, 1169, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(2, 5, 1, 0, 5599, 1009, 23, NULL, 23, NULL, NULL, NULL, '|12|0|12|0|0|0|', 2, NULL),
(3, 4, 1, 0, 4121, 1137, 23, NULL, NULL, NULL, NULL, NULL, '|4|0|0|0|0|0|', NULL, NULL),
(4, 4, 1, 0, 6489, 1137, 23, NULL, NULL, NULL, NULL, NULL, '|4|0|0|0|0|0|', NULL, NULL),
(5, 4, 1, 0, 2430, 977, 23, NULL, NULL, NULL, NULL, NULL, '|4|0|0|0|0|0|', NULL, NULL),
(6, 5, 1, 0, 6742, 1041, NULL, NULL, 3, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(7, 6, 2, 1, 841, 977, NULL, NULL, 21, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(8, 8, 2, 0, 411, 1009, 21, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', NULL, NULL),
(9, 6, 2, 1, 841, 977, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(10, 6, 2, 1, 2207, 465, NULL, NULL, NULL, NULL, 9, NULL, '|0|0|0|0|12|0|', NULL, NULL),
(11, 17, 3, 1, 525, 1009, NULL, NULL, 15, 14, 9, NULL, '|0|0|3|3|16|0|', NULL, NULL),
(12, 18, 5, 2, 5215, 7409, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(13, 18, 5, 0, 2390, 977, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(14, 2, 5, 0, 2162, 977, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(15, 19, 6, 2, 7915, 8401, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(16, 19, 7, 2, 9859, 8561, 26, NULL, 26, NULL, NULL, NULL, '|1|0|1|0|0|0|', NULL, NULL),
(17, 6, 10, 2, 6615, 8433, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(18, 6, 10, 2, 9953, 8561, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(19, 6, 10, 2, 8574, 7665, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(20, 6, 10, 2, 8123, 7089, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(21, 6, 10, 2, 6815, 7569, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(22, 6, 10, 2, 6364, 7345, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(23, 6, 10, 2, 5021, 7441, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(24, 6, 11, 2, 6834, 7569, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(25, 6, 11, 2, 5722, 6353, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(26, 10, 11, 2, 5325, 6289, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(27, 25, 12, 2, 5421, 8273, NULL, NULL, 27, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(28, 25, 12, 2, 10822, 6737, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(29, 25, 13, 3, 676, 1169, NULL, NULL, NULL, NULL, 9, NULL, '|0|0|0|0|32|0|', NULL, NULL),
(30, 26, 13, 3, 500, 1361, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(31, 17, 4, 1, 1530, 1073, 29, NULL, 25, NULL, NULL, NULL, '|12|0|8|0|0|0|', NULL, NULL),
(32, 26, 13, 2, 3780, 6449, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(33, 27, 8, 4, 4235, 945, NULL, NULL, NULL, NULL, 23, NULL, '|0|0|0|0|50|0|', NULL, NULL),
(34, 27, 8, 2, 7975, 7089, NULL, NULL, 32, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(35, 27, 9, 4, 4243, 945, NULL, NULL, NULL, NULL, 28, NULL, '|0|0|0|0|1|0|', NULL, NULL),
(36, 10, 14, 4, 4391, 977, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(37, 6, 15, 2, 8081, 7889, 20, NULL, NULL, NULL, NULL, NULL, '|10|0|0|0|0|0|', 2, NULL),
(38, 6, 15, 2, 8193, 6065, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(39, 6, 16, 7, 4454, 881, NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(40, 6, 16, 7, 4671, 881, 29, NULL, NULL, NULL, NULL, NULL, '|10|0|0|0|0|0|', NULL, NULL),
(41, 6, 16, 7, 4590, 881, 35, NULL, NULL, NULL, NULL, NULL, '|8|0|0|0|0|0|', NULL, NULL),
(42, 8, 16, 7, 4142, 881, NULL, NULL, NULL, NULL, NULL, NULL, '|8|0|0|0|0|0|', NULL, NULL),
(43, 6, 16, 7, 4590, 881, NULL, NULL, NULL, NULL, NULL, NULL, '|8|0|0|0|0|0|', NULL, NULL),
(44, 25, 17, 2, 8128, 7889, NULL, NULL, NULL, NULL, NULL, NULL, '|8|0|0|0|0|0|', NULL, NULL),
(45, 25, 17, 3, 762, 1169, NULL, NULL, NULL, NULL, 23, NULL, '|8|0|0|0|32|0|', NULL, NULL),
(46, 25, 17, 2, 6316, 7569, NULL, NULL, 9, NULL, NULL, NULL, '|0|0|50|0|0|0|', NULL, NULL);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_world`
--

CREATE TABLE `tw_bots_world` (
  `ID` int(11) NOT NULL,
  `BotName` varchar(32) NOT NULL DEFAULT 'Bot name',
  `SkinName` varchar(128) NOT NULL DEFAULT '''bear standard standard standard standard standard''' COMMENT 'body marking deco hands feet eyes',
  `SkinColor` varchar(128) NOT NULL DEFAULT '''0 0 0 0 0 0''' COMMENT 'body marking deco hands feet eyes	',
  `SlotHammer` int(11) DEFAULT NULL,
  `SlotGun` int(11) DEFAULT NULL,
  `SlotShotgun` int(11) DEFAULT NULL,
  `SlotGrenade` int(11) DEFAULT NULL,
  `SlotRifle` int(11) DEFAULT NULL,
  `SlotWings` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_bots_world`
--

INSERT INTO `tw_bots_world` (`ID`, `BotName`, `SkinName`, `SkinColor`, `SlotHammer`, `SlotGun`, `SlotShotgun`, `SlotGrenade`, `SlotRifle`, `SlotWings`) VALUES
(1, 'Captain', 'flokes  unipento standard standard sunglasses', '16711922 -970932064 0 1082745 1114231 1507328', 10010, NULL, NULL, NULL, NULL, NULL),
(2, 'Sailor', 'standard bear hair standard standard standard', '1082745 -15634776 65408 1082745 1147174 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(3, 'Carpenter', 'standard belly1 twinbopp standard standard standard', '13583205 -15661127 13514071 1048622 1117573 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(4, 'Worker', 'standard cammostripes  standard standard standard', '1821867 -14840320 65408 750848 1944919 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(5, 'Mr. Worker', 'trela cammostripes  standard standard standard', '1662583 -14840320 65408 750848 1944919 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(6, 'Diana', 'raccoon coonfluff  standard standard standard', '965254 -15235151 65408 1769643 1305243 1085234', NULL, NULL, NULL, NULL, NULL, NULL),
(7, 'Mr. Guard', 'flokes flokes_gray  standard standard standard', '1638493 -15138752 65408 1769472 1638422 255', NULL, NULL, NULL, NULL, NULL, NULL),
(8, 'Brother', 'bear panda1 hair standard standard standard', '9834574 -6411543 65408 1769630 1835070 41215', NULL, NULL, NULL, NULL, NULL, NULL),
(9, 'Green slime', 'bear downdony hair duotone standard standard', '3981441 -29333592 5313052 14500779 5468601 6321790', NULL, NULL, NULL, NULL, NULL, NULL),
(10, 'Apostle Elfia', 'kitty bear  duotone standard standard', '9568256 -1695744161 12254015 3972544 15204215 7470034', 10024, NULL, NULL, NULL, NULL, 10017),
(11, 'Craftsman', 'bear tiger1 twinpen duotone standard colorable', '12770027 828507979 11162385 6849346 44458 8581506', NULL, NULL, NULL, NULL, NULL, NULL),
(12, 'Auctionist', 'kitty saddo twinbopp duotone standard colorable', '12201075 -855657567 2205432 3349551 6943484 13531062', NULL, NULL, NULL, NULL, NULL, NULL),
(13, 'Teacher', 'mouse purelove unibop duotone standard moustache', '8467692 -1394954365 12408709 11534535 3010026 5627093', NULL, NULL, NULL, NULL, NULL, NULL),
(14, 'Nurse', 'kitty toptri twinpen duotone standard colorable', '15672283 -2012161487 3858792 7940502 3052250 1873584', NULL, NULL, NULL, NULL, NULL, NULL),
(15, 'Gunsmith Eric', 'kitty twinbelly hair duotone standard standard', '9778522 -208509303 9813768 10102581 4298037 6156206', NULL, NULL, NULL, NULL, NULL, NULL),
(16, 'Mr. Sentry', 'kitty cammo2 hair2 duotone standard colorable', '10598472 -580149911 15528239 3610230 8475852 70818', NULL, NULL, NULL, NULL, NULL, NULL),
(17, 'Sentry', 'kitty cammo2 hair2 duotone standard colorable', '10568082 -580149911 15528239 3610230 8475852 70818', NULL, NULL, NULL, NULL, NULL, NULL),
(18, 'Daughter Sailor', 'kitty whisker hair duotone standard standard', '3981441 -18874784 5313052 14500779 8614329 6321790', NULL, NULL, NULL, NULL, NULL, NULL),
(19, 'Miner', 'standard toptri hair duotone standard colorable', '16718165 -2012161487 3858792 7940502 3052250 1873584', NULL, NULL, NULL, NULL, NULL, NULL),
(20, 'Customs officer', 'kitty cammo1 hair duotone standard standardreal', '6083870 -2125570185 6324018 7470633 16541982 2000684', NULL, NULL, NULL, NULL, NULL, NULL),
(21, 'Seller artifact', 'flokes belly1  duotone standard colorable', '4325407 -1400836299 1777019 3269995 16520019 11906960', NULL, NULL, NULL, NULL, NULL, NULL),
(22, 'Goblin', 'bear hipbel  duotone standard standard', '6301461 -1695632164 12254015 3972544 9710385 7470034', NULL, NULL, NULL, NULL, NULL, NULL),
(23, 'Blue slime', 'bear bear  duotone standard standard', '9610633 -1695632164 12254015 3972544 9729630 7470034', NULL, NULL, NULL, NULL, NULL, NULL),
(25, 'Adventurer Koto', 'bear belly1 hair2 duotone standard standardreal', '2307609 -167295050 13142259 5885245 9371648 7807949', NULL, NULL, NULL, NULL, NULL, NULL),
(26, 'Messenger', 'puar duodonny  duotone standard negative', '7018125 -2047512102 9555542 2060938 1379442 14886533', NULL, NULL, NULL, NULL, NULL, NULL),
(27, 'Salantra', 'kitty bear  duotone standard colorable', '15557993 -346596366 16130358 701247 11272539 4707488', NULL, NULL, NULL, NULL, NULL, NULL),
(28, 'Pink Slime', 'bear bear hair2 duotone standard standard', '14212759 -2065039105 16307984 12212874 16062570 8064920', NULL, NULL, NULL, NULL, NULL, NULL),
(29, 'Dead miner', 'kitty stripe hair duotone standard colorable', '15995954 121878199 10365700 1972392 8301545 1240658', NULL, NULL, NULL, NULL, NULL, NULL),
(30, 'Hobgoblin', 'bear cammo2 twinmello duotone standard negative', '4331554 -1830111313 6962262 8759142 9574495 10700700', NULL, NULL, NULL, NULL, NULL, NULL),
(31, 'Orc', 'bear downdony  duotone standard standard', '4136201 -406977393 5258828 12493916 2359072 5233408', NULL, NULL, NULL, NULL, NULL, NULL),
(32, 'Leader Orcs', 'monkey twinbelly  duotone standard standardreal', '16495882 -1326302539 9549353 8668187 2063263 1406656', 10019, 10020, 10021, 10022, 10023, 10016),
(33, 'Twins', 'bear bear  duotone standard standard', '9683673 672388260 12653934 1134912 4360420 8598003', NULL, NULL, NULL, NULL, NULL, NULL),
(34, 'Kappa', 'standard sidemarks unibop duotone standard colorable', '4821534 821909703 1852219 1132310 5264192 15588285', NULL, NULL, NULL, NULL, NULL, NULL);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_craft_list`
--

CREATE TABLE `tw_craft_list` (
  `ID` int(11) NOT NULL,
  `GetItem` int(11) DEFAULT NULL,
  `GetItemCount` int(11) NOT NULL,
  `ItemNeed0` int(11) DEFAULT NULL,
  `ItemNeed1` int(11) DEFAULT NULL,
  `ItemNeed2` int(11) DEFAULT NULL,
  `ItemNeedCount` varchar(32) NOT NULL DEFAULT '0 0 0',
  `Price` int(11) NOT NULL DEFAULT '100',
  `Type` int(11) DEFAULT NULL,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_craft_list`
--

INSERT INTO `tw_craft_list` (`ID`, `GetItem`, `GetItemCount`, `ItemNeed0`, `ItemNeed1`, `ItemNeed2`, `ItemNeedCount`, `Price`, `Type`, `WorldID`) VALUES
(1, 15, 3, 22, 29, 18, '3 3 8', 50, 4, 2),
(2, 26, 1, 30, NULL, NULL, '16 0 0', 150, 5, 2),
(3, 33, 1, 37, 31, NULL, '3 30 0', 240, 3, 2),
(4, 34, 1, 37, 31, NULL, '5 50 0', 240, 3, 2),
(5, 10019, 1, 37, 30, 31, '16 42 24', 320, 5, 2),
(6, 10020, 1, 37, 30, 31, '12 32 18', 320, 5, 2),
(7, 10021, 1, 37, 30, 31, '12 32 18', 320, 5, 2),
(8, 10022, 1, 37, 30, 31, '12 32 18', 320, 5, 2),
(9, 10023, 1, 37, 30, 31, '12 32 18', 320, 5, 2),
(10, 10016, 1, 37, NULL, NULL, '40 0 0', 640, 5, 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_dungeons`
--

CREATE TABLE `tw_dungeons` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL DEFAULT 'Unknown',
  `Level` int(11) NOT NULL DEFAULT '1',
  `DoorX` int(11) NOT NULL DEFAULT '0',
  `DoorY` int(11) NOT NULL DEFAULT '0',
  `OpenQuestID` int(11) NOT NULL DEFAULT '-1',
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_dungeons`
--

INSERT INTO `tw_dungeons` (`ID`, `Name`, `Level`, `DoorX`, `DoorY`, `OpenQuestID`, `WorldID`) VALUES
(1, 'Abandoned mine', 10, 1105, 1521, -1, 6);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_dungeons_door`
--

CREATE TABLE `tw_dungeons_door` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL DEFAULT 'Write here name dungeon',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `BotID` int(11) NOT NULL,
  `DungeonID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `tw_dungeons_door`
--

INSERT INTO `tw_dungeons_door` (`ID`, `Name`, `PosX`, `PosY`, `BotID`, `DungeonID`) VALUES
(1, 'Write here name dungeon', 4302, 1940, 29, 1),
(2, 'Write here name dungeon', 1808, 3600, 30, 1),
(3, 'Write here name dungeon', 750, 4850, 31, 1),
(4, 'Write here name dungeon', 2255, 4850, 28, 1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_dungeons_records`
--

CREATE TABLE `tw_dungeons_records` (
  `ID` int(11) NOT NULL,
  `OwnerID` int(11) NOT NULL,
  `DungeonID` int(11) NOT NULL,
  `Seconds` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds`
--

CREATE TABLE `tw_guilds` (
  `ID` int(11) NOT NULL,
  `GuildName` varchar(32) NOT NULL DEFAULT 'Member name',
  `OwnerID` int(11) DEFAULT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Experience` int(11) NOT NULL DEFAULT '0',
  `Bank` int(11) NOT NULL DEFAULT '5000',
  `Score` int(11) NOT NULL DEFAULT '0',
  `AvailableSlots` int(11) NOT NULL DEFAULT '2',
  `ChairExperience` int(11) NOT NULL DEFAULT '1',
  `ChairMoney` int(11) NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_decorations`
--

CREATE TABLE `tw_guilds_decorations` (
  `ID` int(11) NOT NULL,
  `X` int(11) NOT NULL,
  `Y` int(11) NOT NULL,
  `HouseID` int(11) NOT NULL,
  `DecoID` int(11) NOT NULL,
  `WorldID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_history`
--

CREATE TABLE `tw_guilds_history` (
  `ID` int(11) NOT NULL,
  `GuildID` int(11) NOT NULL,
  `Text` varchar(64) NOT NULL,
  `Time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_houses`
--

CREATE TABLE `tw_guilds_houses` (
  `ID` int(11) NOT NULL,
  `OwnerMID` int(11) DEFAULT NULL,
  `WorldID` int(11) NOT NULL,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `DoorX` int(11) NOT NULL,
  `DoorY` int(11) NOT NULL,
  `TextX` int(11) NOT NULL,
  `TextY` int(11) NOT NULL,
  `Price` int(11) NOT NULL DEFAULT '50000'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_guilds_houses`
--

INSERT INTO `tw_guilds_houses` (`ID`, `OwnerMID`, `WorldID`, `PosX`, `PosY`, `DoorX`, `DoorY`, `TextX`, `TextY`, `Price`) VALUES
(1, NULL, 2, 4250, 6352, 4496, 6461, 4206, 6224, 50000);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_invites`
--

CREATE TABLE `tw_guilds_invites` (
  `ID` int(11) NOT NULL,
  `GuildID` int(11) NOT NULL,
  `OwnerID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_ranks`
--

CREATE TABLE `tw_guilds_ranks` (
  `ID` int(11) NOT NULL,
  `Access` int(11) NOT NULL DEFAULT '3',
  `Name` varchar(32) NOT NULL DEFAULT 'Rank name',
  `GuildID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_houses`
--

CREATE TABLE `tw_houses` (
  `ID` int(11) NOT NULL,
  `OwnerID` int(11) NOT NULL DEFAULT '0',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `DoorX` int(11) NOT NULL,
  `DoorY` int(11) NOT NULL,
  `Class` varchar(32) NOT NULL DEFAULT 'Class name',
  `Price` int(11) NOT NULL,
  `HouseBank` int(11) NOT NULL,
  `PlantID` int(11) NOT NULL,
  `PlantX` int(11) NOT NULL,
  `PlantY` int(11) NOT NULL,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_houses`
--

INSERT INTO `tw_houses` (`ID`, `OwnerID`, `PosX`, `PosY`, `DoorX`, `DoorY`, `Class`, `Price`, `HouseBank`, `PlantID`, `PlantX`, `PlantY`, `WorldID`) VALUES
(1, 447, 8995, 7672, 8752, 7740, 'Elven class', 92000, 313956, 20, 9456, 7766, 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_houses_decorations`
--

CREATE TABLE `tw_houses_decorations` (
  `ID` int(11) NOT NULL,
  `X` int(11) NOT NULL,
  `Y` int(11) NOT NULL,
  `HouseID` int(11) NOT NULL,
  `DecoID` int(11) NOT NULL,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_items_list`
--

CREATE TABLE `tw_items_list` (
  `ItemID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL DEFAULT 'Item name',
  `Description` varchar(64) NOT NULL DEFAULT 'Item desc',
  `Icon` varchar(16) NOT NULL DEFAULT 'some1',
  `Type` int(11) DEFAULT '-1',
  `Function` int(11) DEFAULT '-1',
  `Desynthesis` int(11) NOT NULL DEFAULT '100',
  `Selling` int(11) NOT NULL DEFAULT '100',
  `Stat_0` int(11) DEFAULT NULL,
  `Stat_1` int(11) DEFAULT NULL,
  `StatCount_0` int(11) NOT NULL DEFAULT '0',
  `StatCount_1` int(11) NOT NULL,
  `EnchantMax` int(11) NOT NULL DEFAULT '0',
  `ProjectileID` int(11) NOT NULL DEFAULT '-1' COMMENT 'only for weapons'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_items_list`
--

INSERT INTO `tw_items_list` (`ItemID`, `Name`, `Description`, `Icon`, `Type`, `Function`, `Desynthesis`, `Selling`, `Stat_0`, `Stat_1`, `StatCount_0`, `StatCount_1`, `EnchantMax`, `ProjectileID`) VALUES
(1, 'Gold', 'Major currency', 'gold', -1, -1, 0, 0, 16, NULL, 0, 0, 0, -1),
(2, 'Hammer', 'A normal hammer', 'hammer', 6, 0, 0, 0, 16, 6, 10, 3, 5, -1),
(3, 'Gun', 'Conventional weapon', 'gun', 6, 1, 0, 10, 17, NULL, 10, 0, 5, -1),
(4, 'Shotgun', 'Conventional weapon', 'shotgun', 6, 2, 0, 10, 18, NULL, 5, 0, 5, -1),
(5, 'Grenade', 'Conventional weapon', 'grenade', 6, 3, 0, 10, 19, NULL, 10, 0, 5, -1),
(6, 'Rifle', 'Conventional weapon', 'rifle', 6, 4, 200, 10, 20, NULL, 10, 0, 5, -1),
(7, 'Material', 'Required to improve weapons', 'material', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(8, 'Ticket Guild', 'Required to create guilds', 'ticket', 4, -1, 500, 10, NULL, NULL, 0, 0, 0, -1),
(9, 'Skill Point', 'Skill point', 'skill_point', -1, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(10, 'Decoration Armor', 'Decoration for house!', 'deco_house', 7, -1, 1500, 10, NULL, NULL, 0, 0, 0, -1),
(11, 'Decoration Hearth Elite', 'Decoration for house!', 'deco_house', 7, -1, 3000, 10, NULL, NULL, 0, 0, 0, -1),
(12, 'Decoration Ninja Elite', 'Decoration for house!', 'deco_house', 7, -1, 3000, 10, NULL, NULL, 0, 0, 0, -1),
(13, 'Decoration Hearth', 'Decoration for house!', 'deco_house', 7, -1, 1500, 10, NULL, NULL, 0, 0, 0, -1),
(14, 'Potion mana regen', 'Regenerate +5%, 15sec every sec.\n', 'potion_b', 8, 8, 30, 10, NULL, NULL, 0, 0, 0, -1),
(15, 'Potion health regen', 'Regenerate +3% health, 15sec every sec.', 'potion_r', 8, 8, 30, 10, NULL, NULL, 0, 0, 0, -1),
(16, 'Capsule survival experience', 'You got 10-50 experience survival', 'potion_g', 1, 9, 0, 10, NULL, NULL, 0, 0, 0, -1),
(17, 'Little bag of gold', 'You got 10-50 gold', 'pouch', 1, 9, 0, 10, NULL, NULL, 0, 0, 0, -1),
(18, 'Mirt', 'Information added later.', 'some1', 4, 11, 5, 10, NULL, NULL, 0, 0, 0, -1),
(20, 'Potato', 'Material need for craft!', 'potato', 4, 11, 8, 10, NULL, NULL, 0, 0, 0, -1),
(21, 'Notebook', 'In it, something is written', 'paper', 4, -1, 1, 10, NULL, NULL, 0, 0, 0, -1),
(22, 'Glue', 'I wonder what it\'s for?', 'some4', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(23, 'Board', 'Plain Board', 'board', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(24, 'Mushroom', 'Material need for craft!', 'mushroom', 4, 11, 3, 10, NULL, NULL, 0, 0, 0, -1),
(25, 'Potion resurrection', 'Resuscitates in the zone where you died!', 'potion_p', 8, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(26, 'Goblin Pickaxe', 'It happens sometimes', 'pickaxe', 6, 5, 300, 10, 14, 15, 2, 3, 8, -1),
(27, 'Young fighter\'s ring', 'It happens sometimes', 'ring', 3, 10, 0, 10, 8, NULL, 125, 0, 3, -1),
(28, 'Small ammo bag', 'Adds a small amount of ammunition', 'pouch', 3, 10, 0, 10, 13, NULL, 3, 0, 5, -1),
(29, 'Gel', 'I wonder what it\'s for?', 'some4', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(30, 'Goblin Ingot', 'Information added later.', 'ignot_g', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(31, 'Copper Ingot', 'Information added later.', 'ignot_o', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(32, 'Salantra Blessing', 'It happens sometimes', 'ticket', 3, 10, 0, 10, 8, NULL, 250, 0, 3, -1),
(33, 'Explosive module for gun', 'It happens sometimes', 'module', 3, 10, 0, 10, 17, NULL, 5, 0, 3, -1),
(34, 'Explosive module for shotgun', 'It happens sometimes', 'module', 3, 10, 0, 10, 18, NULL, 5, 0, 3, -1),
(35, 'Kappa meat', 'Information added later.', 'meat', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(36, 'Ring of Arvida', 'It happens sometimes', 'ring_light', 3, 10, 0, 10, 11, 10, 10, 10, 3, -1),
(37, 'Relic of the Orc Lord', 'Information added later.', 'lucky_r', 4, -1, 0, 10, NULL, NULL, 0, 0, 0, -1),
(10000, 'Heavenly hammer', 'Reinforced kick', 'h_heaven', 6, 0, 0, 10, 16, NULL, 1, 0, 5, -1),
(10001, 'Heavenly gun', 'It look doesn\'t bad', 'g_heaven', 6, 1, 0, 10, 17, NULL, 10, 0, 5, 3),
(10002, 'Heavenly shotgun', 'It look doesn\'t bad', 's_heaven', 6, 2, 0, 10, 18, NULL, 10, 0, 5, 4),
(10003, 'Heavenly grenade', 'It look doesn\'t bad', 'gr_heaven', 6, 3, 0, 10, 19, NULL, 10, 0, 5, 5),
(10004, 'Heavenly rifle', 'It look doesn\'t bad', 'r_heaven', 6, 4, 0, 10, 20, NULL, 10, 0, 15, -1),
(10005, 'Shadow wings', 'Dark history', 'wings', 6, 6, 0, 10, 10, NULL, 338, 0, 5, -1),
(10006, 'Neptune wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 11, NULL, 180, 0, 5, -1),
(10007, 'Angel wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 510, 0, 5, -1),
(10008, 'Heavenly wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 720, 0, 5, -1),
(10009, 'Rainbow wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 720, 0, 5, -1),
(10010, 'Magitech hammer', 'Reinforced kick', 'h_magitech', 6, 0, 0, 10, 16, NULL, 1, 5, 5, -1),
(10011, 'Magitech gun', 'It look doesn\'t bad', 'g_magitech', 6, 1, 0, 10, 17, NULL, 10, 0, 5, 0),
(10012, 'Magitech shotgun', 'It look doesn\'t bad', 's_magitech', 6, 2, 0, 10, 18, NULL, 10, 0, 5, 1),
(10013, 'Magitech grenade', 'It look doesn\'t bad', 'gr_magitech', 6, 3, 0, 10, 19, NULL, 10, 0, 5, 2),
(10014, 'Magitech rifle', 'It look doesn\'t bad', 'r_magitech', 6, 4, 0, 10, 20, NULL, 10, 0, 15, -1),
(10015, 'Stars wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 720, 0, 5, -1),
(10016, 'Bat wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 270, 0, 5, -1),
(10017, 'Little eagle wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 720, 0, 5, -1),
(10018, 'Necromante wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 720, 0, 5, -1),
(10019, 'Goblin hammer', 'Reinforced kick', 'h_goblin', 6, 0, 0, 10, 16, NULL, 30, 0, 5, -1),
(10020, 'Goblin gun', 'It look doesn\'t bad', 'g_goblin', 6, 1, 0, 10, 17, NULL, 30, 0, 5, 6),
(10021, 'Goblin shotgun', 'It look doesn\'t bad', 's_goblin', 6, 2, 0, 10, 18, NULL, 20, 0, 5, 7),
(10022, 'Goblin grenade', 'It look doesn\'t bad', 'gr_goblin', 6, 3, 0, 10, 19, NULL, 30, 0, 5, 8),
(10023, 'Goblin rifle', 'It look doesn\'t bad', 'r_goblin', 6, 4, 0, 10, 20, NULL, 30, 0, 5, -1),
(10024, 'Scythe', 'Reinforced kick', 'h_scythe', 6, 0, 300, 10, 16, NULL, 1, 0, 10, -1),
(15000, 'Theme Couple', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 8, NULL, 147, 0, 18, -1),
(15001, 'Theme Final Fantasy', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 9, NULL, 181, 0, 15, -1),
(15002, 'Theme Aion', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 6, NULL, 165, 0, 15, -1),
(15003, 'Theme Dragon Nest', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 8, NULL, 180, 0, 12, -1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_logicworld`
--

CREATE TABLE `tw_logicworld` (
  `ID` int(11) NOT NULL,
  `MobID` int(11) NOT NULL,
  `Mode` int(11) NOT NULL DEFAULT '0' COMMENT '(1,3) 0 up 1 left',
  `ParseInt` int(11) NOT NULL COMMENT '(2) health (3)itemid key',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `WorldID` int(11) NOT NULL,
  `Comment` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_mailshop`
--

CREATE TABLE `tw_mailshop` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `Count` int(11) NOT NULL,
  `NeedItem` int(11) NOT NULL DEFAULT '1',
  `Price` int(11) NOT NULL,
  `OwnerID` int(11) NOT NULL DEFAULT '0',
  `Enchant` int(11) NOT NULL DEFAULT '0',
  `StorageID` int(11) DEFAULT NULL,
  `Time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_mailshop`
--

INSERT INTO `tw_mailshop` (`ID`, `ItemID`, `Count`, `NeedItem`, `Price`, `OwnerID`, `Enchant`, `StorageID`, `Time`) VALUES
(1, 3, 1, 1, 80, 0, 0, 1, '2020-05-10 18:36:16'),
(2, 4, 1, 1, 240, 0, 0, 1, '2020-05-10 18:36:16'),
(3, 5, 1, 1, 250, 0, 0, 1, '2020-05-10 18:36:16'),
(4, 6, 1, 1, 300, 0, 0, 1, '2020-05-10 18:36:16'),
(5, 28, 1, 1, 920, 0, 0, 2, '2020-05-13 21:19:28'),
(6, 36, 1, 1, 420, 0, 0, 2, '2020-05-13 21:19:28'),
(7, 8, 1, 1, 3800, 0, 0, 2, '2020-05-13 21:19:28');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_position_miner`
--

CREATE TABLE `tw_position_miner` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Health` int(11) NOT NULL DEFAULT '100',
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Distance` int(11) NOT NULL DEFAULT '300',
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_position_miner`
--

INSERT INTO `tw_position_miner` (`ID`, `ItemID`, `Level`, `Health`, `PositionX`, `PositionY`, `Distance`, `WorldID`) VALUES
(1, 7, 1, 150, 5742, 686, 300, 0),
(2, 31, 3, 240, 1485, 4100, 300, 5),
(3, 31, 3, 240, 3525, 4100, 2750, 5);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_position_plant`
--

CREATE TABLE `tw_position_plant` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Distance` int(11) NOT NULL DEFAULT '300',
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_position_plant`
--

INSERT INTO `tw_position_plant` (`ID`, `ItemID`, `Level`, `PositionX`, `PositionY`, `Distance`, `WorldID`) VALUES
(1, 18, 1, 320, 3585, 250, 5);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_quests_list`
--

CREATE TABLE `tw_quests_list` (
  `ID` int(11) NOT NULL,
  `Name` varchar(24) NOT NULL DEFAULT 'Quest name',
  `Money` int(11) NOT NULL,
  `Exp` int(11) NOT NULL,
  `StoryLine` varchar(24) NOT NULL DEFAULT 'Hero'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_quests_list`
--

INSERT INTO `tw_quests_list` (`ID`, `Name`, `Money`, `Exp`, `StoryLine`) VALUES
(1, 'Help for workers', 15, 20, 'Zero: Arrival'),
(2, 'Helping a girl', 15, 20, 'Zero: Arrival'),
(3, 'The fight with slime!', 25, 20, 'Trader Sentry'),
(4, 'Gel? Why do you need it?', 25, 20, 'Trader Sentry'),
(5, 'Search for a daughter', 40, 30, 'Sailor'),
(6, 'What is ore mining?', 50, 30, 'Mining'),
(7, 'My first pickaxe', 100, 30, 'Mining'),
(8, 'Help destruction of dirt', 80, 50, 'Deputy'),
(9, 'Raid on the dirt', 80, 50, 'Deputy'),
(10, 'Acquaintance', 15, 20, 'One: Apostle Elfia'),
(11, 'Apostle', 20, 20, 'One: Apostle Elfia'),
(12, 'Adventurer?', 50, 40, 'One: Apostle Elfia'),
(13, 'Something is starting', 10, 20, 'One: Apostle Elfia'),
(14, 'History from Apostle', 10, 20, 'One: Apostle Elfia'),
(15, 'Diana has a problem', 20, 30, 'One: Apostle Elfia'),
(16, 'Mmm delicious', 50, 30, 'One: Apostle Elfia'),
(17, 'Time learn something', 60, 50, 'One: Apostle Elfia');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_skills_list`
--

CREATE TABLE `tw_skills_list` (
  `ID` int(11) NOT NULL,
  `SkillName` varchar(64) NOT NULL,
  `SkillDesc` varchar(64) NOT NULL,
  `BonusInfo` varchar(64) NOT NULL DEFAULT '''name''',
  `BonusCount` int(11) NOT NULL DEFAULT '1',
  `ManaProcent` int(11) NOT NULL DEFAULT '10',
  `Price` int(11) NOT NULL,
  `MaxLevel` int(11) NOT NULL,
  `Passive` tinyint(1) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_skills_list`
--

INSERT INTO `tw_skills_list` (`ID`, `SkillName`, `SkillDesc`, `BonusInfo`, `BonusCount`, `ManaProcent`, `Price`, `MaxLevel`, `Passive`) VALUES
(1, 'Health turret', 'Creates turret a recovery health ', 'life span', 3, 25, 24, 8, 0),
(2, 'Sleepy Gravity', 'Magnet mobs to itself', 'radius', 20, 25, 28, 10, 0),
(3, 'Craft Discount', 'Will give discount on the price of craft items', '% discount gold for craft item', 1, 0, 28, 50, 1),
(4, 'Proficiency with weapons', 'You can perform an automatic fire', 'can perform an auto fire with all types of weapons', 1, 0, 120, 1, 1),
(5, 'Blessing of God of war', 'The blessing restores ammo', '% recovers ammo within a radius of 800', 25, 50, 28, 4, 0);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_storages`
--

CREATE TABLE `tw_storages` (
  `ID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL DEFAULT '''Bussines name''',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_storages`
--

INSERT INTO `tw_storages` (`ID`, `Name`, `PosX`, `PosY`, `WorldID`) VALUES
(1, 'Weapons for young adventurers', 9417, 6817, 2),
(2, 'Elfinia Artifacts', 6256, 6417, 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_talk_other_npc`
--

CREATE TABLE `tw_talk_other_npc` (
  `ID` int(11) NOT NULL,
  `MobID` int(11) NOT NULL,
  `PlayerTalked` tinyint(1) NOT NULL DEFAULT '0',
  `Style` int(11) NOT NULL DEFAULT '-1',
  `TalkingEmote` int(11) NOT NULL DEFAULT '-1',
  `GivingQuest` int(11) DEFAULT NULL,
  `TalkText` varchar(512) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `tw_talk_other_npc`
--

INSERT INTO `tw_talk_other_npc` (`ID`, `MobID`, `PlayerTalked`, `Style`, `TalkingEmote`, `GivingQuest`, `TalkText`) VALUES
(1, 1, 0, 2, 2, NULL, '**He said surprised**: What a wonderful weather today!'),
(2, 1, 1, 0, 2, NULL, 'Yes, I think so too.'),
(3, 1, 0, 0, 0, NULL, 'Well, how do you like on our ship [Player]? Captures the ocean, i think I\'ll stay here a little longer, it\'s wonderful here.'),
(4, 1, 1, 0, 0, NULL, 'I also really like it. Okay, I\'ll go, I wish you a good rest [Talked]!'),
(5, 1, 0, 2, 2, NULL, 'And I wish you good luck too [Player]!'),
(6, 2, 0, 1, 1, NULL, '**He looks very tired**: Very hot, impossible to work.'),
(7, 3, 1, 0, 5, NULL, 'Hello, I arrived from a small village beyond the mountains and the ocean, my name is [Player], I have a desire to become an adventurer. But I do not know about this place, could you help me?'),
(8, 3, 0, 0, 0, NULL, 'Hello [Player], Yes of course I will help you, explain where you need to go, but you help my workers, they do not cope at all.'),
(9, 3, 1, 2, 2, NULL, 'What you need help with, I\'m good at dealing with predators without meat you will not leave! *laughter*'),
(10, 3, 0, 0, 5, NULL, 'Oh, you\'re a Joker, no thanks go a little further guys will explain everything to you!'),
(11, 3, 1, 0, 0, 1, 'OK, thanks!'),
(12, 9, 0, 0, 5, NULL, 'Hi [Player], buy something!'),
(13, 9, 1, 0, 5, NULL, 'Hmm weapons. Where do you get it from?'),
(14, 9, 0, 0, 0, NULL, 'My father is a craftsman, so he made it. The quality is good, I guarantee it.'),
(15, 9, 1, 0, 0, NULL, 'Yes, of course I will see what you have.'),
(16, 10, 1, 0, 0, NULL, 'Hi my name is [Player], [Talked] are you working?'),
(17, 10, 0, 0, 0, NULL, 'Hello [Player], Yes, I am on duty as a guard, and I also trade a little'),
(18, 10, 1, 0, 5, NULL, 'Can you tell me how difficult it is to get into the service?'),
(19, 10, 0, 2, 2, NULL, 'If that\'s what you want, it\'s easy. And if you don\'t then you won\'t be able to get in ;)'),
(20, 10, 0, 0, 5, NULL, 'By the way [Player], I need help, there are slugs nearby, again raging, and I can\'t leave the post. At the same time you will test your skills young adventurer.'),
(21, 10, 1, 2, 2, 3, 'Yes, no problem, wait [Talked]!!'),
(22, 11, 0, 0, 1, NULL, '**He says with a look of horror on his face**: Help me find my daughter, she went to the village for food and never returned.'),
(23, 11, 1, 0, 0, NULL, 'To the village? [Talked] Yes of course I will help. That\'s where I need to go!'),
(24, 11, 0, 0, 1, 5, 'Thanks a lot'),
(30, 13, 0, 0, 0, NULL, 'Hello, would you like to learn the basics of mining?'),
(31, 13, 1, 0, 0, NULL, 'Yes, of course, I don\'t mind'),
(32, 13, 0, 0, 0, 6, 'All right follow me I\'ll explain the basics to you!'),
(33, 14, 0, 0, 0, NULL, 'Hello, and so you\'re new here?'),
(34, 14, 1, 0, 5, NULL, 'Yes.'),
(35, 14, 0, 0, 0, NULL, 'I need to register you. Please introduce yourself, and the purpose of your arrival, and the time you to stay here'),
(36, 14, 1, 0, 0, NULL, 'My name is [Player]. I came here to become an adventurer. I can\'t tell you the exact time of stay.'),
(37, 14, 0, 1, 4, NULL, 'And who for you should know the time of your stay here.'),
(38, 14, 0, 0, 5, NULL, 'Okay well, I will write you down, in case of violation of the rules of our village, we have every right to prohibit your presence here, and report to superiors.'),
(39, 14, 0, 2, 2, NULL, 'Have a good day!'),
(40, 14, 1, 0, 5, 10, 'Thank\'s [Talked]!'),
(41, 16, 0, 0, 0, NULL, '[Stranger]Greetings, wanderer. *bowing*'),
(42, 16, 0, 0, 0, NULL, 'I am the Deputy of the Apostle, and I also protect her. '),
(43, 16, 0, 0, 0, NULL, 'I think you\'ve already heard, that the monsters are raging.'),
(44, 16, 0, 0, 0, NULL, 'I have a request for you [Player]'),
(45, 16, 1, 0, 0, NULL, 'Which one?'),
(46, 16, 0, 0, 0, NULL, 'Help in the southern part to win over the monsters. We can\'t drive them away but we can scare them away.'),
(47, 16, 1, 0, 0, 8, 'Of course I will.'),
(48, 16, 0, 2, 2, NULL, 'Thank\'s [Player]');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_talk_quest_npc`
--

CREATE TABLE `tw_talk_quest_npc` (
  `ID` int(11) NOT NULL,
  `MobID` int(11) NOT NULL,
  `RequestComplete` tinyint(1) NOT NULL DEFAULT '0',
  `Style` int(11) NOT NULL DEFAULT '-1',
  `TalkingEmote` int(11) NOT NULL DEFAULT '-1',
  `PlayerTalked` tinyint(1) NOT NULL DEFAULT '0',
  `TalkText` varchar(512) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `tw_talk_quest_npc`
--

INSERT INTO `tw_talk_quest_npc` (`ID`, `MobID`, `RequestComplete`, `Style`, `TalkingEmote`, `PlayerTalked`, `TalkText`) VALUES
(1, 1, 0, 0, 0, 1, 'Hello, I was sent to you by a carpenter to help you.'),
(2, 1, 0, 0, 0, 0, '**He asks with a smile**: Hello, have you just arrived? '),
(3, 1, 0, 0, 0, 1, 'Yes, I came to you to become an adventurer, I need money.'),
(4, 1, 0, 0, 1, 0, 'Well, I\'ll tell you [Player], things have been very bad here lately, the residents are living in fear. Recently, a little girl was killed, as we understand there is a dead force involved.'),
(5, 1, 0, 1, 3, 1, '**With horror in his eyes**: What ... Dead People?'),
(6, 1, 0, 0, 0, 0, '**He said hurriedly**: I don\'t know for sure. Over time, the residents will tell you everything, or you will feel for yourself what is happening there.'),
(7, 1, 1, 0, 5, 1, 'I didn\'t expect it to be this bad. Well, what can I help you with?'),
(8, 1, 1, 0, 0, 0, 'Come with me, our guys dropped boards, help me collect them.'),
(9, 2, 1, 0, 0, 0, 'Okay generally help me collect all the boards! I will reward you for your help.'),
(10, 2, 0, 0, 1, 1, '**Weakness in asks**: Oh, hell, they\'re heavy, I\'ve got them all.'),
(11, 2, 0, 2, 2, 0, 'Great job! Your muscles won\'t hurt you anyway [Player] *laughter*'),
(12, 2, 0, 2, 2, 1, '*laughter*'),
(13, 2, 0, 0, 0, 0, 'Now hand them out to my boys and let them go on with their business.'),
(14, 2, 0, 0, 0, 1, 'Good [Talked]!'),
(15, 3, 1, 0, 0, 1, 'Here you go, your boss told me to give it to you!'),
(16, 3, 0, 0, 0, 0, 'Thanks for the help.'),
(17, 4, 1, 0, 0, 1, 'Here you go, your boss told me to give it to you!'),
(18, 4, 0, 0, 0, 0, 'Thanks for the help.'),
(19, 5, 1, 0, 0, 1, 'Here you go, your boss told me to give it to you!'),
(20, 5, 0, 0, 0, 0, 'Thanks for the help.'),
(21, 6, 0, 0, 1, 1, 'Is tired. But I did it!'),
(22, 6, 1, 2, 2, 0, 'Well done, take this. I think you need it if you\'re going to be an adventurer. Go straight, and you will go straight to the residents, if you ask where to go.'),
(23, 6, 0, 2, 2, 1, '**With a smile**: Well thank you, good luck with your work!'),
(29, 7, 0, 0, 0, 1, '[Stranger]Hi, my name is [Player], how do I find the nearest village?'),
(30, 7, 0, 0, 2, 0, '**She concerned**: Hello, my name is [Talked].'),
(31, 7, 0, 0, 1, 0, 'I\'ll be happy to help you settle in, but could you give my brother a note? He works now on the ship that you arrived on, i could have done it myself, but they won\'t let me in.'),
(32, 7, 1, 2, 2, 1, 'Yes, of course i help, will you be here [Talked]?'),
(33, 7, 0, 0, 0, 0, 'I\'ll be here waiting!'),
(34, 8, 0, 0, 0, 1, 'Hello, are you Diana\'s brother?'),
(35, 8, 0, 1, 3, 0, '**He said rudely**: Yes. Did something happen to her?..'),
(36, 8, 1, 0, 5, 1, 'No, don\'t worry, she just asked me to pass you the notebook.'),
(37, 8, 0, 2, 2, 0, 'Well, thank you very much, and tell her I\'ll be seeing her this evening.'),
(38, 8, 0, 2, 2, 1, 'Yes, good luck with your work!'),
(39, 9, 0, 0, 0, 0, '**She says impatiently**: Oh, you\'re already here, you\'re fast!'),
(40, 9, 0, 2, 2, 1, 'Really?'),
(41, 9, 0, 0, 0, 0, 'Yes. How is he?'),
(42, 9, 0, 0, 0, 1, 'He told me that he is busy now, and will visit you later in the evening!'),
(43, 9, 0, 0, 1, 0, 'I\'m so happy! They won\'t let me in because of the problems that are happening. Dead men kidnap girls, and then they find them dead.'),
(44, 9, 0, 0, 5, 1, '**He sounds surprised**: Only girls? Why the dead?'),
(45, 9, 0, 0, 1, 0, '**She\'s upset**: Honestly, I don\'t know. The dead or who it is, our headman says that most likely the dead ;('),
(46, 9, 0, 0, 0, 1, 'I came to you to become an adventurer, as I know you have magical creatures.'),
(47, 9, 0, 0, 3, 0, '**She sounds like she knows it**: An adventurer? We have a Guild of adventurers, but to become an adventurer you need to pass the tests.'),
(48, 9, 0, 0, 5, 1, 'Tests?....'),
(49, 9, 0, 0, 0, 0, 'Yes the test [Player]. First, you will need to collect the minimum equipment. To apply and pay the fee. And only after that you will be assigned the position of adventurer.'),
(50, 9, 0, 2, 2, 1, 'Is everything so strict? Yes, I think I can handle it.'),
(51, 9, 1, 2, 2, 0, 'All right come on I\'ll show you our village [Player].'),
(52, 9, 0, 2, 2, 1, 'Yes come on ;)'),
(53, 10, 0, 0, 1, 0, ' **She made it sound like there was something wrong with them**: Oh, my God, where are the slime from?.'),
(54, 10, 0, 0, 5, 1, 'I don\'t know [Talked], here even have slimes. Wahoo'),
(55, 10, 0, 0, 1, 0, 'Well, Yes [Player], but now they are becoming more frequent'),
(56, 10, 1, 2, 2, 1, '**You speak proudly**: All right [Talked], I\'ll clear the way!'),
(57, 10, 0, 1, 4, 1, '**You said in disgust**: Phew, the slime is so disgusting...'),
(58, 10, 0, 2, 2, 0, '**She says with a smile**: Well you could use a bath [Player] ;)'),
(59, 10, 0, 2, 2, 1, 'I will not refuse such an honor ;)'),
(60, 10, 0, 0, 0, 0, 'Ah here is and decided, we\'ll get there I\'ll show you where I live. And I\'ll introduce you to the places. And you wash :)'),
(61, 10, 0, 0, 2, 1, 'All right, shall we continue on our way?'),
(62, 10, 0, 0, 0, 0, 'Yes let\'s go'),
(63, 11, 1, 0, 0, 0, '**He said with surprise**: Did you defeat the slugs on the instructions of our master? Take this the master asked me to give you.'),
(64, 11, 0, 2, 2, 1, 'Yes, I could. They were very slippery.'),
(65, 11, 0, 2, 2, 0, 'Hehe.. What did you think?'),
(66, 11, 0, 0, 0, 1, 'Well, not so difficult, at the same time I warmed up.'),
(67, 11, 0, 2, 2, 0, 'Okay I need to serve, good luck to you, don\'t give up ;)'),
(68, 12, 0, 1, 4, 0, '**She said very rudely**: Who the hell are you? What do you want?'),
(69, 12, 0, 0, 0, 1, 'Do you know that your father is looking for you?'),
(70, 12, 0, 1, 4, 0, 'I hate this old guy, he doesn\'t have time for me......'),
(71, 12, 0, 1, 4, 0, 'He can only go there. Do it. And we can\'t spend time together...'),
(72, 12, 0, 1, 4, 0, 'Enrages...'),
(73, 12, 1, 2, 2, 1, 'Calm down let\'s go back together and tell him everything? Maybe he just cares about you like that.'),
(74, 12, 0, 0, 1, 0, 'All right, let\'s go back.'),
(75, 13, 1, 0, 0, 1, 'Are you ready?'),
(76, 13, 0, 0, 1, 0, 'Yes..'),
(77, 14, 0, 0, 4, 0, 'So where was she?.'),
(78, 14, 0, 0, 0, 1, 'You just don\'t get angry, everything\'s fine!'),
(79, 14, 0, 0, 0, 1, 'In fact, she didn\'t want to come back, she was offended that you weren\'t spending time together. You can\'t rest.'),
(80, 14, 1, 0, 3, 0, 'Damn why didn\'t she ask for it herself? Okay, don\'t worry thanks for the work :) Everything will be fine, we will decide something with daughter'),
(81, 14, 0, 0, 2, 1, 'Well, nice :) Good luck to you'),
(82, 15, 0, 0, 0, 0, 'To extract any ore [Player], you will need a pickaxe.'),
(83, 15, 0, 0, 0, 0, 'You can only get it by creating. But in some cases merchants can sell.'),
(84, 15, 0, 0, 0, 0, 'Also, the picks have a durability of 100-0. If you durability reach 0. Don\'t worry it won\'t break.'),
(85, 15, 0, 0, 0, 0, 'But you can\'t work with break pickaxe it. You can repair it at any store.'),
(86, 15, 0, 0, 0, 0, 'You can also improve picks this will depend on their effectiveness.'),
(87, 15, 0, 0, 0, 0, 'Each ore has its own strength. If you have a weak pick and the strength of the ore is high, it can take a long time to extract it. So don\'t forget about the improvements. Or craft even better picks.'),
(88, 15, 0, 0, 0, 0, 'Everything is clearly explained, I will not repeat it again, if you need you will understand everything!'),
(89, 15, 1, 0, 0, 1, 'Yes of course thank you for the explanation'),
(90, 15, 0, 0, 0, 0, 'Well don\'t worry, I\'ll give you a couple of tasks to get used to in this area'),
(91, 16, 0, 0, 0, 1, 'Did you want something?'),
(92, 16, 0, 0, 0, 0, 'I think it\'s time for you to get your first pickaxe.'),
(93, 16, 0, 0, 0, 0, 'I think as crafted something, you don\'t need to explain!'),
(94, 16, 0, 0, 0, 0, 'So let\'s get right to the point, try to make a pickaxe bring it to me, I\'ll look at it.'),
(95, 16, 1, 0, 0, 1, 'Done. (you need to have an idea of how to craft items, you can see them near Craftsman in votes)'),
(96, 16, 0, 0, 0, 0, 'Well, it\'s not bad, so use it for the first time.'),
(97, 16, 0, 0, 0, 0, 'All right so let\'s start with your first ore?'),
(98, 17, 0, 2, 2, 0, 'Well, as [Player], passed customs?'),
(99, 17, 0, 0, 5, 1, 'Yes [Talked]. He was very rude.'),
(100, 17, 0, 0, 0, 0, 'Well, you have to understand that we also have a lot of problems lately, too many monsters, problems with kidnappings.'),
(101, 17, 1, 0, 0, 1, 'Come on, show me [Here] seats!'),
(102, 17, 0, 0, 0, 0, 'Oh sure. By the way, in our village, the chief Apostle.'),
(103, 17, 0, 0, 3, 1, 'Wait an Apostle? They are ancient beings sent by the gods. Do they exist?'),
(104, 17, 0, 0, 0, 0, 'Yes [Player]. They exist to keep order.'),
(105, 17, 0, 0, 5, 0, 'Okay [Player] let\'s go and show you our [Here]'),
(106, 18, 0, 0, 2, 0, '**She said with a smile**: Here, you can do a craft items, make your own equipment potions, or what to eat.'),
(107, 18, 0, 0, 0, 0, '[Player] even if you provide items, the craftsman you must pay for the use of the equipment'),
(108, 18, 0, 0, 0, 1, 'Yes I think I can use it. Thanks for the explanation!'),
(109, 18, 1, 0, 0, 0, 'Let\'s move on!'),
(110, 18, 0, 0, 2, 1, 'Yes let\'s go :)'),
(111, 19, 0, 0, 0, 0, 'So here we have someone\'s house.'),
(112, 19, 0, 0, 0, 0, 'You can also buy houses, decorate them or use the safety Deposit box.'),
(113, 19, 1, 0, 0, 1, 'Sounds good [Talked]!'),
(114, 19, 0, 0, 2, 0, '**She likes to talk**: Well, okay, let\'s move on!'),
(115, 20, 0, 0, 0, 0, 'This is the ether, it allows you to use instantaneous movement around the world.'),
(116, 20, 0, 0, 0, 0, 'But only there in the area where you have already visited.'),
(117, 20, 0, 0, 0, 1, 'Is it free [Talked]?'),
(118, 20, 0, 0, 0, 0, 'Not always, there is nothing free in our world!'),
(119, 20, 1, 0, 5, 0, '**Apparently she didn\'t like something**: Okay [Player] let\'s move on!'),
(120, 20, 0, 0, 0, 1, 'Yes let\'s go [Talked]!'),
(121, 21, 0, 0, 0, 0, 'Here you will be able to raise his skills adventurer, to explore something new. Our world is divided into 3 classes (Healer, Tank, DPS)'),
(122, 21, 0, 0, 3, 1, 'So I can become something?'),
(123, 21, 0, 0, 5, 0, '[Player], you can be anyone you want (as well as fine-tune your game mode)'),
(124, 21, 0, 0, 5, 1, '[Talked] so I can be both a Tank and a Defender?'),
(125, 21, 0, 0, 0, 0, 'Yes quite but do not forget about the main characteristics (Strength, Hardness, idk)'),
(126, 21, 1, 2, 2, 1, 'Thank you for a clear explanation :)'),
(127, 21, 0, 0, 5, 0, 'That\'s a small part of what I\'ve been able to tell you.'),
(128, 22, 0, 0, 0, 0, 'And here, you will be treated. '),
(129, 22, 0, 1, 5, 0, '**She said very jealously**: True, I hate those nurses, all the guys from the village are taken away....'),
(130, 22, 1, 0, 5, 0, 'Don\'t marry them...'),
(131, 22, 0, 2, 2, 1, 'Yes I will not marry anyone [Talked]! Thanks for the care!'),
(133, 23, 0, 0, 5, 0, '**Yawning she said**: Okay I\'m tired I\'ll go home.'),
(134, 23, 1, 0, 0, 0, 'And you don\'t disappear, and [Player] go wash up :)'),
(135, 23, 0, 2, 2, 1, 'Well have a good day [Talked] :)'),
(136, 24, 0, 0, 0, 0, 'Are you ready?'),
(137, 24, 0, 0, 3, 1, 'What, ready?'),
(138, 24, 0, 0, 0, 0, 'Introduce you [Player] to the Apostle of our village. '),
(139, 24, 0, 0, 3, 1, '[Talked] so what\'s it to them for me?'),
(140, 24, 0, 1, 4, 0, '**She said rudely**: No, don\'t think so bad, they actually think of us as their children.'),
(141, 24, 0, 0, 0, 1, 'Why so rude? I understand you, okay, I\'m worried, but we\'ll see what happens.'),
(142, 24, 0, 0, 1, 0, 'I\'m sorry, but she\'s really close to me.'),
(143, 24, 0, 0, 5, 1, 'Are you one of them? Hm...'),
(144, 24, 1, 1, 4, 0, 'Fool let\'s go already.'),
(145, 24, 0, 0, 1, 1, 'Well ... Let\'s go'),
(146, 25, 1, 0, 5, 0, 'Well, just don\'t mess with as a joke, it\'s Apostle, he is watching over us for many years.'),
(147, 25, 0, 0, 0, 1, 'I understand [Talked].'),
(148, 26, 0, 0, 0, 1, 'Hello dear Apostle *bowing*. I have come to you to wisdom.'),
(149, 26, 0, 0, 0, 0, 'Hello, lift your head. I knew you\'d come. But I\'m not sure you\'re the one I saw.'),
(150, 26, 0, 0, 0, 1, 'See it?'),
(151, 26, 0, 0, 0, 0, 'I can\'t communicate with my brothers and sisters, since our paths have diverged, but we have a common mind.'),
(152, 26, 0, 0, 0, 0, 'I had a vision about a year ago that there would soon be a man who was as tenacious as the Apostles.'),
(153, 26, 0, 0, 0, 0, 'But in the end, he was going to die. My visions have always come true, but when they will come true is unknown.'),
(154, 26, 0, 0, 0, 0, 'So please be careful [Player]. I\'m worried about everyone, but I\'ve also been weak lately.'),
(155, 26, 0, 0, 0, 1, 'I .. Not to worry, I can handle it even if I turn out to be one of your visions'),
(156, 26, 1, 0, 0, 0, 'I hope this is not the case, get used to it, then go to Adventurer Koto.'),
(157, 26, 0, 0, 0, 1, 'Thanks a lot.'),
(158, 27, 0, 0, 0, 1, 'Hello [Talked], the Apostle sent me to you. I want to be an adventurer!'),
(159, 27, 0, 1, 5, 0, '**He as if he doesn\'t want me to become an adventurer**: Will you be strong enough? Well, let\'s see, I think you already understand that you first need to test, only after I say whether you are worthy or not to become an Adventurer?'),
(160, 27, 0, 0, 0, 1, 'Well, what exactly is required?'),
(161, 27, 0, 1, 5, 0, 'If you pass my first test, pay the fee, I encourage you, and send you to the exam. And then we will decide whether you are worthy or not'),
(162, 27, 0, 0, 0, 1, 'Okay, where do we start [Talked]?'),
(163, 27, 1, 0, 0, 0, 'Ha ha where do we start? Don\'t joke with me young fighter. First, take this and go to the southern part of the village.'),
(164, 27, 0, 0, 0, 1, 'Well'),
(165, 28, 0, 1, 4, 0, 'You did.'),
(166, 28, 0, 0, 5, 1, 'What could I do?'),
(167, 28, 0, 1, 4, 0, 'I thought you were joking, you wouldn\'t take it seriously, but you came.'),
(168, 28, 1, 0, 0, 1, 'Well, let\'s go.'),
(169, 28, 0, 0, 0, 0, 'Let\'s go.'),
(170, 29, 0, 1, 4, 0, 'Why are you shivering [Player]?'),
(171, 29, 1, 2, 2, 0, 'Am I that scary? Don\'t worry I\'m kind *laughs*. Well, let\'s see how you cope. Kill for me ....'),
(172, 29, 0, 0, 5, 1, 'I managed it, and it wasn\'t difficult.'),
(173, 29, 0, 2, 2, 0, 'I noticed how well you did. But don\'t relax this is just the beginning ^^'),
(174, 29, 0, 0, 5, 1, 'Ah.. Okay [Talked].'),
(175, 29, 0, 0, 0, 0, 'Well today we\'re finished you can relax. You\'ll find me as soon as you\'re ready.'),
(176, 29, 0, 0, 2, 1, 'Okay, well, have a good day.'),
(183, 30, 0, 0, 0, 0, 'Hello, I am the personal Ambassador of the Apostle.'),
(184, 30, 0, 0, 0, 0, 'She wants to see you!'),
(185, 30, 1, 0, 0, 0, 'Please follow me to village, I will be waiting for you there'),
(186, 30, 0, 0, 0, 1, 'Okay, I\'m coming'),
(187, 31, 0, 0, 0, 0, 'Listen there\'s another request!'),
(188, 31, 1, 0, 0, 0, '[Player] if you come across a gel, I\'ll be happy to exchange it. You can take your time I\'ll wait!'),
(189, 31, 0, 0, 0, 1, 'Here, it wasn\'t easy but I got it [Talked].'),
(190, 31, 0, 2, 2, 0, 'Thank you very much for your help. All right I\'ll go back to my post, good luck to you [Player]!'),
(191, 31, 0, 2, 2, 1, 'Thank\'s [Talked]'),
(192, 32, 1, 0, 0, 0, '[Player] you can go in!'),
(193, 32, 0, 0, 0, 1, 'Well [Talked], thank you for seeing me off!'),
(194, 33, 1, 0, 0, 0, '[Player] first we need to win over the slime. Dirty but necessary.'),
(195, 33, 0, 0, 0, 1, 'I report everything, I coped with the task'),
(196, 33, 0, 2, 2, 0, 'Well done, but that\'s not all.'),
(197, 33, 0, 0, 0, 1, 'Anything else?'),
(198, 33, 0, 0, 0, 0, 'Yes I think you know about the ethers find me next to him'),
(199, 34, 0, 0, 0, 0, 'I think you know that these crystals are actually not just for beauty. '),
(200, 34, 0, 0, 0, 0, 'You use them as a quick transition. But in fact, these are crystals (all the light of good prosperity) precisely because they exist.'),
(201, 34, 1, 0, 0, 0, 'Me when once before as became defender of the Apostle, I was able to meet with the crystal. Yes, he has a mind. From it warmth on the soul, all the sins immediately fall from the soul. I\'m sure you\'ll meet him one day.'),
(202, 34, 0, 0, 0, 1, 'Thanks. I will remember that. (Auther text: This is how the player\'s rapprochement with the deities began)'),
(203, 34, 0, 0, 0, 0, 'Well, I\'ll wait for you in the Apostle\'s chambers. We will continue the expulsion of the monsters'),
(204, 35, 0, 0, 0, 0, 'Our priority is higher now.'),
(205, 35, 1, 0, 0, 0, '[Player] you need to defeat the main slug, in the southern part.'),
(206, 35, 0, 0, 0, 1, 'That\'s it, I\'ve met my goal. Clearing dirt.'),
(207, 35, 0, 0, 0, 0, 'Well done, we\'ll meet again.'),
(219, 36, 0, 0, 0, 0, 'I want to talk to you [Player].'),
(220, 36, 0, 0, 5, 0, 'I know that your path will not stop here. One day you will become stronger and move on.'),
(221, 36, 0, 0, 0, 0, 'I have a request to you, if you meet other apostles, it is better to avoid them. Don\'t trust them, we apostles are not as pure as you think'),
(222, 36, 0, 0, 5, 1, 'Maybe you, too, then the enemy really is?'),
(223, 36, 0, 2, 2, 0, 'I don\'t know *laughter*. The villagers I think will clearly tell you how it really is'),
(224, 36, 0, 0, 0, 0, 'Diana told me about you in General terms.'),
(225, 36, 0, 2, 2, 0, '**She said it with concern**: You turn out to be a Joker, and a vulgar *laughter*'),
(226, 36, 0, 0, 5, 1, 'No [Talked]......'),
(227, 36, 0, 0, 0, 0, 'Always listen to your heart. I think one day you will be able to bring light. But the main thing is perseverance!'),
(228, 36, 1, 0, 0, 0, 'Do not delay with your title of adventurer. You can go now!'),
(229, 36, 0, 0, 0, 1, 'I\'m trying, thank you very much.'),
(230, 37, 0, 0, 0, 1, 'You look sad today [Talked].'),
(231, 37, 1, 0, 1, 0, 'Oh man I\'m too confused today.... [Player] help me collect lost.'),
(232, 37, 0, 0, 2, 1, 'Take your nose up [Talked], what\'s wrong with you?'),
(233, 37, 0, 0, 1, 0, 'My brother has to stop by today, and I want to cook something to feed him, but everything falls out of my hands..'),
(234, 37, 0, 0, 2, 1, 'So let me help you?'),
(235, 37, 0, 0, 0, 0, 'And you I look [Player] today cheerful, well help. I will not remain in debt'),
(236, 37, 0, 2, 2, 1, 'Of course [Talked] ;)'),
(237, 38, 0, 0, 0, 0, 'Well here\'s my house! Ah as my not my, father is a master, and even brother. In General, our family'),
(238, 38, 1, 0, 0, 0, 'All right, come on in, I\'ll cook, and I\'ll ask you to bring something. All the help me need'),
(239, 38, 0, 0, 0, 1, 'I will help you, especially since you are my first close friend in this village, I already trust you, how you can be abandoned.'),
(240, 39, 0, 0, 0, 0, 'Well [Player], let\'s get started, little by little'),
(241, 39, 0, 0, 2, 1, 'Your house comfortable! [Talked]'),
(242, 39, 1, 0, 0, 1, 'Do you really think so? Well thank you.'),
(243, 39, 0, 2, 2, 0, 'All right let\'s get down to cooking'),
(244, 40, 0, 0, 0, 0, 'Well, let\'s get started [Player]!'),
(245, 40, 0, 0, 0, 1, 'Oh sure [Talked]'),
(246, 40, 1, 0, 0, 0, '[Player] get me some first, we\'ll use it as oil'),
(247, 40, 0, 2, 2, 1, 'I think everyone has their own way of making butter *laughter*'),
(248, 40, 0, 2, 2, 0, 'Yeah you\'re right, good sense of humor damn *laughter*'),
(249, 41, 0, 0, 0, 0, 'So now I need meat. How to cook without meat.'),
(250, 41, 1, 0, 0, 0, 'I think you\'ve heard of Kappa, they live near water, their meat is very tasty. So get it for me (you can self find them habitat)'),
(251, 41, 0, 0, 2, 1, 'All delicious fine meat, looks great, take it!'),
(252, 41, 0, 0, 2, 0, 'Well i let\'s start cooking :)'),
(253, 41, 0, 0, 0, 1, '**You see like she didn\'t know how to cook**: Are you sure you can cook?'),
(254, 41, 0, 1, 4, 0, '**She said angrily**: You know what I\'ll tell you [Player]... You don\'t know how to compliment girls at all..'),
(255, 41, 0, 0, 5, 1, '**Someone knocks on the door**: Who could it be? lovers?'),
(256, 41, 0, 0, 3, 0, 'Oh it\'s probably my brother.'),
(261, 42, 0, 0, 5, 0, '**He said angrily and rudely**: Oh, what people, I\'ve seen you somewhere.'),
(262, 42, 1, 0, 5, 1, 'You must have imagined it. '),
(263, 42, 0, 0, 0, 0, 'Oh well.'),
(264, 43, 0, 0, 2, 0, '[Player] you look sad.'),
(265, 43, 0, 0, 0, 1, 'Okay [Talked]. I\'ll probably go extra here'),
(266, 43, 0, 0, 5, 0, 'Yes calm down you why so decided to?'),
(267, 43, 0, 2, 2, 1, 'Yes, everything is fine. I just have things to do, too. I\'ll go!'),
(268, 43, 0, 0, 1, 0, 'You offend me [Talked].....'),
(269, 43, 1, 0, 0, 1, 'Sorry I\'m really busy [Talked].'),
(270, 43, 0, 0, 5, 0, 'Well, take care of yourself.'),
(271, 44, 0, 0, 0, 0, 'So let\'s continue our training.'),
(272, 44, 0, 0, 0, 1, 'Yes, of course, it\'s about time!'),
(273, 44, 0, 1, 4, 0, '**He speaks rudely**: I don\'t really like you. But let\'s see what you can achieve. Let\'s go back to the Deep Cave.'),
(274, 45, 0, 0, 0, 0, '**He speaks proudly**: Well, now we have more serious goals.'),
(275, 45, 1, 0, 0, 0, 'Exterminate the stronger slime. I\'ll see.'),
(276, 45, 0, 0, 5, 1, 'Everything is ready, they are vile I do not like slime, maybe something new?'),
(277, 45, 0, 0, 0, 0, 'Well, I\'ll think about your request. Follow me.'),
(278, 46, 1, 0, 0, 0, 'Take it, use it wisely. You will need it very much in the future.'),
(279, 46, 0, 0, 3, 1, '**You were surprised by the gift provided**: Oh, thank you very much. Do I owe you something?'),
(280, 46, 0, 0, 5, 0, 'I hope you make it to the end, at least I hope you do. I didn\'t think you\'d be Apostle\'s favorite.'),
(281, 46, 0, 0, 3, 1, 'Favorite [Talked]?'),
(282, 46, 0, 1, 4, 0, '**He said very rudely**: I\'ll go, meet you later.');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_world_swap`
--

CREATE TABLE `tw_world_swap` (
  `ID` int(11) NOT NULL,
  `OpenQuestID` int(11) DEFAULT NULL,
  `WorldID` int(11) DEFAULT NULL,
  `PositionX` int(11) DEFAULT NULL,
  `PositionY` int(11) DEFAULT NULL,
  `TwoWorldID` int(11) DEFAULT NULL,
  `TwoPositionX` int(11) DEFAULT NULL,
  `TwoPositionY` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_world_swap`
--

INSERT INTO `tw_world_swap` (`ID`, `OpenQuestID`, `WorldID`, `PositionX`, `PositionY`, `TwoWorldID`, `TwoPositionX`, `TwoPositionY`) VALUES
(1, 1, 0, 6900, 1000, 1, 335, 490),
(2, 2, 1, 4605, 1067, 2, 3570, 7950),
(3, 12, 2, 13760, 6680, 3, 400, 1260),
(4, 13, 2, 3510, 6340, 4, 4740, 900),
(5, NULL, 3, 4560, 1205, 5, 610, 4500),
(6, 15, 2, 8328, 6020, 7, 4135, 840);

--
-- Индексы сохранённых таблиц
--

--
-- Индексы таблицы `ENUM_CRAFT_TABS`
--
ALTER TABLE `ENUM_CRAFT_TABS`
  ADD PRIMARY KEY (`TabID`);

--
-- Индексы таблицы `ENUM_EFFECTS_LIST`
--
ALTER TABLE `ENUM_EFFECTS_LIST`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Name` (`Name`);

--
-- Индексы таблицы `ENUM_EMOTES`
--
ALTER TABLE `ENUM_EMOTES`
  ADD PRIMARY KEY (`ID`);

--
-- Индексы таблицы `ENUM_ITEMS_FUNCTIONAL`
--
ALTER TABLE `ENUM_ITEMS_FUNCTIONAL`
  ADD PRIMARY KEY (`FunctionID`);

--
-- Индексы таблицы `ENUM_ITEMS_TYPES`
--
ALTER TABLE `ENUM_ITEMS_TYPES`
  ADD PRIMARY KEY (`TypeID`);

--
-- Индексы таблицы `ENUM_MMO_PROJ`
--
ALTER TABLE `ENUM_MMO_PROJ`
  ADD KEY `ID` (`ID`);

--
-- Индексы таблицы `ENUM_QUEST_INTERACTIVE`
--
ALTER TABLE `ENUM_QUEST_INTERACTIVE`
  ADD KEY `ID` (`ID`);

--
-- Индексы таблицы `ENUM_TALK_STYLES`
--
ALTER TABLE `ENUM_TALK_STYLES`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `ID_2` (`ID`);

--
-- Индексы таблицы `ENUM_WORLDS`
--
ALTER TABLE `ENUM_WORLDS`
  ADD PRIMARY KEY (`WorldID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `Name` (`Name`),
  ADD KEY `SafeZoneWorldID` (`RespawnWorld`),
  ADD KEY `WorldID_2` (`WorldID`);

--
-- Индексы таблицы `tw_accounts`
--
ALTER TABLE `tw_accounts`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `Password` (`Password`),
  ADD KEY `Username` (`Username`);

--
-- Индексы таблицы `tw_accounts_data`
--
ALTER TABLE `tw_accounts_data`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `Nick` (`Nick`),
  ADD KEY `MemberID` (`GuildID`),
  ADD KEY `DiscordID` (`DiscordID`),
  ADD KEY `tw_accounts_data_ibfk_3` (`WorldID`),
  ADD KEY `GuildRank` (`GuildRank`),
  ADD KEY `Level` (`Level`);

--
-- Индексы таблицы `tw_accounts_inbox`
--
ALTER TABLE `tw_accounts_inbox`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `ItemID` (`ItemID`),
  ADD KEY `OwnerID` (`OwnerID`);

--
-- Индексы таблицы `tw_accounts_items`
--
ALTER TABLE `tw_accounts_items`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`),
  ADD KEY `ItemID` (`ItemID`);

--
-- Индексы таблицы `tw_accounts_locations`
--
ALTER TABLE `tw_accounts_locations`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`OwnerID`),
  ADD KEY `TeleportID` (`TeleportID`);

--
-- Индексы таблицы `tw_accounts_miner`
--
ALTER TABLE `tw_accounts_miner`
  ADD PRIMARY KEY (`AccountID`),
  ADD UNIQUE KEY `AccountID` (`AccountID`);

--
-- Индексы таблицы `tw_accounts_plants`
--
ALTER TABLE `tw_accounts_plants`
  ADD PRIMARY KEY (`AccountID`),
  ADD UNIQUE KEY `AccountID` (`AccountID`);

--
-- Индексы таблицы `tw_accounts_quests`
--
ALTER TABLE `tw_accounts_quests`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`OwnerID`),
  ADD KEY `TalkProgress` (`Progress`),
  ADD KEY `tw_accounts_quests_ibfk_4` (`QuestID`);

--
-- Индексы таблицы `tw_accounts_skills`
--
ALTER TABLE `tw_accounts_skills`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `SkillID` (`SkillID`),
  ADD KEY `OwnerID` (`OwnerID`);

--
-- Индексы таблицы `tw_aethers`
--
ALTER TABLE `tw_aethers`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_attributs`
--
ALTER TABLE `tw_attributs`
  ADD PRIMARY KEY (`ID`);

--
-- Индексы таблицы `tw_bots_mobs`
--
ALTER TABLE `tw_bots_mobs`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `MobID` (`BotID`),
  ADD KEY `it_drop_0` (`it_drop_0`),
  ADD KEY `it_drop_1` (`it_drop_1`),
  ADD KEY `it_drop_2` (`it_drop_2`),
  ADD KEY `it_drop_3` (`it_drop_3`),
  ADD KEY `it_drop_4` (`it_drop_4`),
  ADD KEY `it_drop_5` (`it_drop_5`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `Effect` (`Effect`);

--
-- Индексы таблицы `tw_bots_npc`
--
ALTER TABLE `tw_bots_npc`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `MobID` (`BotID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `tw_bots_npc_ibfk_3` (`Emote`);

--
-- Индексы таблицы `tw_bots_quest`
--
ALTER TABLE `tw_bots_quest`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `MobID` (`BotID`),
  ADD KEY `it_need_0` (`it_need_0`),
  ADD KEY `tw_bots_quest_ibfk_3` (`it_need_1`),
  ADD KEY `tw_bots_quest_ibfk_4` (`it_reward_0`),
  ADD KEY `it_reward_1` (`it_reward_1`),
  ADD KEY `QuestID` (`QuestID`),
  ADD KEY `tw_bots_quest_ibfk_6` (`mob_0`),
  ADD KEY `tw_bots_quest_ibfk_7` (`mob_1`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `interactive_type` (`interactive_type`);

--
-- Индексы таблицы `tw_bots_world`
--
ALTER TABLE `tw_bots_world`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `SlotWings` (`SlotWings`),
  ADD KEY `SlotHammer` (`SlotHammer`),
  ADD KEY `SlotGun` (`SlotGun`),
  ADD KEY `tw_bots_world_ibfk_4` (`SlotShotgun`),
  ADD KEY `SlotGrenade` (`SlotGrenade`),
  ADD KEY `SlotRifle` (`SlotRifle`);

--
-- Индексы таблицы `tw_craft_list`
--
ALTER TABLE `tw_craft_list`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `CraftType` (`Type`),
  ADD KEY `CraftIID` (`GetItem`),
  ADD KEY `Craft_Item_0` (`ItemNeed0`),
  ADD KEY `Craft_Item_1` (`ItemNeed1`),
  ADD KEY `Craft_Item_2` (`ItemNeed2`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_dungeons`
--
ALTER TABLE `tw_dungeons`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_dungeons_door`
--
ALTER TABLE `tw_dungeons_door`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `tw_dungeons_door_ibfk_1` (`DungeonID`),
  ADD KEY `tw_dungeons_door_ibfk_2` (`BotID`);

--
-- Индексы таблицы `tw_dungeons_records`
--
ALTER TABLE `tw_dungeons_records`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `tw_dungeons_records_ibfk_1` (`OwnerID`),
  ADD KEY `DungeonID` (`DungeonID`),
  ADD KEY `Seconds` (`Seconds`);

--
-- Индексы таблицы `tw_guilds`
--
ALTER TABLE `tw_guilds`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`OwnerID`),
  ADD KEY `Bank` (`Bank`),
  ADD KEY `Level` (`Level`);

--
-- Индексы таблицы `tw_guilds_decorations`
--
ALTER TABLE `tw_guilds_decorations`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `tw_guilds_decorations_ibfk_2` (`DecoID`),
  ADD KEY `tw_guilds_decorations_ibfk_3` (`WorldID`),
  ADD KEY `HouseID` (`HouseID`);

--
-- Индексы таблицы `tw_guilds_history`
--
ALTER TABLE `tw_guilds_history`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `MemberID` (`GuildID`);

--
-- Индексы таблицы `tw_guilds_houses`
--
ALTER TABLE `tw_guilds_houses`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerMID` (`OwnerMID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_guilds_invites`
--
ALTER TABLE `tw_guilds_invites`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`OwnerID`),
  ADD KEY `MemberID` (`GuildID`);

--
-- Индексы таблицы `tw_guilds_ranks`
--
ALTER TABLE `tw_guilds_ranks`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `MemberID` (`GuildID`);

--
-- Индексы таблицы `tw_houses`
--
ALTER TABLE `tw_houses`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`OwnerID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `PlantID` (`PlantID`);

--
-- Индексы таблицы `tw_houses_decorations`
--
ALTER TABLE `tw_houses_decorations`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `HouseID` (`HouseID`),
  ADD KEY `DecoID` (`DecoID`);

--
-- Индексы таблицы `tw_items_list`
--
ALTER TABLE `tw_items_list`
  ADD PRIMARY KEY (`ItemID`),
  ADD UNIQUE KEY `ItemID` (`ItemID`),
  ADD KEY `ItemBonus` (`Stat_0`),
  ADD KEY `ItemID_2` (`ItemID`),
  ADD KEY `ItemType` (`Type`),
  ADD KEY `tw_items_list_ibfk_3` (`Function`),
  ADD KEY `ItemProjID` (`ProjectileID`),
  ADD KEY `tw_items_list_ibfk_5` (`Stat_1`);

--
-- Индексы таблицы `tw_logicworld`
--
ALTER TABLE `tw_logicworld`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `MobID` (`MobID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `ParseInt` (`ParseInt`);

--
-- Индексы таблицы `tw_mailshop`
--
ALTER TABLE `tw_mailshop`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `ItemID` (`ItemID`),
  ADD KEY `OwnerID` (`OwnerID`),
  ADD KEY `StorageID` (`StorageID`),
  ADD KEY `Time` (`Time`),
  ADD KEY `NeedItem` (`NeedItem`),
  ADD KEY `Price` (`Price`);

--
-- Индексы таблицы `tw_position_miner`
--
ALTER TABLE `tw_position_miner`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `ItemID` (`ItemID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_position_plant`
--
ALTER TABLE `tw_position_plant`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `ItemID` (`ItemID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_quests_list`
--
ALTER TABLE `tw_quests_list`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`);

--
-- Индексы таблицы `tw_skills_list`
--
ALTER TABLE `tw_skills_list`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`);

--
-- Индексы таблицы `tw_storages`
--
ALTER TABLE `tw_storages`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_talk_other_npc`
--
ALTER TABLE `tw_talk_other_npc`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `tw_talk_other_npc_ibfk_1` (`MobID`),
  ADD KEY `tw_talk_other_npc_ibfk_2` (`Style`),
  ADD KEY `tw_talk_other_npc_ibfk_3` (`TalkingEmote`),
  ADD KEY `tw_talk_other_npc_ibfk_4` (`GivingQuest`);

--
-- Индексы таблицы `tw_talk_quest_npc`
--
ALTER TABLE `tw_talk_quest_npc`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `tw_talk_quest_npc_ibfk_1` (`MobID`),
  ADD KEY `tw_talk_quest_npc_ibfk_2` (`Style`),
  ADD KEY `tw_talk_quest_npc_ibfk_4` (`TalkingEmote`);

--
-- Индексы таблицы `tw_world_swap`
--
ALTER TABLE `tw_world_swap`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `TwoWorldID` (`TwoWorldID`),
  ADD KEY `tw_world_swap_ibfk_3` (`OpenQuestID`);

--
-- AUTO_INCREMENT для сохранённых таблиц
--

--
-- AUTO_INCREMENT для таблицы `ENUM_CRAFT_TABS`
--
ALTER TABLE `ENUM_CRAFT_TABS`
  MODIFY `TabID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;
--
-- AUTO_INCREMENT для таблицы `ENUM_EFFECTS_LIST`
--
ALTER TABLE `ENUM_EFFECTS_LIST`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;
--
-- AUTO_INCREMENT для таблицы `ENUM_ITEMS_FUNCTIONAL`
--
ALTER TABLE `ENUM_ITEMS_FUNCTIONAL`
  MODIFY `FunctionID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=13;
--
-- AUTO_INCREMENT для таблицы `ENUM_ITEMS_TYPES`
--
ALTER TABLE `ENUM_ITEMS_TYPES`
  MODIFY `TypeID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;
--
-- AUTO_INCREMENT для таблицы `ENUM_TALK_STYLES`
--
ALTER TABLE `ENUM_TALK_STYLES`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT для таблицы `tw_accounts`
--
ALTER TABLE `tw_accounts`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
--
-- AUTO_INCREMENT для таблицы `tw_accounts_data`
--
ALTER TABLE `tw_accounts_data`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
--
-- AUTO_INCREMENT для таблицы `tw_accounts_inbox`
--
ALTER TABLE `tw_accounts_inbox`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT для таблицы `tw_accounts_items`
--
ALTER TABLE `tw_accounts_items`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=163;
--
-- AUTO_INCREMENT для таблицы `tw_accounts_locations`
--
ALTER TABLE `tw_accounts_locations`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;
--
-- AUTO_INCREMENT для таблицы `tw_accounts_quests`
--
ALTER TABLE `tw_accounts_quests`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=173;
--
-- AUTO_INCREMENT для таблицы `tw_accounts_skills`
--
ALTER TABLE `tw_accounts_skills`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;
--
-- AUTO_INCREMENT для таблицы `tw_aethers`
--
ALTER TABLE `tw_aethers`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;
--
-- AUTO_INCREMENT для таблицы `tw_bots_mobs`
--
ALTER TABLE `tw_bots_mobs`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=13;
--
-- AUTO_INCREMENT для таблицы `tw_bots_npc`
--
ALTER TABLE `tw_bots_npc`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=18;
--
-- AUTO_INCREMENT для таблицы `tw_bots_quest`
--
ALTER TABLE `tw_bots_quest`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=47;
--
-- AUTO_INCREMENT для таблицы `tw_bots_world`
--
ALTER TABLE `tw_bots_world`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=35;
--
-- AUTO_INCREMENT для таблицы `tw_craft_list`
--
ALTER TABLE `tw_craft_list`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;
--
-- AUTO_INCREMENT для таблицы `tw_dungeons`
--
ALTER TABLE `tw_dungeons`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT для таблицы `tw_dungeons_door`
--
ALTER TABLE `tw_dungeons_door`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
--
-- AUTO_INCREMENT для таблицы `tw_dungeons_records`
--
ALTER TABLE `tw_dungeons_records`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_guilds`
--
ALTER TABLE `tw_guilds`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_guilds_decorations`
--
ALTER TABLE `tw_guilds_decorations`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_guilds_history`
--
ALTER TABLE `tw_guilds_history`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_guilds_houses`
--
ALTER TABLE `tw_guilds_houses`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT для таблицы `tw_guilds_invites`
--
ALTER TABLE `tw_guilds_invites`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_guilds_ranks`
--
ALTER TABLE `tw_guilds_ranks`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_houses`
--
ALTER TABLE `tw_houses`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT для таблицы `tw_houses_decorations`
--
ALTER TABLE `tw_houses_decorations`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_items_list`
--
ALTER TABLE `tw_items_list`
  MODIFY `ItemID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=15004;
--
-- AUTO_INCREMENT для таблицы `tw_logicworld`
--
ALTER TABLE `tw_logicworld`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `tw_mailshop`
--
ALTER TABLE `tw_mailshop`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;
--
-- AUTO_INCREMENT для таблицы `tw_position_miner`
--
ALTER TABLE `tw_position_miner`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;
--
-- AUTO_INCREMENT для таблицы `tw_position_plant`
--
ALTER TABLE `tw_position_plant`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT для таблицы `tw_quests_list`
--
ALTER TABLE `tw_quests_list`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=18;
--
-- AUTO_INCREMENT для таблицы `tw_skills_list`
--
ALTER TABLE `tw_skills_list`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;
--
-- AUTO_INCREMENT для таблицы `tw_storages`
--
ALTER TABLE `tw_storages`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT для таблицы `tw_talk_other_npc`
--
ALTER TABLE `tw_talk_other_npc`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=49;
--
-- AUTO_INCREMENT для таблицы `tw_talk_quest_npc`
--
ALTER TABLE `tw_talk_quest_npc`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=283;
--
-- AUTO_INCREMENT для таблицы `tw_world_swap`
--
ALTER TABLE `tw_world_swap`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;
--
-- Ограничения внешнего ключа сохраненных таблиц
--

--
-- Ограничения внешнего ключа таблицы `tw_accounts_data`
--
ALTER TABLE `tw_accounts_data`
  ADD CONSTRAINT `tw_accounts_data_ibfk_1` FOREIGN KEY (`ID`) REFERENCES `tw_accounts` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_data_ibfk_2` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_data_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_data_ibfk_4` FOREIGN KEY (`GuildRank`) REFERENCES `tw_guilds_ranks` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_inbox`
--
ALTER TABLE `tw_accounts_inbox`
  ADD CONSTRAINT `tw_accounts_inbox_ibfk_1` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_inbox_ibfk_2` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_items`
--
ALTER TABLE `tw_accounts_items`
  ADD CONSTRAINT `tw_accounts_items_ibfk_1` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_items_ibfk_2` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_locations`
--
ALTER TABLE `tw_accounts_locations`
  ADD CONSTRAINT `tw_accounts_locations_ibfk_1` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_locations_ibfk_2` FOREIGN KEY (`TeleportID`) REFERENCES `tw_aethers` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_miner`
--
ALTER TABLE `tw_accounts_miner`
  ADD CONSTRAINT `tw_accounts_miner_ibfk_1` FOREIGN KEY (`AccountID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_plants`
--
ALTER TABLE `tw_accounts_plants`
  ADD CONSTRAINT `tw_accounts_plants_ibfk_1` FOREIGN KEY (`AccountID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_quests`
--
ALTER TABLE `tw_accounts_quests`
  ADD CONSTRAINT `tw_accounts_quests_ibfk_3` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_quests_ibfk_4` FOREIGN KEY (`QuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_skills`
--
ALTER TABLE `tw_accounts_skills`
  ADD CONSTRAINT `tw_accounts_skills_ibfk_1` FOREIGN KEY (`SkillID`) REFERENCES `tw_skills_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_skills_ibfk_2` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_aethers`
--
ALTER TABLE `tw_aethers`
  ADD CONSTRAINT `tw_aethers_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_mobs`
--
ALTER TABLE `tw_bots_mobs`
  ADD CONSTRAINT `tw_bots_mobs_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_world` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_10` FOREIGN KEY (`it_drop_1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_11` FOREIGN KEY (`it_drop_2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_12` FOREIGN KEY (`it_drop_3`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_13` FOREIGN KEY (`it_drop_4`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_14` FOREIGN KEY (`it_drop_5`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_15` FOREIGN KEY (`Effect`) REFERENCES `ENUM_EFFECTS_LIST` (`Name`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_8` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_9` FOREIGN KEY (`it_drop_0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_npc`
--
ALTER TABLE `tw_bots_npc`
  ADD CONSTRAINT `tw_bots_npc_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_world` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_npc_ibfk_3` FOREIGN KEY (`Emote`) REFERENCES `ENUM_EMOTES` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `tw_bots_npc_ibfk_4` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_quest`
--
ALTER TABLE `tw_bots_quest`
  ADD CONSTRAINT `tw_bots_quest_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_world` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_10` FOREIGN KEY (`interactive_type`) REFERENCES `ENUM_QUEST_INTERACTIVE` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_2` FOREIGN KEY (`it_need_0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_3` FOREIGN KEY (`it_need_1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_4` FOREIGN KEY (`it_reward_0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_5` FOREIGN KEY (`it_reward_1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_6` FOREIGN KEY (`mob_0`) REFERENCES `tw_bots_world` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_7` FOREIGN KEY (`mob_1`) REFERENCES `tw_bots_world` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_8` FOREIGN KEY (`QuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_9` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_world`
--
ALTER TABLE `tw_bots_world`
  ADD CONSTRAINT `tw_bots_world_ibfk_1` FOREIGN KEY (`SlotWings`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_world_ibfk_2` FOREIGN KEY (`SlotHammer`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_world_ibfk_3` FOREIGN KEY (`SlotGun`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_world_ibfk_4` FOREIGN KEY (`SlotShotgun`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_world_ibfk_5` FOREIGN KEY (`SlotGrenade`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_world_ibfk_6` FOREIGN KEY (`SlotRifle`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_craft_list`
--
ALTER TABLE `tw_craft_list`
  ADD CONSTRAINT `tw_craft_list_ibfk_1` FOREIGN KEY (`GetItem`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_craft_list_ibfk_2` FOREIGN KEY (`ItemNeed0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_craft_list_ibfk_3` FOREIGN KEY (`ItemNeed1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_craft_list_ibfk_4` FOREIGN KEY (`ItemNeed2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_craft_list_ibfk_5` FOREIGN KEY (`Type`) REFERENCES `ENUM_CRAFT_TABS` (`TabID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `tw_craft_list_ibfk_6` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_dungeons`
--
ALTER TABLE `tw_dungeons`
  ADD CONSTRAINT `tw_dungeons_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE NO ACTION ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_dungeons_door`
--
ALTER TABLE `tw_dungeons_door`
  ADD CONSTRAINT `tw_dungeons_door_ibfk_1` FOREIGN KEY (`DungeonID`) REFERENCES `tw_dungeons` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_dungeons_door_ibfk_2` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_world` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_dungeons_records`
--
ALTER TABLE `tw_dungeons_records`
  ADD CONSTRAINT `tw_dungeons_records_ibfk_1` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_dungeons_records_ibfk_2` FOREIGN KEY (`DungeonID`) REFERENCES `tw_dungeons` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds`
--
ALTER TABLE `tw_guilds`
  ADD CONSTRAINT `tw_guilds_ibfk_1` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds_decorations`
--
ALTER TABLE `tw_guilds_decorations`
  ADD CONSTRAINT `tw_guilds_decorations_ibfk_2` FOREIGN KEY (`DecoID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_guilds_decorations_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_guilds_decorations_ibfk_4` FOREIGN KEY (`HouseID`) REFERENCES `tw_guilds_houses` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds_history`
--
ALTER TABLE `tw_guilds_history`
  ADD CONSTRAINT `tw_guilds_history_ibfk_1` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds_houses`
--
ALTER TABLE `tw_guilds_houses`
  ADD CONSTRAINT `tw_guilds_houses_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_guilds_houses_ibfk_2` FOREIGN KEY (`OwnerMID`) REFERENCES `tw_guilds` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds_invites`
--
ALTER TABLE `tw_guilds_invites`
  ADD CONSTRAINT `tw_guilds_invites_ibfk_1` FOREIGN KEY (`OwnerID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_guilds_invites_ibfk_2` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds_ranks`
--
ALTER TABLE `tw_guilds_ranks`
  ADD CONSTRAINT `tw_guilds_ranks_ibfk_1` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_houses`
--
ALTER TABLE `tw_houses`
  ADD CONSTRAINT `tw_houses_ibfk_1` FOREIGN KEY (`PlantID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_houses_ibfk_2` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_houses_decorations`
--
ALTER TABLE `tw_houses_decorations`
  ADD CONSTRAINT `tw_houses_decorations_ibfk_1` FOREIGN KEY (`HouseID`) REFERENCES `tw_houses` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_houses_decorations_ibfk_2` FOREIGN KEY (`DecoID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_houses_decorations_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_items_list`
--
ALTER TABLE `tw_items_list`
  ADD CONSTRAINT `tw_items_list_ibfk_1` FOREIGN KEY (`Stat_0`) REFERENCES `tw_attributs` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_items_list_ibfk_2` FOREIGN KEY (`Type`) REFERENCES `ENUM_ITEMS_TYPES` (`TypeID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_items_list_ibfk_3` FOREIGN KEY (`Function`) REFERENCES `ENUM_ITEMS_FUNCTIONAL` (`FunctionID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_items_list_ibfk_4` FOREIGN KEY (`ProjectileID`) REFERENCES `ENUM_MMO_PROJ` (`ID`) ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_items_list_ibfk_5` FOREIGN KEY (`Stat_1`) REFERENCES `tw_attributs` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_logicworld`
--
ALTER TABLE `tw_logicworld`
  ADD CONSTRAINT `tw_logicworld_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `tw_world_swap` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_logicworld_ibfk_2` FOREIGN KEY (`ParseInt`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_mailshop`
--
ALTER TABLE `tw_mailshop`
  ADD CONSTRAINT `tw_mailshop_ibfk_1` FOREIGN KEY (`StorageID`) REFERENCES `tw_storages` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_mailshop_ibfk_2` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_mailshop_ibfk_3` FOREIGN KEY (`NeedItem`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_position_miner`
--
ALTER TABLE `tw_position_miner`
  ADD CONSTRAINT `tw_position_miner_ibfk_1` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_position_miner_ibfk_2` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_position_plant`
--
ALTER TABLE `tw_position_plant`
  ADD CONSTRAINT `tw_position_plant_ibfk_1` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_position_plant_ibfk_2` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_storages`
--
ALTER TABLE `tw_storages`
  ADD CONSTRAINT `tw_storages_ibfk_2` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_talk_other_npc`
--
ALTER TABLE `tw_talk_other_npc`
  ADD CONSTRAINT `tw_talk_other_npc_ibfk_1` FOREIGN KEY (`MobID`) REFERENCES `tw_bots_npc` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_talk_other_npc_ibfk_2` FOREIGN KEY (`Style`) REFERENCES `ENUM_TALK_STYLES` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_talk_other_npc_ibfk_3` FOREIGN KEY (`TalkingEmote`) REFERENCES `ENUM_EMOTES` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `tw_talk_other_npc_ibfk_4` FOREIGN KEY (`GivingQuest`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_talk_quest_npc`
--
ALTER TABLE `tw_talk_quest_npc`
  ADD CONSTRAINT `tw_talk_quest_npc_ibfk_1` FOREIGN KEY (`MobID`) REFERENCES `tw_bots_quest` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_talk_quest_npc_ibfk_2` FOREIGN KEY (`Style`) REFERENCES `ENUM_TALK_STYLES` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_talk_quest_npc_ibfk_3` FOREIGN KEY (`TalkingEmote`) REFERENCES `ENUM_EMOTES` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `tw_talk_quest_npc_ibfk_4` FOREIGN KEY (`TalkingEmote`) REFERENCES `ENUM_EMOTES` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Ограничения внешнего ключа таблицы `tw_world_swap`
--
ALTER TABLE `tw_world_swap`
  ADD CONSTRAINT `tw_world_swap_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_world_swap_ibfk_2` FOREIGN KEY (`TwoWorldID`) REFERENCES `ENUM_WORLDS` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_world_swap_ibfk_3` FOREIGN KEY (`OpenQuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
