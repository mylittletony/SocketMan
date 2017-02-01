#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <resolv.h>
#include <netinet/tcp.h>
#include "dbg.h"
#include "options.h"
#include "tools.h"
#include "system.h"
#include "collector.h"
#include "platform.h"
#include "helper.h"
#include "ping.h"

// When offline, the interval for re-check
#define OFFLINE_INTERVAL 10

// Max time to wait before restarting the network
#define MAX_INTERVAL 600

// The delay between network restarts when offline
int delay = 5;

time_t went_offline;
int tried_backup;

void restart_or_reboot();
void check_dns_and_recover();
void check_connection();
void monitor();
/* void recover_connection(); */

void reset()
{
  went_offline = 0;
  delay = 5;
}

void monitor()
{
  int rc = 0;

  // Should run the check, nothing else
  rc = connection_check();

  // Go offline - can happen before the init. loop
  check_connection(rc);

  // Will exit fast if the device isn't initialized. Helps fast init.
  if (!options.initialized) {
    return;
  }

  // Tests the connection every 2 heartbeats
  ping();

  // Collect the data and send to API
  collect_and_send_data(rc);

  // Sleep for monitor interval
  debug("Sleeping for %d seconds.", options.monitor);
  sleep(options.monitor);
}

void check_connection(int reason) {
  if (reason > 0) {
    return;
  }

  if (went_offline == 0)
    went_offline = time(NULL);

  debug("Device went offline at %lld. Reason code %d, attemping recovery.", (long long) went_offline, reason);

  /* restart_or_reboot(); */
}

/* void restore_original() */
/* { */
/*   if ((remove(NETWORK_FILE) == 0) && (rename(NETWORK_ORIGINAL, NETWORK_FILE) == 0)) {; */
/*     debug("Restoring original network"); */
/*     return; */
/*   } */
/*   if (copy_file(NETWORK_BAK, NETWORK_FILE) != 1) { */
/*     if (access(NETWORK_FILE, F_OK ) == -1) */
/*       recover_network(); */
/*     debug("I HAVE NO NETWORK FILE!"); */
/*   } */
/* } */

/* int should_recover() */
/* { */
/*   if (file_present(NETWORK_ORIGINAL)) { */
/*     debug("I AM GOING TO RESTORE THE ORIGINAL FILE!"); */
/*     restore_original(); */
/*   } else if (!tried_backup && file_present(NETWORK_BAK)) { */
/*     tried_backup = 1; */
/*     return 1; */
/*   } */
/*   return 0; */
/* } */

/* void recover_network_config() */
/* { */
/*   if (copy_file(NETWORK_FILE, NETWORK_ORIGINAL)) */
/*     debug("Backing up the original network file"); */
/*   copy_file(NETWORK_BAK, NETWORK_FILE); */
/* } */

/* void attempt_recovery() */
/* { */
/*   if (should_recover()) */
/*     debug("Trying to connect with the backup config."); */
/*   recover_network_config(); */
/* } */

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

/* int network_restart() { */
/*   if (should_restart_network()) */
/*     restart_network(); */
/*   return 0; */
/* } */

void restart_or_reboot()
{
  /* if (should_reboot()) { */
  /*   if (reboot() != 0) { */
  /*     debug("Could not restart the device."); */
  /*     reset(); */
  /*   } */
  /*   return; */
  /* } else { */
  /*   network_restart(); */
  /* } */
  /* if (delay <= MAX_INTERVAL) */
  /*   delay *= 2; */

  /* debug("Network restart delay set to %d seconds. Sleeping for %d before connection check.", delay, OFFLINE_INTERVAL); */
  /* sleep(OFFLINE_INTERVAL); */
  /* monitor(); */
}

void recover_connection() {
  debug("Connection recovered!");
  reset();
}

void backup_config()
{
  // TODO Needs to NOT backup each time
  if (copy_file(NETWORK_FILE, NETWORK_BAK)) {
    debug("Backing up %s config to %s ", NETWORK_FILE, NETWORK_BAK);
    return;
  }
  debug("Could not backup the network config!");
  return;
}

/* void heartbeat(int offline_reason) */
/* { */
/*   // Better logic here - we now run each time, even if offline */
/*   if (went_offline == 0) */
/*     debug("Connection check passed, all systems go."); */
/*   /1* } else { *1/ */
/*   /1*   recover_connection(); *1/ */
/*   /1* } *1/ */
/*   /1* backup_config(); *1/ */

/*   // Will exit fast if the device isn't initialized. Helps fast init. */
/*   if (options.initialized) { */
/*     collect_and_send_data(offline_reason); */
/*     debug("Sleeping for %d seconds.", options.monitor); */
/*   } */
/* } */
