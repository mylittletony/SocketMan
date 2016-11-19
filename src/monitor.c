#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <resolv.h>
#include <netinet/tcp.h>
#include "dbg.h"
#include "options.h"
#include "network.h"
#include "tools.h"
#include "system.h"
#include "collector.h"
#include "platform.h"
#include "helper.h"

// When offline, the interval for re-check
#define OFFLINE_INTERVAL 10

// Max time to wait before restarting the network
#define MAX_INTERVAL 600

// The delay between network restarts when offline
int delay = 5;

time_t went_offline;
int tried_backup;

void restart_or_reboot();
void heartbeat();
void attempt_recovery();
void check_dns_and_recover();
void go_offline();
void monitor();
void recover_connection();

void reset()
{
  went_offline = 0;
  delay = 5;
}

void monitor()
{
  int rc = 0;
  struct defaultRoute dr = route();

  debug("Running the network monitor");
  if (strlen(dr.ip) != 0)
  {
    debug("Default route: %s", dr.ip);
    rc = connection_check();
    if (rc == 0)
    {
      heartbeat();
      return;
    }
  } else {
    debug("No route found");
  }
  go_offline(rc);
}

void go_offline(int reason) {
  int online = 0;
  if (went_offline == 0)
    went_offline = time(NULL);

  debug("Device went offline at %lld. Reason code %d, attemping recovery.", (long long) went_offline, reason);

  attempt_recovery();
  collect_and_send_data(online);
  restart_or_reboot();
}

void restore_original()
{
  if ((remove(NETWORK_FILE) == 0) && (rename(NETWORK_ORIGINAL, NETWORK_FILE) == 0)) {;
    debug("Restoring original network");
    return;
  }
  if (copy_file(NETWORK_BAK, NETWORK_FILE) != 1) {
    if (access(NETWORK_FILE, F_OK ) == -1)
      recover_network();
    debug("I HAVE NO NETWORK FILE!");
  }
}

int should_recover()
{
  if (file_present(NETWORK_ORIGINAL)) {
    debug("I AM GOING TO RESTORE THE ORIGINAL FILE!");
    restore_original();
  } else if (!tried_backup && file_present(NETWORK_BAK)) {
    tried_backup = 1;
    return 1;
  }
  return 0;
}

void recover_network_config()
{
  if (copy_file(NETWORK_FILE, NETWORK_ORIGINAL))
    debug("Backing up the original network file");
  copy_file(NETWORK_BAK, NETWORK_FILE);
}

void attempt_recovery()
{
  if (should_recover())
    debug("Trying to connect with the backup config.");
  recover_network_config();
}

int should_reboot()
{
  if (options.reboot > 0 && went_offline > 0)
    return (time(NULL) - went_offline) >= options.reboot;
  return 0;
}

int should_restart_network()
{
  if ((delay >= MAX_INTERVAL) ||
    ((time(NULL) - went_offline) >= delay)) {
      debug("Restarting network after %d second delay", delay);
      return 1;
    }
  return 0;
}

int network_restart() {
  if (should_restart_network())
    restart_network();
  return 0;
}

void restart_or_reboot()
{
  if (should_reboot()) {
    if (reboot() != 0) {
      debug("Could not restart the device.");
      reset();
    }
    return;
  } else {
    network_restart();
  }
  if (delay <= MAX_INTERVAL)
    delay *= 2;

  debug("Network restart delay set to %d seconds. Sleeping for %d before connection check.", delay, OFFLINE_INTERVAL);
  sleep(OFFLINE_INTERVAL);
  monitor();
}

void recover_connection() {
  debug("Connection recovered!");
  reset();
}

void backup_config()
{
  if (copy_file(NETWORK_FILE, NETWORK_BAK)) {
    debug("Backing up %s config to %s ", NETWORK_FILE, NETWORK_BAK);
    return;
  }
  debug("Could not backup the network config!");
}

void heartbeat()
{
  if (went_offline == 0) {
    debug("Connection check passed, all systems go.");
  } else {
    recover_connection();
  }
  backup_config();
  collect_and_send_data(1);

  if (options.initialized == 1) {
    debug("Sleeping for %d seconds.", options.monitor);
    sleep(options.monitor);
  }
}
