#ifndef __HTTP_H
#define __HTTP_H

#include <json-c/json.h>
#include <curl/curl.h>
#include <compiler.h>

int post_json(const char *postData);
int post_cache();
int run_init(char *f, char *m, char * mac);
void send_boot_message();
void append_url_token(char *url, char *buf);
void fetch_ca(char *buff);
void install_ca();
long do_curl(CURL *curl, char *url);
size_t write_null(UNUSED(void *buffer), size_t size, size_t nmemb, UNUSED(void *u));
void http_init();
void http_cleanup();
#endif
