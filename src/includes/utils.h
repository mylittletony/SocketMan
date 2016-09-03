#ifndef __CTUTILS_H_
#define __CTUTILS_H_

#include "stdint.h"
#include <json-c/json.h>

void format_bssid(uint8_t *mac, char *buf);

const struct dhcp_ops dhcp_exec;
const struct splash_ops splash_exec;

struct dhcp_list {
  char mac[18];
  char ip[20];
  char name[255];
  struct dhcp_list *next;
}*conductor;

struct dhcp_ops {
  void (*clients)(struct dhcp_list **);
};

struct splash_list {
  char mac[18];
  char ip[20];
  char client_state[5];
  char auth_state[5];
  struct splash_list *next;
}*splash_conductor;

struct splash_ops {
  void (*clients)(struct splash_list **);
};

int mac_addr_a2n(unsigned char *mac_addr, char *arg);

#endif
