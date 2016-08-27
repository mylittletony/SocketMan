#include <stdio.h>
#include <stdint.h>

void format_bssid(uint8_t *mac, char *buf)
{
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
