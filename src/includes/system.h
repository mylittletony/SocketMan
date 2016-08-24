#ifndef SYSTEM_STATS
#define SYSTEM_STATS
#define LINUX_SYSINFO_LOADS_SCALE 65536.0

struct SystemInfo
{
  uint64_t uptime;
  double totalram;
  double freeram;
  double percent_used;
  int procs;
  double load_1;
  double load_5;
  double load_15;
};
#endif

void reboot();
void machine_type(char *type);
void clear_caches();
struct SystemInfo system_info();
