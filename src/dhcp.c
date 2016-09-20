#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "dbg.h"
#include "platform.h"

void get_clients(struct dhcp_list **buf)
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  fp = fopen(DHCP_LEASES, "r");
  if(fp == NULL)
    return;

  int created;
  char mask[100];
  ssize_t read;

  struct dhcp_list *ptr;
  ptr = malloc(sizeof(struct dhcp_list));

  if(ptr == NULL){
      fclose(fp);
      return;
  }

  ptr->next = NULL;
  conductor = ptr;

  while ((read = getline(&line, &len, fp)) != -1) {
    ptr->next = malloc(sizeof(struct dhcp_list));
    if (ptr->next == NULL) break;

    sscanf(line, "%d %17s %19s %254s %99s\n",
        &created,
        ptr->mac,
        ptr->ip,
        ptr->name,
        mask);

    ptr = ptr->next;
    ptr->next = NULL;
  }
  fclose(fp);

  if (line != NULL)
    free(line);

  *buf = conductor;
}

const struct dhcp_ops dhcp_exec = {
  .clients       = get_clients,
};
