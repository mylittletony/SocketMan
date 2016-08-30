#ifndef INTERFACE_STATS
#define INTERFACE_STATS
struct InterfaceStats
{
  uint64_t tx, rx, txerrors, rxerrors;
};

struct defaultRoute {
  char ip[255];
  char if_name[24];
};

#endif

struct defaultRoute route();
int monitor_interface();
void interface_ip(char *interface, char *wan_ip);
void interface_stats(char *interface, uint64_t *a, uint64_t *b);
struct InterfaceStats stats();
