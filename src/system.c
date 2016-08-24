#include "dbg.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <inttypes.h>
#include "system.h"
#include "options.h"
/* #include <sys/types.h> */
/* #include <sys/stat.h> */
#include <fcntl.h>

void reboot() {
  debug("Rebooooottting!");
  debug("Rebooooottting!");
  debug("Rebooooottting!");
  debug("Rebooooottting!");
  system("reboot");
}

void machine_type(char *type)
{
  if (strlen(options.machine) > 0) {
    strcpy(type, options.machine);
  } else {
    FILE* fp;
    size_t bytes_read;
    char *match;
    char buffer[1024];
#ifdef __OPENWRT__
    fp = fopen ("/proc/cpuinfo", "r");
#elif __linux
    fp = fopen ("/etc/os-release", "r");
#endif
    bytes_read = fread (buffer, 1, sizeof (buffer), fp);
    fclose (fp);

    if (bytes_read == 0 || bytes_read == sizeof (buffer))
      return;

    buffer[bytes_read] = '\0';
#ifdef __OPENWRT__
    match = strstr (buffer, "machine");
    if (match == NULL)
      return;
    sscanf (match, "machine : %[^\t\n]", type);
#elif __linux
    match = strstr(buffer, "NAME");

    if (match == NULL)
      return;
    sscanf(match, "NAME=\"%[^\t\n]\"", type);
#endif
  }
}

void clear_caches()
{
  int fd;
  char* data = "3";

  sync();
  fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
  write(fd, data, sizeof(char));
  close(fd);
}

struct SystemInfo system_info() {
  struct sysinfo info;
  if ( sysinfo (&info) != -1 ){

    double pf = (double)info.freeram / (double)info.totalram;
    struct SystemInfo s;
    s.uptime = info.uptime;
    s.uptime = info.uptime;
    s.totalram = info.totalram;
    s.freeram = info.freeram;
    s.percent_used = pf;
    s.procs = info.procs;
    s.load_1 = info.loads[0] / LINUX_SYSINFO_LOADS_SCALE;
    s.load_5 = info.loads[1] / LINUX_SYSINFO_LOADS_SCALE;
    s.load_15 = info.loads[2] / LINUX_SYSINFO_LOADS_SCALE;
    return s;
  }
}
