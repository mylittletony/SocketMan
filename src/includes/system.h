#ifndef SYSTEM_STATS
#define SYSTEM_STATS
#define LINUX_SYSINFO_LOADS_SCALE 65536.0
#include <inttypes.h>
#include <stddef.h>

struct SystemInfo
{
  uint64_t uptime;
  float totalram;
  float freeram;
  double percent_used;
  int procs;
  double load_1;
  double load_5;
  double load_15;
};
#endif

int reboot();
void machine_type(char *type, size_t len);
void clear_caches();
struct SystemInfo system_info();
