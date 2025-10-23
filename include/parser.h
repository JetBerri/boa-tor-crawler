#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

// Parse HTML and extract all .onion links
void extract_onion_links(const char *html, const char *query, const char *engine_name);

#endif
