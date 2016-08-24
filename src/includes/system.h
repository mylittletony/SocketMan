#ifndef SYSTEM_STATS
#define SYSTEM_STATS
struct SystemInfo
{
  uint64_t uptime;
  double totalram, freeram;
  int procs;
/* #, rx, txerrors, rxerrors; */
};
#endif

void reboot();
void machine_type(char *type);
struct SystemInfo system_info();
