#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "dbg.h"
#include "platform.h"

void get_clients_cb(void *buf) {
  FILE *fp;
  char line[128];

  struct dhcp_list *ptr = buf;
  /* struct dhcp_list *ptr; */
  /* ptr = (struct dhcp_list*)malloc(sizeof(struct dhcp_list)); */

  ptr->next = NULL;
  conductor = ptr;

  fp = fopen(DHCP_LEASES, "r");
  // error check

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
    /* free(ptr->next); // ? */

  }
  fclose(fp);

  /* free(conductor); */

  /* struct dhcp_list *freeMe = conductor; */
  /* struct dhcp_list *holdMe = NULL; */
  /* while(freeMe->next != NULL) { */
  /*   holdMe = freeMe->next; */
  /*   free(freeMe); */
  /*   freeMe = holdMe; */
  /* } */
  /* free(ptr); */

  //need free for each node
  //need free for each node
  //need free for each node
  //need free for each node
  //need free for each node
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
