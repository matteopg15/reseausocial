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
	char *login, *mdp, *query, *message_erreur, *totpsecret, *activite, *code, *totpcode, *url_avatar;
	int succes = 0; //succes=0 --> échec // succes=1 --> Verifications OK

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
	url_avatar     = (char*)malloc(20000);            //URL de l'avatar de l'util


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
	else if (succes == 1){
		//Suivi de l'activiter (online/offline)
		long epoch = time(NULL);
		db_clear_result(result);
		sprintf(query,"UPDATE Compte SET activite='%lu' WHERE nom='%s';", epoch, login);
		db_query(handler, query);

		db_clear_result(result);
		sprintf(query, "SELECT avatar FROM Compte WHERE nom='%s';", login);
		db_query(handler, query);
		db_getvalue(result,0,0,url_avatar,20000);
		printf("<body data-theme='b'>\n\
			<div data-role=\"page\" data-theme=\"b\">\n\
			<div data-role=\"header\" data-theme=\"b\" data-position=\"fixed\">\n\
				<h1 class=\"ui-title\" role=\"heading\" aria-level=\"1\"><img src=\"/reseausocial/icon.png\"></h1>\n\
			</div>\n\
	<img src=\"%s\" align=\"center\" style='display: block; margin-left: auto; margin-right: auto; margin-top: 10px; margin-bottom: 10px; width: 20%%'>\n\
	\n\
	<form autocomplete='off' action=\"/cgi-bin/reseausocial_appliquer_avatar.cgi\" method=\"POST\">\n\
	<div data-role='fieldcontain'>\n\
		<label for=''>URL de l'avatar : </label>\n\
		<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
		<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
		<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
		<input type=\"text\" name=\"text-url\" id=\"text-url\", value='%s'>\n\
	</div>\n\
	<button type=\"submit\" data-mini='false' data-inline='false' data-icon=\"user\">Appliquer l'avatar</button>\n\
	</form>\n\
	<br><hr>\n\
	<form autocomplete='off' action=\"/cgi-bin/reseausocial_changer_mdp.cgi\" method=\"POST\">\n\
	<div data-role='fieldcontain'>\n\
		<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
		<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
		<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
		\n\
		<label for=''>Mot de passe actuel : </label>\n\
		<input type=\"password\" name=\"text-ancien-mdp\" id=\"text-ancien-mdp\">\n\
		<label for=''>Nouveau mot de passe : </label>\n\
		<input type=\"password\" name=\"text-nv-mdp\" id=\"text-nv-mdp\">\n\
		<label for=''>Confirmer le mot de passe actuel : </label>\n\
		<input type=\"password\" name=\"text-nv-mdp2\" id=\"text-nv-mdp2\">\n\
	</div>\n\
	<button type=\"submit\" data-mini='false' data-inline='false' data-icon=\"lock\">Changer le mot de passe</button>\n\
	</form><br><br><hr>\n", url_avatar, login, mdp, code, url_avatar, login, mdp, code); ///truc à faire
	if (strcmp(totpsecret, "")==0){
		printf("<form autocomplete='off' action=\"/cgi-bin/reseausocial_activer_2FA.cgi\" method=\"POST\">\n\
			<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
			<button type=\"submit\" data-mini='false' data-inline='true' data-icon=\"lock\">Activer la 2FA</button>\n\
			</form>\n", login, mdp, code);
	}else{
		printf("<form autocomplete='off' action=\"/cgi-bin/reseausocial_desactiver_2FA.cgi\" method=\"POST\">\n\
			<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
			<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
			<button type=\"submit\" data-mini='false' data-inline='true' data-icon=\"lock\">Désactiver la 2FA</button>\n\
			</form>", login, mdp, code);
	}

	printf("<div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\" role=\"contentinfo\" class=\"ui-footer ui-bar-b ui-footer-fixed slideup\">\n\
			<div data-role=\"navbar\" class=\"ui-navbar\" role=\"navigation\">\n\
			<ul class=\"ui-grid-c\">\n\
               <li class=\"ui-block-a\">\n\
               	  	<form autocomplete=false action='/cgi-bin/reseausocial_principale.cgi' method='POST'>\n\
                  	<button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-icon=\"back\" class=\"ui-shadow ui-btn ui-btn-b ui-icon-back ui-btn-icon-top\"></button>\n\
				  	<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
					<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
					<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
					<input type=\"hidden\" name=\"txt-entree\" id=\"txt-entree\" value=\"n\">\n\
				  </form>\n\
				</li>\n\
			</ul>\n\
			</div></div>\n", login, mdp, code);



	}
	printf("</div></body>\n</html>\n");

}