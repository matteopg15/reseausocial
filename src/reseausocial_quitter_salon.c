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
	char *login, *mdp, *query, *message_erreur, *totpsecret, *activite, *code, *totpcode, *salon, *gerant ;
	int succes = 0; //succes=0 --> échec // succes=1 --> Verifications OK
	int idmaxi;

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
	salon          = (char*)malloc(query_size+20000); //Salon de l'utilisateur
	gerant         = (char*)malloc(query_size+20000); //Gerant du salon

	get_chaine(envoi, 1, gerant);
	get_chaine(envoi, 2, salon);
	get_chaine(envoi, 3, code);
	get_chaine(envoi, 4, mdp);
	get_chaine(envoi, 5, login);

	tamb(gerant);
	tamb(salon);
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
				sprintf(query, "SELECT * FROM Abonne WHERE compte='%s' AND salon='%s' AND gerant='%s';",login, salon, gerant);
				db_query(handler, query);
				


				if(db_ntuples(result)==0) strcpy(message_erreur, "Vous n'êtes pas membre de ce salon.");
				else{
					
					db_clear_result(result);
					sprintf(query, "SELECT actif FROM Compte WHERE nom='%s' ;",login);
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
		
		 //Quiter salon
		db_clear_result(result);
		sprintf(query, "DELETE FROM Abonne WHERE compte='%s' AND salon='%s' AND gerant='%s';", login, salon, gerant);
		db_query(handler, query);

			db_clear_result(result);
			sprintf(query, "SELECT * FROM Message WHERE salon='%s' AND gerant='%s';", salon, gerant);
			db_query(handler, query);
			idmaxi=0;
			if (db_ntuples(result)){

			//Prévenir le groupe
			db_clear_result(result);
			sprintf(query, "SELECT MAX(id) FROM Message WHERE salon='%s' AND gerant='%s';", salon, gerant);
			db_query(handler, query);

			char* buf;
			buf = (char*)malloc(20000);
			db_getvalue(result, 0, 0, buf, 20000);
			idmaxi = atoi(buf);
		}

		char* horodatage;
		horodatage = (char*)malloc(20000);
		time_t t = time(NULL); //Get Epoch
		struct tm * tm = localtime(&t); //LocalTime from Epoch
		strftime(horodatage, 20000, "%a %d %b %Y %X", tm);

		db_clear_result(result);
		sprintf(query, "INSERT INTO Message VALUES ('Admin', '%s', '%s', '%s a quitté le salon', '', '', '', '', '', '%s', %d);", salon, gerant, login, horodatage, idmaxi+1);
		db_query(handler, query);

		printf("<body data-theme='b'> <div data-role=\"page\" data-theme=\"b\">Vous avez bien quitter le salon\n\
			<form autocomplete='off' action='/cgi-bin/reseausocial_principale.cgi' method='POST'>\n\
				<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
				<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
				<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
				<input type=\"hidden\" name=\"txt-entrer\" id=\"txt-entrer\" value=\"n\">\n\
				<button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-icon=\"back\" class=\"ui-shadow ui-btn ui-btn-b ui-icon-back ui-btn-icon-top\">Retour à la liste des salons</button>\n\
			</div></form>\n", login, mdp, code);
	}


	printf("</body>\n</html>\n");

}