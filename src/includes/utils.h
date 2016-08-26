#ifndef __CTUTILS_H_
#define __CTUTILS_H_

#include "stdint.h"

void format_bssid(uint8_t *mac, char *buf);

struct dhcp_ops {
  int (*clients)(char *, int *);
};

const struct dhcp_ops dhcp_exec;

struct dhcp_list {
  char mac[18];
  char ip[20];
  char name[255];
  struct dhcp_list *next;
}*conductor;

#endif
