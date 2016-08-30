#ifndef INTERFACE_STATS
#define INTERFACE_STATS
struct InterfaceStats
{
  uint64_t tx, rx, txerrors, rxerrors;
};

typedef struct {
  char ip[255];
  char if_name[24];
} defaultRoute;

#endif

int route();
int monitor_interface();
void interface_ip(char *interface, char *wan_ip);
void interface_stats(char *interface, uint64_t *a, uint64_t *b);
struct InterfaceStats stats();
