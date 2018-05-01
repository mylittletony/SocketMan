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
#include "cache.h"
#include <stdio.h>
#include "zlib.h"
#include <compiler.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

struct CurlResponse {
  char *memory;
  size_t size;
};

size_t write_null(UNUSED(void *buffer), size_t size, size_t nmemb, UNUSED(void *u))
{
     return size * nmemb;
}

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
    strcat(buf, "&mac=");
    strcat(buf, options.mac);
  }
  if (strcmp(CT_VERSION, "") != 0) {
    debug("ssssssssssssssssssssssssssssssssss %s", CT_VERSION);
    strcat(buf, "&v=");
    char version[6];
    snprintf(version, sizeof version, "%.10s", CT_VERSION);
    strcat(buf, version);
  }
}

long do_curl(CURL *curl, char *url)
{
  long http_code = 0;
  if (options.debug) {
    /* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */
  }
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  return http_code;
}

int post_backup(CURL *curl)
{
  if (strcmp(options.backup_stats_url, "") == 0) {
    debug("No backup URL, moving on.");
    return 0;
  }

  char buff[255]; // should clear URL buff and use instead
  debug("Attempting to send to backup URL");
  append_url_token(options.backup_stats_url, buff);
  return do_curl(curl, buff);
}

// This sends the pure json, no caching
int post_json(const char *postData)
{

  if (strcmp(options.stats_url, "") == 0) {
    return 0;
  }

  CURL *curl;
  char url[255];

  append_url_token(options.stats_url, url);

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
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/bundle.pem");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);

  long resp = do_curl(curl, url);

  if (resp == 0) {
    goto offline;
  }

  if (resp != 200 && resp != 201 && resp != 401) {
    debug("Could not connect to %s (%ld), trying backup.", options.stats_url, resp);
    long tmp = post_backup(curl);
    if (tmp != 0) {
      resp = tmp;
    }
  }

  if ((resp == 200 || resp == 201) && c.size > 0) {
    debug("Stats successfully sent (%ld)", resp);
    process_response(c.memory);
    free(c.memory);
    c.memory = NULL;
  }

  // Exit monitor and poll for a config
  if (resp == 401) {
    debug("This device is not authorized.");
    options.initialized = 0;
  }

  goto cleanup;

offline:
  /* cache(postData); */
  goto cleanup;

cleanup:
  if (c.memory) {
    free(c.memory);
  }
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  return 1;
}

int post_cache()
{

  if (strcmp(options.stats_url, "") == 0) {
    return 1;
  }

  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;

  CURL *curl;
  char url[255];

  append_url_token(options.stats_url, url);

  curl = curl_easy_init();
  if (!curl)
    return 1;

  struct curl_slist *headers = NULL;

  headers = curl_slist_append(headers, "Content-Encoding: gzip");

  struct CurlResponse c;
  init_chunk(&c);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&c);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/bundle.pem");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  curl_formadd(&post, &last, CURLFORM_COPYNAME, "data",
      CURLFORM_FILE, options.archive, CURLFORM_END);

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

  long resp = do_curl(curl, url);

  if (resp == 0) {
    debug("DATA NOT SENT. DEVICE NOT SENDING STATS. Code (%ld)", resp);
    goto cleanup;
  }

  if (resp != 200 && resp != 201 && resp != 401) {
    debug("Could not connect to %s (%ld), trying backup.", options.stats_url, resp);
    long tmp = post_backup(curl);
    if (tmp != 0) {
      resp = tmp;
    }
  }

  if ((resp == 200 || resp == 201) && c.size > 0) {
    debug("Data sent (%ld)", resp);
    process_response(c.memory);
    free(c.memory);
    c.memory = NULL;
  }

  // Exit monitor and poll for a config
  if (resp == 401) {
    debug("This device is not authorized.");
    options.initialized = 0;
  }

  goto cleanup;

cleanup:
  if (c.memory) {
    free(c.memory);
  }
  curl_formfree(post);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  return resp;
}

int run_init(char *f, char *m, char *mac) {

  int response = 0;
  CURL *curl;
  char url[255];

  if (strcmp(options.init, "") == 0) {
    strcpy(url, "https://api.ctapp.io/api/v2/init");
  } else {
    strcpy(url, options.init);
  }
  debug("Initializing against %s", url);

  strcat(url, "?mac=");
  strcat(url, mac);
  strcat(url, "&machine=");
  strcat(url, curl_easy_escape(curl, m, 0));
  strcat(url, "&firmware=");
  strcat(url, curl_easy_escape(curl, f, 0));

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
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/bundle.pem");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  if (options.insecure) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
  }

  long resp = do_curl(curl, url);

  // Exit monitor and poll for a config
  if (resp != 201 || resp != 200) {
    debug("Could not initialize, device not found or not authorized (%ld).", resp);
  }

  if ((resp == 200 || resp == 201) && c.size > 0) {
    debug("Device found, extracting configs.........");
    save_config(options.config, c.memory);
    free(c.memory);
    c.memory = NULL;
    response = 1;
  }

  if (c.memory) {
    free(c.memory);
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);

  return response;
}

void send_boot_message()
{
  if (strcmp(options.boot_url, "") == 0) {
    return;
  }

  char url[255];
  long http_code = 0;
  struct curl_slist *headers = NULL;

  debug("Sending GET request to boot URL");
  CURL *curl;

  append_url_token(options.boot_url, url);

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
  curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/bundle.pem");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  res = curl_easy_perform(curl);

  // The response codes are specific to CT and don't need to be used this way
  // 200 - the device will process the output, handy for emergencies
  // 201 - the servers responds with the config.json file
  // 401 - unauthorized or not found, either way it won't boot NiU

  if(res == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code == 200 && c.size > 0) {
      process_response(c.memory);
    } else if (http_code == 201) {

    } else if (http_code == 401) {

    }
  }

  curl_easy_cleanup(curl);
  if (c.memory)
    free(c.memory);

  curl_slist_free_all(headers);
}

void fetch_ca(char *buff) {

  CURL *curl;
  char *url = "http://s3.amazonaws.com/puffin-certs/md5.txt";

  curl = curl_easy_init();
  if (!curl)
    return;

  struct CurlResponse c;
  init_chunk(&c);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&c);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
  curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/bundle.pem");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  long resp = do_curl(curl, url);
  if (resp != 200) {
    debug("MD5 not found. %s", url);
  }

  if ((resp == 200) && c.size > 0) {
    strcpy(buff, c.memory);
  }

  if (c.memory) {
    free(c.memory);
  }

  curl_easy_cleanup(curl);
  return;
}

void install_ca() {

  CURL *curl;
  char *url = "http://s3.amazonaws.com/puffin-certs/current.ca";

  curl = curl_easy_init();
  if (!curl)
    return;

  struct CurlResponse c;
  init_chunk(&c);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&c);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
  curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/bundle.pem");

  long resp = do_curl(curl, url);
  if (resp != 200) {
    debug("Cert not found. %s", url);
  }

  if ((resp == 200) && c.size > 0) {
    save_config(options.cacrt, c.memory);
  }

  if (c.memory) {
    free(c.memory);
  }

  curl_easy_cleanup(curl);
  return;
}

void http_init() {
  curl_global_init( CURL_GLOBAL_ALL );
}

void http_cleanup() {
  curl_global_cleanup();
}
