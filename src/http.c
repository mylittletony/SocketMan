#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "options.h"
#include "network.h"
#include "system.h"
#include "helper.h"
#include "phy.h"
#include "dbg.h"
#include "cleaner.h"
#include "dhcp.h"
#include "utils.h"
#include <ctype.h>
#include "http.h"
#include "message.h"
#include "utils.h"

struct CurlResponse {
  char *memory;
  size_t size;
};

void init_chunk(struct CurlResponse *c)
{
  c->memory = malloc(1);
  c->size = 0;
}

size_t write_data(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct CurlResponse *mem = (struct CurlResponse *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    printf("Not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

void append_url_token(char *url, char *buf)
{
  strcpy(buf, url);
  if (strcmp(options.token, "") != 0) {
    strcat(buf, "?access_token=");
    strcat(buf, options.token);
  }
}

int do_curl(CURL *curl, char *url)
{
  long http_code = 0;
  CURLcode res;

  curl_easy_setopt(curl, CURLOPT_URL, url);
  res = curl_easy_perform(curl);

  if(res == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code == 200)
      return 1;
  }
  return 0;
}

void post_backup(CURL *curl)
{
  if (strcmp(options.backup_url, "") != 0) {
    char buff[255]; // should clear URL buff and use instead
    debug("Attempting to send to backup URL");
    append_url_token(options.backup_url, buff);
    do_curl(curl, buff);
  }
  debug("No backup URL, moving on.");
}

int post(json_object *json) {

  if (strcmp(options.url, "") != 0) {
    CURL *curl;
    char url[255];
    append_url_token(options.url, url);

    curl_global_init( CURL_GLOBAL_ALL );

    curl = curl_easy_init();
    if (!curl)
      return 0;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    struct CurlResponse c;
    init_chunk(&c);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&c);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));

    // Remove when not testing
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

    if(do_curl(curl, url) == 0) {
      debug("Could not connect to %s, trying backup.", url);
      post_backup(curl);
    } else {
      if (c.memory) {
        process_response(c.memory);
        free(c.memory);
        c.memory = NULL;
      }
    }

    if (c.memory)
      free(c.memory);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    curl_slist_free_all(headers);

    return 1;
  } else {
    debug("Not sending data, invalid URL");
    return 0;
  }
}

void send_boot_message()
{
  if (strcmp(options.boot_url, "") != 0) {
    debug("Sending GET request to boot URL");
    CURL *curl;
    curl_global_init( CURL_GLOBAL_ALL );

    char url[100];
    append_url_token(options.boot_url, url);

    long http_code = 0;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    CURLcode res;

    curl = curl_easy_init();
    if (!curl)
      return;

    struct CurlResponse c;
    init_chunk(&c);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&c);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

    res = curl_easy_perform(curl);
    if(res == CURLE_OK) {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
      if (http_code == 200 && c.memory)
        process_response(c.memory);
    }

    curl_easy_cleanup(curl);
    if (c.memory)
      free(c.memory);

    curl_global_cleanup();
    curl_slist_free_all(headers);
  }
}
