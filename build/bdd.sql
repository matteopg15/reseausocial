/*!999999\- enable the sandbox mode */ 
-- MariaDB dump 10.19  Distrib 10.11.8-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: reseausocial
-- ------------------------------------------------------
-- Server version	10.11.8-MariaDB-1

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
-- Table structure for table `Abonne`
--

DROP TABLE IF EXISTS `Abonne`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Abonne` (
  `compte` varchar(200) NOT NULL,
  `salon` varchar(200) NOT NULL,
  `gerant` varchar(200) NOT NULL,
  `idfirst` int(11) DEFAULT NULL,
  `idlast` int(11) DEFAULT NULL,
  PRIMARY KEY (`compte`,`salon`,`gerant`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Abonne`
--

LOCK TABLES `Abonne` WRITE;
/*!40000 ALTER TABLE `Abonne` DISABLE KEYS */;
INSERT INTO `Abonne` VALUES
('Admin','Mailbox','Admin',1,0),
('Admin','Mobile','matteo',1,0),
('Admin','Personnel','Gilles',1,0),
('Admin','Public','Admin',1,5),
('Gilles','Personnel','Gilles',6,5),
('Gilles','Public','Admin',3,2),
('kiwi','Public','Admin',1,0),
('kiwi','Public','Admin',1,0),
('matteo','Mobile','matteo',3,2),
('matteo','Public','Admin',3,5),
('patate','Public','Admin',1,0),
('test','Public','Admin',1,0),
('test1','Public','Admin',1,0),
('test2','Mobile','matteo',1,0),
('test2','Public','Admin',1,0);
/*!40000 ALTER TABLE `Abonne` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Compte`
--

DROP TABLE IF EXISTS `Compte`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Compte` (
  `nom` varchar(200) NOT NULL,
  `mdp` text DEFAULT NULL,
  `avatar` text DEFAULT NULL,
  `sexe` text DEFAULT NULL,
  `actif` tinyint(1) DEFAULT NULL,
  `totpsecret` text DEFAULT NULL,
  `totpcode` text DEFAULT NULL,
  `activite` text DEFAULT NULL,
  PRIMARY KEY (`nom`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Compte`
--

LOCK TABLES `Compte` WRITE;
/*!40000 ALTER TABLE `Compte` DISABLE KEYS */;
INSERT INTO `Compte` VALUES
('Admin','admin','/reseausocial/admin.png','homme',1,'','','1718369550'),
('Gilles','àgilles','/reseausocial/homme.png','homme',1,'','','1718351604'),
('kiwi','wiki','/reseausocial/homme.png','homme',0,'','','1718355763'),
('matteo','matteo','/reseausocial/homme.png','homme',1,'','','1718369703'),
('Mobile','1234','/reseausocial/homme.png','homme',1,'','','1718028073'),
('patate','1234','/reseausocial/femme.png','femme',1,'','','1718021736'),
('test','1234','/reseausocial/homme.png','homme',1,'','','0'),
('test1','1234','/reseausocial/homme.png','homme',1,'','','0'),
('test2','1234','/reseausocial/homme.png','homme',1,'','','0');
/*!40000 ALTER TABLE `Compte` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Message`
--

DROP TABLE IF EXISTS `Message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Message` (
  `emetteur` varchar(200) DEFAULT NULL,
  `salon` varchar(200) NOT NULL,
  `gerant` varchar(200) NOT NULL,
  `contenu` text DEFAULT NULL,
  `lien` text DEFAULT NULL,
  `image` text DEFAULT NULL,
  `youtube` text DEFAULT NULL,
  `dailymotion` text DEFAULT NULL,
  `autre` text DEFAULT NULL,
  `horodatage` text DEFAULT NULL,
  `id` int(11) NOT NULL,
  PRIMARY KEY (`salon`,`gerant`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Message`
--

LOCK TABLES `Message` WRITE;
/*!40000 ALTER TABLE `Message` DISABLE KEYS */;
INSERT INTO `Message` VALUES
('matteo','Mobile','matteo','pdt','','','','','','Fri 14 Jun 2024 14:12:18',2),
('Admin','Personnel','Gilles','matteo a rejoint le salon','','','','','','Thu 13 Jun 2024 16:19:14',1),
('Admin','Personnel','Gilles','patate a rejoint le salon','','','','','','Thu 13 Jun 2024 16:19:19',2),
('matteo','Personnel','Gilles','sgfdlhkjgfdslhkjgfdshlkj','','','','','','Thu 13 Jun 2024 16:24:34',3),
('Admin','Personnel','Gilles','matteo a été exclu du salon','','','','','','Fri 14 Jun 2024 08:31:07',4),
('Admin','Personnel','Gilles','patate a été exclu du salon','','','','','','Fri 14 Jun 2024 08:31:12',5),
('matteo','Public','Admin','pomme de terre','','','','','','Thu 13 Jun 2024 15:39:40',2),
('Admin','Public','Admin',':Z+6hKa.9w8Rr}G2Cu9%G3)h','','','','','','Fri 14 Jun 2024 11:04:01',3),
('matteo','Public','Admin','abc','https://xpresso.ddns.net/valerian','/reseausocial/imgtest.png','https://www.youtube.com/embed/4A6_mBW03EI/','https://www.dailymotion.com/embed/video/x907zpw','/reseausocial/videotest.mp4','Fri 14 Jun 2024 11:35:53',4),
('Admin','Public','Admin','pomme de terre','','','','','','Fri 14 Jun 2024 14:52:30',5);
/*!40000 ALTER TABLE `Message` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Salon`
--

DROP TABLE IF EXISTS `Salon`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Salon` (
  `nom` varchar(200) NOT NULL,
  `gerant` varchar(200) NOT NULL,
  PRIMARY KEY (`nom`,`gerant`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Salon`
--

LOCK TABLES `Salon` WRITE;
/*!40000 ALTER TABLE `Salon` DISABLE KEYS */;
INSERT INTO `Salon` VALUES
('Mailbox','Admin'),
('Mobile','matteo'),
('Personnel','Gilles'),
('Public','Admin');
/*!40000 ALTER TABLE `Salon` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-06-14 14:55:54
