#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define __USE_XOPEN
#include <time.h>
#include "commun.c"
#include "mysql.h"
#include "password.h" //Contient le Define du PASSWORD


int main(void){
	//sleep(2);
	char *envoi; //Données reçues
	char *login, *mdp, *query, *message_erreur, *totpsecret, *activite;
	int succes = 0; //succes=0 --> échec // succes=1 --> TOTP Actif // succes=2 --> TOTP Inactif

	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));
	
	login          = (char*)malloc(query_size+20000); //identifiant
	mdp            = (char*)malloc(query_size+20000); //identifiant
	query          = (char*)malloc(20000);            //Requête SQL
	totpsecret     = (char*)malloc(20000);			  //Secret TOTP
	activite       = (char*)malloc(100);              //Compte actif ?
	message_erreur = (char*)malloc(query_size+20000); //Eventuel message d'erreur

	get_chaine(envoi, 1, mdp);
	get_chaine(envoi, 2, login);

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


			if (check_password(mdp, mdp_bdd)==1) strcpy(message_erreur,"Mauvais mot de passe.");           //Vérification mot de passe
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
					else succes = 2; //==> TOTP Actif
				}
			}
		}
	}
	printf("Content-Type: text/html\n\n\
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
			</head>\n"); //login, mdp);
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

		printf("<body data-theme='b'><div data-role=\"page\" data-theme=\"b\">\n\
			<h5>Il serait plus sécurisé d'activer la 2FA ! Vous prenez des risques en continuant de vous authentifier avec un seul facteur</h5>\n\
			<br>\n\
			<form autocomplete='off' action='/cgi-bin/reseausocial_principale.cgi' method='POST'>\n\
			<br>\n\
				<button type=\"submit\" data-mini='false' data-inline='true' data-icon=\"plus\">Continuer</button>\n\
				<input id='text-login' name='text-login' type='hidden' value='%s'>\n\
				<input id='text-mdp' name='text-mdp' type='hidden' value='%s'>\n\
				<input id='text-code' name='text-code' type='hidden' value=''>\n\
				<input id='text-entrer' name='text-entrer' type='hidden' value='y'>\n\
			</form>\n\
			\n ", login, mdp);
	}
	else if (succes==2){

		//Suivi de l'activiter (online/offline)
		long epoch = time(NULL);
		db_clear_result(result);
		
		sprintf(query,"UPDATE Compte SET activite='%lu' WHERE nom='%s';", epoch, login);
		db_query(handler, query);

		printf("<body data-theme='b'><div data-role=\"page\" data-theme=\"b\">\n\
			<form autocomplete='off' method='POST' action='/cgi-bin/reseausocial_principale.cgi'>\n\
				<div data-role='fieldcontain'>\n\
					<input id='text-login' name='text-login' type='hidden' value='%s'>\n\
					<input id='text-mdp' name='text-mdp' type='hidden' value='%s'>\n\
					\n\
					Veuillez saisir le code de deuxième authentifiction.<br>\n\
					<label for='text-code'>Code : </label>\n\
					<input id='text-code' name='text-code' required='required'>\n\
					<input id='text-entrer'  name='text-entrer' type='hidden' value='y'>\n\
					<br><br><br><br>\n\
					<button type=\"submit\" data-mini='false' data-inline='true' data-icon=\"plus\">Continuer</button>\n\
				</div>\n\
			\n\
			</form></div></div>", login, mdp);
	}
	printf("</body></html>\n");

	

}