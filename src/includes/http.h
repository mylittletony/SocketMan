#ifndef __HTTP_H
#define __HTTP_H

#include <json-c/json.h>
#include <curl/curl.h>

int post(json_object *json);
int post_cache(char *file);
int run_init(char *f, char *m, char * mac);
void send_boot_message();
void append_url_token(char *url, char *buf);
void fetch_ca(char *buff);
void install_ca();
long do_curl(CURL *curl, char *url);

#endif
