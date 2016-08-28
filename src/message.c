#include <json-c/json.h>
#include "notify.h"
#include "dbg.h"

#define BUFF_SIZE 100000 // ? too much ?

void parse_message(const char *msg) {

  int save = 0;
  char id[100];
  char cmd[100];

  int response;
  json_object *jobj = json_tokener_parse(msg);

  if (!is_error(jobj)) {
    enum json_type type;

    json_object_object_foreach(jobj, key, val) {
      type = json_object_get_type(val);
      switch (type) {
        case json_type_boolean:
          if (strcmp(key, "save") == 0) {
            if (json_object_get_boolean(val))
              save = 1;
          }
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
  if (save == 1) {
    response = 0;

    FILE *fp;
    char buffer[BUFF_SIZE];
    buffer[0] = '\0';

    fp = popen(cmd, "r");
    if (fp == NULL) {
      printf("Failed to run command\n" );
      return;
    }

    memset(buffer, '\0', sizeof(buffer));
    fread(buffer, sizeof(char), BUFF_SIZE, fp);
    pclose(fp);
    cmd_notify(response, id, buffer);

  } else {
    response = system(cmd);
    cmd_notify(response, id, NULL);
  }
}

void process_message(const char *msg) {
  parse_message(msg);
}
