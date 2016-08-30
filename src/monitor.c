#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <resolv.h>
#include <netinet/tcp.h>
#include "dbg.h"
#include "options.h"
#include "network.h"
#include "tools.h"
#include "collector.h"
#include "platform.h"
#include "helper.h"

// When offline, the interval for re-check
#define OFFLINE_INTERVAL 10

// Max time to wait before restarting the network
#define MAX_INTERVAL 300

// The delay between connection checks when offline
int delay = 5;

time_t went_offline;
int online, tried_backup;

void restart_or_reboot();
void heartbeat();
void attempt_recovery();
void check_dns_and_recover();
void go_offline();
void monitor();
void recover_connection();

void run_monitor()
{
  struct defaultRoute dr = route();

  debug("Running the network monitor");
  if (strlen(dr.ip) != 0)
  {
    debug("Default route: %s", dr.ip);
    if ( connection_check() )
    {
      if (online) {
        debug("CONNECTION WORKING !!!!");
      } else {
        recover_connection();
      }
      heartbeat();
      return;
    }
  } else {
    debug("NO ROUTE");
  }
  go_offline();
}

void go_offline() {
  online = 0;
  debug("DEVICE OFFLINE");
  if (went_offline == 0) {
    went_offline = time(NULL);
    /* debug("Setting offline to %lld", went_offline); */
  }
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

int should_restart_network() {
  time_t now = time(NULL);
  return (delay >= MAX_INTERVAL) || ((now - went_offline) >= delay);
}

int network_restart() {
  if (should_restart_network())
    restart_network();
}

void restart_or_reboot()
{
  if (options.reboot > 60 && (delay >= options.reboot)) {
    reboot();
    return;
  } else {
    network_restart();
  }
  delay *= 2;
  debug("Delay now set to %d seconds. Sleeping for %d before re-check.", delay, OFFLINE_INTERVAL);
  sleep(OFFLINE_INTERVAL);
  monitor();
}

void recover_connection() {
  debug("CONNECTION RECOVERED!");
  delay = 5;
  went_offline = 0;
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
  backup_config();
  collect_and_send_data(online);

  debug("Sleeping for %d seconds.", options.monitor);
  sleep(options.monitor);
  monitor();
}

void monitor()
{
  // remove this when not testing collector
  /* online = 1; */
  /* collect_and_send_data(online); */
  // remove this when not testing collector

  do
  {
    online = 1;
    run_monitor();
  }
  while(1);
}
