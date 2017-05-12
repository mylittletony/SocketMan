#include <curl/curl.h>
#include <json-c/json.h>
#include "options.h"
#include "http.h"
#include "dbg.h"
#include <compiler.h>

void update_operation(json_object *json, char *uid) {

  if (strcmp(options.api_url, "") == 0) {
    return;
  }

  CURL *curl;
  char url[500] = "";

  strcpy(url, options.api_url);
  strcat(url, "/operations/");
  strcat(url, uid);

  if (strcmp(options.token, "") != 0) {
    strcat(url, "?access_token=");
    strcat(url, options.token);
  }

  curl = curl_easy_init();
  if (!curl)
    return;

  struct curl_slist *headers = NULL;

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_null);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));

  if (options.insecure)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  if (do_curl(curl, url) == 0)
    debug("Could not connect to %s", url);

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);

  return;
}
