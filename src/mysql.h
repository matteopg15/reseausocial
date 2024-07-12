#ifndef MYSQL_H_INCLUDED
#define MYSQL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#define _MYSQL

#define DBHANDLER MYSQL *
#define DBRESULT MYSQL_RES *

DBHANDLER db_opendatabase(char *databasename, char *host, char *login, char *password);
DBRESULT db_query(DBHANDLER handler, char *query);
int db_ntuples(DBRESULT result);
void db_clear_result(DBRESULT result);
void db_close(DBHANDLER handler);
void db_getvalue(DBRESULT result, int row, int col, char *value, int max);


#endif // MYSQL_H_INCLUDED
