#ifndef INTERFACE_STATS
#define INTERFACE_STATS
struct InterfaceStats
{
  uint64_t tx, rx, txerrors, rxerrors;
};

struct defaultRoute {
  char ip[24];
  char if_name[24];
};

struct defaultRoute route();
int monitor_interface();
void interface_ip(char *interface, char *wan_ip, size_t len);
void interface_stats(char *interface, uint64_t *a, uint64_t *b);
struct InterfaceStats stats();
void recover_network();
void restart_network();

#endif
