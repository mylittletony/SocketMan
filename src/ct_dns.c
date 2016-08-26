#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "dbg.h"
#include "platform.h"

int get_clients_cb(void *buf) {
  FILE *fp;
  char line[128];
  /* struct dhcp_list *current, *head; */

  struct dhcp_list *ptr;
  ptr = (struct dhcp_list*)malloc(sizeof(struct dhcp_list));
  ptr->next = NULL;
  conductor = ptr;

  fp = fopen(DHCP_LEASES, "r");

  char created[10];
  char ip_address[24];
  char device_name[255];
  char mask[500];

  /* char buffer[1024]; */
  /* memset(line, 0, 1024); */

  while(fgets(line, sizeof(line), fp) != NULL){
    ptr->next = malloc(sizeof(struct dhcp_list));
    if (ptr->next == NULL) break;

    sscanf(line, "%s %s %s %s %s\n",
        created,
        ptr->mac,
        ip_address,
        device_name,
        mask);

    debug("xxxxxxxxxxxx: %s", ptr->mac);
    ptr = ptr->next;
    ptr->next = NULL;

    /* if(test == NULL){ */
    /*   conductor = test = ptr; */
    /* } else { */
    /*   conductor = conductor->next = ptr; */
    /* } */

  }
  fclose(fp);
  //test print

  /* while(temp!=NULL) */
  /* { */
  /*   printf("MAC ADDRESS: %s\n", temp->mac); */
  /*   temp = temp->next; */
  /* } */

  struct dhcp_list *current = conductor;
  for(current = conductor; current; current=current->next){
    if (current->next != NULL)
      printf("MAC ADDRESS: %s\n", current->mac);
  }

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
  /* struct dhcp_list cl = { .next = (struct dhcp_list*)buf }; */
  /* struct dhcp_list cl = { .c = (struct dhcp_entry *)buf }; */
  get_clients_cb(buf);
  return 1;
  /* *len = cl.len * sizeof(struct dhcp_entry); */
  /* return *len ? 1 : 0; */
}

const struct dhcp_ops dhcp_exec = {
  .clients       = get_clients,
};
