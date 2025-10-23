#include <stdio.h>
#include "log.h"
#include "crawler.h"

int main(void) {
    log_init();
    log_message("INFO", "Starting Tor crawler session");
    start_crawl();
    log_message("INFO", "Crawler finished execution");
    log_close();
    return 0;
}
