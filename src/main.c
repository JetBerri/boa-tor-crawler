#include "crawler.h"
#include "db.h"
#include "log.h"

int main(void) {
    log_init("/app/logs/crawler_2025-10-24.log");

    PGconn *conn = db_connect("host=tor-postgres dbname=torcrawl user=crawler password=crawlerpass");
    if (!conn) {
        log_message("ERROR", "Failed to connect to database");
        return 1;
    }

    db_ensure_schema(conn);
    start_crawl(conn);
    db_close(conn);
    log_close();

    return 0;
}
