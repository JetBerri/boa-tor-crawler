#ifndef CRAWLER_H
#define CRAWLER_H

#include <libpq-fe.h>

void start_crawl(PGconn *conn);

#endif
