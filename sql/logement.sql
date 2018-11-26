-------------------------------------------------------
-- Q2: Détruire toutes les tables de notre base de donnée
-------------------------------------------------------
DROP TABLE IF EXISTS MESURE;
DROP TABLE IF EXISTS FACTURE;
DROP TABLE IF EXISTS TYPE_CAPTEUR;
DROP TABLE IF EXISTS CAPTEUR;
DROP TABLE IF EXISTS PIECE;
DROP TABLE IF EXISTS LOGEMENT;

----------------------------------------------------
-- Q3: Creer toutes les tables de notre base de donnée
----------------------------------------------------
-- UNSIGNED: un id n'est pas signé
-- AUTO_INCREMENT: permet de laisser le sgbd gérer l'incrémentation du champ
-- NOT NULL: permet d'empecher quelqu'un d'entrer une valeur nulle dans ce champ
-- TIMESTAMP DEFAULT CURRENT_TIMESTAMP: permet de laisser le sgbd completer la date d'insertion
CREATE TABLE LOGEMENT(
	ID_LOGEMENT integer PRIMARY KEY AUTOINCREMENT, 
	ip varchar(63) NOT NULL,
	adresse varchar(255) NOT NULL,
	num_tel varchar(15),
	date_insert TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE PIECE(
	ID_PIECE integer PRIMARY KEY AUTOINCREMENT,
	id_logement integer UNSIGNED NOT NULL,
	capteurs varchar(255),
	position varchar(15) NOT NULL,

	FOREIGN KEY(id_logement) REFERENCES LOGEMENT(ID_LOGEMENT)
);

CREATE TABLE CAPTEUR(
	ID_CAPTEUR integer PRIMARY KEY AUTOINCREMENT,
	id_piece integer UNSIGNED NOT NULL,
	type varchar(63) NOT NULL,
	reference varchar(63) NOT NULL,
	port integer UNSIGNED NOT NULL,
	date_insert TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

	FOREIGN KEY(id_piece) REFERENCES PIECE(ID_PIECE)	
);

CREATE TABLE FACTURE(
	id_logement integer UNSIGNED NOT NULL,
	type varchar(15) NOT NULL,
	date_f TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	montant float(8,2) NOT NULL,
	valeur integer NOT NULL,

	FOREIGN KEY(id_logement) REFERENCES LOGEMENT(ID_LOGEMENT)	
);

CREATE TABLE MESURE(
	id_capteur integer UNSIGNED NOT NULL,
	valeur integer NOT NULL,
	date_mesure TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

	FOREIGN KEY(id_capteur) REFERENCES CAPTEUR(ID_CAPTEUR)
);

CREATE TABLE TYPE_CAPTEUR(
	TYPE varchar(15) PRIMARY KEY NOT NULL,
	precision varchar(15),
	unite varchar(31) NOT NULL
);

---------------------------------------
-- Q4: Creer un logement avec 4 pièces
---------------------------------------
INSERT INTO LOGEMENT (adresse, ip) VALUES ("25 Rue de la Guerre, YNOHTNA", "192.168.0.1");
INSERT INTO PIECE (id_logement, capteurs, position) VALUES (1,"c1,c2,c3","0,2,3");
INSERT INTO PIECE (id_logement, capteurs, position) VALUES (1,"c4,c32","1,2,3");
INSERT INTO PIECE (id_logement, capteurs, position) VALUES (1,"c30","1,1,1");
INSERT INTO PIECE (id_logement, capteurs, position) VALUES (1,"c9","1,2,1");

-----------------------------------------------
-- Q5: Creer 4 types de capteurs / actionneurs
-----------------------------------------------
INSERT INTO TYPE_CAPTEUR(TYPE, precision, unite) VALUES ("temperature","0.1", "°K");
INSERT INTO TYPE_CAPTEUR(TYPE, precision, unite) VALUES ("electricite","0.2", "W");
INSERT INTO TYPE_CAPTEUR(TYPE, precision, unite) VALUES ("pression","0.1", "N.m⁻2");
INSERT INTO TYPE_CAPTEUR(TYPE, precision, unite) VALUES ("masse","0.1", "Kg");

--------------------------------------
-- Q6: Creer 2 capteurs / actionneurs
--------------------------------------
INSERT INTO CAPTEUR(id_piece,type,reference,port) VALUES (1,"electricite","ESP32a",25565);
INSERT INTO CAPTEUR(id_piece,type,reference,port) VALUES (2,"electricite","ESP32b",25565);
INSERT INTO CAPTEUR(id_piece,type,reference,port) VALUES (2,"eau","ESP32c",25564);
INSERT INTO CAPTEUR(id_piece,type,reference,port) VALUES (1,"gaz","ESP86a",25564);
INSERT INTO CAPTEUR(id_piece,type,reference,port) VALUES (3,"gaz","ESP86b",25564);

----------------------------------------------
-- Q7: Creer 2 mesures par capteur/actionneur
----------------------------------------------
INSERT INTO MESURE(id_capteur,valeur) VALUES (1,39873);
INSERT INTO MESURE(id_capteur,valeur) VALUES (1,1415);
INSERT INTO MESURE(id_capteur,valeur) VALUES (2,1);
INSERT INTO MESURE(id_capteur,valeur) VALUES (2,21);

-----------------------
-- Q7: Creer 4 factures
-----------------------
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"gaz",400,100);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"eau",1000,352);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"electricite",5823,120);

INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"gaz",200,30);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"eau",800,200);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"electricite",2000,200);

INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"gaz",200,352);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"eau",800,200);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"electricite",2000,220);

INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"gaz",200,130);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"eau",800,270);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"electricite",2000,300);

INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"gaz",200,200);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"eau",800,220);
INSERT INTO FACTURE(id_logement,type,montant,valeur) VALUES(1,"electricite",2000,290);

-- LIST TABLES
-- .tables
