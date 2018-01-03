#include "mqtt.h"
#include "http.h"
#include "options.h"

// Only send the data once per heartbeat cycle
int last_ping = 0;
int should_ping() {
  int sleep = options.ping_interval;
  time_t now = time(NULL);
  int diff = now - last_ping;
  if (last_ping == 0 || diff >= sleep) {
    last_ping = time(NULL);
    return 1;
  }
  return 0;
}

void ping()
{
  if (options.noping) {
    return;
  }

  if (should_ping() == 0)
    return;

  ping_mqtt();
}
