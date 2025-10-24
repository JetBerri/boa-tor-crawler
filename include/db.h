#ifndef DB_H
#define DB_H

#include <libpq-fe.h>

PGconn *db_connect(const char *conninfo);
void db_close(PGconn *conn);
int db_ensure_schema(PGconn *conn);
int db_insert_link(PGconn *conn, const char *url, const char *query, const char *engine, const char *status);

#endif // DB_H
