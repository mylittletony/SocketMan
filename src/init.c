#include <unistd.h>
#include "dbg.h"
#include "helper.h"
#include "http.h"

int init()
{
  char mac[17];
  mac[0]= '\0';

  readlineToBuffer("/etc/mac", mac);

  if (mac[0] == '\0') {
    debug("No mac found in file %s", "/etc/mac");
    return 0;
  }

  if (valid_mac(mac) == 0) {
    debug("Invalid MAC address %s", mac);
    return 0;
  }

  do {
    if (run_init("f", "m", mac) == 1) {
      break;
    }
    sleep(15);
  }
  while(1);
  return 1;
}
