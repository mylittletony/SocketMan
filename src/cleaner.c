#include "system.h"
#include "dbg.h"
#include <unistd.h>
#include <fcntl.h>

void clear_caches()
{
  int fd;
  char* data = "3";

  sync(); // Not sure we should include, this increases the # of files
  fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
  if (fd != -1) {
    write(fd, data, sizeof(char));
    close(fd);
  }
}

void run_cleanup(struct SystemInfo info)
{
  if (info.percent_used >= (float)0.90) {
    int pu = (float)info.percent_used * 100;
    debug("MEMORY USAGE AT %d%% WIPING CACHES", pu);
    clear_caches();
  }
}

