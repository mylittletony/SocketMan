#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "options.h"
#include "network.h"
#include "system.h"
#include "helper.h"
#include "ct_iw.h"
#include "dbg.h"
#include "cleaner.h"
#include "dhcp.h"
#include "utils.h"
#include <ctype.h>
#include "http.h"

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  return size * nmemb;
}

char append_url_token(char *url, char *token, char *buf)
{
  if (strcmp(url, "") != 0) {
    strcpy(buf, url);
    strcat(buf, "?access_token=");
    strcat(buf, token);
  }
}

int do_curl(CURL *curl, char *url,
    struct curl_slist *headers, json_object *json
    )
{
  long http_code = 0;
  CURLcode res;

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  res = curl_easy_perform(curl);
  if(res == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK) {
      // Process CMD output
      return 1;
    }
  }
  debug("Initial CURL failed, will try backup.");
  return 0;
}

int post(json_object *json) {

  CURL *curl;
  char url[100];
  struct curl_slist *headers = NULL;
  append_url_token(options.url, options.token, url);

  curl_global_init( CURL_GLOBAL_ALL );

  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl = curl_easy_init();
  if (!curl)
    return 0;

  if(do_curl(curl, url, headers, json) == 0) {
    debug("Could not connect to %s", url);
    if (strcmp(options.backup_url, "") != 0) {
      char buff[100]; // should clear URL buff and use instead
      debug("Attempting to send to backup URL");
      append_url_token(options.backup_url, options.token, buff);
      do_curl(curl, buff, headers, json);
    } else {
      debug("No backup URL, moving on.");
    }
  }

  curl_easy_cleanup(curl);
  curl_global_cleanup();
  curl_slist_free_all(headers);
  return 1;
}
