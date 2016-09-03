#include <stdio.h>
#include <stdint.h>
#include <netlink/attr.h>
#include "iw.h"

void format_bssid(uint8_t *mac, char *buf)
{
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int mac_addr_a2n(unsigned char *mac_addr, char *arg)
{
  int i;

  for (i = 0; i < ETH_ALEN ; i++) {
    int temp;
    char *cp = strchr(arg, ':');
    if (cp) {
      *cp = 0;
      cp++;
    }
    if (sscanf(arg, "%x", &temp) != 1)
      return -1;
    if (temp < 0 || temp > 255)
      return -1;

    mac_addr[i] = temp;
    if (!cp)
      break;
    arg = cp;
  }
  if (i < ETH_ALEN - 1)
    return -1;

  return 0;
}
