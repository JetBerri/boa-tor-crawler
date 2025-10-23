#include "parser.h"
#include "log.h"
#include <string.h>

void extract_onion_links(const char *html, const char *query, const char *engine_name) {
    regex_t regex;
    regmatch_t matches[2];

    const char *pattern = "([a-z2-7]{16,56}\\.onion)";
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE)) {
        log_message("ERROR", "Could not compile regex for .onion extraction");
        return;
    }

    int count = 0;
    const char *cursor = html;

    while (regexec(&regex, cursor, 2, matches, 0) == 0) {
        int start = matches[1].rm_so;
        int end = matches[1].rm_eo;
        int len = end - start;

        char onion[128];
        strncpy(onion, cursor + start, len);
        onion[len] = '\0';

        log_message("FOUND", "[%s | %s] %s", engine_name, query, onion);

        cursor += end;
        count++;
    }

    if (count == 0)
        log_message("INFO", "No .onion links found for [%s | %s]", engine_name, query);
    else
        log_message("SUCCESS", "Extracted %d .onion links for [%s | %s]", count, engine_name, query);

    regfree(&regex);
}
