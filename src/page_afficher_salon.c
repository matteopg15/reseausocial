#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define __USE_XOPEN
#include <time.h>
#include "password.h"



void page_salon(const char *login, const char *mdp, const char *code, const char* salon, const char* gerant){

	char *query;

	int idmaxi, idfirst, nb; //Pour stocker l'id maxi //Pour stocker l'id du premier message à afficher //Pour stocker le nombre de messages à afficher

	query          = (char*)malloc(20000);            //Requête SQL


	//Suivi de l'activiter (online/offline)
		long epoch = time(NULL);
		db_clear_result(result);
		sprintf(query,"UPDATE Compte SET activite='%lu' WHERE nom='%s';", epoch, login);
		db_query(handler, query);


		//MàJ Message non lu
		idmaxi = 0;
		db_clear_result(result);
		sprintf(query, "SELECT * FROM Message WHERE salon='%s' AND gerant='%s';", salon, gerant);
		db_query(handler, query);

		if (db_ntuples(result) > 0){
			char* buf = (char*)malloc(200);
			db_clear_result(result);
			sprintf(query, "SELECT MAX(id) FROM Message WHERE salon='%s' AND gerant='%s';", salon, gerant);
			db_query(handler, query);
			db_getvalue(result, 0, 0, buf, 200);

			idmaxi = atoi(buf);

			//Réglage pour messages non lus (=/= marquer tout lu)
			db_clear_result(result);
			sprintf(query, "UPDATE Abonne SET idlast=%d WHERE salon='%s' AND gerant='%s' AND compte='%s';", idmaxi, salon, gerant, login);
			db_query(handler, query);
		}




			printf("<body data-theme='b'><div data-role=\"page\" data-theme=\"b\">\n\
			<div data-role=\"header\" data-theme=\"b\" data-position=\"fixed\">\n\
				<div data-role=\"navbar\">\n\
					<ul>\n\
						<li>\n\
							<form autocomplete=false action='/cgi-bin/reseausocial_afficher_salon.cgi' method='POST'>\n\
							<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
							<button type='submit' data-mini='false' data-iniline='false' data-theme='b' data-icon=\"refresh\" class='ui-shadow'></button>\n\
							</form>\n\
						</li>\n\
						<li>\n\
							<form autocomplete=false action='/cgi-bin/reseausocial_page_poster.cgi' method='POST'>\n\
							<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
							<button type='submit' data-mini='false' data-iniline='false' data-theme='b' data-icon=\"comment\" class='ui-shadow'></button>\n\
							</form>\n\
						</li>\n\
						<li>\n\
							<form autocomplete=false action='/cgi-bin/reseausocial_liste_membres.cgi' method='POST'>\n\
							<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
							<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
							<button type='submit' data-mini='false' data-iniline='false' data-theme='b' data-icon=\"user\" class='ui-shadow'></button>\n\
							</form>\n\
						</li>\n",login, mdp, code, salon, gerant, login, mdp, code, salon, gerant, login, mdp, code, salon, gerant);
						if (strcmp(gerant, login)!=0 && (strcmp(salon, "Public") != 0 && strcmp(gerant, "Admin") != 0)){
							printf("<li>\n\
								<form autocomplete=false action='/cgi-bin/reseausocial_confirmation_quitter.cgi' method='POST'>\n\
									<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
									<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
									<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
									<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
									<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
									<button type='submit' data-mini='false' data-iniline='false' data-theme='b' data-icon=\"delete\" class='ui-shadow'></button>\n\
								</form>\n\
									</li>\n", login, mdp, code, salon, gerant);}
					printf("</ul>\n\
				</div>\n\
				<h1 class=\"ui-title\" role=\"heading\" aria-level=\"1\"><img src=\"/reseausocial/icon.png\"></h1>\n\
			</div>\n\
			 <div role=\"main\" class=\"ui-content\" data-theme=\"b\">\n\
				<br>\n");
			if (strcmp(login, gerant) == 0){
	
				printf("<form autocomplete=false action='/cgi-bin/reseausocial_gerer_salon.cgi'method='POST'>\n\
					<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
					<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
					<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
					<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
					<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
					<button data-inline=\"true\" data-theme=\"b\" class=\"ui-btn ui-btn-b ui-btn-inline ui-shadow ui-corner-all\" data-icon=\"gear\"\">Gérer le salon</button>\n\
				</form>\n", login, mdp, code, salon, gerant);
			}
			printf("<form autocomplete=false action='/cgi-bin/reseausocial_marquer_tout_lu.cgi' method='POST'>\n\
				<button data-inline=\"true\" data-theme=\"b\" class=\"ui-btn ui-btn-b ui-btn-inline ui-shadow ui-corner-all\" data-icon=\"comment\"\">Marquer tout lu</button>\n\
				<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
				<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
				<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
				<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
				<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
				</form>\n", login , mdp, code, salon, gerant);
	
			//Nombre de messages disponibles (lus+non lus)
			db_clear_result(result);
			sprintf(query, "SELECT * FROM Message WHERE salon='%s' AND gerant='%s';", salon, gerant);
			db_query(handler, query);
			nb = db_ntuples(result);
			if (nb>0){
				//Dernier message disponible
				db_clear_result(result);
				sprintf(query, "SELECT MAX(id) FROM Message WHERE salon='%s' AND gerant='%s' ORDER BY id DESC;", salon, gerant);
				db_query(handler, query);
				char *bufmaxi = (char*)malloc(20000);
				db_getvalue(result, 0, 0, bufmaxi, 20000);
				idmaxi = atoi(bufmaxi);
			}else idmaxi = 0;

			//Premier message non lu par l'util
			db_clear_result(result);
			sprintf(query, "SELECT idfirst FROM Abonne WHERE compte='%s' AND salon='%s' AND gerant='%s';", login, salon, gerant);
			db_query(handler, query);

			char* bufidfirst;
			bufidfirst = (char*)malloc(20000);
			db_getvalue(result,0,0,bufidfirst,20000);
			

			idfirst = atoi(bufidfirst);
			

			if (idfirst>idmaxi){
				idmaxi=0;
			}

			if(idmaxi == 0){
				printf("<br><br><hr>Aucun message à afficher !");
			}


			for (int i=0; i<nb; i++){
				char *buf;
				unsigned int id;
				buf = (char*)malloc(20000); //Variable temporaire
				db_clear_result(result);
				sprintf(query, "SELECT emetteur, horodatage, contenu, lien, image, youtube, dailymotion, autre, id FROM Message WHERE salon='%s' AND gerant='%s' ORDER BY id DESC;", salon, gerant);
				db_query(handler, query);
				db_getvalue(result,i,8,buf,20000);
				id=atoi(buf);
				if (id>=idfirst){
					
					char *emetteur, *url_avatar, *txt, *lien, *img, *yt, *dm, *autre, *horodatage;
					unsigned long epoch_derniere_activite=0;//Epoch de la dernière activité
					
					short presence = 0;
					emetteur       = (char*)malloc(20000); //Login de l'emetteur du message
					horodatage     = (char*)malloc(20000); //Horodatage de l'envoi du message
					url_avatar     = (char*)malloc(20000); //URL de l'avatar de l'util
					txt            = (char*)malloc(20000); //Contenu textuel du message
					lien           = (char*)malloc(20000); //URL dans le message
					img            = (char*)malloc(20000); //Url de l'image 
					yt             = (char*)malloc(20000); //URL embed YT
					dm             = (char*)malloc(20000); //URL embed DM
					autre          = (char*)malloc(20000); //Autre URL à mettre en iframe
						
						
						
					db_getvalue(result,i,0,emetteur,20000);
					db_getvalue(result,i,1,horodatage,20000);
					db_getvalue(result,i,2,txt,20000);
					db_getvalue(result,i,3,lien,20000);
					db_getvalue(result,i,4,img,20000);
					db_getvalue(result,i,5,yt,20000);
					db_getvalue(result,i,6,dm,20000);
					db_getvalue(result,i,7,autre,20000);
					db_getvalue(result,i,8,buf,20000);
					id=atoi(buf);
									
					//Calculer presence+import avatar
					db_clear_result(result);
					sprintf(query, "SELECT activite, avatar FROM Compte WHERE nom='%s';", emetteur);
					db_query(handler, query);	
						
					db_getvalue(result,0,1,url_avatar,20000);
					db_getvalue(result,0,0,buf,20000);		
					epoch_derniere_activite = atoi(buf);
					if (time(NULL)-epoch_derniere_activite <= 300){
									presence=1;
					} 
									
						
					//Afficher l'horodatage
					printf("<br><br><hr>\n\
					<p style=\"font-size: 10pt;\"><b>%s</b> le <u>%s</u></p>\n",emetteur, horodatage);
						
					//Afficher l'avatar
					printf("<img src=\"%s\" alt=\"Avatar de %s\" style=\"width: 10%%\">",url_avatar, emetteur);
						
					//Afficher présence
					if (presence == 1){
						printf("<img src=\"/reseausocial/online.png\" alt=\"Present\" style=\"width: 2%%;\">");
					}
						else{
							printf("<img src=\"/reseausocial/offline.png\" alt=\"Present\" style=\"width: 2%%;\">");
						}
									
					//Afficher message txt
					if (strcmp(txt, "")!=0)	printf("<p>%s<p>", txt);
			
					//Afficher lien
					if (strcmp(lien, "") != 0) {
						printf("<a href='%s'>%s</a>", lien, lien);
						printf("<form autocomplete=false action='/cgi-bin/reseausocial_iframe_lien.cgi' method='POST'>\n\
				               	<button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-icon=\"action\" >Ouvrir dans l'appli</button>\n\
							  	<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-lien\" id=\"txt-lien\" value=\"%s\">\n\
							  	</form><br><br>\n", login, mdp, code, salon, gerant, lien);
					}
								
						
					//Afficher image
					if (strcmp(img, "")!=0) printf("<img src='%s' style='width: 280px;height:157px;'>", img);
								
					//Afficher YT
					if (strcmp(yt, "")!=0) printf("<br><br><iframe width=\"100%%\" height=\"40vh\" src=\"%s\" title=\"YouTube video player\" frameborder=\"0\" allow=\"accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share\" referrerpolicy=\"strict-origin-when-cross-origin\" allowfullscreen style=\"height:40vh; max-width:100%%; object-fit: contain;\"></iframe>", yt);
						
					//Afficher DM
					if (strcmp(dm, "") !=0) printf("<br><div style=\"position:relative;padding-bottom:157px;height:2px;overflow:hidden;\"> <iframe style=\"height:40vh; max-width:100%%; object-fit: contain;\" frameborder=\"0\" type=\"text/html\" src=\"%s\" width=\"100%%\" height=\"100%%\" allowfullscreen title=\"Dailymotion Video Player\" allow=\"autoplay; web-share\"> </iframe> </div>", dm);
									
					//Afficher autre
					if (strcmp(autre, "")!=0) printf("<iframe style=\"width:100%%; height:40vh; position:relative;\" src=\"%s\"></iframe>", autre);

					if(strcmp(emetteur, login)==0){
						printf("<form autocomplete=false action='/cgi-bin/reseausocial_supprimer_message.cgi' method='POST'>\n\
				               	<button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-icon=\"delete\" >Supprimer</button>\n\
							  	<input type=\"hidden\" name=\"txt-login\" id=\"txt-login\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-mdp\" id=\"txt-mdp\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-salon\" id=\"txt-salon\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-gerant\" id=\"txt-gerant\" value=\"%s\">\n\
								<input type=\"hidden\" name=\"txt-id\" id=\"txt-id\" value=\"%d\">\n\
							  	</form>\n", login, mdp, code, salon, gerant, id);
					}
				}
			}
		printf("</div><div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\" role=\"contentinfo\" class=\"ui-footer ui-bar-b ui-footer-fixed slideup\">\n\
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
		printf("</body></html>");
}