#include "dbg.h"
#include <stdbool.h>
#include "http.h"

void cmd_notify(int response, char *id, char *buf)
{
  bool r = response == 0 ? true : false;
  debug("Sending %s message notification", r ? "success" : "failure" );
  json_object *jobj = json_object_new_object();
  json_object *jattr = json_object_new_object();

  json_object *jresp = json_object_new_boolean(r);
  json_object_object_add(jattr, "success", jresp);

  if (id != NULL) {
    json_object *jid = json_object_new_string(id);
    json_object_object_add(jattr, "id", jid);
  }

  if (buf != NULL && *buf != '\0') {
    json_object *jbuf = json_object_new_string(buf);
    json_object_object_add(jattr, "output", jbuf);
  }

  json_object_object_add(jobj, "message", jattr);

  post(jobj);

  json_object_put(jobj);
}
