#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "dbg.h"
#include "platform.h"
#include <json-c/json.h>

void parse_splash_clients(json_object *json_sessions, struct splash_list *ptr)
{
  int i;
  json_object *jobj, *array_obj, *state, *ip, *mac, *auth;

  enum json_type type;
  json_object_object_foreach(json_sessions, key, val) {
    type = json_object_get_type(val);
    switch(type) {
      case json_type_string:
      case json_type_null:
      case json_type_int:
      case json_type_object:
      case json_type_double:
      case json_type_boolean:
      case json_type_array:
        json_object_object_get_ex(json_sessions, key, &jobj);
        int arraylen = json_object_array_length(jobj);
        for (i = 0; i < arraylen; i++) {

          ptr->next = malloc(sizeof(struct splash_list));
          if (ptr->next == NULL) break;

          array_obj = json_object_array_get_idx(jobj, i);
          json_object_object_get_ex(array_obj, "clientState", &state);
          json_object_object_get_ex(array_obj, "ipAddress", &ip);
          json_object_object_get_ex(array_obj, "macAddress", &mac);
          json_object_object_get_ex(array_obj, "dhcpState", &auth);

          strcpy(ptr->mac, json_object_get_string(mac));
          strcpy(ptr->ip, json_object_get_string(ip));
          strcpy(ptr->client_state, json_object_get_string(state));
          strcpy(ptr->auth_state, json_object_get_string(auth));

          ptr = ptr->next;
          ptr->next = NULL;
        }
        break;
    }
  }
}

void get_splash_clients(struct splash_list **buf)
{
  // Should check which captive portal daemon is running
  FILE *fp;
  char path[1035];
  path[0] = '\0';
  json_object *json_sessions = NULL;

  fp = popen("chilli_query -json list", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    return;
  }

  struct splash_list *ptr;
  ptr = malloc(sizeof(struct splash_list));

  if (ptr == NULL) {
    printf("Out of memory\n");
    return;
  }

  ptr->next = NULL;
  splash_conductor = ptr;

  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    json_sessions = json_tokener_parse(path);
    if (is_error(json_sessions)) {
      pclose(fp);
      free(ptr);
      return;
    }
  }
  pclose(fp);

  if (path[0] == '\0') {
    free(ptr);
    return;
  } else if (json_sessions) {
    parse_splash_clients(json_sessions, ptr);
  }

  *buf = splash_conductor;
  if (json_sessions)
    json_object_put(json_sessions);
}

const struct splash_ops splash_exec = {
  .clients       = get_splash_clients,
};
