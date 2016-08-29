#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <resolv.h>
#include <netinet/tcp.h>
#include "dbg.h"
#include <syslog.h>
#include "options.h"
#include <string.h>
#include <signal.h>
#include "helper.h"
#include "mqtt.h"
#include "monitor.h"
#include <json-c/json.h>

int parent;

void check_config();
void boot();

void *parse_config(char *buffer)
{
  json_object *json_config = json_tokener_parse(buffer);

  if (!is_error(json_config)) {

    enum json_type type;
    json_object_object_foreach(json_config, key, val) {
      type = json_object_get_type(val);

      switch (type) {
        case json_type_int:
          if (strcmp(key, "no_scan") == 0)
            options.no_scan = 1;
          if (strcmp(key, "no_survey") == 0)
            options.no_survey = 1;
          if (strcmp(key, "sleep") == 0)
            options.sleep = json_object_get_int(val);
          if (strcmp(key, "port") == 0)
            options.port = json_object_get_int(val);
          if (strcmp(key, "timeout") == 0)
            options.timeout = json_object_get_int(val);
          break;
        case json_type_null:
          break;
        case json_type_array:
          break;
        case json_type_boolean:
          if (strcmp(key, "tls") == 0)
            options.tls = 1;
          break;
        case json_type_object:
          break;
        case json_type_double:
          break;
        case json_type_string:
          if (strcmp(key, "username") == 0)
            strcpy(options.username, json_object_get_string(val));
          if (strcmp(key, "password") == 0)
            strcpy(options.password, json_object_get_string(val));
          if (strcmp(key, "topic") == 0)
            strcpy(options.topic, json_object_get_string(val));
          if (strcmp(key, "status_topic") == 0)
            strcpy(options.status_topic, json_object_get_string(val));
          if (strcmp(key, "key") == 0)
            strcpy(options.key, json_object_get_string(val));
          if (strcmp(key, "cacrt") == 0)
            strcpy(options.cacrt, json_object_get_string(val));
          if (strcmp(key, "mqtt_host") == 0)
            strcpy(options.mqtt_host, json_object_get_string(val));
          if (strcmp(key, "url") == 0)
            strcpy(options.url, json_object_get_string(val));
          if (strcmp(key, "backup_url") == 0)
            strcpy(options.backup_url, json_object_get_string(val));
          if (strcmp(key, "mac") == 0)
            strcpy(options.mac, json_object_get_string(val));
          if (strcmp(key, "mac_file") == 0)
            strcpy(options.mac_file, json_object_get_string(val));
          if (strcmp(key, "token") == 0)
            strcpy(options.token, json_object_get_string(val));
          break;
      }
    }
    json_object_put(json_config);
  }
  else {
    debug("Invalid json in file");
  }
}

void initialised()
{
  if ((strlen(options.token) != 0) &&
      (strlen(options.username) != 0) &&
      (strlen(options.password) != 0)) {
    options.initialized = 1;
  }
}

void run_collector()
{
  /* int status; */
  /* int exit_status; */

  debug("Actually starting Socketman.");

  int pid = fork();

  if (pid == 0) {
    mqtt_connect();
  } else {
    parent = pid;
    monitor();
  };
}

void check_config()
{
  if (strlen(options.config) != 0){
    debug("Checking config...");
    char *buffer = read_config(options.config);
    if (buffer) {
      parse_config(buffer);
      initialised();
    } else {
      debug("Config not found.");
    }
    free(buffer);
  } else {
    sleep(1);
    options.initialized = 1;
  }
}

void sleepy() {
  do {
    debug("Sleeping forever, I don't have a valid config.");
    sleep(30);
  }
  while(1);
}

void boot()
{
  /* Should send a post to a URL */
  /* recover(); */
  /* Should send a post to a URL */
  check_config();
  if (options.initialized)
    run_collector();
  sleepy();
}
