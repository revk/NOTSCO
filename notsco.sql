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
  `bearer` text NOT NULL,
  `expiry` datetime NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `bearer` (`bearer`(768)),
  KEY `auth_tester` (`tester`),
  KEY `expiry` (`expiry`),
  CONSTRAINT `auth_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=349 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
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
  `ts` datetime(3) DEFAULT curtime(3),
  `description` tinytext DEFAULT NULL,
  `status` int(3) unsigned DEFAULT NULL,
  `ip` varchar(39) DEFAULT NULL,
  `ms` int(11) DEFAULT NULL,
  `rxcorrelation` tinytext DEFAULT NULL,
  `rx` text DEFAULT NULL,
  `rxerror` text DEFAULT NULL,
  `tx` text DEFAULT NULL,
  `txerror` text DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `log_tester` (`tester`),
  KEY `ts` (`ts`),
  CONSTRAINT `log_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=2947 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pending`
--

DROP TABLE IF EXISTS `pending`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pending` (
  `correlation` uuid NOT NULL,
  `request` varchar(50) DEFAULT NULL,
  `sent` datetime NOT NULL DEFAULT curtime(),
  `recd` datetime DEFAULT NULL,
  `tester` int(10) unsigned DEFAULT NULL,
  UNIQUE KEY `tester` (`tester`,`correlation`,`request`),
  KEY `pending_tester` (`tester`),
  KEY `sent` (`sent`),
  CONSTRAINT `pending_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `scorecard`
--

DROP TABLE IF EXISTS `scorecard`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `scorecard` (
  `tester` int(10) unsigned NOT NULL,
  `routing` varchar(50) DEFAULT NULL,
  `direction` enum('Rx','Tx') DEFAULT NULL,
  `status` enum('CLEAN','ERRORS') DEFAULT NULL,
  `first` datetime DEFAULT NULL,
  `last` datetime DEFAULT NULL,
  `count` int(10) unsigned DEFAULT NULL,
  UNIQUE KEY `tester` (`tester`,`routing`,`direction`,`status`),
  KEY `scorecard_tester` (`tester`),
  KEY `last` (`last`),
  CONSTRAINT `scorecard_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
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
  `rcpid` char(4) DEFAULT NULL,
  `sor` uuid NOT NULL,
  `issuedby` enum('US','THEM') DEFAULT NULL,
  `created` datetime NOT NULL DEFAULT curtime(),
  `dated` date DEFAULT NULL,
  `status` enum('new','confirmed','updated','triggered','cancelled') NOT NULL DEFAULT 'new',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `tester` (`tester`,`rcpid`,`sor`),
  KEY `sor_tester` (`tester`),
  KEY `created` (`created`),
  CONSTRAINT `sor_tester` FOREIGN KEY (`tester`) REFERENCES `tester` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=555 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
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
  `emailed` datetime DEFAULT NULL,
  `lastlogin` datetime DEFAULT NULL,
  `company` tinytext DEFAULT NULL,
  `auth` enum('OAUTH2','OAUTH2Scope','APIKEY') NOT NULL DEFAULT 'OAUTH2',
  `apikey` tinytext DEFAULT NULL,
  `clientid` char(20) NOT NULL,
  `clientsecret` tinytext NOT NULL,
  `rcpid` char(4) DEFAULT NULL,
  `tokenurl` tinytext DEFAULT NULL,
  `apiurl` tinytext DEFAULT NULL,
  `farclientid` tinytext DEFAULT NULL,
  `farclientsecret` tinytext DEFAULT NULL,
  `sentto` tinytext DEFAULT NULL,
  `sentto2` tinytext DEFAULT NULL,
  `ontref` tinytext DEFAULT NULL,
  `ontport` int(11) NOT NULL DEFAULT 0,
  `dn1` varchar(20) DEFAULT NULL,
  `dn3` varchar(20) DEFAULT NULL,
  `dn2` varchar(20) DEFAULT NULL,
  `iasdn` varchar(20) DEFAULT NULL,
  `iasaction` enum('Normal','ForcedCease','ServiceWithAnotherRCP','ServiceWithAnotherCust') DEFAULT 'Normal',
  `alid` varchar(20) DEFAULT NULL,
  `bearer` text DEFAULT NULL,
  `expiry` datetime DEFAULT NULL,
  `delay` int(3) unsigned NOT NULL DEFAULT 5,
  `servicename` tinytext DEFAULT '1GB Broadband',
  `iasnetworkoperator` char(4) NOT NULL DEFAULT 'A001',
  `cupid` int(3) unsigned zerofill DEFAULT 000,
  `fromrcpid` char(4) DEFAULT 'ZZZZ',
  `brand` tinytext DEFAULT NULL,
  `surname` tinytext DEFAULT NULL,
  `account` tinytext DEFAULT NULL,
  `uprn` bigint(12) unsigned DEFAULT 0,
  `address1` tinytext DEFAULT NULL,
  `address2` tinytext DEFAULT NULL,
  `address3` tinytext DEFAULT NULL,
  `address4` tinytext DEFAULT NULL,
  `address5` tinytext DEFAULT NULL,
  `posttown` tinytext DEFAULT NULL,
  `postcode` varchar(8) DEFAULT NULL,
  `circuit` tinytext DEFAULT NULL,
  `portdn` varchar(11) DEFAULT NULL,
  `identifydn` varchar(11) DEFAULT NULL,
  `matchresponse` int(4) NOT NULL DEFAULT 1,
  `orderresponse` int(4) NOT NULL DEFAULT 1,
  `nbicsnetworkoperator` char(4) NOT NULL DEFAULT 'A000',
  `nbicsaction` enum('Normal','ForcedCease','ServiceWithAnotherRCP','ServiceWithAnotherCust') DEFAULT 'Normal',
  PRIMARY KEY (`ID`),
  KEY `clientid` (`clientid`),
  KEY `email` (`email`(255)),
  KEY `emailed` (`emailed`),
  KEY `lastlogin` (`lastlogin`),
  KEY `apikey` (`apikey`(255))
) ENGINE=InnoDB AUTO_INCREMENT=694 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-08-26  7:41:15
