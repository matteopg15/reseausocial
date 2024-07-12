#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define __USE_XOPEN
#include <time.h>
#include "mysql.c"
#include "sha256.c"

//Prototype des fonctions
void debug(char* chaine);
void debugf(char *chaine);
char* lien_yt(const char* lien);
char* lien_dm(const char* lien);
void prepare_contenu(char *chaine);
void url_decode(char* entree);
char* read_POST(void); 
void tamb(char *avatar);
int get_chaine(char *chaine,int pos,char *resultat);

int query_size;

/**
 * @brief Permet d'afficher la chaine dans la page HTML. Debug fonctionne dans tous les cas (hors <script>)
 * @param (char*) chaine - Chaîne à écrire sur la page HTML
*/
void debug(char* chaine)
{
printf("Content-type: text/html\n\n\
	<html><body>-%s-</body></html>\n",chaine);
exit(0);
}


/**
 * @brief Envoie le contenu de chaine dans un fichier /tmp/res
 * @param (char*) chaine - Chaîne à écrire sur la page HTML
*/
void debugf(char *chaine)
{
int fd;

fd=open("/tmp/res",O_CREAT|O_TRUNC|O_WRONLY,0664);
write(fd,chaine,strlen(chaine));
close(fd);
//exit(0);
}
/**
 * @brief Convertie un lien de type youtube.com/watch et youtu.be en lien youtube.com/embed à des fins d'intégration dans la page HTML. Renvoie NULL en cas d'échec de la conversion
 * @param (const char*) lien - lien à convertir
*/
char* lien_yt(const char* lien){
	char *lien_embed, *id;
	
	lien_embed    = (char*)malloc(20000);
	id            = (char*)malloc(20000);

	int len_lien = strlen(lien);
	if (strstr(lien, "youtu.be")!=NULL){
		int start = 0;
		for (int i = 17; i<len_lien; i++){
			if (lien[i] == '?' || lien[i] == '/') break;
			id[start] = lien[i];
			start++;
		}
		

	}else if (strstr(lien, "youtube.com")!=NULL){

		int start = -1;
		for(int i = 0; i<len_lien; i++){
			if (start > -1){
				//On est dans l'id du lien
				id[start] = lien[i];
				start++;
			}else if (lien[i] == '='){
				//On commence l'id du lien
				start = 0;
			}
		}
	}else if(strstr(lien, "https://www.youtube.com/embed/")!=NULL){
		strcpy(lien_embed, lien);
		return lien_embed;
	}else return NULL;

	sprintf(lien_embed, "https://www.youtube.com/embed/%s/", id);
	return lien_embed;
}
/**
 * @brief Convertie un lien de type dailymotion.com/video et dai.ly en lien dailymotion.com/embed/video/ à des fins d'intégration dans la page HTML. Renvoie NULL en cas d'échec de la conversion
 * @param (const char*) lien - lien à convertir
*/
char* lien_dm(const char* lien){
	char *lien_embed, *id;
	
	lien_embed    = (char*)malloc(20000);
	id            = (char*)malloc(20000);

	int len_lien = strlen(lien);
	if (strstr(lien, "dai.ly")!=NULL){
		int start = 0;
		for (int i = 15; i<len_lien; i++){
			if (lien[i] == '?' || lien[i] == '/') break;
			id[start] = lien[i];
			start++;
		sprintf(lien_embed, "https://www.dailymotion.com/embed/video/%s", id);
	}
		
	}
	else if (strstr(lien, "www.dailymotion.com")!=NULL){

		int start = 0;
		for (int i = 34; i<len_lien; i++){
			if (lien[i] == '?' || lien[i] == '/') break;
			id[start] = lien[i];
			start++;
		sprintf(lien_embed, "https://www.dailymotion.com/embed/video/%s", id);
		}

	}
	else if(strstr(lien, "https://www.dailymotion.com/embed/video/")!=NULL){
		strcpy(lien_embed, lien);
		return lien_embed;
	}
	else return NULL;
	return lien_embed;
}

/**
 * @brief Prépare le contenu de la requête POST avant sa conversion en UTF-8 en échappant les potentielles injections SQL. Cette fonction modifie directement la variable d'entrée
 * @param (char*) chaine - chaine à préparer
*/
void prepare_contenu(char *chaine)
{
int i,start;
char *res;

res=(char *)malloc(strlen(chaine)*2);
i=start=0;
do
	{
	if(chaine[i]=='\'' || chaine[i]=='\"')
		{
		res[start]='\\';
		start++;
		}
	res[start]=chaine[i];
	start++;
	i++;
	}
while(i<=strlen(chaine));
strcpy(chaine,res);
free(res);
}

/**
 * @brief Convertie une chaine renvoyée par la requête POST en chaîne UTF-8 intelligible pour l'utilisateur et le programme de traitement. Elle échappe les caractère '<' afin d'éviter le XSS. Cette fonction modifie directement la variable d'entrée
 * @param (char*) chaine - chaine à décoder
*/
void url_decode(char* entree){
	char *chaine_originale, *chaine_sortie;
	int len, curseur_original, curseur_sortie;
	chaine_sortie    = (char*)malloc(strlen(entree)+1000);
	chaine_originale = (char*)malloc(strlen(entree)+1000);


	strcpy(chaine_originale, entree);

	strcpy(chaine_sortie, "");


	len = strlen(chaine_originale);
	curseur_original = 0;
	curseur_sortie   = 0;



	while (curseur_original<len){
		if (chaine_originale[curseur_original] == '%'){
			//Type : %AB --> 3 caracs pour 1 en sortie

			//Cas particulier des "<" 3 caracs pour 4 en sortie
			if (chaine_originale[curseur_original+1] == '3' && chaine_originale[curseur_original+2] == 'C'){
					chaine_sortie[curseur_sortie] = '&';
					chaine_sortie[curseur_sortie+1] = 'l';
					chaine_sortie[curseur_sortie+2] = 't';
					chaine_sortie[curseur_sortie+3] = ';';

					curseur_sortie   += 4;
					curseur_original += 3;

			}
			//Cas particulier des retours à la ligne %OD%0A -> <br> (6 en entrée et 4 en sortie)
			//0D = Fin de ligne // 0A = Retour chariot
			if (chaine_originale[curseur_original+1] == '0' && chaine_originale[curseur_original+2] == 'D' && chaine_originale[curseur_original+4] == '0' && chaine_originale[curseur_original+5] == 'A'){
				chaine_sortie[curseur_sortie]='<';
				chaine_sortie[curseur_sortie+1]='b';
				chaine_sortie[curseur_sortie+2]='r';
				chaine_sortie[curseur_sortie+3]='>';
				
				curseur_original += 6;
				curseur_sortie   += 4;
			}
			else{
					int val = 0;
					char buf[10];
					 //Conversion en hex
					sprintf(buf, "0x%c%c", chaine_originale[curseur_original+1], chaine_originale[curseur_original+2]);
					val = strtol(buf, NULL, 16);
			
					//Inscription dans la sortie
					chaine_sortie[curseur_sortie] = val;

					curseur_sortie ++;
					curseur_original += 3;
				}
				
		}
		else{
			chaine_sortie[curseur_sortie] = chaine_originale[curseur_original];
			curseur_original++;
			curseur_sortie++;
		}
	}
	strcpy(entree, chaine_sortie);
	entree[curseur_sortie] = '\0';
	free(chaine_originale);
	free(chaine_sortie);
}
/**
 * @brief Renvoie le contenu de la requête POST
*/
char* read_POST() 
{
query_size=atoi(getenv("CONTENT_LENGTH"));
char* query_string = (char*) malloc(query_size+100);
if (query_string != NULL) fread(query_string,query_size,1,stdin);
query_string[query_size]=0;
return query_string;
}

/**
 * @brief Utilise les fonctions url_decode et prepare_contenu afin de transformer le contenu de la requête POST en une chaine de caractère intelligible pour le programme de traitement et l'utilisateur ne présentant pas de risque d'injection. La variable d'entrée est directement modifiée
 * @param (char*) avatar - chaine à décoder
*/
void tamb(char *avatar)
{
int i;


for(i=0;i<strlen(avatar);i++)
	{
	if(avatar[i]=='+') avatar[i]=' ';
	}

url_decode(avatar);

prepare_contenu(avatar);
}
/**
 * @brief Prends en paramètre (chaine) le contenu de la requête POST et inscrit le contenu du paramètre d'indice pos (indice commence à 1) dans la variable resultat. La position est inversé. Le résultat d'indice 1 est le dernier passé dans la requête POST
 * @param (char*) chaine - Contenu de la requête POST
 * @param (int) pos - position du paramètre recherché (commence à 1 et ordre inversé)
 * @param (char*) resultat - variable dans laquelle la valeur recherchée est inscrite
*/
int get_chaine(char *chaine,int pos,char *resultat)
{
int i,n,start;

i=strlen(chaine);
for(n=0; n<pos;n++) 
	{
	while(chaine[i]!='=') i--;
	i--;
	}
i+=2;
start=i;
do
	{
	resultat[i-start]=chaine[i];
	i++;
	}
while(chaine[i-1]!=0 && chaine[i-1]!='&');
resultat[i-start-1]=0;
return 0;
}

short check_password(const char* mdp_clair, const char* mdp_hash){
	char resultat_hash[SHA256_HEX_SIZE];

	sha256_hex(mdp_clair, strlen(mdp_clair), resultat_hash);

	if (strcmp(mdp_hash, resultat_hash)!=0) return 1; // 1 ==  OK
	else return 0;                             // 0 == !OK
}

void hash256(char* mdp){
	char resultat_hash[SHA256_HEX_SIZE];
	sha256_hex(mdp, strlen(mdp), resultat_hash);
	strcpy(mdp, resultat_hash);
}