#include <json-c/json.h>
#include "notify.h"
#include "dbg.h"

#define BUFF_SIZE 100000 // ? too much ?

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
  cmd_notify(response, id, buffer);
}

void process_cmd(char *cmd, char *id)
{
  save_and_notify(id, cmd);
  return;
}

void process_message(const char *msg, char *cmd, char *id)
{
  /* int save = 0; */
  /* char cmd[10000]; */

  json_object *jobj = json_tokener_parse(msg);

  if (!is_error(jobj)) {
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
      type = json_object_get_type(val);
      switch (type) {
        case json_type_boolean:
          /* if ((strcmp(key, "save") == 0) && */
          /*     json_object_get_boolean(val)) { */
          /*   save = 1; */
          /* } */
        case json_type_string:
          if (strcmp(key, "cmd") == 0)
            strcpy(cmd, json_object_get_string(val));
          if (strcmp(key, "id") == 0)
            strcpy(id, json_object_get_string(val));
        default:
          break;
      }
    }
    json_object_put(jobj);
  }
}

void process_response(char *msg)
{
  json_object *jobj;

  jobj = json_tokener_parse(msg);
  if (is_error(jobj))
    return;

  json_object *jcmd;
  json_object_object_get_ex(jobj, "cmd", &jcmd);

  if (jcmd == NULL)
    return;

  int i = json_object_get_string_len(jcmd);
  char cmd[i+1];
  cmd[0] = '\0';
  strcpy(cmd, json_object_get_string(jcmd));
  json_object_put(jobj);
  if (cmd[0] != '\0') {
    FILE * fp = popen(cmd, "r");
    if ( fp == 0 ) {
      fprintf(stderr, "Could not execute cmd\n");
      return;
    }
    debug("Running response CMD");
    pclose(fp);
  }
}
