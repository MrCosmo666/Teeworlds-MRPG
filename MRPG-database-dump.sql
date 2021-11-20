-- phpMyAdmin SQL Dump
-- version 5.1.1
-- https://www.phpmyadmin.net/
--
-- Хост: 127.0.0.1
-- Время создания: Ноя 15 2021 г., 07:47
-- Версия сервера: 10.4.21-MariaDB
-- Версия PHP: 8.0.11

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- База данных: `mrpg`
--

-- --------------------------------------------------------

--
-- Структура таблицы `enum_behavior_mobs`
--

CREATE TABLE `enum_behavior_mobs` (
  `ID` int(11) NOT NULL,
  `Behavior` varchar(32) NOT NULL DEFAULT 'Standard'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `enum_behavior_mobs`
--

INSERT INTO `enum_behavior_mobs` (`ID`, `Behavior`) VALUES
(3, 'Sleepy'),
(2, 'Slime'),
(1, 'Standard');

-- --------------------------------------------------------

--
-- Структура таблицы `enum_effects_list`
--

CREATE TABLE `enum_effects_list` (
  `ID` int(11) NOT NULL,
  `Name` varchar(16) CHARACTER SET utf8mb4 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `enum_effects_list`
--

INSERT INTO `enum_effects_list` (`ID`, `Name`) VALUES
(3, 'Fire'),
(2, 'Poison'),
(1, 'Slowdown');

-- --------------------------------------------------------

--
-- Структура таблицы `enum_emotes`
--

CREATE TABLE `enum_emotes` (
  `ID` int(11) NOT NULL,
  `Emote` varchar(64) NOT NULL DEFAULT 'nope'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `enum_emotes`
--

INSERT INTO `enum_emotes` (`ID`, `Emote`) VALUES
(0, 'Normal Emote'),
(1, 'Pain Emote'),
(2, 'Happy Emote'),
(3, 'Surprise Emote'),
(4, 'Angry Emote'),
(5, 'Blink Emote');

-- --------------------------------------------------------

--
-- Структура таблицы `enum_items_functional`
--

CREATE TABLE `enum_items_functional` (
  `FunctionID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `enum_items_functional`
--

INSERT INTO `enum_items_functional` (`FunctionID`, `Name`) VALUES
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
-- Структура таблицы `enum_items_types`
--

CREATE TABLE `enum_items_types` (
  `TypeID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `enum_items_types`
--

INSERT INTO `enum_items_types` (`TypeID`, `Name`) VALUES
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
-- Структура таблицы `enum_mmo_proj`
--

CREATE TABLE `enum_mmo_proj` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `enum_mmo_proj`
--

INSERT INTO `enum_mmo_proj` (`ID`, `Name`) VALUES
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
-- Структура таблицы `enum_quest_interactive`
--

CREATE TABLE `enum_quest_interactive` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `enum_quest_interactive`
--

INSERT INTO `enum_quest_interactive` (`ID`, `Name`) VALUES
(1, 'Randomly accept or refuse with the item'),
(2, 'Pick up items that NPC will drop.');

-- --------------------------------------------------------

--
-- Структура таблицы `enum_worlds`
--

CREATE TABLE `enum_worlds` (
  `WorldID` int(11) NOT NULL,
  `Name` varchar(32) CHARACTER SET utf8 NOT NULL,
  `RespawnWorld` int(11) DEFAULT NULL,
  `MusicID` int(11) NOT NULL DEFAULT -1
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Дамп данных таблицы `enum_worlds`
--

INSERT INTO `enum_worlds` (`WorldID`, `Name`, `RespawnWorld`, `MusicID`) VALUES
(0, 'Pier Elfinia', NULL, 53),
(1, 'Way to the Elfinia', 1, 54),
(2, 'Elfinia', 2, 53),
(3, 'Elfinia Deep cave', 2, 54),
(4, 'Elfia home room', 2, 53),
(5, 'Elfinia occupation of goblins', 5, 54),
(6, 'Elfinia Abandoned mine', NULL, 56),
(7, 'Diana home room', 2, 53),
(8, 'Noctis Resonance', NULL, 55),
(9, 'Departure', 9, 53),
(10, 'Underwater of Neptune', 10, 55),
(11, 'Yugasaki', 11, 53);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts`
--

CREATE TABLE `tw_accounts` (
  `ID` int(11) NOT NULL,
  `Username` varchar(64) NOT NULL,
  `Password` varchar(64) NOT NULL,
  `PasswordSalt` varchar(64) DEFAULT NULL,
  `RegisterDate` varchar(64) NOT NULL,
  `LoginDate` varchar(64) NOT NULL DEFAULT 'First log in',
  `RegisteredIP` varchar(64) NOT NULL DEFAULT '0.0.0.0',
  `LoginIP` varchar(64) NOT NULL DEFAULT '0.0.0.0',
  `Language` varchar(8) NOT NULL DEFAULT 'en'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts`
--

INSERT INTO `tw_accounts` (`ID`, `Username`, `Password`, `PasswordSalt`, `RegisterDate`, `LoginDate`, `RegisteredIP`, `LoginIP`, `Language`) VALUES
(1, 'kuro', '7dd4cd59370ae03ef3f73b0711cd702df5e1853bf97bf8705244c0c22d759db1', 'TYdfB9M75CLUNGkPCm6PF46C', '2021-10-21 17:26:48', '2021-11-07 14:57:43', '192.168.56.1', '192.168.9.2', 'ru'),
(2, 'kuro', 'bc5905cfaa9e0188814fa2a56ecd6a998a4d39c6345ff9bff296bd5086dee93e', 'g8KmHG6T72CGXjRnn2CBq7Zt', '2021-11-04 08:38:30', '2021-11-04 23:23:59', '192.168.9.2', '192.168.42.35', 'ru'),
(3, 'kuro', 'b46d7846dd83b73171009b49847825aa45abbb37071021d6b4219319a680ba27', 'eVE2pT8AbnS8Lc7AUMLtWRkS', '2021-11-04 12:43:33', '2021-11-04 21:18:05', '192.168.9.2', '192.168.9.2', 'ru'),
(4, 'kuro', 'da6eee14f58fc11fefade37e6974b2fe18259a1619cffc84279a0d3fd0101d1b', 'Yhg56c79pk6LC97TZVjdbLVg', '2021-11-05 06:51:11', '2021-11-05 14:25:01', '192.168.9.2', '192.168.9.2', 'en');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_aethers`
--

CREATE TABLE `tw_accounts_aethers` (
  `ID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL,
  `AetherID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--
-- Дамп данных таблицы `tw_accounts_aethers`
--

INSERT INTO `tw_accounts_aethers` (`ID`, `UserID`, `AetherID`) VALUES
(1, 1, 2),
(2, 1, 1),
(3, 2, 2),
(4, 2, 1),
(5, 3, 2),
(6, 1, 3),
(7, 4, 2),
(8, 4, 1),
(9, 1, 4);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_data`
--

CREATE TABLE `tw_accounts_data` (
  `ID` int(11) NOT NULL,
  `Nick` varchar(32) NOT NULL,
  `DiscordID` varchar(64) NOT NULL DEFAULT 'null',
  `WorldID` int(11) DEFAULT NULL,
  `Level` int(11) NOT NULL DEFAULT 1,
  `Exp` int(11) NOT NULL DEFAULT 0,
  `GuildID` int(11) DEFAULT NULL,
  `GuildDeposit` int(11) NOT NULL DEFAULT 0,
  `GuildRank` int(11) DEFAULT NULL,
  `Upgrade` int(11) NOT NULL DEFAULT 0,
  `DiscordEquip` int(11) NOT NULL DEFAULT -1,
  `SpreadShotgun` int(11) NOT NULL DEFAULT 3,
  `SpreadGrenade` int(11) NOT NULL DEFAULT 1,
  `SpreadRifle` int(11) NOT NULL DEFAULT 1,
  `Dexterity` int(11) NOT NULL DEFAULT 0,
  `CriticalHit` int(11) NOT NULL DEFAULT 0,
  `DirectCriticalHit` int(11) NOT NULL DEFAULT 0,
  `Hardness` int(11) NOT NULL DEFAULT 0,
  `Tenacity` int(11) NOT NULL DEFAULT 0,
  `Lucky` int(11) NOT NULL DEFAULT 0,
  `Piety` int(11) NOT NULL DEFAULT 0,
  `Vampirism` int(11) NOT NULL DEFAULT 0,
  `AmmoRegen` int(11) NOT NULL DEFAULT 0,
  `Ammo` int(11) NOT NULL DEFAULT 0,
  `Efficiency` int(11) NOT NULL DEFAULT 0,
  `Extraction` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_data`
--

INSERT INTO `tw_accounts_data` (`ID`, `Nick`, `DiscordID`, `WorldID`, `Level`, `Exp`, `GuildID`, `GuildDeposit`, `GuildRank`, `Upgrade`, `DiscordEquip`, `SpreadShotgun`, `SpreadGrenade`, `SpreadRifle`, `Dexterity`, `CriticalHit`, `DirectCriticalHit`, `Hardness`, `Tenacity`, `Lucky`, `Piety`, `Vampirism`, `AmmoRegen`, `Ammo`, `Efficiency`, `Extraction`) VALUES
(1, 'kurosio', '571251558457540617', 11, 34, 12952, NULL, 0, NULL, 330, -1, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(2, 'kurosio1', 'null', 2, 3, 99, NULL, 0, NULL, 20, -1, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3, 'kurosio12', 'null', 0, 2, 2, NULL, 0, NULL, 10, -1, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(4, 'kurosio44', 'null', 7, 4, 156, NULL, 0, NULL, 30, -1, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_farming`
--

CREATE TABLE `tw_accounts_farming` (
  `UserID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT 1,
  `Exp` int(11) NOT NULL DEFAULT 0,
  `Quantity` int(11) NOT NULL DEFAULT 1,
  `Upgrade` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--
-- Дамп данных таблицы `tw_accounts_farming`
--

INSERT INTO `tw_accounts_farming` (`UserID`, `Level`, `Exp`, `Quantity`, `Upgrade`) VALUES
(1, 1, 5, 1, 0),
(2, 1, 2, 1, 0),
(3, 1, 0, 1, 0),
(4, 1, 0, 1, 0);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_items`
--

CREATE TABLE `tw_accounts_items` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `Value` int(11) NOT NULL,
  `Settings` int(11) NOT NULL,
  `Enchant` int(11) NOT NULL,
  `Durability` int(11) NOT NULL DEFAULT 100,
  `UserID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

--
-- Дамп данных таблицы `tw_accounts_items`
--

INSERT INTO `tw_accounts_items` (`ID`, `ItemID`, `Value`, `Settings`, `Enchant`, `Durability`, `UserID`) VALUES
(1, 2, 1, 0, 0, 100, 1),
(2, 39, 2, 1, 0, 100, 1),
(5, 3, 1, 1, 0, 100, 1),
(6, 1, 10662, 0, 0, 100, 1),
(9, 27, 1, 1, 0, 100, 1),
(10, 9, 9080, 0, 0, 100, 1),
(11, 22, 94, 0, -858993460, 100, 1),
(12, 32, 1, 1, 0, 100, 1),
(13, 31, 50, 0, 0, 100, 1),
(14, 33, 1, 1, 0, 100, 1),
(15, 35, 67, 0, 0, 100, 1),
(16, 34, 1, 1, 0, 100, 1),
(17, 23, 100, 0, 0, 100, 1),
(18, 21, 100, 0, 0, 100, 1),
(20, 24, 100, 0, 0, 100, 1),
(21, 25, 10099, 0, 0, 100, 1),
(24, 18, 100, 0, 0, 100, 1),
(25, 17, 98, 0, 0, 100, 1),
(27, 15, 10112, 0, 0, 100, 1),
(28, 28, 1, 1, 0, 100, 1),
(29, 29, 50, 0, -858993460, 100, 1),
(30, 10014, 1, 1, 0, 100, 1),
(31, 10019, 1, 1, 1, 100, 1),
(32, 10017, 1, 0, 1, 100, 1),
(33, 10015, 1, 1, 2, 100, 1),
(34, 13, 1, 0, 0, 100, 1),
(35, 14, 12, 0, 0, 100, 1),
(37, 36, 1, 1, 0, 100, 1),
(38, 40, 2, 0, 0, 100, 1),
(39, 43, 7, 0, 0, 100, 1),
(40, 44, 2, 0, 0, 100, 1),
(42, 46, 1, 0, 0, 100, 1),
(43, 47, 1, 1, 0, 100, 1),
(44, 49, 1, 1, 0, 100, 1),
(45, 42, 1, 0, 0, 100, 1),
(46, 50, 1, 0, 0, 100, 1),
(47, 51, 1, 0, 0, 100, 1),
(48, 38, 1, 0, 0, 100, 1),
(49, 7, 1102, 0, 0, 100, 1),
(50, 26, 1, 1, 1, 100, 1),
(52, 30, 1, 0, 0, 100, 1),
(53, 45, 1, 0, 0, 100, 1),
(54, 4, 1, 1, 0, 100, 1),
(55, 5, 1, 1, 0, 100, 1),
(56, 6, 1, 0, 0, 100, 1),
(57, 2, 1, 1, 0, 100, 2),
(58, 39, 1, 1, 0, 100, 2),
(61, 3, 1, 1, 0, 100, 2),
(62, 1, 179, 0, 0, 100, 2),
(64, 27, 1, 1, 0, 100, 2),
(65, 2, 1, 1, 0, 100, 3),
(66, 39, 1, 1, 0, 100, 3),
(69, 3, 1, 1, 0, 100, 3),
(70, 1, 15, 0, 0, 100, 3),
(72, 20, 2, 0, 0, 100, 2),
(73, 16, 3, 0, 0, 100, 1),
(74, 2, 1, 1, 0, 100, 4),
(75, 39, 1, 1, 0, 100, 4),
(78, 3, 1, 1, 0, 100, 4),
(79, 1, 317, 0, 0, 100, 4),
(81, 9, 2, 0, 0, 100, 4),
(82, 27, 1, 1, 0, 100, 4),
(83, 22, 3, 0, -858993460, 100, 4);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_mailbox`
--

CREATE TABLE `tw_accounts_mailbox` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) DEFAULT NULL,
  `ItemValue` int(11) DEFAULT NULL,
  `Enchant` int(11) DEFAULT NULL,
  `Name` varchar(64) NOT NULL,
  `Description` varchar(64) NOT NULL,
  `UserID` int(11) NOT NULL,
  `IsRead` tinyint(4) NOT NULL DEFAULT 0,
  `FromSend` varchar(32) NOT NULL DEFAULT 'Game'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_mining`
--

CREATE TABLE `tw_accounts_mining` (
  `UserID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT 1,
  `Exp` int(11) NOT NULL DEFAULT 0,
  `Upgrade` int(11) NOT NULL DEFAULT 0,
  `Quantity` int(11) NOT NULL DEFAULT 1
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--
-- Дамп данных таблицы `tw_accounts_mining`
--

INSERT INTO `tw_accounts_mining` (`UserID`, `Level`, `Exp`, `Upgrade`, `Quantity`) VALUES
(1, 1, 0, 0, 1),
(2, 1, 0, 0, 1),
(3, 1, 0, 0, 1),
(4, 1, 0, 0, 1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_quests`
--

CREATE TABLE `tw_accounts_quests` (
  `ID` int(11) NOT NULL,
  `QuestID` int(11) DEFAULT NULL,
  `UserID` int(11) NOT NULL,
  `Step` int(11) NOT NULL DEFAULT 1,
  `Type` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_quests`
--

INSERT INTO `tw_accounts_quests` (`ID`, `QuestID`, `UserID`, `Step`, `Type`) VALUES
(1, 1, 1, 1, 2),
(2, 2, 1, 1, 2),
(3, 6, 1, 1, 2),
(4, 10, 1, 1, 2),
(5, 7, 1, 1, 2),
(6, 50, 1, 1, 2),
(7, 11, 1, 1, 2),
(8, 12, 1, 1, 2),
(9, 13, 1, 1, 2),
(10, 14, 1, 1, 2),
(11, 8, 1, 1, 1),
(12, 15, 1, 1, 2),
(13, 16, 1, 1, 2),
(14, 17, 1, 1, 2),
(15, 55, 1, 1, 2),
(16, 3, 1, 1, 2),
(17, 5, 1, 1, 2),
(18, 4, 1, 1, 1),
(19, 1, 2, 1, 2),
(20, 5, 2, 1, 1),
(21, 2, 2, 1, 2),
(22, 3, 2, 1, 1),
(23, 10, 2, 1, 2),
(24, 11, 2, 1, 2),
(25, 12, 2, 1, 2),
(26, 50, 2, 1, 1),
(27, 6, 2, 1, 2),
(28, 7, 2, 1, 1),
(29, 55, 2, 1, 1),
(30, 13, 2, 1, 1),
(31, 1, 3, 1, 2),
(32, 5, 3, 1, 1),
(33, 2, 3, 1, 1),
(34, 18, 1, 1, 2),
(35, 19, 1, 1, 2),
(36, 20, 1, 1, 2),
(37, 60, 1, 1, 1),
(38, 21, 1, 3, 1),
(39, 22, 1, 1, 2),
(40, 1, 4, 1, 2),
(41, 2, 4, 1, 2),
(42, 5, 4, 1, 2),
(43, 10, 4, 1, 2),
(44, 6, 4, 1, 2),
(45, 7, 4, 1, 1),
(46, 50, 4, 1, 1),
(47, 11, 4, 1, 2),
(48, 12, 4, 1, 2),
(49, 55, 4, 1, 1),
(50, 13, 4, 1, 2),
(51, 3, 4, 1, 1),
(52, 14, 4, 1, 2),
(53, 8, 4, 1, 1),
(54, 15, 4, 1, 2),
(55, 16, 4, 1, 1),
(56, 64, 1, 1, 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_accounts_skills`
--

CREATE TABLE `tw_accounts_skills` (
  `ID` int(11) NOT NULL,
  `SkillID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL,
  `Level` int(11) NOT NULL,
  `UsedByEmoticon` int(11) DEFAULT -1
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_accounts_skills`
--

INSERT INTO `tw_accounts_skills` (`ID`, `SkillID`, `UserID`, `Level`, `UsedByEmoticon`) VALUES
(1, 1, 1, 8, -1),
(2, 2, 1, 10, -1),
(3, 5, 1, 4, -1),
(4, 6, 1, 4, -1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_aethers`
--

CREATE TABLE `tw_aethers` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL DEFAULT 'Teleport name',
  `WorldID` int(11) NOT NULL,
  `TeleX` int(11) NOT NULL,
  `TeleY` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_aethers`
--

INSERT INTO `tw_aethers` (`ID`, `Name`, `WorldID`, `TeleX`, `TeleY`) VALUES
(1, 'Crossroad', 2, 8033, 7089),
(2, 'Pier', 0, 3680, 1150),
(3, 'Guard post', 5, 1536, 4396),
(4, 'Yugasaki', 11, 7070, 1980);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_attributs`
--

CREATE TABLE `tw_attributs` (
  `ID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `FieldName` varchar(32) NOT NULL DEFAULT 'unfield',
  `Price` int(11) NOT NULL,
  `Type` int(11) NOT NULL COMMENT '0.tank1.healer2.dps3.weapon4.hard5.jobs 6. others',
  `Divide` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_attributs`
--

INSERT INTO `tw_attributs` (`ID`, `Name`, `FieldName`, `Price`, `Type`, `Divide`) VALUES
(1, 'Shotgun Spread', 'SpreadShotgun', 100, 3, 0),
(2, 'Grenade Spread', 'SpreadGrenade', 100, 3, 0),
(3, 'Rifle Spread', 'SpreadRifle', 100, 3, 0),
(4, 'Strength', 'unfield', 0, 4, 10),
(5, 'Dexterity', 'Dexterity', 1, 2, 5),
(6, 'Crit Dmg', 'CriticalHit', 1, 2, 5),
(7, 'Direct Crit Dmg', 'DirectCriticalHit', 1, 2, 5),
(8, 'Hardness', 'Hardness', 1, 0, 5),
(9, 'Lucky', 'Lucky', 1, 0, 5),
(10, 'Piety', 'Piety', 1, 1, 5),
(11, 'Vampirism', 'Vampirism', 1, 1, 5),
(12, 'Ammo Regen', 'AmmoRegen', 1, 3, 5),
(13, 'Ammo', 'Ammo', 30, 3, 0),
(14, 'Efficiency', 'unfield', -1, 5, 0),
(15, 'Extraction', 'unfield', -1, 5, 0),
(16, 'Hammer Power', 'unfield', -1, 4, 10),
(17, 'Gun Power', 'unfield', -1, 4, 10),
(18, 'Shotgun Power', 'unfield', -1, 4, 10),
(19, 'Grenade Power', 'unfield', -1, 4, 10),
(20, 'Rifle Power', 'unfield', -1, 4, 10),
(21, 'Lucky items', 'unfield', -1, 6, 5);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_info`
--

CREATE TABLE `tw_bots_info` (
  `ID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL DEFAULT 'Bot name',
  `SkinName` varchar(128) NOT NULL DEFAULT '''bear standard standard standard standard standard''' COMMENT 'body marking deco hands feet eyes',
  `SkinColor` varchar(128) NOT NULL DEFAULT '''0 0 0 0 0 0''' COMMENT 'body marking deco hands feet eyes	',
  `SlotHammer` int(11) DEFAULT NULL,
  `SlotGun` int(11) DEFAULT NULL,
  `SlotShotgun` int(11) DEFAULT NULL,
  `SlotGrenade` int(11) DEFAULT NULL,
  `SlotRifle` int(11) DEFAULT NULL,
  `SlotWings` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--
-- Дамп данных таблицы `tw_bots_info`
--

INSERT INTO `tw_bots_info` (`ID`, `Name`, `SkinName`, `SkinColor`, `SlotHammer`, `SlotGun`, `SlotShotgun`, `SlotGrenade`, `SlotRifle`, `SlotWings`) VALUES
(1, 'Captain', 'puar warstripes hair standard standard standardreal', '16187160 -16645889 131327 16177260 7624169 65408', 10010, NULL, NULL, NULL, NULL, NULL),
(2, 'Sailor', 'standard bear hair standard standard standard', '1082745 -15634776 65408 1082745 1147174 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(3, 'Carpenter', 'puar tiger2  duotone standard moustache', '15007529 -14563483 65408 3827951 1310537 65422', NULL, NULL, NULL, NULL, NULL, NULL),
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
(14, 'Nurse', 'flokes downdony hair duotone standard standard', '15920331 1593835335 4147122 3795484 16279737 10976418', NULL, NULL, NULL, NULL, NULL, NULL),
(15, 'Gunsmith Eric', 'kitty cammostripes hair duotone standard standard', '5209108 1463395762 12628238 8169037 3830859 2771259', NULL, NULL, NULL, NULL, NULL, NULL),
(16, 'Mr. Sentry', 'raccoon tripledon hair duotone standard standard', '10060032 1690185928 11278269 5677608 886610 13831075', NULL, NULL, NULL, NULL, NULL, NULL),
(18, 'Daughter Sailor', 'kitty whisker hair duotone standard standard', '3981441 -18874784 5313052 14500779 8614329 6321790', NULL, NULL, NULL, NULL, NULL, NULL),
(19, 'Miner', 'standard toptri hair duotone standard colorable', '16718165 -2012161487 3858792 7940502 3052250 1873584', NULL, NULL, NULL, NULL, NULL, NULL),
(20, 'Customs officer', 'kitty cammo1 hair duotone standard standardreal', '6083870 -2125570185 6324018 7470633 16541982 2000684', NULL, NULL, NULL, NULL, NULL, NULL),
(21, 'Seller artifact', 'flokes saddo hair duotone standard colorable', '13769006 -35537528 12473004 465468 14009515 803609', NULL, NULL, NULL, NULL, NULL, NULL),
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
(34, 'Kappa', 'standard sidemarks unibop duotone standard colorable', '4821534 821909703 1852219 1132310 5264192 15588285', NULL, NULL, NULL, NULL, NULL, NULL),
(35, 'Orc warrior', 'fox cammostripes  standard zilly!0007 zilly!0007', '65408 -16711808 65408 65408 65408 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(36, 'Brown slime', 'bear duodonny hair duotone standard standard', '1091872 -394592059 12674866 8116231 1151 14198436', NULL, NULL, NULL, NULL, NULL, NULL),
(37, 'Officer Henry', 'flokes warstripes hair2 duotone standard standard', '5482497 -1837124528 15809857 10752857 7539007 3358899', NULL, NULL, NULL, NULL, NULL, NULL),
(38, 'Daughter Maria', 'flokes duodonny hair duotone standard standardreal', '2653584 -36345458 3470770 9908420 14444105 738196', NULL, NULL, NULL, NULL, NULL, NULL),
(39, 'Noctis', 'kitty mice hair duotone standard standard', '11017984 1596522751 5772960 14417927 5570560 9381961', 10010, NULL, NULL, NULL, NULL, 10017),
(40, 'Chocobo', 'kitty bear hair duotone standard standard', '2490221 -1792213144 2096979 2157678 1735229 1103953', NULL, NULL, NULL, NULL, NULL, NULL),
(41, 'Benny', 'bear tricircular hair duotone standard colorable', '16066652 -1087601154 3829858 1314019 641021 1376511', NULL, NULL, NULL, NULL, NULL, NULL),
(42, 'Rogalia', 'monkey donny unibop duotone standard colorable', '8347469 -852808362 15666037 15456343 5845918 7003270', NULL, NULL, NULL, NULL, NULL, NULL),
(43, 'Kengo', 'monkey donny unibop duotone standard colorable', '8347469 -852808362 15666037 15456343 5845918 7003270', NULL, NULL, NULL, NULL, NULL, NULL),
(44, 'Goshii', 'bear whisker hair2 duotone standard standard', '10571316 -1915300733 1789119 12372457 338092 9300965', 10019, 10020, 10021, 10022, 10023, 10018),
(45, 'Maid', 'flokes downdony twinmello duotone standard standardreal', '4260095 -1698513476 10819474 1784648 14990515 10338195', NULL, NULL, NULL, NULL, NULL, NULL),
(46, 'Yasue San', 'mouse flokes unibop duotone standard colorable', '8925949 -1578020608 9437184 14815652 862504 8671829', NULL, NULL, NULL, NULL, NULL, NULL),
(47, 'Dead soul', 'bear sidemarks horns duotone standard standardreal', '692505 -1831891179 13771590 7098987 3994129 8711018', NULL, NULL, NULL, NULL, NULL, NULL),
(48, 'Skeleton', 'standard mixture1 twinpen duotone standard colorable', '9830655 1328873727 4260095 11171578 11796735 4135487', NULL, NULL, NULL, NULL, NULL, NULL),
(49, 'Librarian', 'trela bear twinmello duotone standard sunglasses', '11686168 -1962207864 7675723 14445598 12185649 15772890', NULL, NULL, NULL, NULL, NULL, NULL),
(50, 'Neptune', 'spiky cammo1 twinmello duotone standard standardreal', '7526428 -1490205206 9650234 16143024 5143432 7378889', 10019, 10020, 10021, 10022, 10023, 10006),
(51, 'Farmer', 'standard cammostripes  standard standard standard', '1821867 -14840320 65408 750848 1944919 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(53, 'Master', 'puar mice  duotone standard moustache', '11763922 -15685931 65408 1102450 1232060 1376256', NULL, NULL, NULL, NULL, NULL, NULL),
(54, 'Visitor', 'koala twinbelly  standard standard standard', '184 -15397662 65408 184 9765959 65408', NULL, NULL, NULL, NULL, NULL, NULL),
(55, 'Chef', 'standard   standard standard moustache', '1572863 -16711808 65408 1572863 1572863 65408', NULL, NULL, NULL, NULL, NULL, NULL);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_mobs`
--

CREATE TABLE `tw_bots_mobs` (
  `ID` int(11) NOT NULL,
  `BotID` int(11) NOT NULL DEFAULT -1,
  `WorldID` int(11) DEFAULT NULL,
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Effect` varchar(16) DEFAULT NULL,
  `Behavior` varchar(32) NOT NULL DEFAULT 'Standard',
  `Level` int(11) NOT NULL DEFAULT 1,
  `Power` int(11) NOT NULL DEFAULT 10,
  `Spread` int(11) NOT NULL DEFAULT 0,
  `Number` int(11) NOT NULL DEFAULT 1,
  `Respawn` int(11) NOT NULL DEFAULT 1,
  `Boss` tinyint(1) NOT NULL DEFAULT 0,
  `it_drop_0` int(11) DEFAULT NULL,
  `it_drop_1` int(11) DEFAULT NULL,
  `it_drop_2` int(11) DEFAULT NULL,
  `it_drop_3` int(11) DEFAULT NULL,
  `it_drop_4` int(11) DEFAULT NULL,
  `it_drop_count` varchar(64) NOT NULL DEFAULT '[0][0][0][0][0]',
  `it_drop_chance` varchar(64) NOT NULL DEFAULT '[0][0][0][0][0]'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_bots_mobs`
--

INSERT INTO `tw_bots_mobs` (`ID`, `BotID`, `WorldID`, `PositionX`, `PositionY`, `Effect`, `Behavior`, `Level`, `Power`, `Spread`, `Number`, `Respawn`, `Boss`, `it_drop_0`, `it_drop_1`, `it_drop_2`, `it_drop_3`, `it_drop_4`, `it_drop_count`, `it_drop_chance`) VALUES
(1, 36, 1, 4049, 890, 'Slowdown', 'Slime', 2, 8, 0, 5, 5, 0, 22, 29, NULL, NULL, NULL, '|1|1|0|0|0|', '|1.08|1.08|0|0|0|'),
(2, 23, 3, 3057, 2577, 'Slowdown', 'Slime', 4, 18, 0, 12, 5, 0, 29, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|3.2|0|0|0|0|'),
(3, 9, 3, 1890, 1160, 'Slowdown', 'Slime', 2, 10, 0, 12, 5, 0, 22, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|3.2|0|0|0|0|'),
(4, 28, 3, 3057, 2577, 'Slowdown', 'Slime', 10, 240, 0, 1, 320, 1, 29, 22, NULL, NULL, NULL, '|3|3|0|0|0|', '|100|100|0|0|0|'),
(5, 22, 5, 1345, 2600, NULL, 'Standard', 8, 15, 1, 14, 5, 0, 30, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|1.56|0|0|0|0|'),
(7, 29, 6, 2825, 2430, NULL, 'Standard', 12, 50, 1, 10, 1, 0, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|', '|0|0|0|0|0|'),
(8, 30, 6, 4840, 2560, 'Fire', 'Standard', 12, 80, 2, 8, 1, 0, 30, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|4.56|0|0|0|0|'),
(9, 31, 6, 1150, 3700, 'Poison', 'Standard', 12, 50, 1, 10, 1, 0, 42, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|3.47|0|0|0|0|'),
(10, 22, 6, 1440, 5100, NULL, 'Standard', 12, 60, 1, 10, 1, 0, 30, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|3.96|0|0|0|0|'),
(11, 32, 6, 3960, 4595, 'Fire', 'Standard', 15, 800, 1, 1, 1, 1, 37, 42, NULL, NULL, NULL, '|1|1|0|0|0|', '|50|75|0|0|0|'),
(12, 34, 3, 1570, 2915, NULL, 'Standard', 6, 27, 0, 8, 5, 0, 35, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|4.1|0|0|0|0|'),
(13, 35, 5, 1345, 2600, NULL, 'Standard', 14, 620, 1, 1, 400, 1, 30, 42, NULL, NULL, NULL, '|1|1|0|0|0|', '|100|25|0|0|0|'),
(14, 40, 8, 3665, 390, 'Fire', 'Standard', 18, 100, 1, 10, 1, 0, 44, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|7.5|0|0|0|0|'),
(15, 41, 8, 5610, 2865, 'Poison', 'Standard', 19, 120, 1, 10, 1, 0, 44, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|3.5|0|0|0|0|'),
(16, 42, 8, 2400, 3150, 'Fire', 'Standard', 20, 110, 1, 10, 1, 0, 44, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|5.25|0|0|0|0|'),
(17, 43, 8, 1720, 3180, 'Fire', 'Standard', 20, 115, 1, 10, 1, 0, 44, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|7.5|0|0|0|0|'),
(18, 44, 8, 2800, 1640, 'Fire', 'Standard', 25, 2090, 1, 1, 1, 1, 44, NULL, NULL, NULL, NULL, '|3|0|0|0|0|', '|100|0|0|0|0|'),
(19, 47, 10, 2030, 1260, 'Fire', 'Standard', 15, 90, 2, 16, 1, 0, 46, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|2.57|0|0|0|0|'),
(20, 48, 10, 4446, 577, 'Fire', 'Standard', 16, 110, 2, 16, 1, 0, 46, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|2.57|0|0|0|0|'),
(21, 49, 10, 61, 1539, 'Fire', 'Sleepy', 16, 120, 2, 8, 1, 0, 46, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|3.67|0|0|0|0|'),
(22, 50, 10, 6545, 700, 'Fire', 'Standard', 20, 1820, 3, 1, 1, 1, 45, NULL, NULL, NULL, NULL, '|1|0|0|0|0|', '|50|0|0|0|0|');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_npc`
--

CREATE TABLE `tw_bots_npc` (
  `ID` int(11) NOT NULL,
  `BotID` int(11) NOT NULL DEFAULT -1,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `GivesQuestID` int(11) DEFAULT NULL,
  `DialogData` varchar(4096) NOT NULL DEFAULT 'empty',
  `Function` int(11) NOT NULL DEFAULT -1,
  `Static` int(11) NOT NULL,
  `Emote` int(11) NOT NULL DEFAULT 0 COMMENT '1.Pain 2.Happy 3.Surprise 4.Angry 5.Blink	',
  `Number` int(11) NOT NULL DEFAULT 1,
  `WorldID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_bots_npc`
--

INSERT INTO `tw_bots_npc` (`ID`, `BotID`, `PosX`, `PosY`, `GivesQuestID`, `DialogData`, `Function`, `Static`, `Emote`, `Number`, `WorldID`) VALUES
(1, 1, 1985, 977, NULL, '[{\"text\":\"**He said surprised**: What a wonderful weather today!\",\"emote\":\"happy\"},{\"text\":\"[p]Yes, I think so too.\",\"emote\":\"happy\"},{\"text\":\"Well, how do you like on our ship [Player]? Captures the ocean, i think I\'ll stay here a little longer, it\'s wonderful here.\"},{\"text\":\"[p]I also really like it. Okay, I\'ll go, I wish you a good rest [Talked]!\"},{\"text\":\"And I wish you good luck too [Player]!\",\"emote\":\"happy\"}]', -1, 1, 2, 1, 0),
(2, 2, 1022, 1073, NULL, '[{\"text\":\"**He looks very tired**: Very hot, impossible to work.\",\"emote\":\"pain\"}]', -1, 0, 5, 2, 0),
(3, 3, 2691, 1009, 1, '[{\"emote\":\"blink\",\"text\":\"[p]Hello, I arrived from a small village beyond the mountains and the ocean, my name is [Player], I have a desire to become an adventurer. But I do not know about this place, could you help me?\"},{\"text\":\"Hello [Player], Yes of course I will help you, explain where you need to go, but you help my workers, they do not cope at all.\"},{\"emote\":\"happy\",\"text\":\"[p]What you need help with, I\'m good at dealing with predators without meat you will not leave! *laughter*\"},{\"emote\":\"blink\",\"text\":\"Oh, you\'re a Joker, no thanks go a little further guys will explain everything to you!\"},{\"text\":\"[p]OK, thanks!\"}]', -1, 1, 0, 1, 0),
(4, 4, 5312, 1073, NULL, 'empty', -1, 1, 0, 1, 0),
(5, 11, 10092, 8561, NULL, 'empty', -1, 0, 0, 1, 2),
(6, 12, 5693, 8369, NULL, 'empty', -1, 0, 0, 1, 2),
(7, 13, 6471, 7569, NULL, 'empty', -1, 0, 0, 1, 2),
(8, 14, 6451, 7345, NULL, 'empty', 0, 1, 2, 1, 2),
(9, 15, 9464, 6833, 55, '[{\"text\":\"Hey, pst! You, yes, you!\",\"emote\":\"angry\"},{\"text\":\"[p]Yeah? How can I help?\",\"emote\":\"blink\"},{\"text\":\"You cannot help me, kid! I just need...to TEST you, yes...!\",\"emote\":\"blink\"}]', -1, 0, 4, 1, 2),
(10, 16, 264, 1009, 3, '[{\"text\":\"[p]Hi my name is [Player], [Talked] are you working?\",\"emote\":\"blink\"},{\"text\":\"Hello [Player], Yes, I am on duty as a guard, and I also trade a little\"},{\"text\":\"[p]Can you tell me how difficult it is to get into the service?\",\"emote\":\"blink\"},{\"text\":\"If that\'s what you want, it\'s easy. And if you don\'t then you won\'t be able to get in ;)\",\"emote\":\"happy\"},{\"text\":\"By the way [Player], I need help, there are slugs nearby, again raging, and I can\'t leave the post. At the same time you will test your skills young adventurer.\",\"emote\":\"blink\"},{\"text\":\"[p]Yes, no problem, wait [Talked]!!\",\"emote\":\"happy\"}]', -1, 0, 4, 1, 1),
(11, 2, 1234, 689, 5, '[{\"text\":\"**He says with a look of horror on his face**: Help me find my daughter, she went to the village for food and never returned.\",\"emote\":\"pain\"},{\"text\":\"[p]To the village? [Talked] Yes of course I will help. That\'s where I need to go!\"},{\"text\":\"Thanks a lot\",\"emote\":\"pain\"}]', -1, 1, 1, 1, 0),
(12, 14, 419, 1009, NULL, 'empty', 0, 1, 2, 1, 1),
(13, 19, 5739, 7473, 6, '[{\"text\":\"Hello, would you like to learn the basics of mining?\"},{\"text\":\"[p]Yes, of course, I don\'t mind\"},{\"text\":\"All right follow me I\'ll explain the basics to you!\"}]', -1, 1, 0, 1, 2),
(14, 20, 3759, 8209, 10, '[{\"text\":\"Hello, and so you\'re new here?\"},{\"text\":\"[p]Yes.\",\"emote\":\"blink\"},{\"text\":\"I need to register you. Please introduce yourself, and the purpose of your arrival, and the time you to stay here\"},{\"text\":\"[p]My name is [Player]. I came here to become an adventurer. I can\'t tell you the exact time of stay.\"},{\"text\":\"And who for you should know the time of your stay here.\",\"emote\":\"angry\"},{\"text\":\"Okay well, I will write you down, in case of violation of the rules of our village, we have every right to prohibit your presence here, and report to superiors.\",\"emote\":\"blink\"},{\"text\":\"Have a good day!\",\"emote\":\"happy\"},{\"text\":\"Thank\'s [Talked]!\",\"emote\":\"blink\",\"action_step\":1}]', -1, 1, 0, 1, 2),
(15, 21, 6218, 6417, NULL, 'empty', -1, 1, 0, 1, 2),
(16, 27, 4590, 977, 8, '[{\"text\":\"[Stranger]Greetings, wanderer. *bowing*\"},{\"text\":\"I am the Deputy of the Apostle, and I also protect her.\"},{\"text\":\"I think you\'ve already heard, that the monsters are raging.\"},{\"text\":\"I have a request for you [Player]\"},{\"text\":\"[p]Which one?\"},{\"text\":\"Help in the southern part to win over the monsters. We can\'t drive them away but we can scare them away.\",\"emote\":\"blink\"},{\"text\":\"[p]Of course I will.\"},{\"text\":\"Thank\'s [Player]!\",\"emote\":\"happy\"}]', -1, 1, 2, 1, 4),
(17, 14, 1448, 4433, NULL, 'empty', 0, 1, 2, 1, 5),
(18, 37, 7781, 7921, 50, '[{\"text\":\"[p]You look awful today, [Talked]! What happend?\"},{\"text\":\"Oh.. don\'t you worry about me, boy..\",\"emote\":\"angry\"},{\"text\":\"[p]But I DO worry!\",\"emote\":\"blink\"},{\"text\":\"I\'m tired of these fight I have with my wife, it\'s personal bussiness.\",\"emote\":\"blink\"},{\"text\":\"[p]I didn\'t want to be a pain to you, Officer. I will leave you to your problems now.\",\"emote\":\"blink\"},{\"text\":\"No! Wait!\",\"emote\":\"blink\"}]', -1, 1, 4, 1, 2),
(19, 39, 2851, 3473, 60, '[{\"text\":\"Hello [Player], I have come to you from the Final Fantasy universe, my name is [Talked].\"},{\"text\":\"I can\'t say for sure how long I will be here, but for now I will be happy to know your world, and I will be happy to show my world\"},{\"text\":\"[p]Are you serious? Did the author smoke dope?\",\"emote\":\"blink\"},{\"text\":\"Maybe so, I have a couple of things that you can get from me, but not for free. I\'ll need the fragments I lost\"}]', -1, 1, 3, 1, 5),
(20, 51, 9335, 4881, NULL, 'empty', -1, 1, 0, 1, 2),
(21, 53, 7282, 4880, NULL, 'empty', -1, 1, 0, 1, 2),
(22, 13, 7042, 1410, NULL, 'empty', -1, 0, 2, 1, 11),
(23, 14, 7048, 884, NULL, 'empty', 0, 0, 2, 1, 11),
(24, 12, 7101, 1169, NULL, 'empty', -1, 1, 5, 1, 11),
(25, 11, 5710, 1937, NULL, '[{\"text\":\"Welcome home, sir! **she made a bow**\",\"emote\":\"happy\"}]', -1, 0, 4, 1, 11),
(26, 45, 3800, 1521, NULL, 'empty', -1, 1, 2, 1, 11),
(27, 1, 1378, 1073, NULL, '[{\"text\":\"Wow, when i\'m at the helm, feel young.\",\"emote\":\"happy\"},{\"text\":\"[p]A man who is not even 25 told me...\",\"emote\":\"blink\"},{\"text\":\"Oh no, I\'ve been over 50 for a long time... But thank you, it\'s nice to hear. Okay, I\'ll keep an eye on the direction, don\'t bother me.\",\"emote\":\"blink\"}]', -1, 1, 5, 1, 9),
(28, 2, 2781, 1105, 64, '[{\"text\":\"I feel that there will be a storm.\"},{\"text\":\"[p]Do you feel it? How can you feel it?\",\"emote\":\"surprise\"},{\"text\":\"I\'ve been sailing on a ship for a long time, and I\'ve been to many places. And i can already tell it by feeling.\"},{\"text\":\"[p]I see, hope not a strong storm awaits us?\"},{\"text\":\"I don\'t know for sure, but I have a hunch. Go to [Bot_1] and clarify, he is much more experienced than me.\"}]', -1, 1, 0, 1, 9),
(29, 46, 2705, 1265, NULL, 'empty', -1, 1, 0, 1, 9),
(30, 54, 1762, 1137, NULL, 'empty', -1, 0, 5, 1, 9),
(31, 54, 2501, 1265, NULL, 'empty', -1, 0, 5, 2, 9),
(32, 55, 2476, 1137, NULL, 'empty', -1, 1, 5, 1, 9);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_bots_quest`
--

CREATE TABLE `tw_bots_quest` (
  `ID` int(11) NOT NULL,
  `BotID` int(11) NOT NULL DEFAULT -1,
  `QuestID` int(11) NOT NULL DEFAULT -1,
  `Step` int(11) NOT NULL DEFAULT 1,
  `WorldID` int(11) DEFAULT NULL,
  `GenerateSubName` tinyint(4) NOT NULL DEFAULT 0,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `DialogData` varchar(4096) NOT NULL DEFAULT 'empty',
  `RequiredItemID1` int(11) DEFAULT NULL,
  `RequiredItemID2` int(11) DEFAULT NULL,
  `RewardItemID1` int(11) DEFAULT NULL,
  `RewardItemID2` int(11) DEFAULT NULL,
  `RequiredDefeatMobID1` int(11) DEFAULT NULL,
  `RequiredDefeatMobID2` int(11) DEFAULT NULL,
  `Amount` varchar(64) NOT NULL DEFAULT '|0|0|0|0|0|0|',
  `InteractionType` int(11) DEFAULT NULL,
  `InteractionTemp` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_bots_quest`
--

INSERT INTO `tw_bots_quest` (`ID`, `BotID`, `QuestID`, `Step`, `WorldID`, `GenerateSubName`, `PosX`, `PosY`, `DialogData`, `RequiredItemID1`, `RequiredItemID2`, `RewardItemID1`, `RewardItemID2`, `RequiredDefeatMobID1`, `RequiredDefeatMobID2`, `Amount`, `InteractionType`, `InteractionTemp`) VALUES
(1, 5, 1, 1, 0, 0, 3925, 1169, '[{\"text\":\"[p]Hello, I was sent to you by a carpenter to help you.\"},{\"text\":\"**He asks with a smile**: Hello, have you just arrived?\"},{\"text\":\"[p]Yes, I came to you to become an adventurer, I need money.\"},{\"text\":\"Well, I\'ll tell you [Player], things have been very bad here lately, the residents are living in fear. Recently, a little girl was killed, as we understand there is a dead force involved.\",\"emote\":\"pain\"},{\"text\":\"[p]**With horror in his eyes**: What ... Dead People\",\"emote\":\"surprise\"},{\"text\":\"**He said hurriedly**: I don\'t know for sure. Over time, the residents will tell you everything, or you will feel for yourself what is happening there.\"},{\"text\":\"[p]I didn\'t expect it to be this bad. Well, what can I help you with?\",\"emote\":\"blink\",\"action_step\":1},{\"text\":\"Come with me, our guys dropped boards, help me collect them.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(2, 5, 1, 2, 0, 0, 5599, 1009, '[{\"text\":\"Okay generally help me collect all the boards! I will reward you for your help.\",\"action_step\":1},{\"text\":\"[p]**Weakness in asks**: Oh, hell, they\'re heavy, I\'ve got them all.\",\"emote\":\"pain\"},{\"text\":\"Great job! Your muscles won\'t hurt you anyway [Player] *laughter*\",\"emote\":\"happy\"},{\"text\":\"[p]*laughter*\",\"emote\":\"happy\"},{\"text\":\"Now hand them out to my boys and let them go on with their business.\"},{\"text\":\"[p]Good [Talked]!\"}]', 23, NULL, 23, NULL, NULL, NULL, '|12|0|12|0|0|0|', 2, NULL),
(3, 4, 1, 3, 0, 1, 4121, 1137, '[{\"text\":\"[p]Here you go, your boss told me to give it to you!\",\"action_step\":1},{\"text\":\"Thanks for the help.\"}]', 23, NULL, NULL, NULL, NULL, NULL, '|4|0|0|0|0|0|', 2, NULL),
(4, 4, 1, 3, 0, 1, 6489, 1137, '[{\"text\":\"[p]Here you go, your boss told me to give it to you!\",\"action_step\":1},{\"text\":\"Thanks for the help.\"}]', 23, NULL, NULL, NULL, NULL, NULL, '|4|0|0|0|0|0|', 2, NULL),
(5, 4, 1, 3, 0, 1, 2430, 977, '[{\"text\":\"[p]Here you go, your boss told me to give it to you!\",\"action_step\":1},{\"text\":\"Thanks for the help.\"}]', 23, NULL, NULL, NULL, NULL, NULL, '|4|0|0|0|0|0|', 2, NULL),
(6, 5, 1, 4, 0, 0, 6742, 1041, '[{\"text\":\"[p]Is tired. But I did it!\",\"emote\":\"pain\"},{\"text\":\"Well done, take this. I think you need it if you\'re going to be an adventurer. Go straight, and you will go straight to the residents, if you ask where to go.\",\"emote\":\"happy\",\"action_step\":1},{\"text\":\"[p]**With a smile**: Well thank you, good luck with your work!\",\"emote\":\"happy\"}]', NULL, NULL, 3, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(7, 6, 2, 1, 1, 0, 841, 977, '[{\"text\":\"[p][Stranger]Hi, my name is [Player], how do I find the nearest village?\",\"emote\":\"pain\"},{\"text\":\"**She concerned**: Hello, my name is [Talked].\",\"emote\":\"happy\"},{\"text\":\"I\'ll be happy to help you settle in, but could you give my brother a note? He works now on the ship that you arrived on, i could have done it myself, but they won\'t let me in.\",\"emote\":\"pain\"},{\"text\":\"[p]Yes, of course i help, will you be here [Talked]?\",\"emote\":\"happy\",\"action_step\":1},{\"text\":\"I\'ll be here waiting!\"}]', NULL, NULL, 21, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(8, 8, 2, 2, 0, 0, 411, 1009, '[{\"text\":\"[p]Hello, are you Diana\'s brother?\",\"emote\":\"pain\"},{\"text\":\"**He said rudely**: Yes. Did something happen to her?..\",\"emote\":\"surprise\"},{\"text\":\"[p]No, don\'t worry, she just asked me to pass you the notebook.\",\"emote\":\"pain\",\"action_step\":1},{\"text\":\"Well, thank you very much, and tell her I\'ll be seeing her this evening.\",\"emote\":\"happy\"},{\"text\":\"[p]Yes, good luck with your work!\",\"emote\":\"happy\"}]', 21, NULL, NULL, NULL, NULL, NULL, '|1|0|0|0|0|0|', NULL, NULL),
(9, 6, 2, 3, 1, 0, 841, 977, '[{\"text\":\"**She says impatiently**: Oh, you\'re already here, you\'re fast!\"},{\"text\":\"[p]Really?\",\"emote\":\"happy\"},{\"text\":\"Yes. How is he?\"},{\"text\":\"[p]He told me that he is busy now, and will visit you later in the evening!\"},{\"text\":\"I\'m so happy! They won\'t let me in because of the problems that are happening. Dead men kidnap girls, and then they find them dead.\",\"emote\":\"pain\"},{\"text\":\"[p]**He sounds surprised**: Only girls? Why the dead?\",\"emote\":\"blink\"},{\"text\":\"**She\'s upset**: Honestly, I don\'t know. The dead or who it is, our headman says that most likely the dead ;(\",\"emote\":\"pain\"},{\"text\":\"[p]I came to you to become an adventurer, as I know you have magical creatures.\"},{\"text\":\"**She sounds like she knows it**: An adventurer? We have a Guild of adventurers, but to become an adventurer you need to pass the tests.\",\"emote\":\"surprise\"},{\"text\":\"[p]Tests?....\",\"emote\":\"blink\"},{\"text\":\"Yes the test [Player]. First, you will need to collect the minimum equipment. To apply and pay the fee. And only after that you will be assigned the position of adventurer.\"},{\"text\":\"[p]Is everything so strict? Yes, I think I can handle it.\",\"emote\":\"happy\"},{\"text\":\"All right come on I\'ll show you our village [Player].\",\"emote\":\"happy\",\"action_step\":1},{\"text\":\"[p]Yes come on ;)\",\"emote\":\"happy\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(10, 6, 2, 4, 1, 0, 2207, 465, '[{\"text\":\" **She made it sound like there was something wrong with them**: Oh, my God, where are the slime from?.\",\"emote\":\"pain\"},{\"text\":\"[p]I don\'t know [Talked], here even have slimes. Wahoo\",\"emote\":\"blink\"},{\"text\":\"Well, Yes [Player], but now they are becoming more frequent\",\"emote\":\"pain\"},{\"text\":\"[p]**You speak proudly**: All right [Talked], I\'ll clear the way!\",\"emote\":\"happy\",\"action_step\":1},{\"text\":\"[p]**You said in disgust**: Phew, the slime is so disgusting...\",\"emote\":\"angry\"},{\"text\":\"**She says with a smile**: Well you could use a bath [Player] ;)\",\"emote\":\"happy\"},{\"text\":\"[p]I will not refuse such an honor ;)\",\"emote\":\"happy\"},{\"text\":\"Ah here is and decided, we\'ll get there I\'ll show you where I live. And I\'ll introduce you to the places. And you wash :)\"},{\"text\":\"[p]All right, shall we continue on our way?\",\"emote\":\"surprise\"},{\"text\":\"Yes let\'s go\"}]', NULL, NULL, NULL, NULL, 36, NULL, '|0|0|0|0|12|0|', NULL, NULL),
(11, 16, 3, 1, 1, 0, 525, 1009, '[{\"text\":\"**He said with surprise**: Did you defeat the slugs on the instructions of our master? Take this the master asked me to give you.\",\"action_step\":1},{\"text\":\"[p]Yes, I could. They were very slippery.\",\"emote\":\"happy\"},{\"text\":\"Hehe.. What did you think?\",\"emote\":\"happy\"},{\"text\":\"[p]Well, not so difficult, at the same time I warmed up.\"},{\"text\":\"Okay I need to serve, good luck to you, don\'t give up ;)\",\"emote\":\"happy\"}]', NULL, NULL, 15, 14, 36, NULL, '|0|0|3|3|16|0|', NULL, NULL),
(12, 18, 5, 1, 2, 0, 5215, 7409, '[{\"text\":\"**She said very rudely**: Who the hell are you? What do you want?\",\"emote\":\"angry\"},{\"text\":\"[p]Do you know that your father is looking for you?\"},{\"text\":\"I hate this old guy, he doesn\'t have time for me......\",\"emote\":\"angry\"},{\"text\":\"He can only go there. Do it. And we can\'t spend time together...\",\"emote\":\"angry\"},{\"text\":\"Enrages...\",\"emote\":\"angry\"},{\"text\":\"[p]Calm down let\'s go back together and tell him everything? Maybe he just cares about you like that.\",\"emote\":\"happy\",\"action_step\":1},{\"text\":\"All right, let\'s go back.\",\"emote\":\"pain\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(13, 18, 5, 2, 0, 0, 2390, 977, '[{\"text\":\"[p]Are you ready?\",\"action_step\":1},{\"text\":\"Yes..\",\"emote\":\"pain\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(14, 2, 5, 2, 0, 0, 2162, 977, '[{\"emote\":\"angry\",\"text\":\"So where was she?.\"},{\"text\":\"[p]You just don\'t get angry, everything\'s fine!\"},{\"text\":\"[p]In fact, she didn\'t want to come back, she was offended that you weren\'t spending time together. You can\'t rest.\"},{\"action_step\":1,\"emote\":\"surprise\",\"text\":\"Damn why didn\'t she ask for it herself? Okay, don\'t worry thanks for the work :) Everything will be fine, we will decide something with daughter\"},{\"emote\":\"happy\",\"text\":\"[p]Well, nice :) Good luck to you\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(15, 19, 6, 1, 2, 0, 7915, 8401, '[{\"text\":\"To extract any ore [Player], you will need a pickaxe.\"},{\"text\":\"You can only get it by creating. But in some cases merchants can sell.\"},{\"text\":\"Also, the picks have a durability of 100-0. If you durability reach 0. Don\'t worry it won\'t break.\"},{\"text\":\"But you can\'t work with break pickaxe it. You can repair it at any store.\"},{\"text\":\"You can also improve picks this will depend on their effectiveness.\"},{\"text\":\"Each ore has its own strength. If you have a weak pick and the strength of the ore is high, it can take a long time to extract it. So don\'t forget about the improvements. Or craft even better picks.\"},{\"text\":\"Everything is clearly explained, I will not repeat it again, if you need you will understand everything!\"},{\"action_step\":1,\"text\":\"[p]Yes of course thank you for the explanation\"},{\"text\":\"Well don\'t worry, I\'ll give you a couple of tasks to get used to in this area\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(16, 19, 7, 1, 2, 0, 9859, 8561, '[{\"text\":\"[p]Did you want something?\"},{\"text\":\"I think it\'s time for you to get your first pickaxe.\"},{\"text\":\"I think as crafted something, you don\'t need to explain!\"},{\"text\":\"So let\'s get right to the point, try to make a pickaxe bring it to me, I\'ll look at it.\"},{\"action_step\":1,\"text\":\"[p]Done. (you need to have an idea of how to craft items, you can see them near Craftsman in votes)\"},{\"text\":\"Well, it\'s not bad, so use it for the first time.\"},{\"text\":\"All right so let\'s start with your first ore?\"}]', 26, NULL, 26, NULL, NULL, NULL, '|1|0|1|0|0|0|', NULL, NULL),
(17, 6, 10, 1, 2, 0, 6615, 8433, '[{\"emote\":\"happy\",\"text\":\"Well, as [Player], passed customs?\"},{\"emote\":\"blink\",\"text\":\"[p]Yes [Talked]. He was very rude.\"},{\"text\":\"Well, you have to understand that we also have a lot of problems lately, too many monsters, problems with kidnappings.\"},{\"action_step\":1,\"text\":\"[p]Come on, show me [Here] seats!\"},{\"text\":\"Oh sure. By the way, in our village, the chief Apostle.\"},{\"emote\":\"surprise\",\"text\":\"[p]Wait an Apostle? They are ancient beings sent by the gods. Do they exist?\"},{\"text\":\"Yes [Player]. They exist to keep order.\"},{\"emote\":\"blink\",\"text\":\"Okay [Player] let\'s go and show you our [Here]\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(18, 6, 10, 2, 2, 0, 9953, 8561, '[{\"emote\":\"happy\",\"text\":\"**She said with a smile**: Here, you can do a craft items, make your own equipment potions, or what to eat.\"},{\"text\":\"[Player] even if you provide items, the craftsman you must pay for the use of the equipment\"},{\"text\":\"[p]Yes I think I can use it. Thanks for the explanation!\"},{\"action_step\":1,\"text\":\"Let\'s move on!\"},{\"emote\":\"happy\",\"text\":\"[p]Yes let\'s go :)\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(19, 6, 10, 3, 2, 0, 8574, 7665, '[{\"text\":\"So here we have someone\'s house.\"},{\"text\":\"You can also buy houses, decorate them or use the safety Deposit box.\"},{\"action_step\":1,\"text\":\"[p]Sounds good [Talked]!\"},{\"emote\":\"happy\",\"text\":\"**She likes to talk**: Well, okay, let\'s move on!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(20, 6, 10, 4, 2, 0, 8123, 7089, '[{\"text\":\"This is the ether, it allows you to use instantaneous movement around the world.\"},{\"text\":\"But only there in the area where you have already visited.\"},{\"text\":\"[p]Is it free [Talked]?\"},{\"text\":\"Not always, there is nothing free in our world!\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"**Apparently she didn\'t like something**: Okay [Player] let\'s move on!\"},{\"text\":\"[p]Yes let\'s go [Talked]!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(21, 6, 10, 5, 2, 0, 6815, 7569, '[{\"text\":\"Here you will be able to raise his skills adventurer, to explore something new. Our world is divided into 3 classes (Healer, Tank, DPS)\"},{\"emote\":\"surprise\",\"text\":\"[p]So I can become something?\"},{\"emote\":\"blink\",\"text\":\"[Player], you can be anyone you want (as well as fine-tune your game mode)\"},{\"emote\":\"blink\",\"text\":\"[p][Talked] so I can be both a Tank and a Defender?\"},{\"text\":\"Yes quite but do not forget about the main characteristics (Strength, Hardness, idk)\"},{\"action_step\":1,\"emote\":\"happy\",\"text\":\"[p]Thank you for a clear explanation :)\"},{\"emote\":\"blink\",\"text\":\"That\'s a small part of what I\'ve been able to tell you.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(22, 6, 10, 6, 2, 0, 6364, 7345, '[{\"text\":\"And here, you will be treated. \"},{\"emote\":\"blink\",\"text\":\"**She said very jealously**: True, I hate those nurses, all the guys from the village are taken away....\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"Don\'t marry them...\"},{\"emote\":\"happy\",\"text\":\"[p]Yes I will not marry anyone [Talked]! Thanks for the care!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(23, 6, 10, 7, 2, 0, 5021, 7441, '[{\"emote\":\"blink\",\"text\":\"**Yawning she said**: Okay I\'m tired I\'ll go home.\"},{\"action_step\":1,\"text\":\"And you don\'t disappear, and [Player] go wash up :)\"},{\"emote\":\"happy\",\"text\":\"[p]Well have a good day [Talked] :)\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(24, 6, 11, 1, 2, 0, 6834, 7569, '[{\"text\":\"Are you ready?\"},{\"emote\":\"surprise\",\"text\":\"[p]What, ready?\"},{\"text\":\"Introduce you [Player] to the Apostle of our village. \"},{\"emote\":\"surprise\",\"text\":\"[p][Talked] so what\'s it to them for me?\"},{\"emote\":\"angry\",\"text\":\"**She said rudely**: No, don\'t think so bad, they actually think of us as their children.\"},{\"text\":\"[p]Why so rude? I understand you, okay, I\'m worried, but we\'ll see what happens.\"},{\"emote\":\"pain\",\"text\":\"I\'m sorry, but she\'s really close to me.\"},{\"emote\":\"blink\",\"text\":\"[p]Are you one of them? Hm...\"},{\"action_step\":1,\"emote\":\"angry\",\"text\":\"Fool let\'s go already.\"},{\"emote\":\"pain\",\"text\":\"[p]Well ... Let\'s go\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(25, 6, 11, 2, 2, 0, 5722, 6353, '[{\"action_step\":1,\"emote\":\"blink\",\"text\":\"Well, just don\'t mess with as a joke, it\'s Apostle, he is watching over us for many years.\"},{\"text\":\"[p]I understand [Talked].\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(26, 10, 11, 3, 2, 0, 5325, 6289, '[{\"text\":\"[p]Hello dear Apostle *bowing*. I have come to you to wisdom.\"},{\"text\":\"Hello, lift your head. I knew you\'d come. But I\'m not sure you\'re the one I saw.\"},{\"text\":\"[p]See it?\"},{\"text\":\"I can\'t communicate with my brothers and sisters, since our paths have diverged, but we have a common mind.\"},{\"text\":\"I had a vision about a year ago that there would soon be a man who was as tenacious as the Apostles.\"},{\"text\":\"But in the end, he was going to die. My visions have always come true, but when they will come true is unknown.\"},{\"text\":\"So please be careful [Player]. I\'m worried about everyone, but I\'ve also been weak lately.\"},{\"text\":\"[p]I .. Not to worry, I can handle it even if I turn out to be one of your visions\"},{\"action_step\":1,\"text\":\"I hope this is not the case, get used to it, then go to Adventurer Koto.\"},{\"text\":\"[p]Thanks a lot.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(27, 25, 12, 1, 2, 0, 5421, 8273, '[{\"text\":\"[p]Hello [Talked], the Apostle sent me to you. I want to be an adventurer!\"},{\"emote\":\"blink\",\"text\":\"**He as if he doesn\'t want me to become an adventurer**: Will you be strong enough? Well, let\'s see, I think you already understand that you first need to test, only after I say whether you are worthy or not to become an Adventurer?\"},{\"text\":\"[p]Well, what exactly is required?\"},{\"emote\":\"blink\",\"text\":\"If you pass my first test, pay the fee, I encourage you, and send you to the exam. And then we will decide whether you are worthy or not\"},{\"text\":\"[p]Okay, where do we start [Talked]?\"},{\"action_step\":1,\"text\":\"Ha ha where do we start? Don\'t joke with me young fighter. First, take this and go to the southern part of the village.\"},{\"text\":\"[p]Well\"}]', NULL, NULL, 27, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(28, 25, 12, 2, 2, 0, 10822, 6737, '[{\"emote\":\"angry\",\"text\":\"You did.\"},{\"emote\":\"blink\",\"text\":\"[p]What could I do?\"},{\"emote\":\"angry\",\"text\":\"I thought you were joking, you wouldn\'t take it seriously, but you came.\"},{\"action_step\":1,\"text\":\"[p]Well, let\'s go.\"},{\"text\":\"Let\'s go.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(29, 25, 13, 1, 3, 0, 676, 1169, '[{\"emote\":\"angry\",\"text\":\"Why are you shivering [Player]?\"},{\"action_step\":1,\"emote\":\"happy\",\"text\":\"Am I that scary? Don\'t worry I\'m kind *laughs*. Well, let\'s see how you cope. Kill for me ....\"},{\"emote\":\"blink\",\"text\":\"[p]I managed it, and it wasn\'t difficult.\"},{\"emote\":\"happy\",\"text\":\"I noticed how well you did. But don\'t relax this is just the beginning ^^\"},{\"emote\":\"blink\",\"text\":\"[p]Ah.. Okay [Talked].\"},{\"text\":\"Well today we\'re finished you can relax. You\'ll find me as soon as you\'re ready.\"},{\"emote\":\"happy\",\"text\":\"[p]Okay, well, have a good day.\"}]', NULL, NULL, NULL, NULL, 9, NULL, '|0|0|0|0|32|0|', NULL, NULL),
(30, 26, 13, 2, 3, 0, 500, 1361, '[{\"text\":\"Hello, I am the personal Ambassador of the Apostle.\"},{\"text\":\"She wants to see you!\"},{\"action_step\":1,\"text\":\"Please follow me to village, I will be waiting for you there\"},{\"text\":\"[p]Okay, I\'m coming\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(31, 16, 4, 1, 1, 0, 1530, 1073, '[{\"text\":\"Listen there\'s another request!\"},{\"action_step\":1,\"text\":\"[Player] if you find some gel, I\'ll be happy to exchange them. Brown Slimes,Blue Slimes and Pink Slime drop Gel. You can take your time, i\'ll wait!\"},{\"text\":\"[p]Here, it wasn\'t easy but I got it [Talked].\"},{\"emote\":\"happy\",\"text\":\"Thank you very much for your help. All right I\'ll go back to my post, good luck to you [Player]!\"},{\"emote\":\"happy\",\"text\":\"[p]Thank\'s [Talked]\"}]', 29, NULL, 25, NULL, NULL, NULL, '|18|0|8|0|0|0|', NULL, NULL),
(32, 26, 13, 3, 2, 0, 3780, 6449, '[{\"action_step\":1,\"text\":\"[Player] you can go in!\"},{\"text\":\"[p]Well [Talked], thank you for seeing me off!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(33, 27, 8, 1, 4, 0, 4235, 945, '[{\"action_step\":1,\"text\":\"[Player] first we need to win over the slime. Dirty but necessary.\"},{\"text\":\"[p]I report everything, I coped with the task\"},{\"emote\":\"happy\",\"text\":\"Well done, but that\'s not all.\"},{\"text\":\"[p]Anything else?\"},{\"text\":\"Yes I think you know about the ethers find me next to him\"}]', NULL, NULL, NULL, NULL, 23, NULL, '|0|0|0|0|50|0|', NULL, NULL),
(34, 27, 8, 2, 2, 0, 7975, 7089, '[{\"text\":\"I think you know that these crystals are actually not just for beauty. \"},{\"text\":\"You use them as a quick transition. But in fact, these are crystals (all the light of good prosperity) precisely because they exist.\"},{\"action_step\":1,\"text\":\"Me when once before as became defender of the Apostle, I was able to meet with the crystal. Yes, he has a mind. From it warmth on the soul, all the sins immediately fall from the soul. I\'m sure you\'ll meet him one day.\"},{\"text\":\"[p]Thanks. I will remember that. (Auther text: This is how the player\'s rapprochement with the deities began)\"},{\"text\":\"Well, I\'ll wait for you in the Apostle\'s chambers. We will continue the expulsion of the monsters\"}]', NULL, NULL, 32, NULL, NULL, NULL, '|0|0|1|0|0|0|', NULL, NULL),
(35, 27, 9, 1, 4, 0, 4243, 945, '[{\"text\":\"Our priority is higher now.\"},{\"action_step\":1,\"text\":\"[Player] you need to defeat the main slug, in the southern part.\"},{\"text\":\"[p]That\'s it, I\'ve met my goal. Clearing dirt.\"},{\"text\":\"Well done, we\'ll meet again.\"}]', NULL, NULL, NULL, NULL, 28, NULL, '|0|0|0|0|1|0|', NULL, NULL),
(36, 10, 14, 1, 4, 0, 4391, 977, '[{\"text\":\"I want to talk to you [Player].\"},{\"emote\":\"blink\",\"text\":\"I know that your path will not stop here. One day you will become stronger and move on.\"},{\"text\":\"I have a request to you, if you meet other apostles, it is better to avoid them. Don\'t trust them, we apostles are not as pure as you think\"},{\"emote\":\"blink\",\"text\":\"[p]Maybe you, too, then the enemy really is?\"},{\"emote\":\"happy\",\"text\":\"I don\'t know *laughter*. The villagers I think will clearly tell you how it really is\"},{\"text\":\"Diana told me about you in General terms.\"},{\"emote\":\"happy\",\"text\":\"**She said it with concern**: You turn out to be a Joker, and a vulgar *laughter*\"},{\"emote\":\"blink\",\"text\":\"[p]No [Talked]......\"},{\"text\":\"Always listen to your heart. I think one day you will be able to bring light. But the main thing is perseverance!\"},{\"action_step\":1,\"text\":\"Do not delay with your title of adventurer. You can go now!\"},{\"text\":\"[p]I\'m trying, thank you very much.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(37, 6, 15, 1, 2, 0, 8081, 7889, '[{\"text\":\"[p]You look sad today [Talked].\"},{\"action_step\":1,\"emote\":\"pain\",\"text\":\"Oh man I\'m too confused today.... [Player] help me collect lost.\"},{\"emote\":\"happy\",\"text\":\"[p]Take your nose up [Talked], what\'s wrong with you?\"},{\"emote\":\"pain\",\"text\":\"My brother has to stop by today, and I want to cook something to feed him, but everything falls out of my hands..\"},{\"emote\":\"happy\",\"text\":\"[p]So let me help you?\"},{\"text\":\"And you I look [Player] today cheerful, well help. I will not remain in debt\"},{\"emote\":\"happy\",\"text\":\"[p]Of course [Talked] ;)\"}]', 20, NULL, NULL, NULL, NULL, NULL, '|10|0|0|0|0|0|', 2, NULL),
(38, 6, 15, 2, 2, 0, 8193, 6065, '[{\"text\":\"Well here\'s my house! Ah as my not my, father is a master, and even brother. In General, our family\"},{\"action_step\":1,\"text\":\"All right, come on in, I\'ll cook, and I\'ll ask you to bring something. All the help me need\"},{\"text\":\"[p]I will help you, especially since you are my first close friend in this village, I already trust you, how you can be abandoned.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(39, 6, 16, 1, 7, 0, 4454, 881, '[{\"text\":\"Well [Player], let\'s get started, little by little\"},{\"emote\":\"happy\",\"text\":\"[p]Your house comfortable! [Talked]\"},{\"action_step\":1,\"text\":\"[p]Do you really think so? Well thank you.\"},{\"emote\":\"happy\",\"text\":\"All right let\'s get down to cooking\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(40, 6, 16, 2, 7, 0, 4671, 881, '[{\"text\":\"Well, let\'s get started [Player]!\"},{\"text\":\"[p]Oh sure [Talked]\"},{\"action_step\":1,\"text\":\"[Player] get me some first, we\'ll use it as oil\"},{\"emote\":\"happy\",\"text\":\"[p]I think everyone has their own way of making butter *laughter*\"},{\"emote\":\"happy\",\"text\":\"Yeah you\'re right, good sense of humor damn *laughter*\"}]', 29, NULL, NULL, NULL, NULL, NULL, '|18|0|0|0|0|0|', NULL, NULL),
(41, 6, 16, 3, 7, 0, 4590, 881, '[{\"text\":\"So now I need meat. How to cook without meat.\"},{\"action_step\":1,\"text\":\"I think you\'ve heard of Kappa, they live near water, their meat is very tasty. So get it for me (you can self find them habitat)\"},{\"emote\":\"happy\",\"text\":\"[p]All delicious fine meat, looks great, take it!\"},{\"emote\":\"happy\",\"text\":\"Well i let\'s start cooking :)\"},{\"text\":\"[p]**You see like she didn\'t know how to cook**: Are you sure you can cook?\"},{\"emote\":\"angry\",\"text\":\"**She said angrily**: You know what I\'ll tell you [Player]... You don\'t know how to compliment girls at all..\"},{\"emote\":\"blink\",\"text\":\"[p]**Someone knocks on the door**: Who could it be? lovers?\"},{\"emote\":\"surprise\",\"text\":\"Oh it\'s probably my brother.\"}]', 35, NULL, NULL, NULL, NULL, NULL, '|14|0|0|0|0|0|', NULL, NULL),
(42, 8, 16, 4, 7, 0, 4142, 881, '[{\"emote\":\"blink\",\"text\":\"**He said angrily and rudely**: Oh, what people, I\'ve seen you somewhere.\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]You must have imagined it. \"},{\"text\":\"Oh well.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(43, 6, 16, 4, 7, 0, 4590, 881, '[{\"emote\":\"happy\",\"text\":\"[Player] you look sad.\"},{\"text\":\"[p]Okay [Talked]. I\'ll probably go extra here\"},{\"emote\":\"blink\",\"text\":\"Yes calm down you why so decided to?\"},{\"emote\":\"happy\",\"text\":\"[p]Yes, everything is fine. I just have things to do, too. I\'ll go!\"},{\"emote\":\"pain\",\"text\":\"You offend me [Talked].....\"},{\"action_step\":1,\"text\":\"[p]Sorry I\'m really busy [Talked].\"},{\"emote\":\"blink\",\"text\":\"Well, take care of yourself.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(44, 25, 17, 1, 2, 0, 8128, 7889, '[{\"text\":\"So let\'s continue our training.\"},{\"text\":\"[p]Yes, of course, it\'s about time!\"},{\"emote\":\"angry\",\"text\":\"**He speaks rudely**: I don\'t really like you. But let\'s see what you can achieve. Let\'s go back to the Deep Cave.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(45, 25, 17, 2, 3, 0, 762, 1169, '[{\"text\":\"**He speaks proudly**: Well, now we have more serious goals.\"},{\"action_step\":1,\"text\":\"Exterminate the stronger slime. I\'ll see.\"},{\"emote\":\"blink\",\"text\":\"[p]Everything is ready, they are vile I do not like slime, maybe something new?\"},{\"text\":\"Well, I\'ll think about your request. Follow me.\"}]', NULL, NULL, NULL, NULL, 23, NULL, '|0|0|0|0|32|0|', NULL, NULL),
(46, 25, 17, 3, 2, 0, 6316, 7569, '[{\"action_step\":1,\"text\":\"Take it, use it wisely. You will need it very much in the future.\"},{\"emote\":\"surprise\",\"text\":\"[p]**You were surprised by the gift provided**: Oh, thank you very much. Do I owe you something?\"},{\"emote\":\"blink\",\"text\":\"I hope you make it to the end, at least I hope you do. I didn\'t think you\'d be Apostle\'s favorite.\"},{\"emote\":\"surprise\",\"text\":\"[p]Favorite [Talked]?\"},{\"emote\":\"angry\",\"text\":\"**He said very rudely**: I\'ll go, meet you later.\"}]', NULL, NULL, 9, NULL, NULL, NULL, '|0|0|50|0|0|0|', NULL, NULL),
(47, 27, 18, 1, 2, 0, 5244, 6289, '[{\"text\":\"Hello [Player]. Are you free?\"},{\"text\":\"[p]Yes, of course. Everything okay?\"},{\"text\":\"Apostle asked you to see her!\"},{\"action_step\":1,\"text\":\"[p]Well!\"},{\"text\":\"Thank you, I\'m going for a walk!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|1|0|', NULL, NULL),
(48, 10, 18, 2, 4, 0, 4453, 977, '[{\"emote\":\"pain\",\"text\":\"**She speaks in fear**: Hello [Player]. Today I received news that in the southern part of our village, goblins attacked. \"},{\"emote\":\"pain\",\"text\":\"And it\'s not just goblins. This must be the army. The leader brought them.\"},{\"emote\":\"surprise\",\"text\":\"[p]Maybe they kidnapped the residents and terrorized you, and now they decided to attack?\"},{\"action_step\":1,\"text\":\"I think so too. Anyway, I talked to Koko. About your appointment as an adventurer. We should prepare for resistance. Communication nodes they need to be destroyed\"},{\"text\":\"[p]Well, I did the job [Talked]!\"},{\"text\":\"Good job thank\'s, you should meet with Koko, he will explain everything to you, good luck!\"},{\"text\":\"[p]Well, thank you!\"}]', NULL, NULL, 16, NULL, 34, NULL, '|0|0|3|0|32|0|', NULL, NULL),
(49, 25, 19, 1, 4, 0, 4727, 977, '[{\"text\":\"Get ready let\'s go to the Goblin-occupied zone.\"},{\"text\":\"[p]I\'m ready to sort of out right now.\"},{\"emote\":\"happy\",\"text\":\"Well [Player], you\'re fun, Yes, but it\'s worth getting ready.\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]Well, what do need [Talked]?\"},{\"text\":\"I\'ll be waiting for you next to Craftsman.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(50, 25, 19, 2, 2, 0, 9079, 8305, '[{\"text\":\"So first we need potions.\"},{\"action_step\":1,\"text\":\"Go get supplies, and at the same time exterminate a few slugs so that we can get there without any problems.\"},{\"emote\":\"pain\",\"text\":\"[p]**You sound very tired**: We can all move, but the problem is that I\'m tired..\"},{\"emote\":\"blink\",\"text\":\"Nothing to worry about [Player]. You need to toughen up. All right let\'s move out.\"}]', 29, 22, 15, 14, 23, 9, '|16|16|8|8|40|40|', NULL, NULL),
(51, 25, 19, 3, 3, 0, 4509, 1265, '[{\"action_step\":1,\"text\":\"We\'ll get to the guard post now. We\'ll move out there to exterminate the goblins.\"},{\"text\":\"[p]Well [Talked].\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(52, 37, 50, 1, 2, 0, 7781, 7921, '[{\"emote\":\"blink\",\"text\":\"[p][e2]**Grinning**: Huh?\"},{\"emote\":\"pain\",\"text\":\"We can\'t heal her..\"},{\"emote\":\"blink\",\"text\":\"[p]I don\'t understand..\"},{\"emote\":\"pain\",\"text\":\"Our daughter, Maria.. She\'s sick!\"},{\"text\":\"Why don\'t you take her to the Nurse?!\"},{\"action_step\":1,\"emote\":\"pain\",\"text\":\"I tried, but she said she can\'t do anything.. I found something in an old book of mine.. A gel treatment. But I also need Kappa Meat for it..\"},{\"emote\":\"happy\",\"text\":\"Tha..nk.. You..\"},{\"emote\":\"blink\",\"text\":\"Well.. don\'t look at me, heal Maria!\"}]', 29, 35, NULL, NULL, NULL, NULL, '|20|20|0|0|0|0|', NULL, NULL),
(53, 38, 50, 2, 2, 0, 7671, 7921, '[{\"emote\":\"pain\",\"text\":\"F..Father?\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(54, 37, 50, 3, 2, 0, 7781, 7921, '[{\"emote\":\"happy\",\"text\":\"MARIA! You\'re well!\"},{\"emote\":\"happy\",\"text\":\"[p]*with joy* Hi, Maria!\"},{\"text\":\"How can I ever make it up to you?\"},{\"action_step\":1,\"text\":\"[p]Well Sir.. I might need a discount from your old friend, the Craftsman.\"},{\"emote\":\"happy\",\"text\":\"Sure, I\'ll.. let him know you deserve it!\"}]', NULL, NULL, 43, NULL, NULL, NULL, '|0|0|5|0|0|0|', NULL, NULL),
(55, 15, 55, 1, 2, 0, 9463, 6833, '[{\"emote\":\"blink\",\"text\":\"[p]Sure..What can I do?\"},{\"emote\":\"blink\",\"text\":\"I heard you helped the craftsman with his deliveries..I have no time for small things like mining cooper, can you do it for me? I have to save the world in the main time..\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p] *a bit angry* Ok, I will do it.\"},{\"emote\":\"happy\",\"text\":\"Oh, it\'s you kid!\"},{\"emote\":\"angry\",\"text\":\"[p]*you can\'t control yourself* HEY. I\'m NOT A KID!\"},{\"text\":\"Hey.. easy, just a joke, did you get what I wanted?\"},{\"emote\":\"blink\",\"text\":\"[p]Yes.. *puts a heavy cooper bag on Erik\'s table*\"},{\"emote\":\"angry\",\"text\":\"Good, now get out of here!\"},{\"emote\":\"angry\",\"text\":\"[p]Wait.. YOU NOT GONNA PAY ME??\"},{\"emote\":\"angry\",\"text\":\"Haha! I am messing with you.\"}]', 31, NULL, NULL, NULL, NULL, NULL, '|50|0|0|0|0|0|', NULL, NULL),
(56, 39, 60, 1, 5, 0, 2852, 3473, '[{\"text\":\"[p]So can you tell me how you got here?\"},{\"emote\":\"blink\",\"text\":\"**He\'s stuttered**: A long story..\"},{\"emote\":\"blink\",\"text\":\"We tried a new potion mixed with ether, but something went wrong, my hiccups are part of this experiment, and I ended up here after I drank it\"},{\"text\":\"**Sound in the tablet**: Noctis... Noctiis. Do you hear me?\"},{\"text\":\"**Noctis**: I can hear you, how do I get out of here?\"},{\"text\":\"**Sound in the tablet**: Collect all the fragments that ended up here. The connection is lost....\"},{\"text\":\"[p]What were you talking to just now?\"},{\"action_step\":1,\"text\":\"A tablet made from a magic crystal. In General, as I thought I need your help in collecting fragments\"},{\"text\":\"[p]Of course I\'ll help\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(57, 39, 60, 2, 5, 0, 3216, 3537, '[{\"text\":\"Look at what I can give you in return for your help. (Vote menu shop list)\"},{\"text\":\"[p]Wow, [Talked] where can I find them?\"},{\"text\":\"In a distorted dimension.\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]Where is it? How do I get there?\"},{\"text\":\"[Player] way, so we\'ll be preparing to get there.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(58, 39, 61, 1, 5, 0, 2944, 3505, '[{\"action_step\":1,\"text\":\"So first [Player], I\'ll need these things from you. They resonate between our worlds\"},{\"text\":\"[p]Take it [Talked].\"},{\"text\":\"Well thank you, it will be a little easier now.\"},{\"text\":\"**Sound in the tablet**: Noctis. How you to get the fragments we will be able to pick you up.\"},{\"text\":\"**Noctis**: Okay, Sid..\"}]', 31, 35, NULL, NULL, NULL, NULL, '|40|40|0|0|0|0|', NULL, NULL),
(59, 39, 62, 1, 5, 0, 2944, 3505, '[{\"action_step\":1,\"text\":\"Next thing I need to create a resonance. Bring me these items.\"},{\"text\":\"[p]Everything is ready?. [Talked] take it\"},{\"text\":\"[Player] yes.. almost, follow me and we\'ll start resonating.\"}]', 41, 37, NULL, NULL, NULL, NULL, '|1|8|0|0|0|0|', NULL, NULL),
(60, 39, 62, 2, 5, 0, 4566, 4273, '[{\"text\":\"All ready.\"},{\"text\":\"Resonance received. I will wait for you with fragments\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]Can come to resonance?\"},{\"text\":\"Yes. Don\'t worry nothing will happen to you.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(61, 46, 20, 1, 5, 0, 2528, 4305, '[{\"action_step\":1,\"emote\":\"happy\",\"text\":\"I welcome you [Talked], nice to meet you.\"},{\"emote\":\"happy\",\"text\":\"Glad to meet you, i\'am [Player]!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(62, 25, 20, 1, 5, 0, 2409, 4305, '[{\"text\":\"Him you will spend time here. I have to go to the village.\"},{\"action_step\":1,\"emote\":\"happy\",\"text\":\"Need to stand on defense in the village, good luck to you I will wait for good news from you.\"},{\"emote\":\"happy\",\"text\":\"Good luck to you [Talked].\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(63, 46, 20, 2, 5, 0, 2528, 4305, '[{\"text\":\"I came from a far Eastern country, I have some problems in the country so I need to become stronger.\"},{\"text\":\"[p]I came for similar purposes. But I would like to visit your country.\"},{\"emote\":\"happy\",\"text\":\"**He speaks with a smile**: If you wish I will be sailing back soon I can take you with me\"},{\"action_step\":1,\"text\":\"[p]I\'d love to, but I don\'t know, i have here made friends. We\'ll see.\"},{\"emote\":\"blink\",\"text\":\"Well, if we still live of course. And the got to talking and drive away goblins need.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(64, 46, 20, 3, 5, 0, 3867, 3089, '[{\"action_step\":1,\"text\":\"To begin with, we must exterminate many of them as possible. To get to them in zone\"},{\"emote\":\"pain\",\"text\":\"[p]**Very tired voice**: [Talked]. I\'m so tired, there are so many of them.....\"},{\"emote\":\"pain\",\"text\":\"**He sounds very tired**: Yes, I am also very tired, but we have to deal with it.\"},{\"text\":\"All right let\'s keep going before they pile up again.\"}]', NULL, NULL, NULL, NULL, 35, 22, '|0|0|0|0|5|30|', NULL, NULL),
(65, 46, 20, 4, 5, 0, 122, 3377, '[{\"text\":\"Hmmm.. I see a passageway. I think we can go through there and find out where they\'re coming from.\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]I\'m worried about something.. Where did this hole?\"},{\"emote\":\"blink\",\"text\":\"I don\'t know either, honestly. Maybe the goblins have dug through and are coming out. In any case we need to find out.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(66, 46, 21, 1, 6, 0, 881, 1521, '[{\"text\":\"I think this is where they live. \"},{\"text\":\"In any case, if we can not cope with them, there is an option to block the way. But I think they\'ll dig it up again in time.\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]**In anger**: LET\'s BREAK UP THESE FREAKS ALREADY... \"},{\"emote\":\"happy\",\"text\":\"I\'m only for your idea. Let\'s go!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(67, 46, 21, 2, 6, 0, 3121, 4114, '[{\"action_step\":1,\"text\":\"And here is their leader.\"},{\"emote\":\"happy\",\"text\":\"[p]Did we manage?\"},{\"emote\":\"blink\",\"text\":\"We don\'t know yet we need to get out of here....\"},{\"text\":\"I\'ll be waiting for you where we fought the goblins.\"}]', NULL, NULL, NULL, NULL, 32, NULL, '|0|0|0|0|1|0|', NULL, NULL),
(68, 46, 21, 3, 5, 0, 1714, 4433, '[{\"action_step\":1,\"text\":\"[p]Finally we got out how do you think we managed?\"},{\"emote\":\"blink\",\"text\":\"I don\'t know [Player], we have to report this news to the village.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|1|0|', NULL, NULL),
(69, 46, 22, 1, 5, 0, 970, 4529, '[{\"emote\":\"happy\",\"text\":\"Well, I think my work here is done [Player].\"},{\"emote\":\"blink\",\"text\":\"[p]Your [Talked] next trip will be to your homeland?\"},{\"emote\":\"happy\",\"text\":\"Yes, I think it\'s time to go home, I missed it.\"},{\"emote\":\"happy\",\"text\":\"[p]Will you take me with you?\"},{\"emote\":\"happy\",\"text\":\"Yes, of course. Say goodbye to your friends or maybe one of them will go with you?\"},{\"action_step\":1,\"text\":\"No, I don\'t want to expose them to danger.\"},{\"text\":\"Well\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(70, 10, 22, 2, 5, 0, 2337, 4305, '[{\"emote\":\"pain\",\"text\":\"[p]Well, goodbye [Talked], I\'ll keep going. I want to be stronger than I was before.\"},{\"emote\":\"happy\",\"text\":\"All right [Player]. Good luck don\'t forget that we will always be happy to welcome you as a guest ;)\"},{\"action_step\":1,\"emote\":\"happy\",\"text\":\"[p]Don\'t forget, don\'t miss me ;)\"},{\"text\":\"Don\'t worry, Diana is very attached to you. I think she will be offended. Talk to she!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(71, 6, 22, 3, 5, 0, 2546, 4305, '[{\"emote\":\"pain\",\"text\":\"Why didn\'t you tell [Player] me you were going to leave?\"},{\"emote\":\"pain\",\"text\":\"[p]I\'m Sorry [Talked]. I wanted to tell you..\"},{\"emote\":\"pain\",\"text\":\"**She said sadly**: Why? Will you take me with you?\"},{\"emote\":\"pain\",\"text\":\"[p]I don\'t want to put you in danger!\"},{\"emote\":\"pain\",\"text\":\"I want to travel with you!\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]Ok, I\'ll take you. Now let me to say Yasue San.\"},{\"emote\":\"happy\",\"text\":\"**She said joyfully**: True? Thank! ;)\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(72, 46, 22, 4, 5, 0, 4654, 4241, '[{\"emote\":\"blink\",\"text\":\"Well, ready to go?\"},{\"action_step\":1,\"emote\":\"blink\",\"text\":\"[p]Yes, will I take Diana with me?\"},{\"emote\":\"happy\",\"text\":\"Yes, of course, set sail:) I will show you my homeland!\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(73, 1, 64, 1, 9, 0, 1376, 1073, '[{\"text\":\"Do you need something, my young friend?\"},{\"text\":\"[p]The [Bot_2] says we are expecting a storm, I came to clarify.\"},{\"text\":\"Yes, he\'s right. A storm is waiting for us, it can be determined by the waves.\"},{\"text\":\"[p]Waves? Can I ask you more details?\",\"emote\":\"blink\"},{\"text\":\"When a storm is expected, the waves among the desert sea begin to waver.\"},{\"text\":\"[p]I see, how big a storm is expected?\",\"emote\":\"surprise\"},{\"text\":\"I won\'t tell you exactly my friend.\"},{\"text\":\"Don\'t worry, I\'ve been sailing in a storm many times. We\'ll manage somehow! You\'d better go eat and rest with your friends.\", \"action_step\":1},{\"text\":\"[p]OK, thanks for the advice.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(74, 6, 64, 2, 9, 0, 2493, 1265, '[{\"text\":\"Oh [Player], where have you been? I\'ve been looking for you for so long.\",\"emote\":\"pain\"},{\"text\":\"Look at this interesting game from [Bot_46] homeland. It\'s called MAJONG.\",\"emote\":\"happy\"},{\"text\":\"[p]Yeah, I\'m a little worried. I have a bad feeling about it.\",\"emote\":\"blink\"},{\"text\":\"Don\'t worry [Player]. Or have you forgotten who\'s with you? I\'m the strongest girl from the village, don\'t worry!\",\"emote\":\"happy\"},{\"text\":\"Or at least I can cook well...\",\"emote\":\"blink\"},{\"text\":\"[p]Oh, speaking of which, aren\'t you hungry? Shall we go eat?\"},{\"text\":\"And i wanted to offer you something to eat too. Of course you can. A strong girl needs to eat well!\"},{\"text\":\"[p]Diana just don\'t get fat.\",\"emote\":\"happy\",\"action_step\":1},{\"text\":\"[Player] don\'t give me complexes!!! ( ‾́ ◡ ‾́ )\",\"emote\":\"angry\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(75, 55, 64, 3, 9, 0, 2485, 1137, '[{\"text\":\"What will you eat?\"},{\"text\":\"[p]Let me ask my friend for a second.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(76, 6, 64, 3, 9, 0, 2289, 1137, '[{\"text\":\"[p][Bot_6], what are we going to eat?\"},{\"text\":\"Let\'s try pancakes!!! ** Said she with eyes like she saw pancakes for the first time**\",\"emote\":\"happy\"},{\"text\":\"[p]Yes [Talked] i want to try them too.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(77, 6, 64, 4, 9, 0, 2341, 1137, '[{\"text\":\"Hurry up and take them, I want to eat...\",\"emote\":\"angry\"},{\"text\":\"[p] I\'ll take them, I\'ll take them... Don\'t worry, they don\'t have legs!\",\"emote\":\"blink\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(78, 55, 64, 4, 9, 0, 2485, 1137, '[{\"text\":\"Did you choose something?\"},{\"text\":\"[p]Yes we did, can we have pancakes please.\"}, {\"text\": \"Yes of course, take your order.\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(79, 6, 64, 5, 9, 0, 2485, 1265, '[{\"text\":\"Mmm... They\'re really good. But I still cook tastier don\'t you [Player] agree?\", \"emote\": \"happy\"},{\"text\":\"[p]Maybe.\"}, {\"text\": \"What\'s possible.... It\'s not possible, it\'s definitely...\", \"emote\": \"angry\"}, {\"text\": \"[p]Well, well. **Bell sound**\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL),
(80, 2, 64, 6, 9, 0, 2132, 1265, '[{\"text\":\"Please remain calm, a very big storm is coming.\",\"emote\":\"surprise\"},{\"text\":\"[p]A violent storm?\",\"emote\":\"blink\"},{\"text\":\"[e2]HOW DARE YOU... I AM A GOD OF THE SEAS AND OCEANS....\",\"emote\":\"angry\"},{\"text\":\"[e2]I can smell the oracle...\",\"emote\":\"angry\"},{\"text\":\"[e2] WHERE IS THE ORACLE.......\",\"emote\":\"angry\"},{\"text\":\"[e1]What oracle?\",\"emote\":\"pain\"},{\"text\":\"[p][e1]Don\'t joke with me... WAIT FOR YOU IN MY CHAMBERS WITH THE ORACLE... OTHERWISE YOU WILL DIE HERE...\",\"emote\":\"angry\"},{\"text\":\"[p][e1]**no sound of water** I have stopped the flow of the sea...\",\"emote\":\"angry\"},{\"text\":\"[p][e2]How do we find you?\",\"emote\":\"blink\"},{\"text\":\"[e2] Waiting for you under the water...\",\"emote\":\"angry\"}]', NULL, NULL, NULL, NULL, NULL, NULL, '|0|0|0|0|0|0|', NULL, NULL);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_crafts_list`
--

CREATE TABLE `tw_crafts_list` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) DEFAULT NULL,
  `ItemValue` int(11) NOT NULL,
  `RequiredItemID0` int(11) DEFAULT NULL,
  `RequiredItemID1` int(11) DEFAULT NULL,
  `RequiredItemID2` int(11) DEFAULT NULL,
  `RequiredItemsValues` varchar(32) NOT NULL DEFAULT '0 0 0',
  `Price` int(11) NOT NULL DEFAULT 100,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--
-- Дамп данных таблицы `tw_crafts_list`
--

INSERT INTO `tw_crafts_list` (`ID`, `ItemID`, `ItemValue`, `RequiredItemID0`, `RequiredItemID1`, `RequiredItemID2`, `RequiredItemsValues`, `Price`, `WorldID`) VALUES
(1, 15, 3, 22, 29, 18, '9 9 16', 50, 2),
(2, 26, 1, 30, NULL, NULL, '24 0 0', 150, 2),
(3, 33, 1, 37, 31, NULL, '3 30 0', 2500, 2),
(4, 34, 1, 37, 31, NULL, '8 50 0', 2700, 2),
(5, 10019, 1, 37, 30, 31, '18 48 24', 7200, 2),
(6, 10020, 1, 37, 30, 31, '14 38 18', 7200, 2),
(7, 10021, 1, 37, 30, 31, '14 38 18', 7200, 2),
(8, 10022, 1, 37, 30, 31, '14 38 18', 7200, 2),
(9, 10023, 1, 37, 30, 31, '14 38 18', 7200, 2),
(10, 10016, 1, 37, NULL, NULL, '40 0 0', 14400, 2),
(11, 14, 3, 22, 29, 18, '9 9 16', 50, 2),
(12, 41, 1, 42, 30, 18, '32 16 64', 3600, 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_dungeons`
--

CREATE TABLE `tw_dungeons` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL DEFAULT 'Unknown',
  `Level` int(11) NOT NULL DEFAULT 1,
  `DoorX` int(11) NOT NULL DEFAULT 0,
  `DoorY` int(11) NOT NULL DEFAULT 0,
  `RequiredQuestID` int(11) NOT NULL DEFAULT -1,
  `Story` tinyint(4) NOT NULL DEFAULT 0,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_dungeons`
--

INSERT INTO `tw_dungeons` (`ID`, `Name`, `Level`, `DoorX`, `DoorY`, `RequiredQuestID`, `Story`, `WorldID`) VALUES
(1, 'Abandoned mine', 10, 1105, 1521, 20, 1, 6),
(2, 'Resonance Noctis', 18, 1157, 528, 62, 0, 8),
(3, 'Kingdom Neptune', 15, 1084, 532, 64, 1, 10);

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
(4, 'Write here name dungeon', 2255, 4850, 22, 1),
(5, 'Write here name dungeon', 5233, 530, 40, 2),
(6, 'Write here name dungeon', 4432, 2929, 41, 2),
(7, 'Write here name dungeon', 1550, 1970, 42, 2),
(8, 'Write here name dungeon', 1647, 1970, 43, 2),
(9, 'Write here name dungeon', 3213, 369, 47, 3),
(10, 'Write here name dungeon', 400, 881, 48, 3),
(11, 'Write here name dungeon', 4432, 1490, 49, 3);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_dungeons_records`
--

CREATE TABLE `tw_dungeons_records` (
  `ID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL,
  `DungeonID` int(11) NOT NULL,
  `Seconds` int(11) NOT NULL,
  `PassageHelp` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds`
--

CREATE TABLE `tw_guilds` (
  `ID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL DEFAULT 'Member name',
  `UserID` int(11) DEFAULT NULL,
  `Level` int(11) NOT NULL DEFAULT 1,
  `Experience` int(11) NOT NULL DEFAULT 0,
  `Bank` int(11) NOT NULL DEFAULT 0,
  `Score` int(11) NOT NULL DEFAULT 0,
  `AvailableSlots` int(11) NOT NULL DEFAULT 2,
  `ChairExperience` int(11) NOT NULL DEFAULT 1,
  `ChairMoney` int(11) NOT NULL DEFAULT 1
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_decorations`
--

CREATE TABLE `tw_guilds_decorations` (
  `ID` int(11) NOT NULL,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
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
  `Time` datetime NOT NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_houses`
--

CREATE TABLE `tw_guilds_houses` (
  `ID` int(11) NOT NULL,
  `GuildID` int(11) DEFAULT NULL,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `DoorX` int(11) NOT NULL,
  `DoorY` int(11) NOT NULL,
  `TextX` int(11) NOT NULL,
  `TextY` int(11) NOT NULL,
  `Price` int(11) NOT NULL DEFAULT 50000,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_guilds_houses`
--

INSERT INTO `tw_guilds_houses` (`ID`, `GuildID`, `PosX`, `PosY`, `DoorX`, `DoorY`, `TextX`, `TextY`, `Price`, `WorldID`) VALUES
(1, NULL, 4250, 6352, 4496, 6461, 4206, 6224, 240000, 2),
(2, NULL, 9504, 5713, 9180, 5713, 9486, 5495, 280000, 2),
(3, NULL, 3601, 1521, 3890, 1513, 3577, 1346, 320000, 11);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_invites`
--

CREATE TABLE `tw_guilds_invites` (
  `ID` int(11) NOT NULL,
  `GuildID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_guilds_ranks`
--

CREATE TABLE `tw_guilds_ranks` (
  `ID` int(11) NOT NULL,
  `Access` int(11) NOT NULL DEFAULT 3,
  `Name` varchar(32) NOT NULL DEFAULT 'Rank name',
  `GuildID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_houses`
--

CREATE TABLE `tw_houses` (
  `ID` int(11) NOT NULL,
  `UserID` int(11) DEFAULT NULL,
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

INSERT INTO `tw_houses` (`ID`, `UserID`, `PosX`, `PosY`, `DoorX`, `DoorY`, `Class`, `Price`, `HouseBank`, `PlantID`, `PlantX`, `PlantY`, `WorldID`) VALUES
(1, NULL, 8995, 7672, 8752, 7740, 'Elven class', 150000, 160, 18, 9456, 7766, 2),
(2, NULL, 7999, 5297, 8241, 5297, 'Elven class', 150000, 100, 18, 7492, 5329, 2),
(3, NULL, 2036, 593, 1937, 585, 'Asian-style home', 180000, 0, 18, 2559, 593, 11),
(4, NULL, 2036, 913, 1937, 905, 'Asian-style home', 180000, 0, 18, 2559, 913, 11),
(5, NULL, 1216, 593, 1326, 585, 'Asian-style home', 180000, 0, 18, 715, 593, 11),
(6, NULL, 1216, 913, 1326, 905, 'Asian-style home', 180000, 0, 18, 715, 913, 11);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_houses_decorations`
--

CREATE TABLE `tw_houses_decorations` (
  `ID` int(11) NOT NULL,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
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
  `Type` int(11) DEFAULT -1,
  `Function` int(11) DEFAULT -1,
  `Desynthesis` int(11) NOT NULL DEFAULT 100,
  `Selling` int(11) NOT NULL DEFAULT 100,
  `Attribute0` int(11) DEFAULT NULL,
  `Attribute1` int(11) DEFAULT NULL,
  `AttributeValue0` int(11) NOT NULL DEFAULT 0,
  `AttributeValue1` int(11) NOT NULL,
  `ProjectileID` int(11) NOT NULL DEFAULT -1 COMMENT 'only for weapons'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_items_list`
--

INSERT INTO `tw_items_list` (`ItemID`, `Name`, `Description`, `Icon`, `Type`, `Function`, `Desynthesis`, `Selling`, `Attribute0`, `Attribute1`, `AttributeValue0`, `AttributeValue1`, `ProjectileID`) VALUES
(1, 'Gold', 'Major currency', 'gold', -1, -1, 0, 0, 16, NULL, 0, 0, -1),
(2, 'Hammer', 'A normal hammer', 'hammer', 6, 0, 0, 0, 16, 6, 10, 3, -1),
(3, 'Gun', 'Conventional weapon', 'gun', 6, 1, 0, 10, 17, NULL, 10, 0, -1),
(4, 'Shotgun', 'Conventional weapon', 'shotgun', 6, 2, 0, 10, 18, NULL, 5, 0, -1),
(5, 'Grenade', 'Conventional weapon', 'grenade', 6, 3, 0, 10, 19, NULL, 10, 0, -1),
(6, 'Rifle', 'Conventional weapon', 'rifle', 6, 4, 0, 10, 20, NULL, 10, 0, -1),
(7, 'Material', 'Required to improve weapons', 'material', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(8, 'Ticket guild', 'Command: /gcreate <name>', 'ticket', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(9, 'Skill Point', 'Skill point', 'skill_point', -1, -1, 0, 10, NULL, NULL, 0, 0, -1),
(10, 'Decoration Armor', 'Decoration for house!', 'deco_house', 7, -1, 0, 10, NULL, NULL, 0, 0, -1),
(11, 'Decoration Hearth Elite', 'Decoration for house!', 'deco_house', 7, -1, 0, 10, NULL, NULL, 0, 0, -1),
(12, 'Decoration Ninja Elite', 'Decoration for house!', 'deco_house', 7, -1, 0, 10, NULL, NULL, 0, 0, -1),
(13, 'Decoration Hearth', 'Decoration for house!', 'deco_house', 7, -1, 0, 10, NULL, NULL, 0, 0, -1),
(14, 'Potion mana regen', 'Regenerate +5%, 15sec every sec.\n', 'potion_b', 8, 8, 20, 10, NULL, NULL, 0, 0, -1),
(15, 'Potion health regen', 'Regenerate +3% health, 15sec every sec.', 'potion_r', 8, 8, 20, 10, NULL, NULL, 0, 0, -1),
(16, 'Capsule survival experience', 'You got 10-50 experience survival', 'potion_g', 1, 9, 0, 10, NULL, NULL, 0, 0, -1),
(17, 'Little bag of gold', 'You got 10-50 gold', 'pouch', 1, 9, 0, 10, NULL, NULL, 0, 0, -1),
(18, 'Mirt', 'Information added later.', 'some1', 4, 11, 2, 10, NULL, NULL, 0, 0, -1),
(20, 'Potato', 'Material need for craft!', 'potato', 4, 11, 1, 10, NULL, NULL, 0, 0, -1),
(21, 'Notebook', 'In it, something is written', 'paper', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(22, 'Glue', 'I wonder what it\'s for?', 'some4', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(23, 'Board', 'Plain Board', 'board', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(24, 'Mushroom', 'Material need for craft!', 'mushroom', 4, 11, 2, 10, NULL, NULL, 0, 0, -1),
(25, 'Potion resurrection', 'Resuscitates in the zone where you died!', 'potion_p', 8, -1, 0, 10, NULL, NULL, 0, 0, -1),
(26, 'Goblin Pickaxe', 'It happens sometimes', 'pickaxe', 6, 5, 0, 10, 14, 15, 2, 3, -1),
(27, 'Young fighter\'s ring', 'It happens sometimes', 'ring', 3, 10, 0, 10, 8, NULL, 125, 0, -1),
(28, 'Small ammo bag', 'Adds a small amount of ammunition', 'pouch', 3, 10, 0, 10, 13, NULL, 3, 0, -1),
(29, 'Gel', 'I wonder what it\'s for?', 'some4', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(30, 'Goblin Ingot', 'Information added later.', 'ignot_g', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(31, 'Copper Ingot', 'Information added later.', 'ignot_o', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(32, 'Salantra Blessing', 'It happens sometimes', 'ticket', 3, 10, 0, 10, 8, NULL, 250, 0, -1),
(33, 'Explosive module for gun', 'It happens sometimes', 'module', 3, 10, 0, 10, 17, NULL, 5, 0, -1),
(34, 'Explosive module for shotgun', 'It happens sometimes', 'module', 3, 10, 0, 10, 18, NULL, 5, 0, -1),
(35, 'Kappa meat', 'Information added later.', 'meat', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(36, 'Ring of Arvida', 'It happens sometimes', 'ring_light', 3, 10, 0, 10, 11, 10, 5, 10, -1),
(37, 'Relic of the Orc Lord', 'Information added later.', 'lucky_r', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(38, 'Ticket reset class stats', 'Resets only class stats(Dps, Tank, Healer).', 'ticket', 1, 8, 0, 10, NULL, NULL, 0, 0, -1),
(39, 'Mode PVP', 'Settings game.', 'without', 5, 10, 0, 0, NULL, NULL, 0, 0, -1),
(40, 'Ticket reset weapon stats', 'Resets only ammo stats(Ammo).', 'ticket', 1, 8, 0, 10, NULL, NULL, 0, 0, -1),
(41, 'Orc\'s Belt', 'You can feel the light power of mana.', 'mantle', 3, 10, 0, 10, 10, 5, 25, 40, -1),
(42, 'Torn cloth clothes of orcs', 'Information added later.', 'some2', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(43, 'Blessing for discount craft', 'Need dress it, -20% craft price', 'book', 8, 8, 0, 10, NULL, NULL, 0, 0, -1),
(44, 'Noctis fragment', 'Event Final Fantasy.', 'dark_crst', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(45, 'The core of Neptune', 'Information added later.', 'lucky_r', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(46, 'Remains of a dead soul', 'Information added later.', 'pouch', 4, -1, 0, 10, NULL, NULL, 0, 0, -1),
(47, 'Ring of God of War', 'It happens sometimes', 'ring_fire', 3, 10, 0, 10, 4, 0, 5, 0, -1),
(48, 'Bracelet of Fire', 'It happens sometimes', 'bracelet_fire', 3, 10, 0, 10, 4, 0, 7, 0, -1),
(49, 'Big ammo bag', 'Adds a big amount of ammunition', 'pouch', 3, 10, 0, 10, 13, NULL, 8, 0, -1),
(50, 'Strength ', 'Increase your strength', 'skill', -1, -1, 0, 0, 4, NULL, 1, 0, -1),
(51, 'Random home decor', 'Lucky house decor', 'lucky_r', 1, 9, 0, 0, NULL, NULL, 0, 0, -1),
(10000, 'Heavenly hammer', 'Reinforced kick', 'h_heaven', 6, 0, 0, 10, 16, NULL, 1, 0, -1),
(10001, 'Heavenly gun', 'It look doesn\'t bad', 'g_heaven', 6, 1, 0, 10, 17, NULL, 10, 0, 3),
(10002, 'Heavenly shotgun', 'It look doesn\'t bad', 's_heaven', 6, 2, 0, 10, 18, NULL, 10, 0, 4),
(10003, 'Heavenly grenade', 'It look doesn\'t bad', 'gr_heaven', 6, 3, 0, 10, 19, NULL, 10, 0, 5),
(10004, 'Heavenly rifle', 'It look doesn\'t bad', 'r_heaven', 6, 4, 0, 10, 20, NULL, 10, 0, -1),
(10005, 'Shadow wings', 'Dark history', 'wings', 6, 6, 0, 10, 7, NULL, 300, 0, -1),
(10006, 'Neptune wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 11, NULL, 300, 0, -1),
(10007, 'Angel wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 10, NULL, 300, 0, -1),
(10008, 'Heavenly wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 300, 0, -1),
(10009, 'Rainbow wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 300, 0, -1),
(10010, 'Magitech hammer', 'Reinforced kick', 'h_magitech', 6, 0, 0, 10, 16, NULL, 1, 5, -1),
(10011, 'Magitech gun', 'It look doesn\'t bad', 'g_magitech', 6, 1, 0, 10, 17, NULL, 10, 0, 0),
(10012, 'Magitech shotgun', 'It look doesn\'t bad', 's_magitech', 6, 2, 0, 10, 18, NULL, 10, 0, 1),
(10013, 'Magitech grenade', 'It look doesn\'t bad', 'gr_magitech', 6, 3, 0, 10, 19, NULL, 10, 0, 2),
(10014, 'Magitech rifle', 'It look doesn\'t bad', 'r_magitech', 6, 4, 0, 10, 20, NULL, 10, 0, -1),
(10015, 'Stars wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 300, 0, -1),
(10016, 'Bat wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 300, 0, -1),
(10017, 'Little eagle wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 300, 0, -1),
(10018, 'Necromante wings', 'Covered in secrets', 'wings', 6, 6, 0, 10, 8, NULL, 300, 0, -1),
(10019, 'Goblin hammer', 'Reinforced kick', 'h_goblin', 6, 0, 0, 10, 16, NULL, 25, 0, -1),
(10020, 'Goblin gun', 'It look doesn\'t bad', 'g_goblin', 6, 1, 0, 10, 17, NULL, 25, 0, 6),
(10021, 'Goblin shotgun', 'It look doesn\'t bad', 's_goblin', 6, 2, 0, 10, 18, NULL, 15, 0, 7),
(10022, 'Goblin grenade', 'It look doesn\'t bad', 'gr_goblin', 6, 3, 0, 10, 19, NULL, 25, 0, 8),
(10023, 'Goblin rifle', 'It look doesn\'t bad', 'r_goblin', 6, 4, 0, 10, 20, NULL, 25, 0, -1),
(10024, 'Scythe', 'Reinforced kick', 'h_scythe', 6, 0, 0, 10, 16, NULL, 1, 0, -1),
(15000, 'Theme Couple', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 5, NULL, 100, 0, -1),
(15001, 'Theme Final Fantasy', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 5, NULL, 100, 0, -1),
(15002, 'Theme Aion', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 5, NULL, 100, 0, -1),
(15003, 'Theme Dragon Nest', 'Strictly limited as the theme', 'ticket', 6, 7, 0, 10, 5, NULL, 100, 0, -1);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_logics_worlds`
--

CREATE TABLE `tw_logics_worlds` (
  `ID` int(11) NOT NULL,
  `MobID` int(11) NOT NULL,
  `Mode` int(11) NOT NULL DEFAULT 0 COMMENT '(1,3) 0 up 1 left',
  `ParseInt` int(11) NOT NULL COMMENT '(2) health (3)itemid key',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `WorldID` int(11) NOT NULL,
  `Comment` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_positions_farming`
--

CREATE TABLE `tw_positions_farming` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT 1,
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Distance` int(11) NOT NULL DEFAULT 300,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--
-- Дамп данных таблицы `tw_positions_farming`
--

INSERT INTO `tw_positions_farming` (`ID`, `ItemID`, `Level`, `PositionX`, `PositionY`, `Distance`, `WorldID`) VALUES
(1, 18, 1, 320, 3585, 250, 5),
(2, 20, 1, 9952, 4865, 250, 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_positions_mining`
--

CREATE TABLE `tw_positions_mining` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT 1,
  `Health` int(11) NOT NULL DEFAULT 100,
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Distance` int(11) NOT NULL DEFAULT 300,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--
-- Дамп данных таблицы `tw_positions_mining`
--

INSERT INTO `tw_positions_mining` (`ID`, `ItemID`, `Level`, `Health`, `PositionX`, `PositionY`, `Distance`, `WorldID`) VALUES
(1, 7, 1, 150, 5742, 686, 300, 0),
(2, 31, 3, 240, 1485, 4100, 300, 5),
(3, 31, 3, 240, 3525, 4100, 2750, 5);

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
(1, 'Help for workers', 15, 20, 'Main: Arrival'),
(2, 'Helping a girl', 15, 20, 'Main: Arrival'),
(3, 'The fight with slime!', 25, 20, 'Trader Sentry'),
(4, 'Gel? Why do you need it?', 25, 20, 'Trader Sentry'),
(5, 'Search for a daughter', 40, 30, 'Sailor'),
(6, 'What is ore mining?', 50, 30, 'Mining'),
(7, 'My first pickaxe', 100, 30, 'Mining'),
(8, 'Help destruction of dirt', 80, 50, 'Deputy'),
(9, 'Raid on the dirt', 80, 50, 'Deputy'),
(10, 'Acquaintance', 15, 20, 'Main: Apostle Elfia'),
(11, 'Apostle', 20, 20, 'Main: Apostle Elfia'),
(12, 'Adventurer', 50, 40, 'Main: Apostle Elfia'),
(13, 'Something is starting', 10, 20, 'Main: Apostle Elfia'),
(14, 'History from Apostle', 10, 20, 'Main: Apostle Elfia'),
(15, 'Diana has a problem', 20, 30, 'Main: Apostle Elfia'),
(16, 'Mmm delicious', 50, 30, 'Main: Apostle Elfia'),
(17, 'Time learn something', 60, 50, 'Main: Apostle Elfia'),
(18, 'Here are the goblins', 60, 50, 'Main: Apostle Elfia'),
(19, 'Occupation of goblins', 60, 50, 'Main: Apostle Elfia'),
(20, 'Yasue San', 60, 50, 'Main: Apostle Elfia'),
(21, 'Abandoned mine', 60, 50, 'Main: Apostle Elfia'),
(22, 'Goodbye Elfinia', 60, 50, 'Main: Apostle Elfia'),
(50, 'Officer\'s disputes!', 60, 50, 'Officer Henry'),
(55, 'Erik\'s way saying help.', 110, 70, 'Gunsmith Eric'),
(60, 'Why are you here Noctis', 100, 50, 'Final fantasy'),
(61, 'First assignment', 100, 50, 'Final fantasy'),
(62, 'Resonance', 100, 50, 'Final fantasy'),
(64, 'Deity of underwater king', 100, 80, 'Main: Betrayal, Death');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_skills_list`
--

CREATE TABLE `tw_skills_list` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL,
  `Description` varchar(64) NOT NULL,
  `Type` int(11) NOT NULL DEFAULT 0 COMMENT '0-Improvements\r\n1-Healer\r\n2-Dps\r\n3-Tank',
  `BonusName` varchar(64) NOT NULL DEFAULT '''name''',
  `BonusValue` int(11) NOT NULL DEFAULT 1,
  `ManaPercentageCost` int(11) NOT NULL DEFAULT 10,
  `PriceSP` int(11) NOT NULL,
  `MaxLevel` int(11) NOT NULL,
  `Passive` tinyint(1) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_skills_list`
--

INSERT INTO `tw_skills_list` (`ID`, `Name`, `Description`, `Type`, `BonusName`, `BonusValue`, `ManaPercentageCost`, `PriceSP`, `MaxLevel`, `Passive`) VALUES
(1, 'Health turret', 'Creates turret a recovery health ', 1, 'life span', 3, 25, 24, 8, 0),
(2, 'Sleepy Gravity', 'Magnet mobs to itself', 3, 'radius', 20, 25, 28, 10, 0),
(3, 'Craft Discount', 'Will give discount on the price of craft items', 0, '% discount gold for craft item', 1, 0, 28, 50, 1),
(4, 'Proficiency with weapons', 'You can perform an automatic fire', 0, 'can perform an auto fire with all types of weapons', 1, 0, 120, 1, 1),
(5, 'Blessing of God of war', 'The blessing restores ammo', 3, '% recovers ammo within a radius of 800', 25, 50, 28, 4, 0),
(6, 'Noctis Lucis Attack Teleport', 'An attacking teleport that deals damage to all mobs radius', 2, '% your strength', 25, 10, 100, 4, 0);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_storages`
--

CREATE TABLE `tw_storages` (
  `ID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL DEFAULT '''Bussines name''',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `Currency` int(11) NOT NULL DEFAULT 1,
  `WorldID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_storages`
--

INSERT INTO `tw_storages` (`ID`, `Name`, `PosX`, `PosY`, `Currency`, `WorldID`) VALUES
(1, 'Weapons for young adventurers', 9417, 6817, 1, 2),
(2, 'Elfinia Artifacts', 6256, 6417, 1, 2),
(3, 'Noctis Lucis Caelum', 3200, 3520, 44, 5),
(5, 'Master La', 7352, 4859, 9, 2);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_store_items`
--

CREATE TABLE `tw_store_items` (
  `ID` int(11) NOT NULL,
  `ItemID` int(11) NOT NULL,
  `ItemValue` int(11) NOT NULL,
  `RequiredItemID` int(11) NOT NULL DEFAULT 1,
  `Price` int(11) NOT NULL,
  `UserID` int(11) NOT NULL DEFAULT 0,
  `Enchant` int(11) NOT NULL DEFAULT 0,
  `StorageID` int(11) DEFAULT NULL,
  `Time` timestamp NOT NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_store_items`
--

INSERT INTO `tw_store_items` (`ID`, `ItemID`, `ItemValue`, `RequiredItemID`, `Price`, `UserID`, `Enchant`, `StorageID`, `Time`) VALUES
(1, 3, 1, 1, 120, 0, 0, 1, '2020-05-10 18:36:16'),
(2, 4, 1, 1, 310, 0, 0, 1, '2020-05-10 18:36:16'),
(3, 5, 1, 1, 320, 0, 0, 1, '2020-05-10 18:36:16'),
(4, 6, 1, 1, 400, 0, 0, 1, '2020-05-10 18:36:16'),
(5, 28, 1, 1, 980, 0, 0, 2, '2020-05-13 21:19:28'),
(6, 36, 1, 1, 690, 0, 0, 2, '2020-05-13 21:19:28'),
(7, 8, 1, 1, 3800, 0, 0, 2, '2020-05-13 21:19:28'),
(8, 49, 1, 1, 2100, 0, 0, 2, '2020-05-13 22:19:28'),
(12, 47, 1, 1, 1200, 0, 0, 2, '2020-05-13 19:19:28'),
(13, 48, 1, 1, 1500, 0, 0, 2, '2020-05-13 19:19:28'),
(19, 38, 1, 1, 3200, 0, 0, 2, '2020-05-13 21:19:28'),
(22, 40, 1, 1, 2500, 0, 0, 2, '2020-05-13 21:19:28'),
(42, 15001, 1, 44, 300, 0, 0, 3, '2020-05-13 21:19:28'),
(43, 10017, 1, 44, 300, 0, 0, 3, '2020-05-13 21:19:28'),
(45, 50, 1, 9, 5, 0, 0, 5, '2020-05-13 22:19:28');

-- --------------------------------------------------------

--
-- Структура таблицы `tw_voucher`
--

CREATE TABLE `tw_voucher` (
  `ID` int(11) NOT NULL,
  `Code` varchar(32) NOT NULL,
  `Data` text NOT NULL,
  `Multiple` tinyint(1) NOT NULL DEFAULT 0,
  `ValidUntil` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_voucher`
--

INSERT INTO `tw_voucher` (`ID`, `Code`, `Data`, `Multiple`, `ValidUntil`) VALUES
(1, 'VALENTINE2021', '{\r\n	\"exp\": 10000,\r\n	\"items\": [\r\n		{\r\n			\"id\": 17,\r\n			\"value\": 30\r\n		},\r\n		{\r\n			\"id\": 15000,\r\n			\"value\": 1\r\n		}\r\n	]\r\n}', 1, 1614517578);

-- --------------------------------------------------------

--
-- Структура таблицы `tw_voucher_redeemed`
--

CREATE TABLE `tw_voucher_redeemed` (
  `ID` int(11) NOT NULL,
  `VoucherID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL,
  `TimeCreated` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Структура таблицы `tw_world_swap`
--

CREATE TABLE `tw_world_swap` (
  `ID` int(11) NOT NULL,
  `WorldID` int(11) DEFAULT NULL,
  `PositionX` int(11) DEFAULT NULL,
  `PositionY` int(11) DEFAULT NULL,
  `RequiredQuestID` int(11) DEFAULT NULL,
  `TwoWorldID` int(11) DEFAULT NULL,
  `TwoPositionX` int(11) DEFAULT NULL,
  `TwoPositionY` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Дамп данных таблицы `tw_world_swap`
--

INSERT INTO `tw_world_swap` (`ID`, `WorldID`, `PositionX`, `PositionY`, `RequiredQuestID`, `TwoWorldID`, `TwoPositionX`, `TwoPositionY`) VALUES
(1, 0, 6900, 1000, 1, 1, 335, 490),
(2, 1, 4605, 1067, 2, 2, 3570, 7950),
(3, 2, 13760, 6680, 12, 3, 400, 1260),
(4, 2, 3510, 6340, 13, 4, 4740, 900),
(5, 3, 4560, 1205, 19, 5, 610, 4500),
(6, 2, 8328, 6020, 15, 7, 4135, 840),
(7, 5, 4896, 4276, 22, 9, 2905, 1227),
(8, 9, 1491, 1104, NULL, 11, 12604, 2180);

--
-- Индексы сохранённых таблиц
--

--
-- Индексы таблицы `enum_behavior_mobs`
--
ALTER TABLE `enum_behavior_mobs`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Behavior` (`Behavior`);

--
-- Индексы таблицы `enum_effects_list`
--
ALTER TABLE `enum_effects_list`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Name` (`Name`);

--
-- Индексы таблицы `enum_emotes`
--
ALTER TABLE `enum_emotes`
  ADD PRIMARY KEY (`ID`);

--
-- Индексы таблицы `enum_items_functional`
--
ALTER TABLE `enum_items_functional`
  ADD PRIMARY KEY (`FunctionID`);

--
-- Индексы таблицы `enum_items_types`
--
ALTER TABLE `enum_items_types`
  ADD PRIMARY KEY (`TypeID`);

--
-- Индексы таблицы `enum_mmo_proj`
--
ALTER TABLE `enum_mmo_proj`
  ADD KEY `ID` (`ID`);

--
-- Индексы таблицы `enum_quest_interactive`
--
ALTER TABLE `enum_quest_interactive`
  ADD KEY `ID` (`ID`);

--
-- Индексы таблицы `enum_worlds`
--
ALTER TABLE `enum_worlds`
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
-- Индексы таблицы `tw_accounts_aethers`
--
ALTER TABLE `tw_accounts_aethers`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`UserID`),
  ADD KEY `TeleportID` (`AetherID`);

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
  ADD KEY `Level` (`Level`),
  ADD KEY `Exp` (`Exp`);

--
-- Индексы таблицы `tw_accounts_farming`
--
ALTER TABLE `tw_accounts_farming`
  ADD PRIMARY KEY (`UserID`),
  ADD UNIQUE KEY `AccountID` (`UserID`);

--
-- Индексы таблицы `tw_accounts_items`
--
ALTER TABLE `tw_accounts_items`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`UserID`),
  ADD KEY `ItemID` (`ItemID`);

--
-- Индексы таблицы `tw_accounts_mailbox`
--
ALTER TABLE `tw_accounts_mailbox`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`UserID`),
  ADD KEY `tw_accounts_inbox_ibfk_2` (`ItemID`);

--
-- Индексы таблицы `tw_accounts_mining`
--
ALTER TABLE `tw_accounts_mining`
  ADD PRIMARY KEY (`UserID`),
  ADD UNIQUE KEY `AccountID` (`UserID`);

--
-- Индексы таблицы `tw_accounts_quests`
--
ALTER TABLE `tw_accounts_quests`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD UNIQUE KEY `UK_tw_accounts_quests` (`QuestID`,`UserID`),
  ADD KEY `OwnerID` (`UserID`),
  ADD KEY `tw_accounts_quests_ibfk_4` (`QuestID`);

--
-- Индексы таблицы `tw_accounts_skills`
--
ALTER TABLE `tw_accounts_skills`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `SkillID` (`SkillID`),
  ADD KEY `OwnerID` (`UserID`);

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
-- Индексы таблицы `tw_bots_info`
--
ALTER TABLE `tw_bots_info`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `SlotWings` (`SlotWings`),
  ADD KEY `SlotHammer` (`SlotHammer`),
  ADD KEY `SlotGun` (`SlotGun`),
  ADD KEY `tw_bots_world_ibfk_4` (`SlotShotgun`),
  ADD KEY `SlotGrenade` (`SlotGrenade`),
  ADD KEY `SlotRifle` (`SlotRifle`);

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
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `Effect` (`Effect`),
  ADD KEY `Behavior` (`Behavior`);

--
-- Индексы таблицы `tw_bots_npc`
--
ALTER TABLE `tw_bots_npc`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `MobID` (`BotID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `tw_bots_npc_ibfk_3` (`Emote`),
  ADD KEY `tw_bots_npc_ibfk_5` (`GivesQuestID`);

--
-- Индексы таблицы `tw_bots_quest`
--
ALTER TABLE `tw_bots_quest`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `MobID` (`BotID`),
  ADD KEY `it_need_0` (`RequiredItemID1`),
  ADD KEY `tw_bots_quest_ibfk_3` (`RequiredItemID2`),
  ADD KEY `tw_bots_quest_ibfk_4` (`RewardItemID1`),
  ADD KEY `it_reward_1` (`RewardItemID2`),
  ADD KEY `QuestID` (`QuestID`),
  ADD KEY `tw_bots_quest_ibfk_6` (`RequiredDefeatMobID1`),
  ADD KEY `tw_bots_quest_ibfk_7` (`RequiredDefeatMobID2`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `interactive_type` (`InteractionType`);

--
-- Индексы таблицы `tw_crafts_list`
--
ALTER TABLE `tw_crafts_list`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `CraftIID` (`ItemID`),
  ADD KEY `Craft_Item_0` (`RequiredItemID0`),
  ADD KEY `Craft_Item_1` (`RequiredItemID1`),
  ADD KEY `Craft_Item_2` (`RequiredItemID2`),
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
  ADD KEY `tw_dungeons_records_ibfk_1` (`UserID`),
  ADD KEY `DungeonID` (`DungeonID`),
  ADD KEY `Seconds` (`Seconds`);

--
-- Индексы таблицы `tw_guilds`
--
ALTER TABLE `tw_guilds`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`UserID`),
  ADD KEY `Bank` (`Bank`),
  ADD KEY `Level` (`Level`),
  ADD KEY `Experience` (`Experience`);

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
  ADD KEY `OwnerMID` (`GuildID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_guilds_invites`
--
ALTER TABLE `tw_guilds_invites`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `OwnerID` (`UserID`),
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
  ADD KEY `OwnerID` (`UserID`),
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
  ADD KEY `ItemBonus` (`Attribute0`),
  ADD KEY `ItemID_2` (`ItemID`),
  ADD KEY `ItemType` (`Type`),
  ADD KEY `tw_items_list_ibfk_3` (`Function`),
  ADD KEY `ItemProjID` (`ProjectileID`),
  ADD KEY `tw_items_list_ibfk_5` (`Attribute1`);

--
-- Индексы таблицы `tw_logics_worlds`
--
ALTER TABLE `tw_logics_worlds`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `MobID` (`MobID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `ParseInt` (`ParseInt`);

--
-- Индексы таблицы `tw_positions_farming`
--
ALTER TABLE `tw_positions_farming`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `ItemID` (`ItemID`),
  ADD KEY `WorldID` (`WorldID`);

--
-- Индексы таблицы `tw_positions_mining`
--
ALTER TABLE `tw_positions_mining`
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
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `Currency` (`Currency`);

--
-- Индексы таблицы `tw_store_items`
--
ALTER TABLE `tw_store_items`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `ItemID` (`ItemID`),
  ADD KEY `OwnerID` (`UserID`),
  ADD KEY `StorageID` (`StorageID`),
  ADD KEY `Time` (`Time`),
  ADD KEY `NeedItem` (`RequiredItemID`),
  ADD KEY `Price` (`Price`);

--
-- Индексы таблицы `tw_voucher`
--
ALTER TABLE `tw_voucher`
  ADD PRIMARY KEY (`ID`);

--
-- Индексы таблицы `tw_voucher_redeemed`
--
ALTER TABLE `tw_voucher_redeemed`
  ADD PRIMARY KEY (`ID`);

--
-- Индексы таблицы `tw_world_swap`
--
ALTER TABLE `tw_world_swap`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `ID` (`ID`),
  ADD KEY `WorldID` (`WorldID`),
  ADD KEY `TwoWorldID` (`TwoWorldID`),
  ADD KEY `tw_world_swap_ibfk_3` (`RequiredQuestID`);

--
-- AUTO_INCREMENT для сохранённых таблиц
--

--
-- AUTO_INCREMENT для таблицы `enum_behavior_mobs`
--
ALTER TABLE `enum_behavior_mobs`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;

--
-- AUTO_INCREMENT для таблицы `enum_effects_list`
--
ALTER TABLE `enum_effects_list`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;

--
-- AUTO_INCREMENT для таблицы `enum_items_functional`
--
ALTER TABLE `enum_items_functional`
  MODIFY `FunctionID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=13;

--
-- AUTO_INCREMENT для таблицы `enum_items_types`
--
ALTER TABLE `enum_items_types`
  MODIFY `TypeID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- AUTO_INCREMENT для таблицы `tw_accounts`
--
ALTER TABLE `tw_accounts`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- AUTO_INCREMENT для таблицы `tw_accounts_aethers`
--
ALTER TABLE `tw_accounts_aethers`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=10;

--
-- AUTO_INCREMENT для таблицы `tw_accounts_data`
--
ALTER TABLE `tw_accounts_data`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- AUTO_INCREMENT для таблицы `tw_accounts_items`
--
ALTER TABLE `tw_accounts_items`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=85;

--
-- AUTO_INCREMENT для таблицы `tw_accounts_mailbox`
--
ALTER TABLE `tw_accounts_mailbox`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT для таблицы `tw_accounts_quests`
--
ALTER TABLE `tw_accounts_quests`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=57;

--
-- AUTO_INCREMENT для таблицы `tw_accounts_skills`
--
ALTER TABLE `tw_accounts_skills`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- AUTO_INCREMENT для таблицы `tw_aethers`
--
ALTER TABLE `tw_aethers`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- AUTO_INCREMENT для таблицы `tw_bots_info`
--
ALTER TABLE `tw_bots_info`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=56;

--
-- AUTO_INCREMENT для таблицы `tw_bots_mobs`
--
ALTER TABLE `tw_bots_mobs`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=23;

--
-- AUTO_INCREMENT для таблицы `tw_bots_npc`
--
ALTER TABLE `tw_bots_npc`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=33;

--
-- AUTO_INCREMENT для таблицы `tw_bots_quest`
--
ALTER TABLE `tw_bots_quest`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=81;

--
-- AUTO_INCREMENT для таблицы `tw_crafts_list`
--
ALTER TABLE `tw_crafts_list`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=13;

--
-- AUTO_INCREMENT для таблицы `tw_dungeons`
--
ALTER TABLE `tw_dungeons`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;

--
-- AUTO_INCREMENT для таблицы `tw_dungeons_door`
--
ALTER TABLE `tw_dungeons_door`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=12;

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
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;

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
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

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
-- AUTO_INCREMENT для таблицы `tw_logics_worlds`
--
ALTER TABLE `tw_logics_worlds`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT для таблицы `tw_positions_farming`
--
ALTER TABLE `tw_positions_farming`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;

--
-- AUTO_INCREMENT для таблицы `tw_positions_mining`
--
ALTER TABLE `tw_positions_mining`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;

--
-- AUTO_INCREMENT для таблицы `tw_quests_list`
--
ALTER TABLE `tw_quests_list`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=65;

--
-- AUTO_INCREMENT для таблицы `tw_skills_list`
--
ALTER TABLE `tw_skills_list`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- AUTO_INCREMENT для таблицы `tw_storages`
--
ALTER TABLE `tw_storages`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;

--
-- AUTO_INCREMENT для таблицы `tw_store_items`
--
ALTER TABLE `tw_store_items`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=268;

--
-- AUTO_INCREMENT для таблицы `tw_voucher`
--
ALTER TABLE `tw_voucher`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT для таблицы `tw_voucher_redeemed`
--
ALTER TABLE `tw_voucher_redeemed`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT для таблицы `tw_world_swap`
--
ALTER TABLE `tw_world_swap`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- Ограничения внешнего ключа сохраненных таблиц
--

--
-- Ограничения внешнего ключа таблицы `tw_accounts_aethers`
--
ALTER TABLE `tw_accounts_aethers`
  ADD CONSTRAINT `tw_accounts_aethers_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_aethers_ibfk_2` FOREIGN KEY (`AetherID`) REFERENCES `tw_aethers` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_data`
--
ALTER TABLE `tw_accounts_data`
  ADD CONSTRAINT `tw_accounts_data_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_data_ibfk_4` FOREIGN KEY (`GuildRank`) REFERENCES `tw_guilds_ranks` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_data_ibfk_5` FOREIGN KEY (`ID`) REFERENCES `tw_accounts` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_farming`
--
ALTER TABLE `tw_accounts_farming`
  ADD CONSTRAINT `tw_accounts_farming_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_items`
--
ALTER TABLE `tw_accounts_items`
  ADD CONSTRAINT `tw_accounts_items_ibfk_1` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_items_ibfk_2` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_mailbox`
--
ALTER TABLE `tw_accounts_mailbox`
  ADD CONSTRAINT `tw_accounts_mailbox_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_mailbox_ibfk_2` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_mining`
--
ALTER TABLE `tw_accounts_mining`
  ADD CONSTRAINT `tw_accounts_mining_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_quests`
--
ALTER TABLE `tw_accounts_quests`
  ADD CONSTRAINT `tw_accounts_quests_ibfk_3` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_quests_ibfk_4` FOREIGN KEY (`QuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_accounts_skills`
--
ALTER TABLE `tw_accounts_skills`
  ADD CONSTRAINT `tw_accounts_skills_ibfk_1` FOREIGN KEY (`SkillID`) REFERENCES `tw_skills_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_accounts_skills_ibfk_2` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_aethers`
--
ALTER TABLE `tw_aethers`
  ADD CONSTRAINT `tw_aethers_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_info`
--
ALTER TABLE `tw_bots_info`
  ADD CONSTRAINT `tw_bots_info_ibfk_1` FOREIGN KEY (`SlotWings`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_info_ibfk_2` FOREIGN KEY (`SlotHammer`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_info_ibfk_3` FOREIGN KEY (`SlotGun`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_info_ibfk_4` FOREIGN KEY (`SlotShotgun`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_info_ibfk_5` FOREIGN KEY (`SlotGrenade`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_info_ibfk_6` FOREIGN KEY (`SlotRifle`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_mobs`
--
ALTER TABLE `tw_bots_mobs`
  ADD CONSTRAINT `tw_bots_mobs_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_10` FOREIGN KEY (`it_drop_1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_11` FOREIGN KEY (`it_drop_2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_12` FOREIGN KEY (`it_drop_3`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_13` FOREIGN KEY (`it_drop_4`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_15` FOREIGN KEY (`Effect`) REFERENCES `enum_effects_list` (`Name`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_16` FOREIGN KEY (`Behavior`) REFERENCES `enum_behavior_mobs` (`Behavior`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_8` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_mobs_ibfk_9` FOREIGN KEY (`it_drop_0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_npc`
--
ALTER TABLE `tw_bots_npc`
  ADD CONSTRAINT `tw_bots_npc_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_npc_ibfk_3` FOREIGN KEY (`Emote`) REFERENCES `enum_emotes` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `tw_bots_npc_ibfk_4` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_npc_ibfk_5` FOREIGN KEY (`GivesQuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_bots_quest`
--
ALTER TABLE `tw_bots_quest`
  ADD CONSTRAINT `tw_bots_quest_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_10` FOREIGN KEY (`InteractionType`) REFERENCES `enum_quest_interactive` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_2` FOREIGN KEY (`RequiredItemID1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_3` FOREIGN KEY (`RequiredItemID2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_4` FOREIGN KEY (`RewardItemID1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_5` FOREIGN KEY (`RewardItemID2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_6` FOREIGN KEY (`RequiredDefeatMobID1`) REFERENCES `tw_bots_info` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_7` FOREIGN KEY (`RequiredDefeatMobID2`) REFERENCES `tw_bots_info` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_8` FOREIGN KEY (`QuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_bots_quest_ibfk_9` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_crafts_list`
--
ALTER TABLE `tw_crafts_list`
  ADD CONSTRAINT `tw_crafts_list_ibfk_1` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_crafts_list_ibfk_2` FOREIGN KEY (`RequiredItemID0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_crafts_list_ibfk_3` FOREIGN KEY (`RequiredItemID1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_crafts_list_ibfk_4` FOREIGN KEY (`RequiredItemID2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_dungeons`
--
ALTER TABLE `tw_dungeons`
  ADD CONSTRAINT `tw_dungeons_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE NO ACTION ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_dungeons_door`
--
ALTER TABLE `tw_dungeons_door`
  ADD CONSTRAINT `tw_dungeons_door_ibfk_1` FOREIGN KEY (`DungeonID`) REFERENCES `tw_dungeons` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_dungeons_door_ibfk_2` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_dungeons_records`
--
ALTER TABLE `tw_dungeons_records`
  ADD CONSTRAINT `tw_dungeons_records_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_dungeons_records_ibfk_2` FOREIGN KEY (`DungeonID`) REFERENCES `tw_dungeons` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds`
--
ALTER TABLE `tw_guilds`
  ADD CONSTRAINT `tw_guilds_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds_decorations`
--
ALTER TABLE `tw_guilds_decorations`
  ADD CONSTRAINT `tw_guilds_decorations_ibfk_2` FOREIGN KEY (`DecoID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_guilds_decorations_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE,
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
  ADD CONSTRAINT `tw_guilds_houses_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_guilds_houses_ibfk_2` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_guilds_invites`
--
ALTER TABLE `tw_guilds_invites`
  ADD CONSTRAINT `tw_guilds_invites_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
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
  ADD CONSTRAINT `tw_houses_ibfk_2` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_houses_decorations`
--
ALTER TABLE `tw_houses_decorations`
  ADD CONSTRAINT `tw_houses_decorations_ibfk_1` FOREIGN KEY (`HouseID`) REFERENCES `tw_houses` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_houses_decorations_ibfk_2` FOREIGN KEY (`DecoID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_houses_decorations_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_items_list`
--
ALTER TABLE `tw_items_list`
  ADD CONSTRAINT `tw_items_list_ibfk_1` FOREIGN KEY (`Type`) REFERENCES `enum_items_types` (`TypeID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_items_list_ibfk_2` FOREIGN KEY (`Function`) REFERENCES `enum_items_functional` (`FunctionID`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Ограничения внешнего ключа таблицы `tw_logics_worlds`
--
ALTER TABLE `tw_logics_worlds`
  ADD CONSTRAINT `tw_logics_worlds_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `tw_world_swap` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tw_logics_worlds_ibfk_2` FOREIGN KEY (`ParseInt`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
