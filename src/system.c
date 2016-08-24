#include "dbg.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <inttypes.h>
#include "system.h"
#include "options.h"

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

struct SystemInfo system_info() {
  struct sysinfo info;
  sysinfo (&info);
  struct SystemInfo s = { info.uptime, info.totalram, info.freeram, info.procs };
  return s;
}
