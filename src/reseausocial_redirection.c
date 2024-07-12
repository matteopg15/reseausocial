#include <stdio.h>
#include "commun.c"
int main(void){
	char *envoi; //Données reçues
	envoi = read_POST();
	query_size=atoi(getenv("CONTENT_LENGTH"));

	char* lien = (char*)malloc(query_size+20000);

	get_chaine(envoi, 1, lien);
	tamb(lien);
	printf("Location: http://%s/reseausocial/envoyer_message.html\n\n", lien);

}