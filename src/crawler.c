#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "log.h"
#include "parser.h"

struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback for writing data into memory
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        log_message("ERROR", "Not enough memory (realloc returned NULL)");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static void perform_search(const char *engine_name, const char *engine_url, const char *query) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    CURL *curl_tmp = curl_easy_init();
    char *escaped = curl_easy_escape(curl_tmp, query, 0);
    char encoded_query[512];
    snprintf(encoded_query, sizeof(encoded_query), engine_url, escaped);
    curl_free(escaped);
    curl_easy_cleanup(curl_tmp);

    log_message("INFO", "Searching [%s] on engine [%s]", query, engine_name);

    for (int attempt = 1; attempt <= 2; attempt++) {
        curl = curl_easy_init();
        if (!curl) {
            log_message("ERROR", "Failed to init CURL for [%s]", engine_name);
            free(chunk.memory);
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, encoded_query);
        curl_easy_setopt(curl, CURLOPT_PROXY, "socks5h://localhost:9050");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 25L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Robust configuration
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:125.0) Gecko/20100101 Firefox/125.0");
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            log_message("INFO", "Downloaded %zu bytes from [%s]", chunk.size, engine_name);
            extract_onion_links(chunk.memory, query, engine_name);
            curl_easy_cleanup(curl);
            free(chunk.memory);
            return;
        } else {
            log_message("ERROR", "Attempt %d failed for [%s | %s]: %s",
                        attempt, engine_name, query, curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        sleep(2);
    }

    free(chunk.memory);
}

// Entry point for the crawler
void start_crawl(void) {
    FILE *qfile = fopen("config/queries.txt", "r");
    if (!qfile) {
        log_message("ERROR", "Cannot open queries.txt");
        return;
    }

    FILE *efile = fopen("config/engines.yaml", "r");
    if (!efile) {
        log_message("ERROR", "Cannot open engines.yaml");
        fclose(qfile);
        return;
    }

    char query[256];
    char engine_line[1024];
    char engine_name[128];
    char engine_url[512];
    int in_engine = 0, name_next = 0, url_next = 0;

    log_message("INFO", "Starting Tor crawler session");

    while (fgets(engine_line, sizeof(engine_line), efile)) {
        if (strstr(engine_line, "name:")) {
            sscanf(engine_line, " name: %127s", engine_name);
        } else if (strstr(engine_line, "url:")) {
            sscanf(engine_line, " url: \"%511[^\"]\"", engine_url);

            // Loop through queries
            fseek(qfile, 0, SEEK_SET);
            while (fgets(query, sizeof(query), qfile)) {
                query[strcspn(query, "\n")] = 0;
                perform_search(engine_name, engine_url, query);
            }
        }
    }

    fclose(efile);
    fclose(qfile);

    log_message("INFO", "Crawling completed");
}
