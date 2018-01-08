#include <json-c/json.h>
#include "notify.h"
#include "dbg.h"
#include "options.h"
#include "collector.h"

// ? too much ?
#define BUFF_SIZE 100000

void save_and_notify(char *id, char *cmd)
{
  FILE *fp;
  int response = -1;
  char buffer[BUFF_SIZE];
  buffer[0] = '\0';

  fp = popen(cmd, "r");
  if (fp != NULL) {
    response = 0;
    memset(buffer, '\0', sizeof(buffer));
    fread(buffer, sizeof(char), BUFF_SIZE, fp);
    pclose(fp);
  }
  if (options.debug){
    debug("%s", buffer);
  }

  cmd_notify(response, id, buffer);
}

void process_cmd(char *cmd, char *id)
{
  save_and_notify(id, cmd);
  return;
}

void process_response(char *msg)
{
  json_object *jobj;

  jobj = json_tokener_parse(msg);
  if (is_error(jobj))
    return;

  json_object *jcmd;
  json_object_object_get_ex(jobj, "cmd", &jcmd);

  if (jcmd == NULL) {
    return;
  }

  int i = json_object_get_string_len(jcmd);
  char cmd[i+1];
  cmd[0] = '\0';
  strcpy(cmd, json_object_get_string(jcmd));
  json_object_put(jobj);
  if (cmd[0] != '\0') {
    FILE * fp = popen(cmd, "r");
    if ( fp == 0 ) {
      debug("Could not execute cmd");
      return;
    }
    debug("Running response CMD");
    pclose(fp);
  }
}

void save_config(char *file, char *msg)
{
  FILE *f = fopen(file, "w");
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(EXIT_FAILURE);
  }

  fprintf(f, "%s", msg);
  fclose(f);
}

void run_special(char *type)
{
  if (type && strcmp(type, "ping") == 0) {
    debug("Running the ping test brother!");
    // Run ping
    return;
  }

  backup_configs(type);
  return;
}

void process_message(const char *msg, char *cmd, char *id, char *opType, int len)
{
  json_object *jobj = json_tokener_parse(msg);
  enum json_type type;

  if (is_error(jobj)) {
    return;
  }

  json_object_object_foreach(jobj, key, val) {
    type = json_object_get_type(val);
    switch (type) {
      case json_type_object:
        if (strcmp(key, "meta") == 0) {
          json_object_object_foreach(val, keym, valm) {
            switch (json_object_get_type(valm)) {
              case json_type_null:
              case json_type_boolean:
              case json_type_double:
              case json_type_int:
              case json_type_array:
              case json_type_object:
              case json_type_string:
                if (strcmp(keym, "msg") == 0)
                  strncpy(cmd, json_object_get_string(valm), len);
                if (strcmp(keym, "type") == 0)
                  strncpy(opType, json_object_get_string(valm), 10);
            }
          }
        }
      case json_type_boolean:
      case json_type_string:
        if (strcmp(key, "id") == 0)
          strncpy(id, json_object_get_string(val), 36+1);
      default:
        break;
    }
  }
  json_object_put(jobj);
}
