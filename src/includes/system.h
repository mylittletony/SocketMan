#ifndef SYSTEM_STATS
#define SYSTEM_STATS
#define LINUX_SYSINFO_LOADS_SCALE 65536.0

struct SystemInfo
{
  uint64_t uptime;
  double totalram, freeram;
  int procs;
  double load_1;
  double load_5;
  double load_15;
};
#endif

void reboot();
void machine_type(char *type);
struct SystemInfo system_info();
