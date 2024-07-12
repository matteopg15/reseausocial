#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mysql/mysql.h>

#define _MYSQL

#define DBHANDLER MYSQL *
#define DBRESULT MYSQL_RES *

DBHANDLER handler;
DBRESULT result;

char *main_database="mysql";

/**
 * @brief Ouvre la BDD
 * @param (char*) databasename - Nom de la BDD
 * @param (char*) host - Nom de l'hôte
 * @param (char*) login - Nom d'utilisateur
 * @param (char*) password - Mot de passe
*/
DBHANDLER db_opendatabase(char *databasename, char *host, char *login, char *password)
{
MYSQL *handler;

handler=(MYSQL *)malloc(sizeof(MYSQL));
mysql_init(handler);
return(mysql_real_connect(handler, host, login, password, databasename, 0, NULL, 0));
}


/**
 * @brief Exécute la requête dans la BDD
 * @param (DBHANDLER) handler - Handler grâce auquel sera effectué la requête
 * @param (char*) query - Requête à effectuer
*/
DBRESULT db_query(DBHANDLER handler, char *query)
{
char *buf;

buf=(char *)malloc(strlen(query)+10);
strcpy(buf, query);
buf[strlen(buf)-1]=0;
mysql_query(handler, buf);
result=mysql_store_result(handler);
free(buf);
return(result);
}

/**
 * @brief Renvoie le nombre de résultats stockés dans result
 * @param (DBRESULT) result - Résultat de requête à étudier
*/
int db_ntuples(DBRESULT result)
{
	return((int)mysql_num_rows(result));
}

/**
 * @brief Réinitialise le résultat
 * @param (DBRESULT) result - Résultat de requête à réinitialiser
*/
void db_clear_result(DBRESULT result)
{
mysql_free_result(result);
}

/**
 * @brief Ferme la BDD
 * @param (DBHANDLER) handler - Handler lié à la BDD
*/
void db_close(DBHANDLER handler)
{
mysql_close(handler);
free(handler);
}

/**
 * @brief Mets la résultat de ligne et de colonne données dans la variable value. Si la taille de la variable excède max, tout ne sera pas recopié dans value
 * @param (DBRESULT) result - Résultat de requête dans la BDD
 * @param (int) row - Numéro de ligne
 * @param (int) col - Numéro de colonne
 * @param (char*) value - Variable dans laquelle sera stocké le résultat
 * @param (int) max - Nombre maximum de caractères copiés dans la variable value
*/
void db_getvalue(DBRESULT result, int row, int col, char *value, int max)
{

MYSQL_ROW rrow;

mysql_data_seek(result, row);

rrow=mysql_fetch_row(result);
strncpy(value, rrow[col], max-1);
value[max]=0;
}
