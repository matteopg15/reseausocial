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
#include "page_afficher_salon.c"

int main(void){
	char *envoi; //Données reçues
	char *login, *mdp, *query, *message_erreur, *totpsecret, *activite, *code, *totpcode, *salon, *gerant ;
	char *text, *lien, *img, *yt, *dm, *autre;
	int succes = 0; //succes=0 --> échec // succes=1 --> Verifications OK
	int idmaxi; //Pour stocker le nombre de messages //Pour stocker l'id du premier message à afficher

	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));

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

	get_chaine(envoi, 7, gerant);
	get_chaine(envoi, 8, salon);
	get_chaine(envoi, 9, code);
	get_chaine(envoi, 10, mdp);
	get_chaine(envoi, 11, login);
	tamb(gerant);
	tamb(salon);
	tamb(code);
	tamb(mdp);
	tamb(login);

	text    = (char*)malloc(query_size+20000); //Contenu textuel du message
	lien    = (char*)malloc(query_size+20000); //Lien dans le message
	img     = (char*)malloc(query_size+20000); //Image dans le message
	yt      = (char*)malloc(query_size+20000); //Embed YouTube dans le message
	dm      = (char*)malloc(query_size+20000); //Embed DayliMotion dans le message
	autre   = (char*)malloc(query_size+20000); //Autre IFrame

	get_chaine(envoi, 6, text);
	get_chaine(envoi, 5, lien);
	get_chaine(envoi, 4, img);
	get_chaine(envoi, 3, yt);
	get_chaine(envoi, 2, dm);
	get_chaine(envoi, 1, autre);
	
	tamb(text);
	tamb(lien);
	tamb(img);
	tamb(yt);
	tamb(dm);
	tamb(autre);

	if (lien_yt(yt) != NULL){
		strcpy(yt, lien_yt(yt));
	}

	if(lien_dm(dm) != NULL){
		strcpy(dm, lien_dm(dm));
	}

	
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
							sprintf(query, "SELECT totpcode FROM Compte WHERE nom='%s'", login);
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
		
		idmaxi=1;
		db_clear_result(result);
		sprintf(query, "SELECT * FROM Message WHERE salon='%s' AND gerant='%s';", salon, gerant);
		db_query(handler, query);

		if(db_ntuples(result)!=0){
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
		sprintf(query, "INSERT INTO Message VALUES (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%d\");", login, salon, gerant, text, lien, img, yt, dm, autre, horodatage, idmaxi+1);
		db_query(handler, query);


		page_salon(login, mdp, code, salon, gerant);
	}
	printf("</body>\n</html>\n");
}


	