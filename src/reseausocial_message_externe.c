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
	char *emetteur, *destinataire, *query, *message_erreur, *text, *lien, *img, *yt, *dm, *autre ;
	int succes = 0; //succes=0 --> échec // succes=1 --> Verifications OK
	int idmaxi = 1; //Pour stocker le nombre de messages //Pour stocker l'id du premier message à afficher

	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));

	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));
	
	query           = (char*)malloc(query_size+20000); //Requêtes pour la BDD
	message_erreur  = (char*)malloc(query_size+20000); //Mesages d'erreurs pour l'utilisateur

	emetteur        = (char*)malloc(query_size+20000); //Nom de l'émetteur du message
	destinataire    = (char*)malloc(query_size+20000); //Nom du destinataire du message
	text            = (char*)malloc(query_size+20000); //Contenu textuel du message
	lien            = (char*)malloc(query_size+20000); //Lien dans le message
	img             = (char*)malloc(query_size+20000); //Image dans le message
	yt              = (char*)malloc(query_size+20000); //Embed YouTube dans le message
	dm              = (char*)malloc(query_size+20000); //Embed DayliMotion dans le message
	autre           = (char*)malloc(query_size+20000); //Autre IFrame

	get_chaine(envoi, 8, emetteur);
	get_chaine(envoi, 7, destinataire);
	get_chaine(envoi, 6, text);
	get_chaine(envoi, 5, lien);
	get_chaine(envoi, 4, img);
	get_chaine(envoi, 3, yt);
	get_chaine(envoi, 2, dm);
	get_chaine(envoi, 1, autre);
	
	tamb(emetteur);
	tamb(destinataire);
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

		sprintf(query, "SELECT * FROM Compte WHERE nom='%s';", destinataire);
		db_query(handler, query);

		if (db_ntuples(result)!= 1) strcpy(message_erreur,"Le destinataire n'existe pas.");        //Test utilisateur existe
		else succes=1;
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

	if (succes==0){
		printf("<body data-theme='b'>\n\
			<h1>%s</h1>\n\
			<a href=\"/reseausocial/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Retour à l'accueil</a>\n", message_erreur);
	}

	else if (succes==1){
		db_clear_result(result);
		sprintf(query, "SELECT * FROM Salon WHERE nom='Mailbox' AND gerant='%s';", destinataire);
		db_query(handler, query);

		if(db_ntuples(result) == 0){
			db_clear_result(result);
			sprintf(query, "INSERT INTO Salon VALUES ('Mailbox', '%s');", destinataire);
			db_query(handler, query);

			db_clear_result(result);
			sprintf(query, "INSERT INTO Abonne VALUES ('%s', 'Mailbox','%s', 1, 0);", destinataire, destinataire);
			db_query(handler, query);

		}

		db_clear_result(result);
		sprintf(query, "SELECT * FROM Message WHERE salon='Mailbox' AND gerant='%s';", destinataire);
		db_query(handler, query);
		if(db_ntuples(result)!=0){
			db_clear_result(result);
			sprintf(query, "SELECT MAX(id) FROM Message WHERE salon='Mailbox' AND gerant='%s';", destinataire);
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

		char* text1;
		text1 = (char*)malloc(query_size+20000);
		sprintf(text1, "<u>Message de <b>%s</b> :<br></u><br>%s", emetteur, text);

		db_clear_result(result);
		sprintf(query, "INSERT INTO Message VALUES (\"%s\", \"Mailbox\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%d\");", destinataire, destinataire, text1, lien, img, yt, dm, autre, horodatage, idmaxi+1);
		db_query(handler, query);

		printf("<body data-theme='b'><div data-role=\"page\" data-theme=\"b\">\n\
			Le message a été envoyé avec succès !\n\
			<a href=\"/reseausocial/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Retour à l'accueil</a>\n\
			");






	}
	printf("</div>\n\
		</body>\n</html>\n");


}