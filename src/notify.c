#include "dbg.h"
#include <stdbool.h>
#include "http.h"
#include "operations.h"

void cmd_notify(int response, char *id, char *buf)
{
  bool r = response == 0 ? true : false;
  debug("Sending %s message notification", r ? "success" : "failure" );
  json_object *jobj = json_object_new_object();

  json_object *jresp = json_object_new_boolean(r);
  json_object_object_add(jobj, "success", jresp);

  json_object *jbuf = json_object_new_string(buf);
  json_object_object_add(jobj, "msg", jbuf);

  // Should be moved into separate project
  update_operation(jobj, id);

  json_object_put(jobj);
}
