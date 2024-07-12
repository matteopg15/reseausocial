CREATE TABLE Compte (nom VARCHAR(200), mdp text, avatar text, sexe text, actif bool, totpsecret text, totpcode text, activite text, PRIMARY KEY(nom));
CREATE TABLE Salon (nom VARCHAR(200), gerant VARCHAR(200), PRIMARY KEY(nom, gerant));
CREATE TABLE Abonne (compte VARCHAR(200), salon VARCHAR(200), gerant VARCHAR(200), idfirst integer, idlast integer, PRIMARY KEY(compte, salon, gerant));
CREATE TABLE Message(emetteur VARCHAR(200), salon VARCHAR(200), gerant VARCHAR(200), contenu text, lien text, image text, youtube text, dailymotion text, autre text, horodatage text, id integer, PRIMARY KEY(salon, gerant, id));

INSERT INTO Compte VALUES ('Admin', 'admin', '/reseausocial/admin.png', 'homme', true, '', '', '0');
INSERT INTO Salon VALUES('Public', 'Admin');
INSERT INTO Abonne VALUES('Admin', 'Public', 'Admin', 1,0);
INSERT INTO Message VALUES ('Admin', 'Public', 'Admin',"Bienvenue dans le salon Public !", "", "", "", "", "", "01 Jan 1970 à 00:00:00", 1);