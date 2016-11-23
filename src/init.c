#include <unistd.h>
#include "dbg.h"
#include "helper.h"
#include "http.h"
#include "system.h"

int init()
{
  char mac[20];
  mac[0]= '\0';

  char firmware[50];
  firmware[0]= '\0';

  char machine[100];
  machine[0]= '\0';

  readlineToBuffer("/etc/mac", mac);

  if (mac[0] == '\0') {
    debug("No mac found in file %s", "/etc/mac");
    return 0;
  }

  // Should get WAN MAC //

  if (valid_mac(mac) == 0) {
    debug("Invalid MAC address %s", mac);
    return 0;
  }

  machine_type(machine, sizeof(machine));

#ifdef __OPENWRT__
  readlineToBuffer("/etc/openwrt_version", firmware);
#endif

  if (firmware[0] == '\0') {
    strcpy(firmware, "DNE");
  }

  debug("F: %s, M: %s MAC: %s", firmware, machine, mac);
  do {
    if (run_init(firmware, machine, mac) == 1) {
      break;
    }
    sleep(15);
  }
  while(1);
  return 1;
}
