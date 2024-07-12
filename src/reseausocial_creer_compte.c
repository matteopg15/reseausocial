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
	char *envoi; //Données reçues
	char *login, *mdp, *query, *message_erreur, *mdp2, *genre;
	int succes = 0; //succes=0 --> échec // succes=1 --> TOTP Actif // succes=2 --> TOTP Inactif

	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));
	
	login          = (char*)malloc(query_size+20000); //identifiant
	mdp            = (char*)malloc(query_size+20000); //identifiant
	mdp2           = (char*)malloc(query_size+20000); //Confirmation mdp
	genre          = (char*)malloc(query_size+20000); //Genre de l'utilisateur
	query          = (char*)malloc(20000);            //Requête SQL
	message_erreur = (char*)malloc(query_size+20000); //Eventuel message d'erreur


	get_chaine(envoi, 1, mdp2);
	get_chaine(envoi, 2, mdp);
	get_chaine(envoi, 3, genre);
	get_chaine(envoi, 4, login);

	tamb(genre);
	tamb(mdp);
	tamb(mdp2);
	tamb(login);




	if ((handler=db_opendatabase("reseausocial", "localhost", "reseausocial", PASSWORD))==NULL){    //Test BDD
		strcpy(message_erreur, "Impossible d'accéder à la base de données.");
	}
	else{

		sprintf(query, "SELECT * FROM Compte WHERE nom='%s';", login);
		db_query(handler, query);

		if (db_ntuples(result)!= 0) strcpy(message_erreur,"Cet utilisateur existe déjà.");        //Test utilisateur existe
		else{
			if(strcmp(mdp, mdp2)!= 0) strcpy(message_erreur, "Les mots de passe ne correspondent pas.");
			else{
				if(strcmp(genre, "homme")!=0 && strcmp(genre, "femme")!=0) strcpy(message_erreur, "Impossible d'interpréter le genre renseigné.");
				else{
					hash256(mdp);
					db_clear_result(result);
					sprintf(query, "INSERT INTO Compte VALUES ('%s', '%s', '/reseausocial/%s.png', '%s', false, '', '', '0');", login, mdp, genre, genre);
					db_query(handler, query);

					db_clear_result(result);
					sprintf(query, "INSERT INTO Abonne VALUES ('%s', 'Public', 'Admin', 1,0);", login);
					db_query(handler, query);
					
					db_clear_result(result);
					sprintf(query, "SELECT * FROM Compte WHERE nom='%s';", login);
					db_query(handler, query);

					

					if (db_ntuples(result) == 0) strcpy(message_erreur, "Nous avons rencontré une erreur lors de la création de votre compte");
					else succes = 1;
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
			<a href=\"/reseausocial/creer_compte.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Retour à l'accueil</a>\n", message_erreur);
	}else if( succes == 1 ){
		//Suivi de l'activiter (online/offline)
		long epoch = time(NULL);
		db_clear_result(result);
		sprintf(query,"UPDATE Compte SET activite='%lu' WHERE nom='%s';", epoch, login);
		db_query(handler, query);
		
		printf("<body data-theme='b'><div data-role=\"page\" data-theme=\"b\">Votre compte a été créé avec succès. Il faut encore que l'administrateur l'active pour que vous puissiez vous connecter.\n\
			<br><a href='/reseausocial/index.html'>Retourner à la page d'accueil</a></div>\n");
	}
	printf("</body></html>\n");


}