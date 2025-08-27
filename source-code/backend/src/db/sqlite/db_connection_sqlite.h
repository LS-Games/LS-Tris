#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <sqlite3.h>

//It opens database and return a sqlite3* pointer that is ready to use 
//It returns NULL if something goes wrong 

sqlite3* db_open(const char* filename);

void db_close(sqlite3* db);

#endif