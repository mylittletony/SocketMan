#ifndef __CTUTILS_H_
#define __CTUTILS_H_

struct dhcp_ops {
  int (*clients)(char *, int *);
};

const struct dhcp_ops dhcp_exec;

struct dhcp_entry {
  /* char *mac; */
  unsigned char mac[17];
};

/* struct dhcp_list { */
/*   struct dhcp_entry *c; */
/*   int len; */
/* }; */

struct dhcp_list {
  char mac[20];
  int len;
  struct dhcp_list *next;
}*conductor, *test;

#endif
