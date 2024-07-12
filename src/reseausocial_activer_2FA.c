#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define __USE_XOPEN
#include <time.h>
#include "commun.c"
#include "mysql.h"
#include "password.h"


int main(void){

	char *envoi; //Données reçues
	char *login, *mdp, *query, *message_erreur, *totpsecret, *activite, *code, *totpcode, *commande, *reponse;
	int succes = 0; //succes=0 --> échec // succes=1 --> Verifications OK
	int fd, nb; //respectivement pour stocker les files dscriptor et pour stocker le nbrs de résultats

	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));

	
	login          = (char*)malloc(query_size+20000); //identifiant
	mdp            = (char*)malloc(query_size+20000); //identifiant
	query          = (char*)malloc(20000);            //Requête SQL
	totpsecret     = (char*)malloc(20000);			  //Secret TOTP
	totpcode       = (char*)malloc(20000);            //Code TOTP (BDD)
	activite       = (char*)malloc(100);              //Compte actif ?
	code           = (char*)malloc(query_size+20000); //Code TOTP fourni par l'utilisateur
	message_erreur = (char*)malloc(query_size+20000); //Eventuel message d'erreur
	commande       = (char*)malloc(20000);            //Pour stocker les commandes à exécuter
	reponse        = (char*)malloc(20000);            //Pour stocker les réponses des commandes à exécuter

	get_chaine(envoi, 1, code);
	get_chaine(envoi, 2, mdp);
	get_chaine(envoi, 3, login);

	tamb(code);
	tamb(mdp);
	tamb(login);

	if ((handler=db_opendatabase("reseausocial", "localhost", "reseausocial", PASSWORD))==NULL){    //Test BDD
		strcpy(message_erreur, "Impossible d'accéder à la base de données.");
	}
	else{

		sprintf(query, "SELECT * FROM Compte WHERE nom='%s';", login);
		db_query(handler, query);

		if (db_ntuples(result)!= 1) strcpy(message_erreur,"Cet utilisateur n'existe pas.");        //Test utilisateur existe
		else{
			
			char *mdp_bdd = (char*)malloc(20000);
			db_clear_result(result);
			sprintf(query,"SELECT mdp FROM Compte WHERE nom='%s';", login);
			db_query(handler, query);
			db_getvalue(result, 0, 0, mdp_bdd, 20000);


			if (check_password(mdp, mdp_bdd)==1) strcpy(message_erreur,"Mauvais mot de passe.");   //Vérification mot de passe
			else{

				db_clear_result(result);
				sprintf(query, "SELECT actif FROM Compte WHERE nom='%s';",login);
				db_query(handler, query);
				db_getvalue(result, 0,0,activite,100);

				if (strcmp(activite, "0")==0) strcpy(message_erreur, "Votre compte n'a pas été activé par l'administrateur.") ;
				else{

					db_clear_result(result);
					sprintf(query,"SELECT totpsecret FROM Compte WHERE nom='%s';", login);
					db_query(handler, query);
					
					db_getvalue(result,0,0,totpsecret,20000);

					if (strcmp(totpsecret, "") == 0) succes =1; //==> Absence de TOTP
					else{
						
						//On compare simplement le TOTP
						db_clear_result(result);
						sprintf(query, "SELECT totpcode FROM Compte WHERE nom='%s';", login);
						db_query(handler, query);
						db_getvalue(result,0,0,totpcode,20000);
						if(strcmp(code, totpcode) != 0) strcpy(message_erreur, "Le code n'est pas le bon.");
						else succes = 1;
					}
				}
			}
		}
	}

	printf("Content-type: text/html\n\n\
			<!DOCTYPE html>\n\
			<html>\n\
			\n\
			<head>\n\
			<title>Réseau Social Privé</title>\n\
			<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
			<meta charset=\"utf-8\">\n\
			<link rel=\"manifest\" href=\"manifest.json\" />\n\
			<link rel=\"icon\" type=\"image/x-icon\" href=\"icon.png\">\n\
			<link rel=\"stylesheet\" href=\"/reseausocial/themes/reseausocial.min.css\" />\n\
			<link rel=\"stylesheet\" href=\"/reseausocial/themes/jquery.mobile.icons.min.css\" />\n\
			<link rel=\"stylesheet\" href=\"/jq/jquery.mobile.structure-1.4.5.min.css\" />\n\
			<script src=\"/jq/demos/js/jquery.min.js\"></script>\n\
			<script src=\"/jq/jquery.mobile-1.4.5.min.js\"></script>\n\
			</head>\n");
	if (succes == 0){
		printf("<body data-theme='b'>\n\
			<h1>%s</h1>\n\
			<a href=\"/reseausocial/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Retour à l'accueil</a>\n", message_erreur);
	}
	if (succes == 1){
		//Suivi de l'activiter (online/offline)
		long epoch = time(NULL);
		db_clear_result(result);
		sprintf(query,"UPDATE Compte SET activite='%lu' WHERE nom='%s';", epoch, login);
		db_query(handler, query);

		sprintf(commande, "openssl rand 10 | base32 > /tmp/reseausocial_totp_%s.tmp", login);
		system(commande);

		sprintf(commande, "/tmp/reseausocial_totp_%s.tmp", login);
		fd = open(commande, O_RDONLY);
		nb = read(fd, reponse, 100);
		close(fd);
		reponse[nb] = 0; //On s'assure que la chaîne de caractère se termine bien par un \0
		sprintf(commande, "echo \'otpauth://totp/ReseauSocial (%s)?secret=%s\' | qrencode -t png -o \'/var/www/html/reseausocial/qr/qr%s.png\'",login,reponse,login);
		system(commande);

		db_clear_result(result);
		sprintf(query, "UPDATE Compte SET totpsecret='%s', totpcode='' WHERE nom='%s';", reponse, login);
		db_query(handler, query);
		strcpy(code, "");

		printf("<body data-theme='b'><div data-role=\"page\" data-theme=\"b\">\n\
			Voici votre QR Code à scanner avec Google Aunthenticator : <br>\n\
			<img src='/reseausocial/qr/qr%s.png' alt='QR Code pour Google Aunthenticator' style='display: block; width: 20hw'><br>\n\
			Vous pouvez également entrer la clef manuellement dans l'application : %s\n\
			<form autocomplete='off' action=\"/cgi-bin/reseausocial_principale.cgi\" method=\"POST\"><br>\n\
			<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-entrer\" id=\"txt-entrer\" value=\"n\">\n\
			<button type=\"submit\" data-mini='false' data-inline='false' data-icon=\"back\">Retour à la liste des salons</button>\n\
			</form></div>\n", login, reponse, login, mdp, code); 


	}

	printf("</body>\n</html>\n");

}