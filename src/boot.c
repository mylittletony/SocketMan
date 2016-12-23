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
#include "http.h"
#include <sys/prctl.h>
#include "init.h"

int parent;

void check_config();
void boot();
void install();

void parse_config(char *buffer)
{
  json_object *json_config = json_tokener_parse(buffer);

  if (!is_error(json_config)) {

    options.survey = 1;
    options.scan = 1;
    enum json_type type;

    json_object_object_foreach(json_config, key, val) {
      type = json_object_get_type(val);

      switch (type) {
        case json_type_null:
        case json_type_array:
        case json_type_object:
        case json_type_double:
        case json_type_string:
        case json_type_boolean:
          if (strcmp(key, "tls") == 0) {
            options.tls = 0;
            if (json_object_get_boolean(val)) {
              options.tls = 1;
            };
          }
        case json_type_int:
          if (strcmp(key, "debug") == 0)
            options.debug = 1;
          if (strcmp(key, "no-compress") == 0)
            options.nocompress = json_object_get_int(val);
          if (strcmp(key, "scan") == 0)
            options.scan = json_object_get_int(val);
          if (strcmp(key, "rest") == 0)
            options.rest = json_object_get_int(val);
          if (strcmp(key, "survey") == 0)
            options.survey = json_object_get_int(val);
          if (strcmp(key, "sleep") == 0)
            options.sleep = json_object_get_int(val);
          if (strcmp(key, "port") == 0)
            options.port = json_object_get_int(val);
          if (strcmp(key, "timeout") == 0)
            options.timeout = json_object_get_int(val);
          if (strcmp(key, "monitor") == 0)
            options.monitor = json_object_get_int(val);
          if (strcmp(key, "heartbeat") == 0)
            options.heartbeat = json_object_get_int(val);
          if (strcmp(key, "reboot") == 0)
            options.reboot = json_object_get_int(val);
          if (strcmp(key, "health_port") == 0)
            options.health_port = json_object_get_int(val);
          if (strcmp(key, "qos") == 0)
            options.qos = json_object_get_int(val);
          if (strcmp(key, "insecure") == 0)
            options.insecure = json_object_get_int(val);
          if (strcmp(key, "username") == 0)
            strcpy(options.username, json_object_get_string(val));
          if (strcmp(key, "password") == 0)
            strcpy(options.password, json_object_get_string(val));
          if (strcmp(key, "topic") == 0)
            strcpy(options.topic, json_object_get_string(val));
          if (strcmp(key, "key") == 0)
            strcpy(options.key, json_object_get_string(val));
          if (strcmp(key, "cacrt") == 0)
            strcpy(options.cacrt, json_object_get_string(val));
          if (strcmp(key, "mqtt_host") == 0)
            strcpy(options.mqtt_host, json_object_get_string(val));
          if (strcmp(key, "api_url") == 0)
            strcpy(options.api_url, json_object_get_string(val));
          if (strcmp(key, "stats_url") == 0)
            strcpy(options.stats_url, json_object_get_string(val));
          if (strcmp(key, "backup_stats_url") == 0)
            strcpy(options.backup_stats_url, json_object_get_string(val));
          if (strcmp(key, "health_url") == 0)
            strcpy(options.health_url, json_object_get_string(val));
          if (strcmp(key, "boot_url") == 0)
            strcpy(options.boot_url, json_object_get_string(val));
          if (strcmp(key, "boot_cmd") == 0)
            strcpy(options.boot_cmd, json_object_get_string(val));
          if (strcmp(key, "mac") == 0)
            strcpy(options.mac, json_object_get_string(val));
          if (strcmp(key, "mac_file") == 0)
            strcpy(options.mac_file, json_object_get_string(val));
          if (strcmp(key, "token") == 0)
            strcpy(options.token, json_object_get_string(val));
      }
    }
    json_object_put(json_config);
  }
  else {
    debug("Invalid json in file");
    install();
  }

  // Check and set some defaults vals

  // How often to check the network connection
  if (options.monitor < 5)
    options.monitor = 30;

  // How often to collect and send data
  if (options.heartbeat < options.monitor)
    options.heartbeat = options.monitor * 2;

  // Reboot after N seconds offline
  if (options.reboot < 600)
    options.reboot = 600;

  // Used for the DNS check
  if (strcmp(options.health_url, "") == 0)
    strcpy(options.health_url, "health.cucumberwifi.io");

  if (!options.health_port)
    options.health_port = 53;

  if (!options.qos)
    options.qos = 0;
}

void boot_cmd()
{
  if (strcmp(options.boot_cmd, "0") != 0) {
    FILE * fp = popen(options.boot_cmd, "r");
    if ( fp == 0 ) {
      fprintf(stderr, "Could not execute cmd\n");
      return;
    }
    debug("Running boot CMD");
    pclose(fp);
  }
}

void pre_boot_cb()
{
  send_boot_message();
  boot_cmd();
  check_certificates();
}

void initialised()
{
  if ((strlen(options.token) != 0) &&
      (strlen(options.username) != 0) &&
      (strlen(options.password) != 0)) {
    options.initialized = 1;
  }
}

void run_socketman()
{
  debug("Starting Socketman.");
  pre_boot_cb();
  mqtt_connect();
  do
  {
    monitor();
  }
  while(options.initialized);

  debug("Exiting main loop - go into the config block");

  install(); // <--------------------- test

  return;
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
      // Get the config
      debug("Config not found.");
    }
    free(buffer);
  } else {
    // Why the sleep?
    sleep(1);
    options.initialized = 1;
  }
}

void install() {
  do {
    if (init() == 1) {
      break;
    }
    debug("Something wrong with the MAC Address, sleeping 30");
    sleep(30);
  }
  while(1);
  boot();
}

void boot()
{
  check_config();
  if (options.initialized)
    run_socketman();
  install();
}
