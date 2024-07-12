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
#include "totp.c"

int main(void){
	char *envoi; //Données reçues
	char *login, *mdp, *query, *message_erreur, *totpsecret, *activite, *code, *entree, *totpcode, *commande;
	int succes = 0; //succes=0 --> échec // succes=1 --> Verifications OK
	int nvmessage = 0; //nvmessage=0 --> Pas de notif à jouer //nvmessage=1 --> Notif à jouer

	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));
	
	login          = (char*)malloc(query_size+20000); //identifiant
	mdp            = (char*)malloc(query_size+20000); //identifiant
	query          = (char*)malloc(20000);            //Requête SQL
	totpsecret     = (char*)malloc(20000);			  //Secret TOTP
	totpcode       = (char*)malloc(20000);            //Code TOTP (BDD)
	activite       = (char*)malloc(100);              //Compte actif ?
	code           = (char*)malloc(query_size+20000); //Code TOTP fourni par l'utilisateur
	entree         = (char*)malloc(query_size+20000); //Origine de l'utilisateur
	message_erreur = (char*)malloc(query_size+20000); //Eventuel message d'erreur
	commande       = (char*)malloc(20000);            //Commande suppression QRCode


	get_chaine(envoi, 1, entree);
	get_chaine(envoi, 2, code);
	get_chaine(envoi, 3, mdp);

	get_chaine(envoi, 4, login);

	tamb(entree);
	tamb(code);
	tamb(mdp);
	tamb(login);

	//Nettoyage QRCodes 2FA
	strcpy(commande, "rm /var/www/html/reseausocial/qr/*");
	system(commande);
	

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


			if (check_password(mdp, mdp_bdd)==1) strcpy(message_erreur,"Mauvais mot de passe.");     //Vérification mot de passe
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
					else {
						if (strcmp(entree, "y") == 0){
							//On calcule le TOTP
							if(checkcode(totpsecret, code)!=0) {strcpy(message_erreur, "Le code n'est pas bon.");}
							else{

								succes = 1;
								db_clear_result(result);
								sprintf(query, "UPDATE Compte SET totpcode='%s'WHERE nom='%s' ;", code, login);
								db_query(handler, query);

							}

						}
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
	else{

		//Suivi de l'activiter (online/offline)
		long epoch = time(NULL);
		db_clear_result(result);
		sprintf(query,"UPDATE Compte SET activite='%lu' WHERE nom='%s';", epoch, login);
		db_query(handler, query);

			printf("<body>\n\
		<div data-role=\"page\" data-theme=\"b\">\n\
			<div data-role=\"header\" data-theme=\"b\" data-position=\"fixed\">\n\
				<div data-role=\"navbar\">\n\
					<ul>\n\
						<li>\n\
							<form autocomplete=false action='/cgi-bin/reseausocial_saisir_nom.cgi' method='POST'>\n\
							<button type='submit' data-mini='false' data-iniline='false' data-theme='b' data-icon=\"plus\" class='ui-shadow'></button>\n\
							<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
							</form>\n\
						</li>\n\
						<li>\n\
							<form autocomplete=false action='/cgi-bin/reseausocial_principale.cgi' method='POST'>\n\
							<button type='submit' data-mini='false' data-iniline='false' data-theme='b' data-icon=\"refresh\" class='ui-shadow'></button>\n\
							<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-entrer\" id=\"txt-entrer\" value='n'>\n\
							</form>\n\
						</li>\n\
						<li>\n\
							<form autocomplete=false action='/cgi-bin/reseausocial_profil.cgi' method='POST'>\n\
							<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
							<button type='submit' data-mini='false' data-iniline='false' data-theme='b' data-icon=\"user\" class='ui-shadow'></button>\n\
							</form>\n\
						</li>\n\
					</ul>\n\
				</div>\n\
				<h1 class=\"ui-title\" role=\"heading\" aria-level=\"1\"><img src=\"/reseausocial/icon.png\"></h1>\n\
			</div>\n", login, mdp, code, login, mdp, code, login, mdp, code);

			if (strcmp(login, "Admin")==0){
				printf("<form autocomplete=false action='/cgi-bin/reseausocial_reglage_admin.cgi' method='POST'>\n\
			<button data-inline=\"true\" data-theme=\"b\" class=\"ui-btn ui-btn-b ui-btn-inline ui-shadow ui-corner-all\" data-icon=\"gear\"\">Réglages</button>\n\
			<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
			</form>\n", login, mdp, code);

			}
			

			printf("<br><br><hr><audio id='audio'><source src='/reseausocial/notif.mp3' type='audio/mpeg'></audio>\n\
			<h3 style=\"text-align: center;\">Mes salons</h3>\n\
			<hr>\n\
			<!LISTE DES SALONS QUI M'APPARTIENNENT>\n");
	db_clear_result(result);
	sprintf(query,"SELECT salon, gerant FROM Abonne WHERE gerant='%s' AND compte='%s';", login, login);
	db_query(handler, query);
	int nb_resultats = db_ntuples(result);

	for (int i = 0; i < nb_resultats; ++i){

		db_clear_result(result);
		sprintf(query,"SELECT salon, gerant FROM Abonne WHERE gerant='%s' AND compte='%s';", login, login);
		db_query(handler, query);
		char *salon, *gerant, *buf;
		int idlast;
		int nb_non_lus = 0;
		salon = (char*)malloc(20000);
		gerant= (char*)malloc(20000);
		buf   = (char*)malloc(20000);

		//Récupération données
		db_getvalue(result,i,0,salon,20000);
		db_getvalue(result,i,1,gerant,20000);


		//Calcul nb messages non lus
		db_clear_result(result);
		sprintf(query,"SELECT id FROM Message WHERE gerant='%s' AND salon='%s';", gerant, salon);
		db_query(handler, query);

		if(db_ntuples(result) != 0){

			db_clear_result(result);
			sprintf(query,"SELECT idlast FROM Abonne WHERE compte='%s' AND gerant='%s' AND salon='%s';", login, gerant, salon);
			db_query(handler, query);
			db_getvalue(result,0,0,buf,20000);
			idlast = atoi(buf);

			db_clear_result(result);
			sprintf(query,"SELECT * FROM Message WHERE gerant='%s' AND salon='%s' AND id>%d;", gerant, salon, idlast);
			db_query(handler, query);

			nb_non_lus = db_ntuples(result);
			if (nb_non_lus>0){
				nvmessage = 1;
			}



		}


		if (strcmp(salon, "Public") == 0 && strcmp(gerant, "Admin")==0){
				printf("<form method='POST' autocomplete='off' action=\"/cgi-bin/reseausocial_afficher_salon.cgi\" method='POST'>\n\
						<button type='submit' data-mini='false' data-iniline='true' data-theme='b' data-icon=\"comment\" class='ui-shadow'>%s <span class=\"ui-li-count ui-body-inherit\">%d</span></button>\n\
						<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
					</form>\n", salon, nb_non_lus, login, mdp, code, salon, gerant);
		}else {
			printf("<form method='POST' autocomplete='off' action=\"/cgi-bin/reseausocial_afficher_salon.cgi\" method='POST'>\n\
						<button type='submit' data-mini='false' data-iniline='true' data-theme='b' data-icon=\"comment\" class='ui-shadow'>%s [%s]<span class=\"ui-li-count ui-body-inherit\">%d</span></button>\n\
						<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
					</form>\n", salon, gerant,nb_non_lus, login, mdp, code, salon, gerant);
		}


	}
				
	

		printf("<br><br><hr>\n\
			<h3 style=\"text-align: center;\">Les salons dont je suis membre</h3>\n\
			<hr>\n");

		db_clear_result(result);
		sprintf(query,"SELECT salon, gerant FROM Abonne WHERE gerant!='%s' AND compte='%s';", login, login);
		db_query(handler, query);
		nb_resultats = db_ntuples(result);

		for (int i = 0; i < nb_resultats; ++i){
		db_clear_result(result);
		sprintf(query,"SELECT salon, gerant FROM Abonne WHERE gerant!='%s' AND compte='%s';", login, login);
		db_query(handler, query);
		char *salon, *gerant, *buf;
		int idlast;
		int nb_non_lus = 0;

		salon = (char*)malloc(20000);
		gerant= (char*)malloc(20000);
		buf   = (char*)malloc(20000);

		db_getvalue(result,i,0,salon,20000);
		db_getvalue(result,i,1,gerant,20000);


		//Calcul nb messages non lus
		db_clear_result(result);
		sprintf(query,"SELECT id FROM Message WHERE gerant='%s' AND salon='%s';", gerant, salon);
		db_query(handler, query);

		if(db_ntuples(result) != 0){
			//db_clear_result(result);
			//sprintf(query,"SELECT MAX(id) FROM Message WHERE gerant='%s' AND salon='%s';", gerant, salon);
			//db_query(handler, query);

			//db_getvalue(result,0,0,buf,20000);
			//idmax = atoi(buf);

			db_clear_result(result);
			sprintf(query,"SELECT idlast FROM Abonne WHERE compte='%s' AND gerant='%s' AND salon='%s';", login, gerant, salon);
			db_query(handler, query);
			db_getvalue(result,0,0,buf,20000);
			idlast = atoi(buf);

			db_clear_result(result);
			sprintf(query,"SELECT * FROM Message WHERE gerant='%s' AND salon='%s' AND id>%d;", gerant, salon, idlast);
			db_query(handler, query);

			nb_non_lus = db_ntuples(result);
			if (nb_non_lus>0){
				nvmessage = 1;
			}

		}


		if (nvmessage>0 ){
			printf("<script>\n\
					var x = document.getElementById('audio');\n\
					x.play();\n\
				</script>");
		}
		if (strcmp(salon, "Public") == 0 && strcmp(gerant, "Admin")==0){
				printf("<form method='POST' autocomplete='off' action=\"/cgi-bin/reseausocial_afficher_salon.cgi\" method='POST'>\n\
						<button type='submit' data-mini='false' data-iniline='true' data-theme='b' data-icon=\"comment\" class='ui-shadow'>%s <span class=\"ui-li-count ui-body-inherit\">%d</span></button>\n\
						<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
					</form>\n", salon,nb_non_lus, login, mdp, code, salon, gerant);
		}else {
			printf("<form method='POST' autocomplete='off' action=\"/cgi-bin/reseausocial_afficher_salon.cgi\" method='POST'>\n\
						<button type='submit' data-mini='false' data-iniline='true' data-theme='b' data-icon=\"comment\" class='ui-shadow'>%s [%s]<span class=\"ui-li-count ui-body-inherit\">%d</span></button>\n\
						<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
						<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
					</form>\n", salon, gerant,nb_non_lus, login, mdp, code, salon, gerant);
		}

	}
			printf("<div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\" role=\"contentinfo\" class=\"ui-footer ui-bar-b ui-footer-fixed slideup\">\n\
			<div data-role=\"navbar\" class=\"ui-navbar\" role=\"navigation\">\n\
			<ul class=\"ui-grid-c\">\n\
               <li class=\"ui-block-a\">\n\
               	  	<form autocomplete=false action='/reseausocial/index.html' method='GET'>\n\
                  	<button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-icon=\"power\" class=\"ui-shadow ui-btn ui-btn-b ui-icon-power ui-btn-icon-top\"></button>\n\
				  </form>\n\
				</li>\n\
			</ul>\n\
			</div></div>");

	}
	printf("</div></body>\n</html>\n");



}