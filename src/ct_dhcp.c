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

  struct dhcp_list *ptr;
  ptr = malloc(sizeof(struct dhcp_list));

  ptr->next = NULL;
  conductor = ptr;

  fp = fopen(DHCP_LEASES, "r");
  if(NULL == fp)
    return;

  int created;
  char mask[100];
  ssize_t read;

  while ((read = getline(&line, &len, fp)) != -1) {
    ptr->next = malloc(sizeof(struct dhcp_list));
    if (ptr->next == NULL) break;

    sscanf(line, "%d %s %s %s %s\n",
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
