#include "verify.h"
#include "log.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

#define TOR_PROXY "socks5h://tor-proxy:9050"

const char *verify_onion_link(const char *url) {

    if (!url || !strstr(url, ".onion")) {

        return "INVALID";

    }

    CURL *curl = curl_easy_init();

    if (!curl) {

        log_message("ERROR", "verify_onion_link: Failed to initialize CURL");
        return "CURL_INIT_FAIL";

    }

    CURLcode res;
    long response_code = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_PROXY, TOR_PROXY);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);          // HEAD request (no body)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64)");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);

    if (res == CURLE_OK) {

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (response_code >= 200 && response_code < 400) {

            curl_easy_cleanup(curl);

            return "ONLINE";

        } else {

            curl_easy_cleanup(curl);

            return "OFFLINE";

        }

    } else if (res == CURLE_OPERATION_TIMEDOUT) {

        curl_easy_cleanup(curl);
        return "TIMEOUT";

    } else {

        curl_easy_cleanup(curl);
        return "UNREACHABLE";

    }

}
