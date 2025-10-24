#define _GNU_SOURCE
#include "parser.h"
#include "log.h"
#include "db.h"
#include "verify.h"
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Simple URL encoder (for building full URLs if needed) */
char *url_encode(const char *str) {
    if (!str) return NULL;
    const char *hex = "0123456789ABCDEF";
    size_t len = strlen(str);
    char *enc = malloc(len * 3 + 1);
    if (!enc) return NULL;
    char *p = enc;

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)str[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        } else if (c == ' ') {
            *p++ = '+';
        } else {
            *p++ = '%';
            *p++ = hex[c >> 4];
            *p++ = hex[c & 0x0F];
        }
    }
    *p = '\0';
    return enc;
}

/*
 * Extract .onion links from HTML content, verify each one through Tor,
 * and insert/update in PostgreSQL with its current status.
 */
void extract_onion_links(PGconn *conn, const char *html, const char *query, const char *engine_name) {
    if (!html) return;

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
        if (len <= 0 || len >= 256) break;

        char onion[256];
        strncpy(onion, cursor + start, len);
        onion[len] = '\0';

        // build http:// link for Tor verification
        char full_url[512];
        snprintf(full_url, sizeof(full_url), "http://%s", onion);

        // verify through Tor
        const char *status = verify_onion_link(full_url);

        log_message("FOUND", "[%s | %s] %s (status: %s)",
                    engine_name ? engine_name : "?",
                    query ? query : "?",
                    onion,
                    status ? status : "UNKNOWN");

        // insert into DB with the real status
        if (conn) {
            if (db_insert_link(conn, onion, query, engine_name, status) != 0) {
                log_message("ERROR", "Failed to insert %s into DB", onion);
            }
        }

        cursor += end;
        count++;
    }

    if (count == 0)
        log_message("INFO", "No .onion links found for [%s | %s]",
                    engine_name ? engine_name : "?",
                    query ? query : "?");
    else
        log_message("SUCCESS", "Extracted %d .onion links for [%s | %s]",
                    count,
                    engine_name ? engine_name : "?",
                    query ? query : "?");

    regfree(&regex);
}
