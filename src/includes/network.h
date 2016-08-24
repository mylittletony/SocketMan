#ifndef INTERFACE_STATS
#define INTERFACE_STATS
struct InterfaceStats
{
  uint64_t tx, rx, txerrors, rxerrors;
};
#endif

int route();
int monitor_interface();
char gateway[255];
char wan_name[5];
void interface_ip(char *interface, char *wan_ip);
void interface_stats(char *interface, uint64_t *a, uint64_t *b);
struct InterfaceStats stats();
