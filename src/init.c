#include <unistd.h>
#include "dbg.h"
#include "helper.h"
#include "http.h"

void init()
{
  char mac[17];
  mac[0]= '\0';

  read_mac(mac);

  if (mac[0] == '\0') {
    debug("No mac found in file %s", "/etc/mac");
    return;
  }

  if (valid_mac(mac) == 0) {
    debug("Invalid MAC address %s", mac);
    return;
  }

  do {
    if (run_init("f", "m", mac) == 1) {
      break;
    }
    sleep(5);
  }
  while(1);
}
