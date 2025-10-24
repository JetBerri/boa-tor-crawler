#include "db.h"
#include "log.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PGconn *db_connect(const char *conninfo) {
    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        log_message("ERROR", "Postgres connection failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }
    log_message("INFO", "Connected to PostgreSQL database");
    return conn;
}

void db_close(PGconn *conn) {
    if (!conn) return;
    PQfinish(conn);
    log_message("INFO", "Database connection closed");
}

int db_ensure_schema(PGconn *conn) {
    if (!conn) return -1;

    const char *create_sql =
        "CREATE TABLE IF NOT EXISTS onion_links ("
        " id SERIAL PRIMARY KEY, "
        " url TEXT UNIQUE NOT NULL, "
        " query TEXT, "
        " engine TEXT, "
        " status TEXT, "
        " discovered_at TIMESTAMP DEFAULT (now() at time zone 'utc')"
        ");";

    PGresult *res = PQexec(conn, create_sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_message("ERROR", "Failed ensuring schema: %s", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }
    PQclear(res);

    // Crear índice único en URL (por seguridad)
    const char *index_sql =
        "CREATE UNIQUE INDEX IF NOT EXISTS onion_links_url_key ON onion_links(url);";
    res = PQexec(conn, index_sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_message("ERROR", "Failed creating unique index: %s", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }
    PQclear(res);

    log_message("INFO", "Database schema ensured");
    return 0;
}

int db_insert_link(PGconn *conn, const char *url, const char *query, const char *engine, const char *status) {
    if (!conn || !url) return -1;

    const char *paramValues[4];
    paramValues[0] = url;
    paramValues[1] = query ? query : "";
    paramValues[2] = engine ? engine : "";
    paramValues[3] = status ? status : "";

    const char *sql =
        "INSERT INTO onion_links (url, query, engine, status) "
        "VALUES ($1, $2, $3, $4) "
        "ON CONFLICT (url) DO UPDATE "
        "SET query = EXCLUDED.query, "
        "engine = EXCLUDED.engine, "
        "status = EXCLUDED.status, "
        "discovered_at = NOW();";

    PGresult *res = PQexecParams(conn, sql, 4, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_message("ERROR", "db_insert_link failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    log_message("SUCCESS", "Inserted or updated link: %s", url);
    return 0;
}
