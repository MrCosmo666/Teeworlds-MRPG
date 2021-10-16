-- MySQL dump 10.13  Distrib 8.0.23, for Win64 (x86_64)
--
-- Host: 127.0.0.1    Database: mrpg_test
-- ------------------------------------------------------
-- Server version	5.6.51-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `enum_behavior_mobs`
--

DROP TABLE IF EXISTS `enum_behavior_mobs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_behavior_mobs` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Behavior` varchar(32) NOT NULL DEFAULT 'Standard',
  PRIMARY KEY (`ID`),
  KEY `Behavior` (`Behavior`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_behavior_mobs`
--

LOCK TABLES `enum_behavior_mobs` WRITE;
/*!40000 ALTER TABLE `enum_behavior_mobs` DISABLE KEYS */;
INSERT INTO `enum_behavior_mobs` VALUES (3,'Sleepy'),(2,'Slime'),(1,'Standard');
/*!40000 ALTER TABLE `enum_behavior_mobs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_effects_list`
--

DROP TABLE IF EXISTS `enum_effects_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_effects_list` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(16) CHARACTER SET utf8mb4 DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `Name` (`Name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_effects_list`
--

LOCK TABLES `enum_effects_list` WRITE;
/*!40000 ALTER TABLE `enum_effects_list` DISABLE KEYS */;
INSERT INTO `enum_effects_list` VALUES (3,'Fire'),(2,'Poison'),(1,'Slowdown');
/*!40000 ALTER TABLE `enum_effects_list` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_emotes`
--

DROP TABLE IF EXISTS `enum_emotes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_emotes` (
  `ID` int(11) NOT NULL,
  `Emote` varchar(64) NOT NULL DEFAULT 'nope',
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_emotes`
--

LOCK TABLES `enum_emotes` WRITE;
/*!40000 ALTER TABLE `enum_emotes` DISABLE KEYS */;
INSERT INTO `enum_emotes` VALUES (0,'Normal Emote'),(1,'Pain Emote'),(2,'Happy Emote'),(3,'Surprise Emote'),(4,'Angry Emote'),(5,'Blink Emote');
/*!40000 ALTER TABLE `enum_emotes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_items_functional`
--

DROP TABLE IF EXISTS `enum_items_functional`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_items_functional` (
  `FunctionID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(64) NOT NULL,
  PRIMARY KEY (`FunctionID`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_items_functional`
--

LOCK TABLES `enum_items_functional` WRITE;
/*!40000 ALTER TABLE `enum_items_functional` DISABLE KEYS */;
INSERT INTO `enum_items_functional` VALUES (-1,'Not have function'),(0,'Equip hammer(Only equip type)'),(1,'Equip gun(Only equip type)'),(2,'Equip shotgun(Only equip type)'),(3,'Equip grenade(Only equip type)'),(4,'Equip rifle(Only equip type)'),(5,'Equip miner(Only equip type)'),(6,'Equip wings(Only equip type)'),(7,'Equip discord(Only equip type)'),(8,'Once use item x1'),(9,'Several times use item x99'),(10,'Settings(Only settings or modules type)'),(11,'Plants item'),(12,'Mining item');
/*!40000 ALTER TABLE `enum_items_functional` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_items_types`
--

DROP TABLE IF EXISTS `enum_items_types`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_items_types` (
  `TypeID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(32) NOT NULL,
  PRIMARY KEY (`TypeID`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_items_types`
--

LOCK TABLES `enum_items_types` WRITE;
/*!40000 ALTER TABLE `enum_items_types` DISABLE KEYS */;
INSERT INTO `enum_items_types` VALUES (-1,'Invisible'),(1,'Useds'),(2,'Crafts'),(3,'Modules'),(4,'Others'),(5,'Settings'),(6,'Equipping'),(7,'Decorations'),(8,'Potions');
/*!40000 ALTER TABLE `enum_items_types` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_mmo_proj`
--

DROP TABLE IF EXISTS `enum_mmo_proj`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_mmo_proj` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL,
  KEY `ID` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_mmo_proj`
--

LOCK TABLES `enum_mmo_proj` WRITE;
/*!40000 ALTER TABLE `enum_mmo_proj` DISABLE KEYS */;
INSERT INTO `enum_mmo_proj` VALUES (0,'Magitech Gun'),(1,'Magitech Shotgun'),(2,'Magitech Grenade'),(-1,'No Proj'),(3,'Heavenly Gun'),(4,'Heavenly Shotgun'),(5,'Heavenly Grenade'),(6,'Goblin Gun'),(7,'Goblin Shotgun'),(8,'Goblin Grenade');
/*!40000 ALTER TABLE `enum_mmo_proj` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_quest_interactive`
--

DROP TABLE IF EXISTS `enum_quest_interactive`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_quest_interactive` (
  `ID` int(11) NOT NULL,
  `Name` varchar(64) NOT NULL,
  KEY `ID` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_quest_interactive`
--

LOCK TABLES `enum_quest_interactive` WRITE;
/*!40000 ALTER TABLE `enum_quest_interactive` DISABLE KEYS */;
INSERT INTO `enum_quest_interactive` VALUES (1,'Randomly accept or refuse with the item'),(2,'Pick up items that NPC will drop.');
/*!40000 ALTER TABLE `enum_quest_interactive` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_talk_styles`
--

DROP TABLE IF EXISTS `enum_talk_styles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_talk_styles` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Style` varchar(64) NOT NULL DEFAULT 'nope',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `ID_2` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_talk_styles`
--

LOCK TABLES `enum_talk_styles` WRITE;
/*!40000 ALTER TABLE `enum_talk_styles` DISABLE KEYS */;
INSERT INTO `enum_talk_styles` VALUES (0,'Basic Talking'),(1,'Aggresive Talking'),(2,'Happed Talking');
/*!40000 ALTER TABLE `enum_talk_styles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `enum_worlds`
--

DROP TABLE IF EXISTS `enum_worlds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `enum_worlds` (
  `WorldID` int(11) NOT NULL,
  `Name` varchar(32) CHARACTER SET utf8 NOT NULL,
  `RespawnWorld` int(11) DEFAULT NULL,
  `MusicID` int(11) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`WorldID`),
  KEY `WorldID` (`WorldID`),
  KEY `Name` (`Name`),
  KEY `SafeZoneWorldID` (`RespawnWorld`),
  KEY `WorldID_2` (`WorldID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `enum_worlds`
--

LOCK TABLES `enum_worlds` WRITE;
/*!40000 ALTER TABLE `enum_worlds` DISABLE KEYS */;
INSERT INTO `enum_worlds` VALUES (0,'Pier Elfinia',NULL,53),(1,'Way to the Elfinia',1,54),(2,'Elfinia',2,53),(3,'Elfinia Deep cave',2,54),(4,'Elfia home room',2,53),(5,'Elfinia occupation of goblins',5,54),(6,'Elfinia Abandoned mine',NULL,56),(7,'Diana home room',2,53),(8,'Noctis Resonance',NULL,55),(9,'Departure',9,53),(10,'Underwater of Neptune',10,55),(11,'Yugasaki',NULL,-1);
/*!40000 ALTER TABLE `enum_worlds` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts`
--

DROP TABLE IF EXISTS `tw_accounts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Username` varchar(64) NOT NULL,
  `Password` varchar(64) NOT NULL,
  `PasswordSalt` varchar(64) DEFAULT NULL,
  `RegisterDate` varchar(64) NOT NULL,
  `LoginDate` varchar(64) NOT NULL DEFAULT 'First log in',
  `RegisteredIP` varchar(64) NOT NULL DEFAULT '0.0.0.0',
  `LoginIP` varchar(64) NOT NULL DEFAULT '0.0.0.0',
  `Language` varchar(8) NOT NULL DEFAULT 'en',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `Password` (`Password`),
  KEY `Username` (`Username`)
) ENGINE=InnoDB AUTO_INCREMENT=542 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts`
--

LOCK TABLES `tw_accounts` WRITE;
/*!40000 ALTER TABLE `tw_accounts` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_aethers`
--

DROP TABLE IF EXISTS `tw_accounts_aethers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_aethers` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `UserID` int(11) NOT NULL,
  `AetherID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `OwnerID` (`UserID`),
  KEY `TeleportID` (`AetherID`),
  CONSTRAINT `tw_accounts_aethers_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_accounts_aethers_ibfk_2` FOREIGN KEY (`AetherID`) REFERENCES `tw_aethers` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=643 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_aethers`
--

LOCK TABLES `tw_accounts_aethers` WRITE;
/*!40000 ALTER TABLE `tw_accounts_aethers` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_aethers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_data`
--

DROP TABLE IF EXISTS `tw_accounts_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_data` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
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
  PRIMARY KEY (`ID`),
  UNIQUE KEY `Nick` (`Nick`),
  KEY `MemberID` (`GuildID`),
  KEY `DiscordID` (`DiscordID`),
  KEY `tw_accounts_data_ibfk_3` (`WorldID`),
  KEY `GuildRank` (`GuildRank`),
  KEY `Level` (`Level`),
  KEY `Exp` (`Exp`),
  CONSTRAINT `tw_accounts_data_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_accounts_data_ibfk_4` FOREIGN KEY (`GuildRank`) REFERENCES `tw_guilds_ranks` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_accounts_data_ibfk_5` FOREIGN KEY (`ID`) REFERENCES `tw_accounts` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=542 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_data`
--

LOCK TABLES `tw_accounts_data` WRITE;
/*!40000 ALTER TABLE `tw_accounts_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_farming`
--

DROP TABLE IF EXISTS `tw_accounts_farming`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_farming` (
  `UserID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Exp` int(11) NOT NULL DEFAULT '0',
  `Quantity` int(11) NOT NULL DEFAULT '1',
  `Upgrade` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`UserID`),
  UNIQUE KEY `AccountID` (`UserID`),
  CONSTRAINT `tw_accounts_farming_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_farming`
--

LOCK TABLES `tw_accounts_farming` WRITE;
/*!40000 ALTER TABLE `tw_accounts_farming` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_farming` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_items`
--

DROP TABLE IF EXISTS `tw_accounts_items`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_items` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ItemID` int(11) NOT NULL,
  `Value` int(11) NOT NULL,
  `Settings` int(11) NOT NULL,
  `Enchant` int(11) NOT NULL,
  `Durability` int(11) NOT NULL DEFAULT '100',
  `UserID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `OwnerID` (`UserID`),
  KEY `ItemID` (`ItemID`),
  CONSTRAINT `tw_accounts_items_ibfk_1` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_accounts_items_ibfk_2` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=6177 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_items`
--

LOCK TABLES `tw_accounts_items` WRITE;
/*!40000 ALTER TABLE `tw_accounts_items` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_items` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_mailbox`
--

DROP TABLE IF EXISTS `tw_accounts_mailbox`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_mailbox` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ItemID` int(11) DEFAULT NULL,
  `ItemValue` int(11) DEFAULT NULL,
  `Enchant` int(11) DEFAULT NULL,
  `Name` varchar(64) NOT NULL,
  `Description` varchar(64) NOT NULL,
  `UserID` int(11) NOT NULL,
  `IsRead` tinyint(4) NOT NULL DEFAULT '0',
  `FromSend` varchar(32) NOT NULL DEFAULT 'Game',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `OwnerID` (`UserID`),
  KEY `tw_accounts_inbox_ibfk_2` (`ItemID`),
  CONSTRAINT `tw_accounts_mailbox_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_accounts_mailbox_ibfk_2` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=197 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_mailbox`
--

LOCK TABLES `tw_accounts_mailbox` WRITE;
/*!40000 ALTER TABLE `tw_accounts_mailbox` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_mailbox` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_mining`
--

DROP TABLE IF EXISTS `tw_accounts_mining`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_mining` (
  `UserID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Exp` int(11) NOT NULL DEFAULT '0',
  `Upgrade` int(11) NOT NULL DEFAULT '0',
  `Quantity` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`UserID`),
  UNIQUE KEY `AccountID` (`UserID`),
  CONSTRAINT `tw_accounts_mining_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_mining`
--

LOCK TABLES `tw_accounts_mining` WRITE;
/*!40000 ALTER TABLE `tw_accounts_mining` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_mining` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_quests`
--

DROP TABLE IF EXISTS `tw_accounts_quests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_quests` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `QuestID` int(11) DEFAULT NULL,
  `UserID` int(11) NOT NULL,
  `Step` int(11) NOT NULL DEFAULT '1',
  `Type` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  UNIQUE KEY `UK_tw_accounts_quests` (`QuestID`,`UserID`),
  KEY `OwnerID` (`UserID`),
  KEY `tw_accounts_quests_ibfk_4` (`QuestID`),
  CONSTRAINT `tw_accounts_quests_ibfk_3` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_accounts_quests_ibfk_4` FOREIGN KEY (`QuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3448 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_quests`
--

LOCK TABLES `tw_accounts_quests` WRITE;
/*!40000 ALTER TABLE `tw_accounts_quests` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_quests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_accounts_skills`
--

DROP TABLE IF EXISTS `tw_accounts_skills`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_accounts_skills` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `SkillID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL,
  `Level` int(11) NOT NULL,
  `UsedByEmoticon` int(11) DEFAULT '-1',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `SkillID` (`SkillID`),
  KEY `OwnerID` (`UserID`),
  CONSTRAINT `tw_accounts_skills_ibfk_1` FOREIGN KEY (`SkillID`) REFERENCES `tw_skills_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_accounts_skills_ibfk_2` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=110 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_accounts_skills`
--

LOCK TABLES `tw_accounts_skills` WRITE;
/*!40000 ALTER TABLE `tw_accounts_skills` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_accounts_skills` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_aethers`
--

DROP TABLE IF EXISTS `tw_aethers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_aethers` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(64) NOT NULL DEFAULT 'Teleport name',
  `WorldID` int(11) NOT NULL,
  `TeleX` int(11) NOT NULL,
  `TeleY` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `WorldID` (`WorldID`),
  CONSTRAINT `tw_aethers_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_aethers`
--

LOCK TABLES `tw_aethers` WRITE;
/*!40000 ALTER TABLE `tw_aethers` DISABLE KEYS */;
INSERT INTO `tw_aethers` VALUES (1,'Crossroad',2,8033,7089),(2,'Pier',0,3680,1150),(3,'Guard post',5,1536,4396);
/*!40000 ALTER TABLE `tw_aethers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_attributs`
--

DROP TABLE IF EXISTS `tw_attributs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_attributs` (
  `ID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `FieldName` varchar(32) NOT NULL DEFAULT 'unfield',
  `Price` int(11) NOT NULL,
  `Type` int(11) NOT NULL COMMENT '0.tank1.healer2.dps3.weapon4.hard5.jobs 6. others',
  `Divide` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_attributs`
--

LOCK TABLES `tw_attributs` WRITE;
/*!40000 ALTER TABLE `tw_attributs` DISABLE KEYS */;
INSERT INTO `tw_attributs` VALUES (1,'Shotgun Spread','SpreadShotgun',100,3,0),(2,'Grenade Spread','SpreadGrenade',100,3,0),(3,'Rifle Spread','SpreadRifle',100,3,0),(4,'Strength','unfield',0,4,10),(5,'Dexterity','Dexterity',1,2,5),(6,'Crit Dmg','CriticalHit',1,2,5),(7,'Direct Crit Dmg','DirectCriticalHit',1,2,5),(8,'Hardness','Hardness',1,0,5),(9,'Lucky','Lucky',1,0,5),(10,'Piety','Piety',1,1,5),(11,'Vampirism','Vampirism',1,1,5),(12,'Ammo Regen','AmmoRegen',1,3,5),(13,'Ammo','Ammo',30,3,0),(14,'Efficiency','unfield',-1,5,0),(15,'Extraction','unfield',-1,5,0),(16,'Hammer Power','unfield',-1,4,10),(17,'Gun Power','unfield',-1,4,10),(18,'Shotgun Power','unfield',-1,4,10),(19,'Grenade Power','unfield',-1,4,10),(20,'Rifle Power','unfield',-1,4,10),(21,'Lucky items','unfield',-1,6,5);
/*!40000 ALTER TABLE `tw_attributs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_bots_info`
--

DROP TABLE IF EXISTS `tw_bots_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_bots_info` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(32) NOT NULL DEFAULT 'Bot name',
  `SkinName` varchar(128) NOT NULL DEFAULT '''bear standard standard standard standard standard''' COMMENT 'body marking deco hands feet eyes',
  `SkinColor` varchar(128) NOT NULL DEFAULT '''0 0 0 0 0 0''' COMMENT 'body marking deco hands feet eyes	',
  `SlotHammer` int(11) DEFAULT NULL,
  `SlotGun` int(11) DEFAULT NULL,
  `SlotShotgun` int(11) DEFAULT NULL,
  `SlotGrenade` int(11) DEFAULT NULL,
  `SlotRifle` int(11) DEFAULT NULL,
  `SlotWings` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `SlotWings` (`SlotWings`),
  KEY `SlotHammer` (`SlotHammer`),
  KEY `SlotGun` (`SlotGun`),
  KEY `tw_bots_world_ibfk_4` (`SlotShotgun`),
  KEY `SlotGrenade` (`SlotGrenade`),
  KEY `SlotRifle` (`SlotRifle`),
  CONSTRAINT `tw_bots_info_ibfk_1` FOREIGN KEY (`SlotWings`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_info_ibfk_2` FOREIGN KEY (`SlotHammer`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_info_ibfk_3` FOREIGN KEY (`SlotGun`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_info_ibfk_4` FOREIGN KEY (`SlotShotgun`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_info_ibfk_5` FOREIGN KEY (`SlotGrenade`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_info_ibfk_6` FOREIGN KEY (`SlotRifle`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=54 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_bots_info`
--

LOCK TABLES `tw_bots_info` WRITE;
/*!40000 ALTER TABLE `tw_bots_info` DISABLE KEYS */;
INSERT INTO `tw_bots_info` VALUES (1,'Captain','puar warstripes hair standard standard standardreal','16187160 -16645889 131327 16177260 7624169 65408',10010,NULL,NULL,NULL,NULL,NULL),(2,'Sailor','standard bear hair standard standard standard','1082745 -15634776 65408 1082745 1147174 65408',NULL,NULL,NULL,NULL,NULL,NULL),(3,'Carpenter','puar tiger2  duotone standard moustache','15007529 -14563483 65408 3827951 1310537 65422',NULL,NULL,NULL,NULL,NULL,NULL),(4,'Worker','standard cammostripes  standard standard standard','1821867 -14840320 65408 750848 1944919 65408',NULL,NULL,NULL,NULL,NULL,NULL),(5,'Mr. Worker','trela cammostripes  standard standard standard','1662583 -14840320 65408 750848 1944919 65408',NULL,NULL,NULL,NULL,NULL,NULL),(6,'Diana','raccoon coonfluff  standard standard standard','965254 -15235151 65408 1769643 1305243 1085234',NULL,NULL,NULL,NULL,NULL,NULL),(7,'Mr. Guard','flokes flokes_gray  standard standard standard','1638493 -15138752 65408 1769472 1638422 255',NULL,NULL,NULL,NULL,NULL,NULL),(8,'Brother','bear panda1 hair standard standard standard','9834574 -6411543 65408 1769630 1835070 41215',NULL,NULL,NULL,NULL,NULL,NULL),(9,'Green slime','bear downdony hair duotone standard standard','3981441 -29333592 5313052 14500779 5468601 6321790',NULL,NULL,NULL,NULL,NULL,NULL),(10,'Apostle Elfia','kitty bear  duotone standard standard','9568256 -1695744161 12254015 3972544 15204215 7470034',10024,NULL,NULL,NULL,NULL,10017),(11,'Craftsman','bear tiger1 twinpen duotone standard colorable','12770027 828507979 11162385 6849346 44458 8581506',NULL,NULL,NULL,NULL,NULL,NULL),(12,'Auctionist','kitty saddo twinbopp duotone standard colorable','12201075 -855657567 2205432 3349551 6943484 13531062',NULL,NULL,NULL,NULL,NULL,NULL),(13,'Teacher','mouse purelove unibop duotone standard moustache','8467692 -1394954365 12408709 11534535 3010026 5627093',NULL,NULL,NULL,NULL,NULL,NULL),(14,'Nurse','flokes downdony hair duotone standard standard','15920331 1593835335 4147122 3795484 16279737 10976418',NULL,NULL,NULL,NULL,NULL,NULL),(15,'Gunsmith Eric','kitty cammostripes hair duotone standard standard','5209108 1463395762 12628238 8169037 3830859 2771259',NULL,NULL,NULL,NULL,NULL,NULL),(16,'Mr. Sentry','raccoon tripledon hair duotone standard standard','10060032 1690185928 11278269 5677608 886610 13831075',NULL,NULL,NULL,NULL,NULL,NULL),(18,'Daughter Sailor','kitty whisker hair duotone standard standard','3981441 -18874784 5313052 14500779 8614329 6321790',NULL,NULL,NULL,NULL,NULL,NULL),(19,'Miner','standard toptri hair duotone standard colorable','16718165 -2012161487 3858792 7940502 3052250 1873584',NULL,NULL,NULL,NULL,NULL,NULL),(20,'Customs officer','kitty cammo1 hair duotone standard standardreal','6083870 -2125570185 6324018 7470633 16541982 2000684',NULL,NULL,NULL,NULL,NULL,NULL),(21,'Seller artifact','flokes saddo hair duotone standard colorable','13769006 -35537528 12473004 465468 14009515 803609',NULL,NULL,NULL,NULL,NULL,NULL),(22,'Goblin','bear hipbel  duotone standard standard','6301461 -1695632164 12254015 3972544 9710385 7470034',NULL,NULL,NULL,NULL,NULL,NULL),(23,'Blue slime','bear bear  duotone standard standard','9610633 -1695632164 12254015 3972544 9729630 7470034',NULL,NULL,NULL,NULL,NULL,NULL),(25,'Adventurer Koto','bear belly1 hair2 duotone standard standardreal','2307609 -167295050 13142259 5885245 9371648 7807949',NULL,NULL,NULL,NULL,NULL,NULL),(26,'Messenger','puar duodonny  duotone standard negative','7018125 -2047512102 9555542 2060938 1379442 14886533',NULL,NULL,NULL,NULL,NULL,NULL),(27,'Salantra','kitty bear  duotone standard colorable','15557993 -346596366 16130358 701247 11272539 4707488',NULL,NULL,NULL,NULL,NULL,NULL),(28,'Pink Slime','bear bear hair2 duotone standard standard','14212759 -2065039105 16307984 12212874 16062570 8064920',NULL,NULL,NULL,NULL,NULL,NULL),(29,'Dead miner','kitty stripe hair duotone standard colorable','15995954 121878199 10365700 1972392 8301545 1240658',NULL,NULL,NULL,NULL,NULL,NULL),(30,'Hobgoblin','bear cammo2 twinmello duotone standard negative','4331554 -1830111313 6962262 8759142 9574495 10700700',NULL,NULL,NULL,NULL,NULL,NULL),(31,'Orc','bear downdony  duotone standard standard','4136201 -406977393 5258828 12493916 2359072 5233408',NULL,NULL,NULL,NULL,NULL,NULL),(32,'Leader Orcs','monkey twinbelly  duotone standard standardreal','16495882 -1326302539 9549353 8668187 2063263 1406656',10019,10020,10021,10022,10023,10016),(33,'Twins','bear bear  duotone standard standard','9683673 672388260 12653934 1134912 4360420 8598003',NULL,NULL,NULL,NULL,NULL,NULL),(34,'Kappa','standard sidemarks unibop duotone standard colorable','4821534 821909703 1852219 1132310 5264192 15588285',NULL,NULL,NULL,NULL,NULL,NULL),(35,'Orc warrior','fox cammostripes  standard zilly!0007 zilly!0007','65408 -16711808 65408 65408 65408 65408',NULL,NULL,NULL,NULL,NULL,NULL),(36,'Brown slime','bear duodonny hair duotone standard standard','1091872 -394592059 12674866 8116231 1151 14198436',NULL,NULL,NULL,NULL,NULL,NULL),(37,'Officer Henry','flokes warstripes hair2 duotone standard standard','5482497 -1837124528 15809857 10752857 7539007 3358899',NULL,NULL,NULL,NULL,NULL,NULL),(38,'Daughter Maria','flokes duodonny hair duotone standard standardreal','2653584 -36345458 3470770 9908420 14444105 738196',NULL,NULL,NULL,NULL,NULL,NULL),(39,'Noctis','kitty mice hair duotone standard standard','11017984 1596522751 5772960 14417927 5570560 9381961',10010,NULL,NULL,NULL,NULL,10017),(40,'Chocobo','kitty bear hair duotone standard standard','2490221 -1792213144 2096979 2157678 1735229 1103953',NULL,NULL,NULL,NULL,NULL,NULL),(41,'Benny','bear tricircular hair duotone standard colorable','16066652 -1087601154 3829858 1314019 641021 1376511',NULL,NULL,NULL,NULL,NULL,NULL),(42,'Rogalia','monkey donny unibop duotone standard colorable','8347469 -852808362 15666037 15456343 5845918 7003270',NULL,NULL,NULL,NULL,NULL,NULL),(43,'Kengo','monkey donny unibop duotone standard colorable','8347469 -852808362 15666037 15456343 5845918 7003270',NULL,NULL,NULL,NULL,NULL,NULL),(44,'Goshii','bear whisker hair2 duotone standard standard','10571316 -1915300733 1789119 12372457 338092 9300965',10019,10020,10021,10022,10023,10018),(45,'Maid','flokes downdony twinmello duotone standard standardreal','4260095 -1698513476 10819474 1784648 14990515 10338195',NULL,NULL,NULL,NULL,NULL,NULL),(46,'Yasue San','mouse flokes unibop duotone standard colorable','8925949 -1578020608 9437184 14815652 862504 8671829',NULL,NULL,NULL,NULL,NULL,NULL),(47,'Dead soul','bear sidemarks horns duotone standard standardreal','692505 -1831891179 13771590 7098987 3994129 8711018',NULL,NULL,NULL,NULL,NULL,NULL),(48,'Skeleton','standard mixture1 twinpen duotone standard colorable','9830655 1328873727 4260095 11171578 11796735 4135487',NULL,NULL,NULL,NULL,NULL,NULL),(49,'Librarian','trela bear twinmello duotone standard sunglasses','11686168 -1962207864 7675723 14445598 12185649 15772890',NULL,NULL,NULL,NULL,NULL,NULL),(50,'Neptune','spiky cammo1 twinmello duotone standard standardreal','7526428 -1490205206 9650234 16143024 5143432 7378889',10019,10020,10021,10022,10023,10006),(51,'Farmer','standard cammostripes  standard standard standard','1821867 -14840320 65408 750848 1944919 65408',NULL,NULL,NULL,NULL,NULL,NULL),(53,'Master','puar mice  duotone standard moustache','11763922 -15685931 65408 1102450 1232060 1376256',NULL,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `tw_bots_info` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_bots_mobs`
--

DROP TABLE IF EXISTS `tw_bots_mobs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_bots_mobs` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `BotID` int(11) NOT NULL DEFAULT '-1',
  `WorldID` int(11) DEFAULT NULL,
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Effect` varchar(16) DEFAULT NULL,
  `Behavior` varchar(32) NOT NULL DEFAULT 'Standard',
  `Level` int(11) NOT NULL DEFAULT '1',
  `Power` int(11) NOT NULL DEFAULT '10',
  `Spread` int(11) NOT NULL DEFAULT '0',
  `Number` int(11) NOT NULL DEFAULT '1',
  `Respawn` int(11) NOT NULL DEFAULT '1',
  `Boss` tinyint(1) NOT NULL DEFAULT '0',
  `it_drop_0` int(11) DEFAULT NULL,
  `it_drop_1` int(11) DEFAULT NULL,
  `it_drop_2` int(11) DEFAULT NULL,
  `it_drop_3` int(11) DEFAULT NULL,
  `it_drop_4` int(11) DEFAULT NULL,
  `it_drop_count` varchar(64) NOT NULL DEFAULT '[0][0][0][0][0]',
  `it_drop_chance` varchar(64) NOT NULL DEFAULT '[0][0][0][0][0]',
  PRIMARY KEY (`ID`),
  KEY `MobID` (`BotID`),
  KEY `it_drop_0` (`it_drop_0`),
  KEY `it_drop_1` (`it_drop_1`),
  KEY `it_drop_2` (`it_drop_2`),
  KEY `it_drop_3` (`it_drop_3`),
  KEY `it_drop_4` (`it_drop_4`),
  KEY `WorldID` (`WorldID`),
  KEY `Effect` (`Effect`),
  KEY `Behavior` (`Behavior`),
  CONSTRAINT `tw_bots_mobs_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_10` FOREIGN KEY (`it_drop_1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_11` FOREIGN KEY (`it_drop_2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_12` FOREIGN KEY (`it_drop_3`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_13` FOREIGN KEY (`it_drop_4`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_15` FOREIGN KEY (`Effect`) REFERENCES `enum_effects_list` (`Name`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_16` FOREIGN KEY (`Behavior`) REFERENCES `enum_behavior_mobs` (`Behavior`) ON DELETE NO ACTION ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_8` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_mobs_ibfk_9` FOREIGN KEY (`it_drop_0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=23 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_bots_mobs`
--

LOCK TABLES `tw_bots_mobs` WRITE;
/*!40000 ALTER TABLE `tw_bots_mobs` DISABLE KEYS */;
INSERT INTO `tw_bots_mobs` VALUES (1,36,1,4049,890,'Slowdown','Slime',2,8,0,5,5,0,22,29,NULL,NULL,NULL,'|1|1|0|0|0|','|1.08|1.08|0|0|0|'),(2,23,3,3057,2577,'Slowdown','Slime',4,18,0,12,5,0,29,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|3.2|0|0|0|0|'),(3,9,3,1890,1160,'Slowdown','Slime',2,10,0,12,5,0,22,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|3.2|0|0|0|0|'),(4,28,3,3057,2577,'Slowdown','Slime',10,240,0,1,320,1,29,22,NULL,NULL,NULL,'|3|3|0|0|0|','|100|100|0|0|0|'),(5,22,5,1345,2600,NULL,'Standard',8,15,1,14,5,0,30,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|1.56|0|0|0|0|'),(7,29,6,2825,2430,NULL,'Standard',12,50,1,10,1,0,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|','|0|0|0|0|0|'),(8,30,6,4840,2560,'Fire','Standard',12,80,2,8,1,0,30,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|4.56|0|0|0|0|'),(9,31,6,1150,3700,'Poison','Standard',12,50,1,10,1,0,42,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|3.47|0|0|0|0|'),(10,22,6,1440,5100,NULL,'Standard',12,60,1,10,1,0,30,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|3.96|0|0|0|0|'),(11,32,6,3960,4595,'Fire','Standard',15,800,1,1,1,1,37,42,NULL,NULL,NULL,'|1|1|0|0|0|','|50|75|0|0|0|'),(12,34,3,1570,2915,NULL,'Standard',6,27,0,8,5,0,35,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|4.1|0|0|0|0|'),(13,35,5,1345,2600,NULL,'Standard',14,620,1,1,400,1,30,42,NULL,NULL,NULL,'|1|1|0|0|0|','|100|25|0|0|0|'),(14,40,8,3665,390,'Fire','Standard',18,100,1,10,1,0,44,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|7.5|0|0|0|0|'),(15,41,8,5610,2865,'Poison','Standard',19,120,1,10,1,0,44,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|3.5|0|0|0|0|'),(16,42,8,2400,3150,'Fire','Standard',20,110,1,10,1,0,44,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|5.25|0|0|0|0|'),(17,43,8,1720,3180,'Fire','Standard',20,115,1,10,1,0,44,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|7.5|0|0|0|0|'),(18,44,8,2800,1640,'Fire','Standard',25,2090,1,1,1,1,44,NULL,NULL,NULL,NULL,'|3|0|0|0|0|','|100|0|0|0|0|'),(19,47,10,2030,1260,'Fire','Standard',15,90,2,16,1,0,46,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|2.57|0|0|0|0|'),(20,48,10,4446,577,'Fire','Standard',16,110,2,16,1,0,46,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|2.57|0|0|0|0|'),(21,49,10,61,1539,'Fire','Sleepy',16,120,2,8,1,0,46,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|3.67|0|0|0|0|'),(22,50,10,6545,700,'Fire','Standard',20,1820,3,1,1,1,45,NULL,NULL,NULL,NULL,'|1|0|0|0|0|','|50|0|0|0|0|');
/*!40000 ALTER TABLE `tw_bots_mobs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_bots_npc`
--

DROP TABLE IF EXISTS `tw_bots_npc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_bots_npc` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `BotID` int(11) NOT NULL DEFAULT '-1',
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Function` int(11) NOT NULL DEFAULT '-1',
  `Static` int(11) NOT NULL,
  `Emote` int(11) NOT NULL DEFAULT '0' COMMENT '1.Pain 2.Happy 3.Surprise 4.Angry 5.Blink	',
  `Number` int(11) NOT NULL DEFAULT '1',
  `WorldID` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `MobID` (`BotID`),
  KEY `WorldID` (`WorldID`),
  KEY `tw_bots_npc_ibfk_3` (`Emote`),
  CONSTRAINT `tw_bots_npc_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_npc_ibfk_3` FOREIGN KEY (`Emote`) REFERENCES `enum_emotes` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `tw_bots_npc_ibfk_4` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_bots_npc`
--

LOCK TABLES `tw_bots_npc` WRITE;
/*!40000 ALTER TABLE `tw_bots_npc` DISABLE KEYS */;
INSERT INTO `tw_bots_npc` VALUES (1,1,1985,977,-1,1,2,1,0),(2,2,1022,1073,-1,0,5,2,0),(3,3,2691,1009,-1,1,0,1,0),(4,4,5312,1073,-1,1,0,1,0),(5,11,10092,8561,-1,0,0,1,2),(6,12,5693,8369,-1,0,0,1,2),(7,13,6471,7569,-1,0,0,1,2),(8,14,6451,7345,0,1,2,1,2),(9,15,9464,6833,-1,0,4,1,2),(10,16,264,1009,-1,0,4,1,1),(11,2,1234,689,-1,1,1,1,0),(12,14,419,1009,0,1,2,1,1),(13,19,5739,7473,-1,1,0,1,2),(14,20,3759,8209,-1,1,0,1,2),(15,21,6218,6417,-1,1,0,1,2),(16,27,4590,977,-1,1,2,1,4),(17,14,1448,4433,0,1,2,1,5),(18,37,7781,7921,-1,1,4,1,2),(19,39,2851,3473,-1,1,3,1,5),(20,51,9335,4881,-1,1,0,1,2),(21,53,7282,4880,-1,1,0,1,2);
/*!40000 ALTER TABLE `tw_bots_npc` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_bots_quest`
--

DROP TABLE IF EXISTS `tw_bots_quest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_bots_quest` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `BotID` int(11) NOT NULL DEFAULT '-1',
  `QuestID` int(11) NOT NULL DEFAULT '-1',
  `Step` int(11) NOT NULL DEFAULT '1',
  `WorldID` int(11) DEFAULT NULL,
  `generate_nick` tinyint(4) NOT NULL DEFAULT '0',
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
  `interactive_temp` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `MobID` (`BotID`),
  KEY `it_need_0` (`it_need_0`),
  KEY `tw_bots_quest_ibfk_3` (`it_need_1`),
  KEY `tw_bots_quest_ibfk_4` (`it_reward_0`),
  KEY `it_reward_1` (`it_reward_1`),
  KEY `QuestID` (`QuestID`),
  KEY `tw_bots_quest_ibfk_6` (`mob_0`),
  KEY `tw_bots_quest_ibfk_7` (`mob_1`),
  KEY `WorldID` (`WorldID`),
  KEY `interactive_type` (`interactive_type`),
  CONSTRAINT `tw_bots_quest_ibfk_1` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_10` FOREIGN KEY (`interactive_type`) REFERENCES `enum_quest_interactive` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_2` FOREIGN KEY (`it_need_0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_3` FOREIGN KEY (`it_need_1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_4` FOREIGN KEY (`it_reward_0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_5` FOREIGN KEY (`it_reward_1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_6` FOREIGN KEY (`mob_0`) REFERENCES `tw_bots_info` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_7` FOREIGN KEY (`mob_1`) REFERENCES `tw_bots_info` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_8` FOREIGN KEY (`QuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_bots_quest_ibfk_9` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=73 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_bots_quest`
--

LOCK TABLES `tw_bots_quest` WRITE;
/*!40000 ALTER TABLE `tw_bots_quest` DISABLE KEYS */;
INSERT INTO `tw_bots_quest` VALUES (1,5,1,1,0,0,3925,1169,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(2,5,1,2,0,0,5599,1009,23,NULL,23,NULL,NULL,NULL,'|12|0|12|0|0|0|',2,NULL),(3,4,1,3,0,1,4121,1137,23,NULL,NULL,NULL,NULL,NULL,'|4|0|0|0|0|0|',2,NULL),(4,4,1,3,0,1,6489,1137,23,NULL,NULL,NULL,NULL,NULL,'|4|0|0|0|0|0|',2,NULL),(5,4,1,3,0,1,2430,977,23,NULL,NULL,NULL,NULL,NULL,'|4|0|0|0|0|0|',2,NULL),(6,5,1,4,0,0,6742,1041,NULL,NULL,3,NULL,NULL,NULL,'|0|0|1|0|0|0|',NULL,NULL),(7,6,2,1,1,0,841,977,NULL,NULL,21,NULL,NULL,NULL,'|0|0|1|0|0|0|',NULL,NULL),(8,8,2,2,0,0,411,1009,21,NULL,NULL,NULL,NULL,NULL,'|1|0|0|0|0|0|',NULL,NULL),(9,6,2,3,1,0,841,977,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(10,6,2,4,1,0,2207,465,NULL,NULL,NULL,NULL,36,NULL,'|0|0|0|0|12|0|',NULL,NULL),(11,16,3,1,1,0,525,1009,NULL,NULL,15,14,36,NULL,'|0|0|3|3|16|0|',NULL,NULL),(12,18,5,1,2,0,5215,7409,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(13,18,5,2,0,0,2390,977,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(14,2,5,2,0,0,2162,977,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(15,19,6,1,2,0,7915,8401,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(16,19,7,1,2,0,9859,8561,26,NULL,26,NULL,NULL,NULL,'|1|0|1|0|0|0|',NULL,NULL),(17,6,10,1,2,0,6615,8433,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(18,6,10,2,2,0,9953,8561,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(19,6,10,3,2,0,8574,7665,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(20,6,10,4,2,0,8123,7089,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(21,6,10,5,2,0,6815,7569,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(22,6,10,6,2,0,6364,7345,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(23,6,10,7,2,0,5021,7441,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(24,6,11,1,2,0,6834,7569,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(25,6,11,2,2,0,5722,6353,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(26,10,11,3,2,0,5325,6289,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(27,25,12,1,2,0,5421,8273,NULL,NULL,27,NULL,NULL,NULL,'|0|0|1|0|0|0|',NULL,NULL),(28,25,12,2,2,0,10822,6737,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(29,25,13,1,3,0,676,1169,NULL,NULL,NULL,NULL,9,NULL,'|0|0|0|0|32|0|',NULL,NULL),(30,26,13,2,3,0,500,1361,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(31,16,4,1,1,0,1530,1073,29,NULL,25,NULL,NULL,NULL,'|18|0|8|0|0|0|',NULL,NULL),(32,26,13,3,2,0,3780,6449,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(33,27,8,1,4,0,4235,945,NULL,NULL,NULL,NULL,23,NULL,'|0|0|0|0|50|0|',NULL,NULL),(34,27,8,2,2,0,7975,7089,NULL,NULL,32,NULL,NULL,NULL,'|0|0|1|0|0|0|',NULL,NULL),(35,27,9,1,4,0,4243,945,NULL,NULL,NULL,NULL,28,NULL,'|0|0|0|0|1|0|',NULL,NULL),(36,10,14,1,4,0,4391,977,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(37,6,15,1,2,0,8081,7889,20,NULL,NULL,NULL,NULL,NULL,'|10|0|0|0|0|0|',2,NULL),(38,6,15,2,2,0,8193,6065,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(39,6,16,1,7,0,4454,881,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(40,6,16,2,7,0,4671,881,29,NULL,NULL,NULL,NULL,NULL,'|18|0|0|0|0|0|',NULL,NULL),(41,6,16,3,7,0,4590,881,35,NULL,NULL,NULL,NULL,NULL,'|14|0|0|0|0|0|',NULL,NULL),(42,8,16,4,7,0,4142,881,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(43,6,16,4,7,0,4590,881,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(44,25,17,1,2,0,8128,7889,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(45,25,17,2,3,0,762,1169,NULL,NULL,NULL,NULL,23,NULL,'|0|0|0|0|32|0|',NULL,NULL),(46,25,17,3,2,0,6316,7569,NULL,NULL,9,NULL,NULL,NULL,'|0|0|50|0|0|0|',NULL,NULL),(47,27,18,1,2,0,5244,6289,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|1|0|',NULL,NULL),(48,10,18,2,4,0,4453,977,NULL,NULL,16,NULL,34,NULL,'|0|0|3|0|32|0|',NULL,NULL),(49,25,19,1,4,0,4727,977,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(50,25,19,2,2,0,9079,8305,29,22,15,14,23,9,'|16|16|8|8|40|40|',NULL,NULL),(51,25,19,3,3,0,4509,1265,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(52,37,50,1,2,0,7781,7921,29,35,NULL,NULL,NULL,NULL,'|20|20|0|0|0|0|',NULL,NULL),(53,38,50,2,2,0,7671,7921,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(54,37,50,3,2,0,7781,7921,NULL,NULL,43,NULL,NULL,NULL,'|0|0|5|0|0|0|',NULL,NULL),(55,15,55,1,2,0,9463,6833,31,NULL,NULL,NULL,NULL,NULL,'|50|0|0|0|0|0|',NULL,NULL),(56,39,60,1,5,0,2852,3473,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(57,39,60,2,5,0,3216,3537,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(58,39,61,1,5,0,2944,3505,31,35,NULL,NULL,NULL,NULL,'|40|40|0|0|0|0|',NULL,NULL),(59,39,62,1,5,0,2944,3505,41,37,NULL,NULL,NULL,NULL,'|1|8|0|0|0|0|',NULL,NULL),(60,39,62,2,5,0,4566,4273,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(61,46,20,1,5,0,2528,4305,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(62,25,20,1,5,0,2409,4305,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(63,46,20,2,5,0,2528,4305,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(64,46,20,3,5,0,3867,3089,NULL,NULL,NULL,NULL,35,22,'|0|0|0|0|5|30|',NULL,NULL),(65,46,20,4,5,0,122,3377,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(66,46,21,1,6,0,881,1521,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(67,46,21,2,6,0,3121,4114,NULL,NULL,NULL,NULL,32,NULL,'|0|0|0|0|1|0|',NULL,NULL),(68,46,21,3,5,0,1714,4433,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|1|0|',NULL,NULL),(69,46,22,1,5,0,970,4529,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(70,10,22,2,5,0,2337,4305,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(71,6,22,3,5,0,2546,4305,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL),(72,46,22,4,5,0,4654,4241,NULL,NULL,NULL,NULL,NULL,NULL,'|0|0|0|0|0|0|',NULL,NULL);
/*!40000 ALTER TABLE `tw_bots_quest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_crafts_list`
--

DROP TABLE IF EXISTS `tw_crafts_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_crafts_list` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ItemID` int(11) DEFAULT NULL,
  `ItemValue` int(11) NOT NULL,
  `RequiredItemID0` int(11) DEFAULT NULL,
  `RequiredItemID1` int(11) DEFAULT NULL,
  `RequiredItemID2` int(11) DEFAULT NULL,
  `RequiredItemsValues` varchar(32) NOT NULL DEFAULT '0 0 0',
  `Price` int(11) NOT NULL DEFAULT '100',
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `CraftIID` (`ItemID`),
  KEY `Craft_Item_0` (`RequiredItemID0`),
  KEY `Craft_Item_1` (`RequiredItemID1`),
  KEY `Craft_Item_2` (`RequiredItemID2`),
  KEY `WorldID` (`WorldID`),
  CONSTRAINT `tw_crafts_list_ibfk_1` FOREIGN KEY (`ItemID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_crafts_list_ibfk_2` FOREIGN KEY (`RequiredItemID0`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_crafts_list_ibfk_3` FOREIGN KEY (`RequiredItemID1`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `tw_crafts_list_ibfk_4` FOREIGN KEY (`RequiredItemID2`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_crafts_list`
--

LOCK TABLES `tw_crafts_list` WRITE;
/*!40000 ALTER TABLE `tw_crafts_list` DISABLE KEYS */;
INSERT INTO `tw_crafts_list` VALUES (1,15,3,22,29,18,'9 9 16',50,2),(2,26,1,30,NULL,NULL,'24 0 0',150,2),(3,33,1,37,31,NULL,'3 30 0',2500,2),(4,34,1,37,31,NULL,'8 50 0',2700,2),(5,10019,1,37,30,31,'18 48 24',7200,2),(6,10020,1,37,30,31,'14 38 18',7200,2),(7,10021,1,37,30,31,'14 38 18',7200,2),(8,10022,1,37,30,31,'14 38 18',7200,2),(9,10023,1,37,30,31,'14 38 18',7200,2),(10,10016,1,37,NULL,NULL,'40 0 0',14400,2),(11,14,3,22,29,18,'9 9 16',50,2),(12,41,1,42,30,18,'32 16 64',3600,2);
/*!40000 ALTER TABLE `tw_crafts_list` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_dialogs_other_npc`
--

DROP TABLE IF EXISTS `tw_dialogs_other_npc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_dialogs_other_npc` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `MobID` int(11) NOT NULL,
  `Style` int(11) NOT NULL DEFAULT '-1',
  `Emote` int(11) NOT NULL DEFAULT '-1',
  `PlayerSays` tinyint(1) NOT NULL DEFAULT '0',
  `GivesQuestID` int(11) DEFAULT NULL,
  `Text` varchar(512) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `tw_talk_other_npc_ibfk_1` (`MobID`),
  KEY `tw_talk_other_npc_ibfk_2` (`Style`),
  KEY `tw_talk_other_npc_ibfk_3` (`Emote`),
  KEY `tw_talk_other_npc_ibfk_4` (`GivesQuestID`),
  CONSTRAINT `tw_dialogs_other_npc_ibfk_1` FOREIGN KEY (`MobID`) REFERENCES `tw_bots_npc` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_dialogs_other_npc_ibfk_2` FOREIGN KEY (`Style`) REFERENCES `enum_talk_styles` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_dialogs_other_npc_ibfk_3` FOREIGN KEY (`Emote`) REFERENCES `enum_emotes` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `tw_dialogs_other_npc_ibfk_4` FOREIGN KEY (`GivesQuestID`) REFERENCES `tw_quests_list` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=62 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_dialogs_other_npc`
--

LOCK TABLES `tw_dialogs_other_npc` WRITE;
/*!40000 ALTER TABLE `tw_dialogs_other_npc` DISABLE KEYS */;
INSERT INTO `tw_dialogs_other_npc` VALUES (1,1,2,2,0,NULL,'**He said surprised**: What a wonderful weather today!'),(2,1,0,2,1,NULL,'Yes, I think so too.'),(3,1,0,0,0,NULL,'Well, how do you like on our ship [Player]? Captures the ocean, i think I\'ll stay here a little longer, it\'s wonderful here.'),(4,1,0,0,1,NULL,'I also really like it. Okay, I\'ll go, I wish you a good rest [Talked]!'),(5,1,2,2,0,NULL,'And I wish you good luck too [Player]!'),(6,2,1,1,0,NULL,'**He looks very tired**: Very hot, impossible to work.'),(7,3,0,5,1,NULL,'Hello, I arrived from a small village beyond the mountains and the ocean, my name is [Player], I have a desire to become an adventurer. But I do not know about this place, could you help me?'),(8,3,0,0,0,NULL,'Hello [Player], Yes of course I will help you, explain where you need to go, but you help my workers, they do not cope at all.'),(9,3,2,2,1,NULL,'What you need help with, I\'m good at dealing with predators without meat you will not leave! *laughter*'),(10,3,0,5,0,NULL,'Oh, you\'re a Joker, no thanks go a little further guys will explain everything to you!'),(11,3,0,0,1,1,'OK, thanks!'),(16,10,0,0,1,NULL,'Hi my name is [Player], [Talked] are you working?'),(17,10,0,0,0,NULL,'Hello [Player], Yes, I am on duty as a guard, and I also trade a little'),(18,10,0,5,1,NULL,'Can you tell me how difficult it is to get into the service?'),(19,10,2,2,0,NULL,'If that\'s what you want, it\'s easy. And if you don\'t then you won\'t be able to get in ;)'),(20,10,0,5,0,NULL,'By the way [Player], I need help, there are slugs nearby, again raging, and I can\'t leave the post. At the same time you will test your skills young adventurer.'),(21,10,2,2,1,3,'Yes, no problem, wait [Talked]!!'),(22,11,0,1,0,NULL,'**He says with a look of horror on his face**: Help me find my daughter, she went to the village for food and never returned.'),(23,11,0,0,1,NULL,'To the village? [Talked] Yes of course I will help. That\'s where I need to go!'),(24,11,0,1,0,5,'Thanks a lot'),(30,13,0,0,0,NULL,'Hello, would you like to learn the basics of mining?'),(31,13,0,0,1,NULL,'Yes, of course, I don\'t mind'),(32,13,0,0,0,6,'All right follow me I\'ll explain the basics to you!'),(33,14,0,0,0,NULL,'Hello, and so you\'re new here?'),(34,14,0,5,1,NULL,'Yes.'),(35,14,0,0,0,NULL,'I need to register you. Please introduce yourself, and the purpose of your arrival, and the time you to stay here'),(36,14,0,0,1,NULL,'My name is [Player]. I came here to become an adventurer. I can\'t tell you the exact time of stay.'),(37,14,1,4,0,NULL,'And who for you should know the time of your stay here.'),(38,14,0,5,0,NULL,'Okay well, I will write you down, in case of violation of the rules of our village, we have every right to prohibit your presence here, and report to superiors.'),(39,14,2,2,0,NULL,'Have a good day!'),(40,14,0,5,1,10,'Thank\'s [Talked]!'),(41,16,0,0,0,NULL,'[Stranger]Greetings, wanderer. *bowing*'),(42,16,0,0,0,NULL,'I am the Deputy of the Apostle, and I also protect her. '),(43,16,0,0,0,NULL,'I think you\'ve already heard, that the monsters are raging.'),(44,16,0,0,0,NULL,'I have a request for you [Player]'),(45,16,0,0,1,NULL,'Which one?'),(46,16,0,0,0,NULL,'Help in the southern part to win over the monsters. We can\'t drive them away but we can scare them away.'),(47,16,0,0,1,8,'Of course I will.'),(48,16,2,2,0,NULL,'Thank\'s [Player]'),(49,18,0,0,1,NULL,'You look awful today, [Talked]! What happend?'),(50,18,1,4,0,NULL,'Oh.. don\'t you worry about me, boy..'),(51,18,0,5,1,NULL,'But I DO worry!'),(52,18,0,5,0,NULL,'I\'m tired of these fight I have with my wife, it\'s personal bussiness.'),(53,18,0,5,1,NULL,'I didn\'t want to be a pain to you, Officer. I will leave you to your problems now.'),(54,18,0,5,0,50,'No! Wait!'),(55,9,1,4,0,NULL,'Hey, pst! You, yes, you!'),(56,9,0,5,1,NULL,'Yeah? How can I help?'),(57,9,0,5,0,55,'You cannot help me, kid! I just need...to TEST you, yes...!'),(58,19,0,0,0,NULL,'Hello [Player], I have come to you from the Final Fantasy universe, my name is [Talked].'),(59,19,0,0,0,NULL,'I can\'t say for sure how long I will be here, but for now I will be happy to know your world, and I will be happy to show my world'),(60,19,0,5,1,NULL,'Are you serious? Did the author smoke dope?'),(61,19,0,0,0,60,'Maybe so, I have a couple of things that you can get from me, but not for free. I\'ll need the fragments I lost');
/*!40000 ALTER TABLE `tw_dialogs_other_npc` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_dialogs_quest_npc`
--

DROP TABLE IF EXISTS `tw_dialogs_quest_npc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_dialogs_quest_npc` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `MobID` int(11) NOT NULL,
  `RequestComplete` tinyint(1) NOT NULL DEFAULT '0',
  `Style` int(11) NOT NULL DEFAULT '-1',
  `Emote` int(11) NOT NULL DEFAULT '-1',
  `PlayerSays` tinyint(1) NOT NULL DEFAULT '0',
  `Text` varchar(512) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `tw_talk_quest_npc_ibfk_1` (`MobID`),
  KEY `tw_talk_quest_npc_ibfk_2` (`Style`),
  KEY `tw_talk_quest_npc_ibfk_4` (`Emote`),
  CONSTRAINT `tw_dialogs_quest_npc_ibfk_1` FOREIGN KEY (`MobID`) REFERENCES `tw_bots_quest` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_dialogs_quest_npc_ibfk_2` FOREIGN KEY (`Style`) REFERENCES `enum_talk_styles` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_dialogs_quest_npc_ibfk_3` FOREIGN KEY (`Emote`) REFERENCES `enum_emotes` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `tw_dialogs_quest_npc_ibfk_4` FOREIGN KEY (`Emote`) REFERENCES `enum_emotes` (`ID`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=407 DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_dialogs_quest_npc`
--

LOCK TABLES `tw_dialogs_quest_npc` WRITE;
/*!40000 ALTER TABLE `tw_dialogs_quest_npc` DISABLE KEYS */;
INSERT INTO `tw_dialogs_quest_npc` VALUES (1,1,0,0,0,1,'Hello, I was sent to you by a carpenter to help you.'),(2,1,0,0,0,0,'**He asks with a smile**: Hello, have you just arrived? '),(3,1,0,0,0,1,'Yes, I came to you to become an adventurer, I need money.'),(4,1,0,0,1,0,'Well, I\'ll tell you [Player], things have been very bad here lately, the residents are living in fear. Recently, a little girl was killed, as we understand there is a dead force involved.'),(5,1,0,1,3,1,'**With horror in his eyes**: What ... Dead People?'),(6,1,0,0,0,0,'**He said hurriedly**: I don\'t know for sure. Over time, the residents will tell you everything, or you will feel for yourself what is happening there.'),(7,1,1,0,5,1,'I didn\'t expect it to be this bad. Well, what can I help you with?'),(8,1,1,0,0,0,'Come with me, our guys dropped boards, help me collect them.'),(9,2,1,0,0,0,'Okay generally help me collect all the boards! I will reward you for your help.'),(10,2,0,0,1,1,'**Weakness in asks**: Oh, hell, they\'re heavy, I\'ve got them all.'),(11,2,0,2,2,0,'Great job! Your muscles won\'t hurt you anyway [Player] *laughter*'),(12,2,0,2,2,1,'*laughter*'),(13,2,0,0,0,0,'Now hand them out to my boys and let them go on with their business.'),(14,2,0,0,0,1,'Good [Talked]!'),(15,3,1,0,0,1,'Here you go, your boss told me to give it to you!'),(16,3,0,0,0,0,'Thanks for the help.'),(17,4,1,0,0,1,'Here you go, your boss told me to give it to you!'),(18,4,0,0,0,0,'Thanks for the help.'),(19,5,1,0,0,1,'Here you go, your boss told me to give it to you!'),(20,5,0,0,0,0,'Thanks for the help.'),(21,6,0,0,1,1,'Is tired. But I did it!'),(22,6,1,2,2,0,'Well done, take this. I think you need it if you\'re going to be an adventurer. Go straight, and you will go straight to the residents, if you ask where to go.'),(23,6,0,2,2,1,'**With a smile**: Well thank you, good luck with your work!'),(29,7,0,0,0,1,'[Stranger]Hi, my name is [Player], how do I find the nearest village?'),(30,7,0,0,2,0,'**She concerned**: Hello, my name is [Talked].'),(31,7,0,0,1,0,'I\'ll be happy to help you settle in, but could you give my brother a note? He works now on the ship that you arrived on, i could have done it myself, but they won\'t let me in.'),(32,7,1,2,2,1,'Yes, of course i help, will you be here [Talked]?'),(33,7,0,0,0,0,'I\'ll be here waiting!'),(34,8,0,0,0,1,'Hello, are you Diana\'s brother?'),(35,8,0,1,3,0,'**He said rudely**: Yes. Did something happen to her?..'),(36,8,1,0,5,1,'No, don\'t worry, she just asked me to pass you the notebook.'),(37,8,0,2,2,0,'Well, thank you very much, and tell her I\'ll be seeing her this evening.'),(38,8,0,2,2,1,'Yes, good luck with your work!'),(39,9,0,0,0,0,'**She says impatiently**: Oh, you\'re already here, you\'re fast!'),(40,9,0,2,2,1,'Really?'),(41,9,0,0,0,0,'Yes. How is he?'),(42,9,0,0,0,1,'He told me that he is busy now, and will visit you later in the evening!'),(43,9,0,0,1,0,'I\'m so happy! They won\'t let me in because of the problems that are happening. Dead men kidnap girls, and then they find them dead.'),(44,9,0,0,5,1,'**He sounds surprised**: Only girls? Why the dead?'),(45,9,0,0,1,0,'**She\'s upset**: Honestly, I don\'t know. The dead or who it is, our headman says that most likely the dead ;('),(46,9,0,0,0,1,'I came to you to become an adventurer, as I know you have magical creatures.'),(47,9,0,0,3,0,'**She sounds like she knows it**: An adventurer? We have a Guild of adventurers, but to become an adventurer you need to pass the tests.'),(48,9,0,0,5,1,'Tests?....'),(49,9,0,0,0,0,'Yes the test [Player]. First, you will need to collect the minimum equipment. To apply and pay the fee. And only after that you will be assigned the position of adventurer.'),(50,9,0,2,2,1,'Is everything so strict? Yes, I think I can handle it.'),(51,9,1,2,2,0,'All right come on I\'ll show you our village [Player].'),(52,9,0,2,2,1,'Yes come on ;)'),(53,10,0,0,1,0,' **She made it sound like there was something wrong with them**: Oh, my God, where are the slime from?.'),(54,10,0,0,5,1,'I don\'t know [Talked], here even have slimes. Wahoo'),(55,10,0,0,1,0,'Well, Yes [Player], but now they are becoming more frequent'),(56,10,1,2,2,1,'**You speak proudly**: All right [Talked], I\'ll clear the way!'),(57,10,0,1,4,1,'**You said in disgust**: Phew, the slime is so disgusting...'),(58,10,0,2,2,0,'**She says with a smile**: Well you could use a bath [Player] ;)'),(59,10,0,2,2,1,'I will not refuse such an honor ;)'),(60,10,0,0,0,0,'Ah here is and decided, we\'ll get there I\'ll show you where I live. And I\'ll introduce you to the places. And you wash :)'),(61,10,0,0,2,1,'All right, shall we continue on our way?'),(62,10,0,0,0,0,'Yes let\'s go'),(63,11,1,0,0,0,'**He said with surprise**: Did you defeat the slugs on the instructions of our master? Take this the master asked me to give you.'),(64,11,0,2,2,1,'Yes, I could. They were very slippery.'),(65,11,0,2,2,0,'Hehe.. What did you think?'),(66,11,0,0,0,1,'Well, not so difficult, at the same time I warmed up.'),(67,11,0,2,2,0,'Okay I need to serve, good luck to you, don\'t give up ;)'),(68,12,0,1,4,0,'**She said very rudely**: Who the hell are you? What do you want?'),(69,12,0,0,0,1,'Do you know that your father is looking for you?'),(70,12,0,1,4,0,'I hate this old guy, he doesn\'t have time for me......'),(71,12,0,1,4,0,'He can only go there. Do it. And we can\'t spend time together...'),(72,12,0,1,4,0,'Enrages...'),(73,12,1,2,2,1,'Calm down let\'s go back together and tell him everything? Maybe he just cares about you like that.'),(74,12,0,0,1,0,'All right, let\'s go back.'),(75,13,1,0,0,1,'Are you ready?'),(76,13,0,0,1,0,'Yes..'),(77,14,0,0,4,0,'So where was she?.'),(78,14,0,0,0,1,'You just don\'t get angry, everything\'s fine!'),(79,14,0,0,0,1,'In fact, she didn\'t want to come back, she was offended that you weren\'t spending time together. You can\'t rest.'),(80,14,1,0,3,0,'Damn why didn\'t she ask for it herself? Okay, don\'t worry thanks for the work :) Everything will be fine, we will decide something with daughter'),(81,14,0,0,2,1,'Well, nice :) Good luck to you'),(82,15,0,0,0,0,'To extract any ore [Player], you will need a pickaxe.'),(83,15,0,0,0,0,'You can only get it by creating. But in some cases merchants can sell.'),(84,15,0,0,0,0,'Also, the picks have a durability of 100-0. If you durability reach 0. Don\'t worry it won\'t break.'),(85,15,0,0,0,0,'But you can\'t work with break pickaxe it. You can repair it at any store.'),(86,15,0,0,0,0,'You can also improve picks this will depend on their effectiveness.'),(87,15,0,0,0,0,'Each ore has its own strength. If you have a weak pick and the strength of the ore is high, it can take a long time to extract it. So don\'t forget about the improvements. Or craft even better picks.'),(88,15,0,0,0,0,'Everything is clearly explained, I will not repeat it again, if you need you will understand everything!'),(89,15,1,0,0,1,'Yes of course thank you for the explanation'),(90,15,0,0,0,0,'Well don\'t worry, I\'ll give you a couple of tasks to get used to in this area'),(91,16,0,0,0,1,'Did you want something?'),(92,16,0,0,0,0,'I think it\'s time for you to get your first pickaxe.'),(93,16,0,0,0,0,'I think as crafted something, you don\'t need to explain!'),(94,16,0,0,0,0,'So let\'s get right to the point, try to make a pickaxe bring it to me, I\'ll look at it.'),(95,16,1,0,0,1,'Done. (you need to have an idea of how to craft items, you can see them near Craftsman in votes)'),(96,16,0,0,0,0,'Well, it\'s not bad, so use it for the first time.'),(97,16,0,0,0,0,'All right so let\'s start with your first ore?'),(98,17,0,2,2,0,'Well, as [Player], passed customs?'),(99,17,0,0,5,1,'Yes [Talked]. He was very rude.'),(100,17,0,0,0,0,'Well, you have to understand that we also have a lot of problems lately, too many monsters, problems with kidnappings.'),(101,17,1,0,0,1,'Come on, show me [Here] seats!'),(102,17,0,0,0,0,'Oh sure. By the way, in our village, the chief Apostle.'),(103,17,0,0,3,1,'Wait an Apostle? They are ancient beings sent by the gods. Do they exist?'),(104,17,0,0,0,0,'Yes [Player]. They exist to keep order.'),(105,17,0,0,5,0,'Okay [Player] let\'s go and show you our [Here]'),(106,18,0,0,2,0,'**She said with a smile**: Here, you can do a craft items, make your own equipment potions, or what to eat.'),(107,18,0,0,0,0,'[Player] even if you provide items, the craftsman you must pay for the use of the equipment'),(108,18,0,0,0,1,'Yes I think I can use it. Thanks for the explanation!'),(109,18,1,0,0,0,'Let\'s move on!'),(110,18,0,0,2,1,'Yes let\'s go :)'),(111,19,0,0,0,0,'So here we have someone\'s house.'),(112,19,0,0,0,0,'You can also buy houses, decorate them or use the safety Deposit box.'),(113,19,1,0,0,1,'Sounds good [Talked]!'),(114,19,0,0,2,0,'**She likes to talk**: Well, okay, let\'s move on!'),(115,20,0,0,0,0,'This is the ether, it allows you to use instantaneous movement around the world.'),(116,20,0,0,0,0,'But only there in the area where you have already visited.'),(117,20,0,0,0,1,'Is it free [Talked]?'),(118,20,0,0,0,0,'Not always, there is nothing free in our world!'),(119,20,1,0,5,0,'**Apparently she didn\'t like something**: Okay [Player] let\'s move on!'),(120,20,0,0,0,1,'Yes let\'s go [Talked]!'),(121,21,0,0,0,0,'Here you will be able to raise his skills adventurer, to explore something new. Our world is divided into 3 classes (Healer, Tank, DPS)'),(122,21,0,0,3,1,'So I can become something?'),(123,21,0,0,5,0,'[Player], you can be anyone you want (as well as fine-tune your game mode)'),(124,21,0,0,5,1,'[Talked] so I can be both a Tank and a Defender?'),(125,21,0,0,0,0,'Yes quite but do not forget about the main characteristics (Strength, Hardness, idk)'),(126,21,1,2,2,1,'Thank you for a clear explanation :)'),(127,21,0,0,5,0,'That\'s a small part of what I\'ve been able to tell you.'),(128,22,0,0,0,0,'And here, you will be treated. '),(129,22,0,1,5,0,'**She said very jealously**: True, I hate those nurses, all the guys from the village are taken away....'),(130,22,1,0,5,0,'Don\'t marry them...'),(131,22,0,2,2,1,'Yes I will not marry anyone [Talked]! Thanks for the care!'),(133,23,0,0,5,0,'**Yawning she said**: Okay I\'m tired I\'ll go home.'),(134,23,1,0,0,0,'And you don\'t disappear, and [Player] go wash up :)'),(135,23,0,2,2,1,'Well have a good day [Talked] :)'),(136,24,0,0,0,0,'Are you ready?'),(137,24,0,0,3,1,'What, ready?'),(138,24,0,0,0,0,'Introduce you [Player] to the Apostle of our village. '),(139,24,0,0,3,1,'[Talked] so what\'s it to them for me?'),(140,24,0,1,4,0,'**She said rudely**: No, don\'t think so bad, they actually think of us as their children.'),(141,24,0,0,0,1,'Why so rude? I understand you, okay, I\'m worried, but we\'ll see what happens.'),(142,24,0,0,1,0,'I\'m sorry, but she\'s really close to me.'),(143,24,0,0,5,1,'Are you one of them? Hm...'),(144,24,1,1,4,0,'Fool let\'s go already.'),(145,24,0,0,1,1,'Well ... Let\'s go'),(146,25,1,0,5,0,'Well, just don\'t mess with as a joke, it\'s Apostle, he is watching over us for many years.'),(147,25,0,0,0,1,'I understand [Talked].'),(148,26,0,0,0,1,'Hello dear Apostle *bowing*. I have come to you to wisdom.'),(149,26,0,0,0,0,'Hello, lift your head. I knew you\'d come. But I\'m not sure you\'re the one I saw.'),(150,26,0,0,0,1,'See it?'),(151,26,0,0,0,0,'I can\'t communicate with my brothers and sisters, since our paths have diverged, but we have a common mind.'),(152,26,0,0,0,0,'I had a vision about a year ago that there would soon be a man who was as tenacious as the Apostles.'),(153,26,0,0,0,0,'But in the end, he was going to die. My visions have always come true, but when they will come true is unknown.'),(154,26,0,0,0,0,'So please be careful [Player]. I\'m worried about everyone, but I\'ve also been weak lately.'),(155,26,0,0,0,1,'I .. Not to worry, I can handle it even if I turn out to be one of your visions'),(156,26,1,0,0,0,'I hope this is not the case, get used to it, then go to Adventurer Koto.'),(157,26,0,0,0,1,'Thanks a lot.'),(158,27,0,0,0,1,'Hello [Talked], the Apostle sent me to you. I want to be an adventurer!'),(159,27,0,1,5,0,'**He as if he doesn\'t want me to become an adventurer**: Will you be strong enough? Well, let\'s see, I think you already understand that you first need to test, only after I say whether you are worthy or not to become an Adventurer?'),(160,27,0,0,0,1,'Well, what exactly is required?'),(161,27,0,1,5,0,'If you pass my first test, pay the fee, I encourage you, and send you to the exam. And then we will decide whether you are worthy or not'),(162,27,0,0,0,1,'Okay, where do we start [Talked]?'),(163,27,1,0,0,0,'Ha ha where do we start? Don\'t joke with me young fighter. First, take this and go to the southern part of the village.'),(164,27,0,0,0,1,'Well'),(165,28,0,1,4,0,'You did.'),(166,28,0,0,5,1,'What could I do?'),(167,28,0,1,4,0,'I thought you were joking, you wouldn\'t take it seriously, but you came.'),(168,28,1,0,0,1,'Well, let\'s go.'),(169,28,0,0,0,0,'Let\'s go.'),(170,29,0,1,4,0,'Why are you shivering [Player]?'),(171,29,1,2,2,0,'Am I that scary? Don\'t worry I\'m kind *laughs*. Well, let\'s see how you cope. Kill for me ....'),(172,29,0,0,5,1,'I managed it, and it wasn\'t difficult.'),(173,29,0,2,2,0,'I noticed how well you did. But don\'t relax this is just the beginning ^^'),(174,29,0,0,5,1,'Ah.. Okay [Talked].'),(175,29,0,0,0,0,'Well today we\'re finished you can relax. You\'ll find me as soon as you\'re ready.'),(176,29,0,0,2,1,'Okay, well, have a good day.'),(183,30,0,0,0,0,'Hello, I am the personal Ambassador of the Apostle.'),(184,30,0,0,0,0,'She wants to see you!'),(185,30,1,0,0,0,'Please follow me to village, I will be waiting for you there'),(186,30,0,0,0,1,'Okay, I\'m coming'),(187,31,0,0,0,0,'Listen there\'s another request!'),(188,31,1,0,0,0,'[Player] if you find some gel, I\'ll be happy to exchange them. Brown Slimes,Blue Slimes and Pink Slime drop Gel. You can take your time, i\'ll wait!'),(189,31,0,0,0,1,'Here, it wasn\'t easy but I got it [Talked].'),(190,31,0,2,2,0,'Thank you very much for your help. All right I\'ll go back to my post, good luck to you [Player]!'),(191,31,0,2,2,1,'Thank\'s [Talked]'),(192,32,1,0,0,0,'[Player] you can go in!'),(193,32,0,0,0,1,'Well [Talked], thank you for seeing me off!'),(194,33,1,0,0,0,'[Player] first we need to win over the slime. Dirty but necessary.'),(195,33,0,0,0,1,'I report everything, I coped with the task'),(196,33,0,2,2,0,'Well done, but that\'s not all.'),(197,33,0,0,0,1,'Anything else?'),(198,33,0,0,0,0,'Yes I think you know about the ethers find me next to him'),(199,34,0,0,0,0,'I think you know that these crystals are actually not just for beauty. '),(200,34,0,0,0,0,'You use them as a quick transition. But in fact, these are crystals (all the light of good prosperity) precisely because they exist.'),(201,34,1,0,0,0,'Me when once before as became defender of the Apostle, I was able to meet with the crystal. Yes, he has a mind. From it warmth on the soul, all the sins immediately fall from the soul. I\'m sure you\'ll meet him one day.'),(202,34,0,0,0,1,'Thanks. I will remember that. (Auther text: This is how the player\'s rapprochement with the deities began)'),(203,34,0,0,0,0,'Well, I\'ll wait for you in the Apostle\'s chambers. We will continue the expulsion of the monsters'),(204,35,0,0,0,0,'Our priority is higher now.'),(205,35,1,0,0,0,'[Player] you need to defeat the main slug, in the southern part.'),(206,35,0,0,0,1,'That\'s it, I\'ve met my goal. Clearing dirt.'),(207,35,0,0,0,0,'Well done, we\'ll meet again.'),(219,36,0,0,0,0,'I want to talk to you [Player].'),(220,36,0,0,5,0,'I know that your path will not stop here. One day you will become stronger and move on.'),(221,36,0,0,0,0,'I have a request to you, if you meet other apostles, it is better to avoid them. Don\'t trust them, we apostles are not as pure as you think'),(222,36,0,0,5,1,'Maybe you, too, then the enemy really is?'),(223,36,0,2,2,0,'I don\'t know *laughter*. The villagers I think will clearly tell you how it really is'),(224,36,0,0,0,0,'Diana told me about you in General terms.'),(225,36,0,2,2,0,'**She said it with concern**: You turn out to be a Joker, and a vulgar *laughter*'),(226,36,0,0,5,1,'No [Talked]......'),(227,36,0,0,0,0,'Always listen to your heart. I think one day you will be able to bring light. But the main thing is perseverance!'),(228,36,1,0,0,0,'Do not delay with your title of adventurer. You can go now!'),(229,36,0,0,0,1,'I\'m trying, thank you very much.'),(230,37,0,0,0,1,'You look sad today [Talked].'),(231,37,1,0,1,0,'Oh man I\'m too confused today.... [Player] help me collect lost.'),(232,37,0,0,2,1,'Take your nose up [Talked], what\'s wrong with you?'),(233,37,0,0,1,0,'My brother has to stop by today, and I want to cook something to feed him, but everything falls out of my hands..'),(234,37,0,0,2,1,'So let me help you?'),(235,37,0,0,0,0,'And you I look [Player] today cheerful, well help. I will not remain in debt'),(236,37,0,2,2,1,'Of course [Talked] ;)'),(237,38,0,0,0,0,'Well here\'s my house! Ah as my not my, father is a master, and even brother. In General, our family'),(238,38,1,0,0,0,'All right, come on in, I\'ll cook, and I\'ll ask you to bring something. All the help me need'),(239,38,0,0,0,1,'I will help you, especially since you are my first close friend in this village, I already trust you, how you can be abandoned.'),(240,39,0,0,0,0,'Well [Player], let\'s get started, little by little'),(241,39,0,0,2,1,'Your house comfortable! [Talked]'),(242,39,1,0,0,1,'Do you really think so? Well thank you.'),(243,39,0,2,2,0,'All right let\'s get down to cooking'),(244,40,0,0,0,0,'Well, let\'s get started [Player]!'),(245,40,0,0,0,1,'Oh sure [Talked]'),(246,40,1,0,0,0,'[Player] get me some first, we\'ll use it as oil'),(247,40,0,2,2,1,'I think everyone has their own way of making butter *laughter*'),(248,40,0,2,2,0,'Yeah you\'re right, good sense of humor damn *laughter*'),(249,41,0,0,0,0,'So now I need meat. How to cook without meat.'),(250,41,1,0,0,0,'I think you\'ve heard of Kappa, they live near water, their meat is very tasty. So get it for me (you can self find them habitat)'),(251,41,0,0,2,1,'All delicious fine meat, looks great, take it!'),(252,41,0,0,2,0,'Well i let\'s start cooking :)'),(253,41,0,0,0,1,'**You see like she didn\'t know how to cook**: Are you sure you can cook?'),(254,41,0,1,4,0,'**She said angrily**: You know what I\'ll tell you [Player]... You don\'t know how to compliment girls at all..'),(255,41,0,0,5,1,'**Someone knocks on the door**: Who could it be? lovers?'),(256,41,0,0,3,0,'Oh it\'s probably my brother.'),(261,42,0,0,5,0,'**He said angrily and rudely**: Oh, what people, I\'ve seen you somewhere.'),(262,42,1,0,5,1,'You must have imagined it. '),(263,42,0,0,0,0,'Oh well.'),(264,43,0,0,2,0,'[Player] you look sad.'),(265,43,0,0,0,1,'Okay [Talked]. I\'ll probably go extra here'),(266,43,0,0,5,0,'Yes calm down you why so decided to?'),(267,43,0,2,2,1,'Yes, everything is fine. I just have things to do, too. I\'ll go!'),(268,43,0,0,1,0,'You offend me [Talked].....'),(269,43,1,0,0,1,'Sorry I\'m really busy [Talked].'),(270,43,0,0,5,0,'Well, take care of yourself.'),(271,44,0,0,0,0,'So let\'s continue our training.'),(272,44,0,0,0,1,'Yes, of course, it\'s about time!'),(273,44,0,1,4,0,'**He speaks rudely**: I don\'t really like you. But let\'s see what you can achieve. Let\'s go back to the Deep Cave.'),(274,45,0,0,0,0,'**He speaks proudly**: Well, now we have more serious goals.'),(275,45,1,0,0,0,'Exterminate the stronger slime. I\'ll see.'),(276,45,0,0,5,1,'Everything is ready, they are vile I do not like slime, maybe something new?'),(277,45,0,0,0,0,'Well, I\'ll think about your request. Follow me.'),(278,46,1,0,0,0,'Take it, use it wisely. You will need it very much in the future.'),(279,46,0,0,3,1,'**You were surprised by the gift provided**: Oh, thank you very much. Do I owe you something?'),(280,46,0,0,5,0,'I hope you make it to the end, at least I hope you do. I didn\'t think you\'d be Apostle\'s favorite.'),(281,46,0,0,3,1,'Favorite [Talked]?'),(282,46,0,1,4,0,'**He said very rudely**: I\'ll go, meet you later.'),(283,47,0,0,0,0,'Hello [Player]. Are you free?'),(284,47,0,0,0,1,'Yes, of course. Everything okay?'),(285,47,0,0,0,0,'Apostle asked you to see her!'),(286,47,1,0,0,1,'Well!'),(287,47,0,0,0,0,'Thank you, I\'m going for a walk!'),(288,48,0,0,1,0,'**She speaks in fear**: Hello [Player]. Today I received news that in the southern part of our village, goblins attacked. '),(289,48,0,0,1,0,'And it\'s not just goblins. This must be the army. The leader brought them.'),(290,48,0,0,3,1,'Maybe they kidnapped the residents and terrorized you, and now they decided to attack?'),(291,48,1,0,0,0,'I think so too. Anyway, I talked to Koko. About your appointment as an adventurer. We should prepare for resistance. Communication nodes they need to be destroyed'),(292,48,0,0,0,1,'Well, I did the job [Talked]!'),(293,48,0,0,0,0,'Good job thank\'s, you should meet with Koko, he will explain everything to you, good luck!'),(294,48,0,0,0,1,'Well, thank you!'),(295,49,0,0,0,0,'Get ready let\'s go to the Goblin-occupied zone.'),(296,49,0,0,0,1,'I\'m ready to sort of out right now.'),(297,49,0,2,2,0,'Well [Player], you\'re fun, Yes, but it\'s worth getting ready.'),(298,49,1,0,5,1,'Well, what do need [Talked]?'),(299,49,0,0,0,0,'I\'ll be waiting for you next to Craftsman.'),(300,50,0,0,0,0,'So first we need potions.'),(301,50,1,0,0,0,'Go get supplies, and at the same time exterminate a few slugs so that we can get there without any problems.'),(302,50,0,0,1,1,'**You sound very tired**: We can all move, but the problem is that I\'m tired..'),(303,50,0,1,5,0,'Nothing to worry about [Player]. You need to toughen up. All right let\'s move out.'),(304,51,1,0,0,0,'We\'ll get to the guard post now. We\'ll move out there to exterminate the goblins.'),(305,51,0,0,0,1,'Well [Talked].'),(306,52,0,0,5,1,'**Grinning**: Huh?'),(307,52,0,0,1,0,'We can\'t heal her..'),(308,52,0,0,5,1,'I don\'t understand..'),(309,52,0,0,1,0,'Our daughter, Maria.. She\'s sick!'),(310,52,0,0,0,0,'Why don\'t you take her to the Nurse?!'),(311,52,1,0,1,0,'I tried, but she said she can\'t do anything.. I found something in an old book of mine.. A gel treatment. But I also need Kappa Meat for it..'),(312,52,0,0,2,0,'Tha..nk.. You..'),(313,52,0,0,5,0,'Well.. don\'t look at me, heal Maria!'),(314,53,0,0,1,0,'F..Father?'),(315,54,0,2,2,0,'MARIA! You\'re well!'),(316,54,0,0,2,1,'*with joy* Hi, Maria!'),(317,54,0,0,0,0,'How can I ever make it up to you?'),(318,54,1,0,0,1,'Well Sir.. I might need a discount from your old friend, the Craftsman.'),(319,54,0,0,2,0,'Sure, I\'ll.. let him know you deserve it!'),(320,55,0,0,5,1,'Sure..What can I do?\r\n'),(321,55,0,0,5,0,'I heard you helped the craftsman with his deliveries..I have no time for small things like mining cooper, can you do it for me? I have to save the world in the main time..'),(322,55,1,0,5,1,' *a bit angry* Ok, I will do it.'),(323,55,0,0,2,0,'Oh, it\'s you kid!\r\n'),(324,55,0,1,4,1,'*you can\'t control yourself* HEY. I\'M NOT A KID!'),(325,55,0,0,0,0,'Hey.. easy, just a joke, did you get what I wanted?'),(326,55,0,0,5,1,'Yes.. *puts a heavy cooper bag on Erik\'s table*'),(327,55,0,1,4,0,'Good, now get out of here!'),(328,55,0,1,4,1,'Wait.. YOU NOT GONNA PAY ME??'),(329,55,0,1,4,0,'Haha! I am messing with you.'),(330,56,0,0,0,1,'So can you tell me how you got here?'),(331,56,0,0,5,0,'**He\'s stuttered**: A long story..'),(332,56,0,0,5,0,'We tried a new potion mixed with ether, but something went wrong, my hiccups are part of this experiment, and I ended up here after I drank it'),(333,56,0,0,0,0,'**Sound in the tablet**: Noctis... Noctiis. Do you hear me?'),(334,56,0,0,0,0,'**Noctis**: I can hear you, how do I get out of here?'),(335,56,0,0,0,0,'**Sound in the tablet**: Collect all the fragments that ended up here. The connection is lost....'),(336,56,0,0,0,1,'What were you talking to just now?'),(337,56,1,0,0,0,'A tablet made from a magic crystal. In General, as I thought I need your help in collecting fragments'),(338,56,0,0,0,1,'Of course I\'ll help'),(339,57,0,0,0,0,'Look at what I can give you in return for your help. (Vote menu shop list)'),(340,57,0,0,0,1,'Wow, [Talked] where can I find them?'),(341,57,0,0,0,0,'In a distorted dimension.'),(342,57,1,0,5,1,'Where is it? How do I get there?'),(343,57,0,0,0,0,'[Player] way, so we\'ll be preparing to get there.'),(344,58,1,0,0,0,'So first [Player], I\'ll need these things from you. They resonate between our worlds'),(345,58,0,0,0,1,'Take it [Talked].'),(346,58,0,0,0,0,'Well thank you, it will be a little easier now.'),(347,58,0,0,0,0,'**Sound in the tablet**: Noctis. How you to get the fragments we will be able to pick you up.'),(348,58,0,0,0,0,'**Noctis**: Okay, Sid..'),(349,59,1,0,0,0,'Next thing I need to create a resonance. Bring me these items.'),(350,59,0,0,0,1,'Everything is ready?. [Talked] take it'),(351,59,0,0,0,0,'[Player] yes.. almost, follow me and we\'ll start resonating.'),(352,60,0,0,0,0,'All ready.'),(353,60,0,0,0,0,'Resonance received. I will wait for you with fragments'),(354,60,1,0,5,1,'Can come to resonance?'),(355,60,0,0,0,0,'Yes. Don\'t worry nothing will happen to you.'),(356,61,1,2,2,0,'I welcome you [Talked], nice to meet you.'),(357,61,0,2,2,0,'Glad to meet you, i\'am [Player]!'),(358,62,0,0,0,0,'Him you will spend time here. I have to go to the village.'),(359,62,1,2,2,0,'Need to stand on defense in the village, good luck to you I will wait for good news from you.'),(360,62,0,0,2,0,'Good luck to you [Talked].'),(361,63,0,0,0,0,'I came from a far Eastern country, I have some problems in the country so I need to become stronger.'),(362,63,0,0,0,1,'I came for similar purposes. But I would like to visit your country.'),(363,63,0,0,2,0,'**He speaks with a smile**: If you wish I will be sailing back soon I can take you with me'),(364,63,1,0,0,1,'I\'d love to, but I don\'t know, i have here made friends. We\'ll see.'),(365,63,0,0,5,0,'Well, if we still live of course. And the got to talking and drive away goblins need.'),(366,64,1,0,0,0,'To begin with, we must exterminate many of them as possible. To get to them in zone'),(367,64,0,0,1,1,'**Very tired voice**: [Talked]. I\'m so tired, there are so many of them.....'),(368,64,0,0,1,0,'**He sounds very tired**: Yes, I am also very tired, but we have to deal with it.'),(369,64,0,0,0,0,'All right let\'s keep going before they pile up again.'),(370,65,0,0,0,0,'Hmmm.. I see a passageway. I think we can go through there and find out where they\'re coming from.'),(371,65,1,0,5,1,'I\'m worried about something.. Where did this hole?'),(372,65,0,0,5,0,'I don\'t know either, honestly. Maybe the goblins have dug through and are coming out. In any case we need to find out.'),(373,66,0,0,0,0,'I think this is where they live. '),(374,66,0,0,0,0,'In any case, if we can not cope with them, there is an option to block the way. But I think they\'ll dig it up again in time.'),(375,66,1,1,5,1,'**In anger**: LET\'S BREAK UP THESE FREAKS ALREADY... '),(376,66,0,1,2,0,'I\'m only for your idea. Let\'s go!'),(377,67,1,0,0,0,'And here is their leader.'),(378,67,0,2,2,1,'Did we manage?'),(379,67,0,0,5,0,'We don\'t know yet we need to get out of here....'),(380,67,0,0,0,0,'I\'ll be waiting for you where we fought the goblins.'),(384,68,1,0,0,1,'Finally we got out how do you think we managed?'),(385,68,0,0,5,0,'I don\'t know [Player], we have to report this news to the village.'),(386,69,0,0,2,0,'Well, I think my work here is done [Player].'),(387,69,0,0,5,1,'Your [Talked] next trip will be to your homeland?'),(388,69,0,0,2,0,'Yes, I think it\'s time to go home, I missed it.'),(389,69,0,0,2,1,'Will you take me with you?'),(390,69,0,0,2,0,'Yes, of course. Say goodbye to your friends or maybe one of them will go with you?'),(391,69,1,0,0,0,'No, I don\'t want to expose them to danger.'),(392,69,0,0,0,0,'Well'),(393,70,0,0,1,1,'Well, goodbye [Talked], I\'ll keep going. I want to be stronger than I was before.'),(394,70,0,0,2,0,'All right [Player]. Good luck don\'t forget that we will always be happy to welcome you as a guest ;)'),(395,70,1,0,2,1,'Don\'t forget, don\'t miss me ;)'),(396,70,0,0,0,0,'Don\'t worry, Diana is very attached to you. I think she will be offended. Talk to she!'),(397,71,0,0,1,0,'Why didn\'t you tell [Player] me you were going to leave?'),(398,71,0,0,1,1,'I\'m Sorry [Talked]. I wanted to tell you..'),(399,71,0,0,1,0,'**She said sadly**: Why? Will you take me with you?'),(400,71,0,0,1,1,'I don\'t want to put you in danger!'),(401,71,0,0,1,0,'I want to travel with you!'),(402,71,1,0,5,1,'Ok, I\'ll take you. Now let me to say Yasue San.'),(403,71,0,2,2,0,'**She said joyfully**: True? Thank! ;)'),(404,72,0,0,5,0,'Well, ready to go?'),(405,72,1,0,5,1,'Yes, will I take Diana with me?'),(406,72,0,2,2,0,'Yes, of course, set sail:) I will show you my homeland!');
/*!40000 ALTER TABLE `tw_dialogs_quest_npc` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_dungeons`
--

DROP TABLE IF EXISTS `tw_dungeons`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_dungeons` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(64) NOT NULL DEFAULT 'Unknown',
  `Level` int(11) NOT NULL DEFAULT '1',
  `DoorX` int(11) NOT NULL DEFAULT '0',
  `DoorY` int(11) NOT NULL DEFAULT '0',
  `RequiredQuestID` int(11) NOT NULL DEFAULT '-1',
  `Story` tinyint(4) NOT NULL DEFAULT '0',
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `WorldID` (`WorldID`),
  CONSTRAINT `tw_dungeons_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE NO ACTION ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_dungeons`
--

LOCK TABLES `tw_dungeons` WRITE;
/*!40000 ALTER TABLE `tw_dungeons` DISABLE KEYS */;
INSERT INTO `tw_dungeons` VALUES (1,'Abandoned mine',10,1105,1521,20,1,6),(2,'Resonance Noctis',18,1157,528,62,0,8),(3,'Kingdom Neptune',15,1084,532,22,1,10);
/*!40000 ALTER TABLE `tw_dungeons` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_dungeons_door`
--

DROP TABLE IF EXISTS `tw_dungeons_door`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_dungeons_door` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(64) NOT NULL DEFAULT 'Write here name dungeon',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `BotID` int(11) NOT NULL,
  `DungeonID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `tw_dungeons_door_ibfk_1` (`DungeonID`),
  KEY `tw_dungeons_door_ibfk_2` (`BotID`),
  CONSTRAINT `tw_dungeons_door_ibfk_1` FOREIGN KEY (`DungeonID`) REFERENCES `tw_dungeons` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_dungeons_door_ibfk_2` FOREIGN KEY (`BotID`) REFERENCES `tw_bots_info` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_dungeons_door`
--

LOCK TABLES `tw_dungeons_door` WRITE;
/*!40000 ALTER TABLE `tw_dungeons_door` DISABLE KEYS */;
INSERT INTO `tw_dungeons_door` VALUES (1,'Write here name dungeon',4302,1940,29,1),(2,'Write here name dungeon',1808,3600,30,1),(3,'Write here name dungeon',750,4850,31,1),(4,'Write here name dungeon',2255,4850,22,1),(5,'Write here name dungeon',5233,530,40,2),(6,'Write here name dungeon',4432,2929,41,2),(7,'Write here name dungeon',1550,1970,42,2),(8,'Write here name dungeon',1647,1970,43,2),(9,'Write here name dungeon',3213,369,47,3),(10,'Write here name dungeon',400,881,48,3),(11,'Write here name dungeon',4432,1490,49,3);
/*!40000 ALTER TABLE `tw_dungeons_door` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_dungeons_records`
--

DROP TABLE IF EXISTS `tw_dungeons_records`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_dungeons_records` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `UserID` int(11) NOT NULL,
  `DungeonID` int(11) NOT NULL,
  `Seconds` int(11) NOT NULL,
  `PassageHelp` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`),
  KEY `tw_dungeons_records_ibfk_1` (`UserID`),
  KEY `DungeonID` (`DungeonID`),
  KEY `Seconds` (`Seconds`),
  CONSTRAINT `tw_dungeons_records_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_dungeons_records_ibfk_2` FOREIGN KEY (`DungeonID`) REFERENCES `tw_dungeons` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=39 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_dungeons_records`
--

LOCK TABLES `tw_dungeons_records` WRITE;
/*!40000 ALTER TABLE `tw_dungeons_records` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_dungeons_records` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_guilds`
--

DROP TABLE IF EXISTS `tw_guilds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_guilds` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(32) NOT NULL DEFAULT 'Member name',
  `UserID` int(11) DEFAULT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Experience` int(11) NOT NULL DEFAULT '0',
  `Bank` int(11) NOT NULL DEFAULT '0',
  `Score` int(11) NOT NULL DEFAULT '0',
  `AvailableSlots` int(11) NOT NULL DEFAULT '2',
  `ChairExperience` int(11) NOT NULL DEFAULT '1',
  `ChairMoney` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `OwnerID` (`UserID`),
  KEY `Bank` (`Bank`),
  KEY `Level` (`Level`),
  KEY `Experience` (`Experience`),
  CONSTRAINT `tw_guilds_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_guilds`
--

LOCK TABLES `tw_guilds` WRITE;
/*!40000 ALTER TABLE `tw_guilds` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_guilds` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_guilds_decorations`
--

DROP TABLE IF EXISTS `tw_guilds_decorations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_guilds_decorations` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `HouseID` int(11) NOT NULL,
  `DecoID` int(11) NOT NULL,
  `WorldID` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `tw_guilds_decorations_ibfk_2` (`DecoID`),
  KEY `tw_guilds_decorations_ibfk_3` (`WorldID`),
  KEY `HouseID` (`HouseID`),
  CONSTRAINT `tw_guilds_decorations_ibfk_2` FOREIGN KEY (`DecoID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_guilds_decorations_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_guilds_decorations_ibfk_4` FOREIGN KEY (`HouseID`) REFERENCES `tw_guilds_houses` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_guilds_decorations`
--

LOCK TABLES `tw_guilds_decorations` WRITE;
/*!40000 ALTER TABLE `tw_guilds_decorations` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_guilds_decorations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_guilds_history`
--

DROP TABLE IF EXISTS `tw_guilds_history`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_guilds_history` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `GuildID` int(11) NOT NULL,
  `Text` varchar(64) NOT NULL,
  `Time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`ID`),
  KEY `MemberID` (`GuildID`),
  CONSTRAINT `tw_guilds_history_ibfk_1` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=279 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_guilds_history`
--

LOCK TABLES `tw_guilds_history` WRITE;
/*!40000 ALTER TABLE `tw_guilds_history` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_guilds_history` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_guilds_houses`
--

DROP TABLE IF EXISTS `tw_guilds_houses`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_guilds_houses` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `GuildID` int(11) DEFAULT NULL,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `DoorX` int(11) NOT NULL,
  `DoorY` int(11) NOT NULL,
  `TextX` int(11) NOT NULL,
  `TextY` int(11) NOT NULL,
  `Price` int(11) NOT NULL DEFAULT '50000',
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `OwnerMID` (`GuildID`),
  KEY `WorldID` (`WorldID`),
  CONSTRAINT `tw_guilds_houses_ibfk_1` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON UPDATE CASCADE,
  CONSTRAINT `tw_guilds_houses_ibfk_2` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_guilds_houses`
--

LOCK TABLES `tw_guilds_houses` WRITE;
/*!40000 ALTER TABLE `tw_guilds_houses` DISABLE KEYS */;
INSERT INTO `tw_guilds_houses` VALUES (1,NULL,4250,6352,4496,6461,4206,6224,240000,2),(2,NULL,9504,5713,9180,5713,9486,5495,280000,2);
/*!40000 ALTER TABLE `tw_guilds_houses` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_guilds_invites`
--

DROP TABLE IF EXISTS `tw_guilds_invites`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_guilds_invites` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `GuildID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `OwnerID` (`UserID`),
  KEY `MemberID` (`GuildID`),
  CONSTRAINT `tw_guilds_invites_ibfk_1` FOREIGN KEY (`UserID`) REFERENCES `tw_accounts_data` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_guilds_invites_ibfk_2` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_guilds_invites`
--

LOCK TABLES `tw_guilds_invites` WRITE;
/*!40000 ALTER TABLE `tw_guilds_invites` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_guilds_invites` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_guilds_ranks`
--

DROP TABLE IF EXISTS `tw_guilds_ranks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_guilds_ranks` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Access` int(11) NOT NULL DEFAULT '3',
  `Name` varchar(32) NOT NULL DEFAULT 'Rank name',
  `GuildID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `MemberID` (`GuildID`),
  CONSTRAINT `tw_guilds_ranks_ibfk_1` FOREIGN KEY (`GuildID`) REFERENCES `tw_guilds` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=18 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_guilds_ranks`
--

LOCK TABLES `tw_guilds_ranks` WRITE;
/*!40000 ALTER TABLE `tw_guilds_ranks` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_guilds_ranks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_houses`
--

DROP TABLE IF EXISTS `tw_houses`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_houses` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
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
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `OwnerID` (`UserID`),
  KEY `WorldID` (`WorldID`),
  KEY `PlantID` (`PlantID`),
  CONSTRAINT `tw_houses_ibfk_1` FOREIGN KEY (`PlantID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_houses_ibfk_2` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_houses`
--

LOCK TABLES `tw_houses` WRITE;
/*!40000 ALTER TABLE `tw_houses` DISABLE KEYS */;
INSERT INTO `tw_houses` VALUES (1,0,8995,7672,8752,7740,'Elven class',150000,160,18,9456,7766,2),(2,0,7999,5297,8241,5297,'Elven class',150000,100,18,7492,5329,2);
/*!40000 ALTER TABLE `tw_houses` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_houses_decorations`
--

DROP TABLE IF EXISTS `tw_houses_decorations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_houses_decorations` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `HouseID` int(11) NOT NULL,
  `DecoID` int(11) NOT NULL,
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `WorldID` (`WorldID`),
  KEY `HouseID` (`HouseID`),
  KEY `DecoID` (`DecoID`),
  CONSTRAINT `tw_houses_decorations_ibfk_1` FOREIGN KEY (`HouseID`) REFERENCES `tw_houses` (`ID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_houses_decorations_ibfk_2` FOREIGN KEY (`DecoID`) REFERENCES `tw_items_list` (`ItemID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tw_houses_decorations_ibfk_3` FOREIGN KEY (`WorldID`) REFERENCES `enum_worlds` (`WorldID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_houses_decorations`
--

LOCK TABLES `tw_houses_decorations` WRITE;
/*!40000 ALTER TABLE `tw_houses_decorations` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_houses_decorations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_items_list`
--

DROP TABLE IF EXISTS `tw_items_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_items_list` (
  `ItemID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(32) NOT NULL DEFAULT 'Item name',
  `Description` varchar(64) NOT NULL DEFAULT 'Item desc',
  `Icon` varchar(16) NOT NULL DEFAULT 'some1',
  `Type` int(11) DEFAULT '-1',
  `Function` int(11) DEFAULT '-1',
  `Desynthesis` int(11) NOT NULL DEFAULT '100',
  `Selling` int(11) NOT NULL DEFAULT '100',
  `Attribute0` int(11) DEFAULT NULL,
  `Attribute1` int(11) DEFAULT NULL,
  `AttributeValue0` int(11) NOT NULL DEFAULT '0',
  `AttributeValue1` int(11) NOT NULL,
  `ProjectileID` int(11) NOT NULL DEFAULT '-1' COMMENT 'only for weapons',
  PRIMARY KEY (`ItemID`),
  UNIQUE KEY `ItemID` (`ItemID`),
  KEY `ItemBonus` (`Attribute0`),
  KEY `ItemID_2` (`ItemID`),
  KEY `ItemType` (`Type`),
  KEY `tw_items_list_ibfk_3` (`Function`),
  KEY `ItemProjID` (`ProjectileID`),
  KEY `tw_items_list_ibfk_5` (`Attribute1`)
) ENGINE=InnoDB AUTO_INCREMENT=15004 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_items_list`
--

LOCK TABLES `tw_items_list` WRITE;
/*!40000 ALTER TABLE `tw_items_list` DISABLE KEYS */;
INSERT INTO `tw_items_list` VALUES (1,'Gold','Major currency','gold',-1,-1,0,0,16,NULL,0,0,-1),(2,'Hammer','A normal hammer','hammer',6,0,0,0,16,6,10,3,-1),(3,'Gun','Conventional weapon','gun',6,1,0,10,17,NULL,10,0,-1),(4,'Shotgun','Conventional weapon','shotgun',6,2,0,10,18,NULL,5,0,-1),(5,'Grenade','Conventional weapon','grenade',6,3,0,10,19,NULL,10,0,-1),(6,'Rifle','Conventional weapon','rifle',6,4,0,10,20,NULL,10,0,-1),(7,'Material','Required to improve weapons','material',4,-1,0,10,NULL,NULL,0,0,-1),(8,'Ticket guild','Command: /gcreate <name>','ticket',4,-1,0,10,NULL,NULL,0,0,-1),(9,'Skill Point','Skill point','skill_point',-1,-1,0,10,NULL,NULL,0,0,-1),(10,'Decoration Armor','Decoration for house!','deco_house',7,-1,0,10,NULL,NULL,0,0,-1),(11,'Decoration Hearth Elite','Decoration for house!','deco_house',7,-1,0,10,NULL,NULL,0,0,-1),(12,'Decoration Ninja Elite','Decoration for house!','deco_house',7,-1,0,10,NULL,NULL,0,0,-1),(13,'Decoration Hearth','Decoration for house!','deco_house',7,-1,0,10,NULL,NULL,0,0,-1),(14,'Potion mana regen','Regenerate +5%, 15sec every sec.\n','potion_b',8,8,20,10,NULL,NULL,0,0,-1),(15,'Potion health regen','Regenerate +3% health, 15sec every sec.','potion_r',8,8,20,10,NULL,NULL,0,0,-1),(16,'Capsule survival experience','You got 10-50 experience survival','potion_g',1,9,0,10,NULL,NULL,0,0,-1),(17,'Little bag of gold','You got 10-50 gold','pouch',1,9,0,10,NULL,NULL,0,0,-1),(18,'Mirt','Information added later.','some1',4,11,2,10,NULL,NULL,0,0,-1),(20,'Potato','Material need for craft!','potato',4,11,1,10,NULL,NULL,0,0,-1),(21,'Notebook','In it, something is written','paper',4,-1,0,10,NULL,NULL,0,0,-1),(22,'Glue','I wonder what it\'s for?','some4',4,-1,0,10,NULL,NULL,0,0,-1),(23,'Board','Plain Board','board',4,-1,0,10,NULL,NULL,0,0,-1),(24,'Mushroom','Material need for craft!','mushroom',4,11,2,10,NULL,NULL,0,0,-1),(25,'Potion resurrection','Resuscitates in the zone where you died!','potion_p',8,-1,0,10,NULL,NULL,0,0,-1),(26,'Goblin Pickaxe','It happens sometimes','pickaxe',6,5,0,10,14,15,2,3,-1),(27,'Young fighter\'s ring','It happens sometimes','ring',3,10,0,10,8,NULL,125,0,-1),(28,'Small ammo bag','Adds a small amount of ammunition','pouch',3,10,0,10,13,NULL,3,0,-1),(29,'Gel','I wonder what it\'s for?','some4',4,-1,0,10,NULL,NULL,0,0,-1),(30,'Goblin Ingot','Information added later.','ignot_g',4,-1,0,10,NULL,NULL,0,0,-1),(31,'Copper Ingot','Information added later.','ignot_o',4,-1,0,10,NULL,NULL,0,0,-1),(32,'Salantra Blessing','It happens sometimes','ticket',3,10,0,10,8,NULL,250,0,-1),(33,'Explosive module for gun','It happens sometimes','module',3,10,0,10,17,NULL,5,0,-1),(34,'Explosive module for shotgun','It happens sometimes','module',3,10,0,10,18,NULL,5,0,-1),(35,'Kappa meat','Information added later.','meat',4,-1,0,10,NULL,NULL,0,0,-1),(36,'Ring of Arvida','It happens sometimes','ring_light',3,10,0,10,11,10,5,10,-1),(37,'Relic of the Orc Lord','Information added later.','lucky_r',4,-1,0,10,NULL,NULL,0,0,-1),(38,'Ticket reset class stats','Resets only class stats(Dps, Tank, Healer).','ticket',1,8,0,10,NULL,NULL,0,0,-1),(39,'Mode PVP','Settings game.','without',5,10,0,0,NULL,NULL,0,0,-1),(40,'Ticket reset weapon stats','Resets only ammo stats(Ammo).','ticket',1,8,0,10,NULL,NULL,0,0,-1),(41,'Orc\'s Belt','You can feel the light power of mana.','mantle',3,10,0,10,10,5,25,40,-1),(42,'Torn cloth clothes of orcs','Information added later.','some2',4,-1,0,10,NULL,NULL,0,0,-1),(43,'Blessing for discount craft','Need dress it, -20% craft price','book',8,8,0,10,NULL,NULL,0,0,-1),(44,'Noctis fragment','Event Final Fantasy.','dark_crst',4,-1,0,10,NULL,NULL,0,0,-1),(45,'The core of Neptune','Information added later.','lucky_r',4,-1,0,10,NULL,NULL,0,0,-1),(46,'Remains of a dead soul','Information added later.','pouch',4,-1,0,10,NULL,NULL,0,0,-1),(47,'Ring of God of War','It happens sometimes','ring_fire',3,10,0,10,4,0,5,0,-1),(48,'Bracelet of Fire','It happens sometimes','bracelet_fire',3,10,0,10,4,0,7,0,-1),(49,'Big ammo bag','Adds a small amount of ammunition','pouch',3,10,0,10,13,NULL,8,0,-1),(50,'Strength ','Increase your strength','skill',-1,-1,0,0,4,NULL,1,0,-1),(10000,'Heavenly hammer','Reinforced kick','h_heaven',6,0,0,10,16,NULL,1,0,-1),(10001,'Heavenly gun','It look doesn\'t bad','g_heaven',6,1,0,10,17,NULL,10,0,3),(10002,'Heavenly shotgun','It look doesn\'t bad','s_heaven',6,2,0,10,18,NULL,10,0,4),(10003,'Heavenly grenade','It look doesn\'t bad','gr_heaven',6,3,0,10,19,NULL,10,0,5),(10004,'Heavenly rifle','It look doesn\'t bad','r_heaven',6,4,0,10,20,NULL,10,0,-1),(10005,'Shadow wings','Dark history','wings',6,6,0,10,7,NULL,300,0,-1),(10006,'Neptune wings','Covered in secrets','wings',6,6,0,10,11,NULL,300,0,-1),(10007,'Angel wings','Covered in secrets','wings',6,6,0,10,10,NULL,300,0,-1),(10008,'Heavenly wings','Covered in secrets','wings',6,6,0,10,8,NULL,300,0,-1),(10009,'Rainbow wings','Covered in secrets','wings',6,6,0,10,8,NULL,300,0,-1),(10010,'Magitech hammer','Reinforced kick','h_magitech',6,0,0,10,16,NULL,1,5,-1),(10011,'Magitech gun','It look doesn\'t bad','g_magitech',6,1,0,10,17,NULL,10,0,0),(10012,'Magitech shotgun','It look doesn\'t bad','s_magitech',6,2,0,10,18,NULL,10,0,1),(10013,'Magitech grenade','It look doesn\'t bad','gr_magitech',6,3,0,10,19,NULL,10,0,2),(10014,'Magitech rifle','It look doesn\'t bad','r_magitech',6,4,0,10,20,NULL,10,0,-1),(10015,'Stars wings','Covered in secrets','wings',6,6,0,10,8,NULL,300,0,-1),(10016,'Bat wings','Covered in secrets','wings',6,6,0,10,8,NULL,300,0,-1),(10017,'Little eagle wings','Covered in secrets','wings',6,6,0,10,8,NULL,300,0,-1),(10018,'Necromante wings','Covered in secrets','wings',6,6,0,10,8,NULL,300,0,-1),(10019,'Goblin hammer','Reinforced kick','h_goblin',6,0,0,10,16,NULL,25,0,-1),(10020,'Goblin gun','It look doesn\'t bad','g_goblin',6,1,0,10,17,NULL,25,0,6),(10021,'Goblin shotgun','It look doesn\'t bad','s_goblin',6,2,0,10,18,NULL,15,0,7),(10022,'Goblin grenade','It look doesn\'t bad','gr_goblin',6,3,0,10,19,NULL,25,0,8),(10023,'Goblin rifle','It look doesn\'t bad','r_goblin',6,4,0,10,20,NULL,25,0,-1),(10024,'Scythe','Reinforced kick','h_scythe',6,0,0,10,16,NULL,1,0,-1),(15000,'Theme Couple','Strictly limited as the theme','ticket',6,7,0,10,5,NULL,100,0,-1),(15001,'Theme Final Fantasy','Strictly limited as the theme','ticket',6,7,0,10,5,NULL,100,0,-1),(15002,'Theme Aion','Strictly limited as the theme','ticket',6,7,0,10,5,NULL,100,0,-1),(15003,'Theme Dragon Nest','Strictly limited as the theme','ticket',6,7,0,10,5,NULL,100,0,-1);
/*!40000 ALTER TABLE `tw_items_list` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_logics_worlds`
--

DROP TABLE IF EXISTS `tw_logics_worlds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_logics_worlds` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `MobID` int(11) NOT NULL,
  `Mode` int(11) NOT NULL DEFAULT '0' COMMENT '(1,3) 0 up 1 left',
  `ParseInt` int(11) NOT NULL COMMENT '(2) health (3)itemid key',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `WorldID` int(11) NOT NULL,
  `Comment` varchar(64) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `MobID` (`MobID`),
  KEY `WorldID` (`WorldID`),
  KEY `ParseInt` (`ParseInt`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_logics_worlds`
--

LOCK TABLES `tw_logics_worlds` WRITE;
/*!40000 ALTER TABLE `tw_logics_worlds` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_logics_worlds` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_positions_farming`
--

DROP TABLE IF EXISTS `tw_positions_farming`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_positions_farming` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ItemID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Distance` int(11) NOT NULL DEFAULT '300',
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `ItemID` (`ItemID`),
  KEY `WorldID` (`WorldID`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_positions_farming`
--

LOCK TABLES `tw_positions_farming` WRITE;
/*!40000 ALTER TABLE `tw_positions_farming` DISABLE KEYS */;
INSERT INTO `tw_positions_farming` VALUES (1,18,1,320,3585,250,5),(2,20,1,9952,4865,250,2);
/*!40000 ALTER TABLE `tw_positions_farming` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_positions_mining`
--

DROP TABLE IF EXISTS `tw_positions_mining`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_positions_mining` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ItemID` int(11) NOT NULL,
  `Level` int(11) NOT NULL DEFAULT '1',
  `Health` int(11) NOT NULL DEFAULT '100',
  `PositionX` int(11) NOT NULL,
  `PositionY` int(11) NOT NULL,
  `Distance` int(11) NOT NULL DEFAULT '300',
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `ItemID` (`ItemID`),
  KEY `WorldID` (`WorldID`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_positions_mining`
--

LOCK TABLES `tw_positions_mining` WRITE;
/*!40000 ALTER TABLE `tw_positions_mining` DISABLE KEYS */;
INSERT INTO `tw_positions_mining` VALUES (1,7,1,150,5742,686,300,0),(2,31,3,240,1485,4100,300,5),(3,31,3,240,3525,4100,2750,5);
/*!40000 ALTER TABLE `tw_positions_mining` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_quests_list`
--

DROP TABLE IF EXISTS `tw_quests_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_quests_list` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(24) NOT NULL DEFAULT 'Quest name',
  `Money` int(11) NOT NULL,
  `Exp` int(11) NOT NULL,
  `StoryLine` varchar(24) NOT NULL DEFAULT 'Hero',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=63 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_quests_list`
--

LOCK TABLES `tw_quests_list` WRITE;
/*!40000 ALTER TABLE `tw_quests_list` DISABLE KEYS */;
INSERT INTO `tw_quests_list` VALUES (1,'Help for workers',15,20,'Main: Arrival'),(2,'Helping a girl',15,20,'Main: Arrival'),(3,'The fight with slime!',25,20,'Trader Sentry'),(4,'Gel? Why do you need it?',25,20,'Trader Sentry'),(5,'Search for a daughter',40,30,'Sailor'),(6,'What is ore mining?',50,30,'Mining'),(7,'My first pickaxe',100,30,'Mining'),(8,'Help destruction of dirt',80,50,'Deputy'),(9,'Raid on the dirt',80,50,'Deputy'),(10,'Acquaintance',15,20,'Main: Apostle Elfia'),(11,'Apostle',20,20,'Main: Apostle Elfia'),(12,'Adventurer',50,40,'Main: Apostle Elfia'),(13,'Something is starting',10,20,'Main: Apostle Elfia'),(14,'History from Apostle',10,20,'Main: Apostle Elfia'),(15,'Diana has a problem',20,30,'Main: Apostle Elfia'),(16,'Mmm delicious',50,30,'Main: Apostle Elfia'),(17,'Time learn something',60,50,'Main: Apostle Elfia'),(18,'Here are the goblins',60,50,'Main: Apostle Elfia'),(19,'Occupation of goblins',60,50,'Main: Apostle Elfia'),(20,'Yasue San',60,50,'Main: Apostle Elfia'),(21,'Abandoned mine',60,50,'Main: Apostle Elfia'),(22,'Goodbye Elfinia',60,50,'Main: Apostle Elfia'),(50,'Officer\'s disputes!',60,50,'Officer Henry'),(55,'Erik\'s way saying help.',110,70,'Gunsmith Eric'),(60,'Why are you here Noctis',100,50,'Final fantasy'),(61,'First assignment',100,50,'Final fantasy'),(62,'Resonance',100,50,'Final fantasy');
/*!40000 ALTER TABLE `tw_quests_list` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_skills_list`
--

DROP TABLE IF EXISTS `tw_skills_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_skills_list` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(64) NOT NULL,
  `Description` varchar(64) NOT NULL,
  `BonusName` varchar(64) NOT NULL DEFAULT '''name''',
  `BonusValue` int(11) NOT NULL DEFAULT '1',
  `ManaPercentageCost` int(11) NOT NULL DEFAULT '10',
  `PriceSP` int(11) NOT NULL,
  `MaxLevel` int(11) NOT NULL,
  `Passive` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_skills_list`
--

LOCK TABLES `tw_skills_list` WRITE;
/*!40000 ALTER TABLE `tw_skills_list` DISABLE KEYS */;
INSERT INTO `tw_skills_list` VALUES (1,'Health turret','Creates turret a recovery health ','life span',3,25,24,8,0),(2,'Sleepy Gravity','Magnet mobs to itself','radius',20,25,28,10,0),(3,'Craft Discount','Will give discount on the price of craft items','% discount gold for craft item',1,0,28,50,1),(4,'Proficiency with weapons','You can perform an automatic fire','can perform an auto fire with all types of weapons',1,0,120,1,1),(5,'Blessing of God of war','The blessing restores ammo','% recovers ammo within a radius of 800',25,50,28,4,0),(6,'Noctis Lucis Attack Teleport','An attacking teleport that deals damage to all mobs radius','% your strength',25,10,100,4,0);
/*!40000 ALTER TABLE `tw_skills_list` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_storages`
--

DROP TABLE IF EXISTS `tw_storages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_storages` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(32) NOT NULL DEFAULT '''Bussines name''',
  `PosX` int(11) NOT NULL,
  `PosY` int(11) NOT NULL,
  `Currency` int(11) NOT NULL DEFAULT '1',
  `WorldID` int(11) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `WorldID` (`WorldID`),
  KEY `Currency` (`Currency`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_storages`
--

LOCK TABLES `tw_storages` WRITE;
/*!40000 ALTER TABLE `tw_storages` DISABLE KEYS */;
INSERT INTO `tw_storages` VALUES (1,'Weapons for young adventurers',9417,6817,1,2),(2,'Elfinia Artifacts',6256,6417,1,2),(3,'Noctis Lucis Caelum',3200,3520,44,5),(5,'Master La',7352,4859,9,2);
/*!40000 ALTER TABLE `tw_storages` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_store_items`
--

DROP TABLE IF EXISTS `tw_store_items`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_store_items` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ItemID` int(11) NOT NULL,
  `ItemValue` int(11) NOT NULL,
  `RequiredItemID` int(11) NOT NULL DEFAULT '1',
  `Price` int(11) NOT NULL,
  `UserID` int(11) NOT NULL DEFAULT '0',
  `Enchant` int(11) NOT NULL DEFAULT '0',
  `StorageID` int(11) DEFAULT NULL,
  `Time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `ItemID` (`ItemID`),
  KEY `OwnerID` (`UserID`),
  KEY `StorageID` (`StorageID`),
  KEY `Time` (`Time`),
  KEY `NeedItem` (`RequiredItemID`),
  KEY `Price` (`Price`)
) ENGINE=InnoDB AUTO_INCREMENT=268 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_store_items`
--

LOCK TABLES `tw_store_items` WRITE;
/*!40000 ALTER TABLE `tw_store_items` DISABLE KEYS */;
INSERT INTO `tw_store_items` VALUES (1,3,1,1,120,0,0,1,'2020-05-10 18:36:16'),(2,4,1,1,310,0,0,1,'2020-05-10 18:36:16'),(3,5,1,1,320,0,0,1,'2020-05-10 18:36:16'),(4,6,1,1,400,0,0,1,'2020-05-10 18:36:16'),(5,28,1,1,980,0,0,2,'2020-05-13 21:19:28'),(6,36,1,1,690,0,0,2,'2020-05-13 21:19:28'),(7,8,1,1,3800,0,0,2,'2020-05-13 21:19:28'),(8,49,1,1,2100,0,0,2,'2020-05-13 22:19:28'),(12,47,1,1,1200,0,0,2,'2020-05-13 19:19:28'),(13,48,1,1,1500,0,0,2,'2020-05-13 19:19:28'),(19,38,1,1,3200,0,0,2,'2020-05-13 21:19:28'),(22,40,1,1,2500,0,0,2,'2020-05-13 21:19:28'),(42,15001,1,44,300,0,0,3,'2020-05-13 21:19:28'),(43,10017,1,44,300,0,0,3,'2020-05-13 21:19:28'),(45,50,1,9,5,0,0,5,'2020-05-13 22:19:28');
/*!40000 ALTER TABLE `tw_store_items` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_voucher`
--

DROP TABLE IF EXISTS `tw_voucher`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_voucher` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `Code` varchar(32) NOT NULL,
  `Data` text NOT NULL,
  `Multiple` tinyint(1) NOT NULL DEFAULT '0',
  `ValidUntil` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_voucher`
--

LOCK TABLES `tw_voucher` WRITE;
/*!40000 ALTER TABLE `tw_voucher` DISABLE KEYS */;
INSERT INTO `tw_voucher` VALUES (1,'VALENTINE2021','{\r\n	\"exp\": 10000,\r\n	\"items\": [\r\n		{\r\n			\"id\": 17,\r\n			\"value\": 30\r\n		},\r\n		{\r\n			\"id\": 15000,\r\n			\"value\": 1\r\n		}\r\n	]\r\n}',1,1614517578);
/*!40000 ALTER TABLE `tw_voucher` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_voucher_redeemed`
--

DROP TABLE IF EXISTS `tw_voucher_redeemed`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_voucher_redeemed` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `VoucherID` int(11) NOT NULL,
  `UserID` int(11) NOT NULL,
  `TimeCreated` int(11) NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_voucher_redeemed`
--

LOCK TABLES `tw_voucher_redeemed` WRITE;
/*!40000 ALTER TABLE `tw_voucher_redeemed` DISABLE KEYS */;
/*!40000 ALTER TABLE `tw_voucher_redeemed` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tw_world_swap`
--

DROP TABLE IF EXISTS `tw_world_swap`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tw_world_swap` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `WorldID` int(11) DEFAULT NULL,
  `PositionX` int(11) DEFAULT NULL,
  `PositionY` int(11) DEFAULT NULL,
  `RequiredQuestID` int(11) DEFAULT NULL,
  `TwoWorldID` int(11) DEFAULT NULL,
  `TwoPositionX` int(11) DEFAULT NULL,
  `TwoPositionY` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `WorldID` (`WorldID`),
  KEY `TwoWorldID` (`TwoWorldID`),
  KEY `tw_world_swap_ibfk_3` (`RequiredQuestID`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tw_world_swap`
--

LOCK TABLES `tw_world_swap` WRITE;
/*!40000 ALTER TABLE `tw_world_swap` DISABLE KEYS */;
INSERT INTO `tw_world_swap` VALUES (1,0,6900,1000,1,1,335,490),(2,1,4605,1067,2,2,3570,7950),(3,2,13760,6680,12,3,400,1260),(4,2,3510,6340,13,4,4740,900),(5,3,4560,1205,19,5,610,4500),(6,2,8328,6020,15,7,4135,840),(7,5,4896,4276,22,9,2905,1227);
/*!40000 ALTER TABLE `tw_world_swap` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-08-30 18:48:31
