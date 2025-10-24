#ifndef PARSER_H
#define PARSER_H

#include <libpq-fe.h>

void extract_onion_links(PGconn *conn, const char *html, const char *query, const char *engine_name);
char *url_encode(const char *str);

#endif // PARSER_H
