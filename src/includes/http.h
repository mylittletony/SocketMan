#ifndef __HTTP_H
#define __HTTP_H

#include <json-c/json.h>
#include <curl/curl.h>

int post(json_object *json);
void send_boot_message();
void append_url_token(char *url, char *buf);
long do_curl(CURL *curl, char *url);

#endif
