#include "crawler.h"
#include "log.h"
#include "parser.h"
#include "db.h"
#include "verify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <yaml.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

struct Engine {
    char name[128];
    char url[512];
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (!mem->memory) return 0;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

static int load_queries(char ***queries_out) {
    FILE *fp = fopen("config/queries.txt", "r");
    if (!fp) {
        log_message("ERROR", "Could not open config/queries.txt");
        return 0;
    }

    size_t count = 0;
    char **queries = NULL;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
        queries = realloc(queries, sizeof(char *) * (count + 1));
        queries[count] = strdup(line);
        count++;
    }

    fclose(fp);
    *queries_out = queries;
    return (int)count;
}

static int load_engines(struct Engine **engines_out) {
    FILE *fh = fopen("config/engines.yaml", "r");
    if (!fh) {
        log_message("ERROR", "Could not open config/engines.yaml");
        return 0;
    }

    yaml_parser_t parser;
    yaml_event_t event;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, fh);

    struct Engine *engines = NULL;
    int count = 0;
    char key[64] = "";
    int in_engine = 0;

    while (1) {
        if (!yaml_parser_parse(&parser, &event)) break;

        if (event.type == YAML_MAPPING_START_EVENT) {
            in_engine = 1;
            engines = realloc(engines, sizeof(struct Engine) * (count + 1));
            memset(&engines[count], 0, sizeof(struct Engine));
        } else if (event.type == YAML_SCALAR_EVENT && in_engine) {
            if (strcmp((char *)event.data.scalar.value, "name") == 0)
                strcpy(key, "name");
            else if (strcmp((char *)event.data.scalar.value, "url") == 0)
                strcpy(key, "url");
            else if (strcmp(key, "name") == 0)
                strncpy(engines[count].name, (char *)event.data.scalar.value, sizeof(engines[count].name) - 1);
            else if (strcmp(key, "url") == 0)
                strncpy(engines[count].url, (char *)event.data.scalar.value, sizeof(engines[count].url) - 1);
        } else if (event.type == YAML_MAPPING_END_EVENT) {
            count++;
            in_engine = 0;
        } else if (event.type == YAML_STREAM_END_EVENT) {
            break;
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(fh);

    *engines_out = engines;
    return count;
}

static void perform_search(PGconn *conn, const char *engine_name, const char *engine_url, const char *query) {
    if (!engine_name || !engine_url || !query) {
        log_message("ERROR", "Invalid parameters for perform_search");
        return;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        log_message("ERROR", "Failed to initialize CURL");
        return;
    }

    char *escaped = curl_easy_escape(curl, query, 0);
    if (!escaped) {
        log_message("ERROR", "Failed to URL-encode query: %s", query);
        curl_easy_cleanup(curl);
        return;
    }

    char full_url[1024];
    snprintf(full_url, sizeof(full_url), engine_url, escaped);
    curl_free(escaped);

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    curl_easy_setopt(curl, CURLOPT_PROXY, "socks5h://tor-proxy:9050");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 25L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64)");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    log_message("INFO", "Searching [%s | %s] -> %s", engine_name, query, full_url);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        log_message("ERROR", "CURL failed for [%s | %s]: %s", engine_name, query, curl_easy_strerror(res));
        db_insert_link(conn, full_url, query, engine_name, "CURL_FAIL");
    } else {
        extract_onion_links(conn, chunk.memory, query, engine_name);
    }

    free(chunk.memory);
    curl_easy_cleanup(curl);
}

void start_crawl(PGconn *conn) {
    log_message("INFO", "Starting Tor crawler session");

    if (!conn) {
        log_message("ERROR", "No valid database connection");
        return;
    }

    struct Engine *engines = NULL;
    int engine_count = load_engines(&engines);
    if (engine_count == 0) {
        log_message("ERROR", "No engines loaded");
        return;
    }

    char **queries = NULL;
    int query_count = load_queries(&queries);
    if (query_count == 0) {
        log_message("ERROR", "No queries loaded");
        free(engines);
        return;
    }

    for (int i = 0; i < query_count; i++) {
        for (int j = 0; j < engine_count; j++) {
            perform_search(conn, engines[j].name, engines[j].url, queries[i]);
        }
    }

    for (int i = 0; i < query_count; i++) free(queries[i]);
    free(queries);
    free(engines);

    log_message("INFO", "Crawler finished execution");
}
