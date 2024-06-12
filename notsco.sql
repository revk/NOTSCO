-- MariaDB dump 10.19  Distrib 10.11.6-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: notsco
-- ------------------------------------------------------
-- Server version	10.11.6-MariaDB-0+deb12u1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `auth`
--

DROP TABLE IF EXISTS `auth`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `auth` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `tester` int(10) unsigned NOT NULL,
  `bearer` varchar(72) NOT NULL,
  `expiry` datetime NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `bearer` (`bearer`),
  KEY `auth_tester` (`tester`),
  KEY `expiry` (`expiry`),
  CONSTRAINT `auth_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `directory`
--

DROP TABLE IF EXISTS `directory`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `directory` (
  `rcpid` char(4) NOT NULL,
  `company` tinytext DEFAULT NULL,
  `sales` tinytext DEFAULT NULL,
  `support` tinytext DEFAULT NULL,
  `active` enum('N','Y') NOT NULL DEFAULT 'Y',
  PRIMARY KEY (`rcpid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `log`
--

DROP TABLE IF EXISTS `log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `log` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `tester` int(10) unsigned DEFAULT NULL,
  `ts` datetime NOT NULL DEFAULT curtime(),
  `description` tinytext DEFAULT NULL,
  `status` int(3) unsigned DEFAULT NULL,
  `ip` varchar(39) DEFAULT NULL,
  `rx` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL CHECK (json_valid(`rx`)),
  `rxerror` text DEFAULT NULL,
  `tx` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL CHECK (json_valid(`tx`)),
  `txerror` text DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `log_tester` (`tester`),
  KEY `ts` (`ts`),
  CONSTRAINT `log_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `sor`
--

DROP TABLE IF EXISTS `sor`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sor` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `tester` int(10) unsigned DEFAULT NULL,
  `sor` uuid NOT NULL,
  `issuedby` enum('US','THEM') DEFAULT NULL,
  `created` datetime DEFAULT NULL,
  `triggered` datetime DEFAULT NULL,
  `cancelled` datetime DEFAULT NULL,
  `dated` date DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `sor_tester` (`tester`),
  KEY `sor` (`sor`),
  CONSTRAINT `sor_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `tester`
--

DROP TABLE IF EXISTS `tester`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tester` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `email` tinytext NOT NULL,
  `company` tinytext DEFAULT NULL,
  `clientid` char(20) NOT NULL,
  `clientsecret` tinytext NOT NULL,
  `rcpid` char(4) DEFAULT NULL,
  `tokenhost` tinytext DEFAULT NULL,
  `apihost` tinytext DEFAULT NULL,
  `farclientid` tinytext DEFAULT NULL,
  `farclientsecret` tinytext DEFAULT NULL,
  `matchresponse` enum('None','Match','Match+Alt','NoMatch','DeliveryFail') DEFAULT 'Match',
  `matcherror` int(4) NOT NULL DEFAULT 0,
  `ontref` tinytext DEFAULT NULL,
  `ontport` int(11) NOT NULL DEFAULT 0,
  `dn` varchar(20) DEFAULT NULL,
  `partialdn` varchar(2) DEFAULT NULL,
  `alid` varchar(20) DEFAULT NULL,
  `bearer` tinytext DEFAULT NULL,
  `expiry` datetime DEFAULT NULL,
  `delay` int(3) unsigned NOT NULL DEFAULT 5,
  PRIMARY KEY (`ID`),
  KEY `clientid` (`clientid`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-06-12 10:46:27
