#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "dbg.h"
#include "platform.h"

int get_clients_cb(void *buf) {
  FILE *fp;
  char line[128];
  /* struct dhcp_list *current, *head; */

  struct dhcp_list *ptr = buf;
  /* ptr = (struct dhcp_list*)malloc(sizeof(struct dhcp_list)); */
  ptr->next = NULL;
  conductor = ptr;

  fp = fopen(DHCP_LEASES, "r");

  char created[10];
  char ip_address[24];
  char device_name[255];
  char mask[5];

  while(fgets(line, sizeof(line), fp) != NULL){
    ptr->next = malloc(sizeof(struct dhcp_list));
    if (ptr->next == NULL) break;

    sscanf(line, "%s %s %s %s %s\n",
        created,
        ptr->mac,
        ptr->ip,
        ptr->name,
        mask);

    ptr = ptr->next;
    ptr->next = NULL;

  }
  fclose(fp);

  //need free for each node
  //need free for each node
  //need free for each node
  //need free for each node
  //need free for each node

  return 0;
}

struct list {
  char *string;
  struct list *next;
};

typedef struct dhcp_list LIST;

int get_clients(char *buf, int *len)
{
  get_clients_cb(buf);
  return 1;
}

const struct dhcp_ops dhcp_exec = {
  .clients       = get_clients,
};
