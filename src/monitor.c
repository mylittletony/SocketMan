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
#include "network.h"
#include "helper.h"
#include "ping.h"

// When offline, the interval for re-check
#define OFFLINE_INTERVAL 10

// Max time to wait before restarting the network
#define MAX_INTERVAL 600

// The delay between network restarts when offline
int delay = 5;
int network_restarted = 1;
time_t went_offline;
int tried_backup;

void restart_or_reboot();
void check_dns_and_recover();
void check_connection();
void monitor();

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
  if (!options.initialized)
    return;

  // Tests the connection every 2 heartbeats
  ping();

  // Collect the data and send to API
  collect_and_send_data(rc);

  // Sleep for monitor interval
  if (options.initialized) {
    if (options.debug) {
      debug("Sleeping for %d seconds.", options.monitor);
    }
    sleep(options.monitor);
  }
}

void check_connection(int reason) {

  if (options.debug && reason != 9) {
    debug("Connection status: %d", reason);
  }

  if (reason != 9) {
    debug("SocketMan connection issue detected!!! Code (%d)", reason)
  }

  // If no IP or DNS && HTTP fail we restart the network once
  // After N minutes, we restart the AP
  if (reason > 1) {
    return;
  }

  // Don't reboot if this is disabled!!
  if (options.reboot == 0) {
    return;
  }

  if (went_offline == 0) {
    went_offline = time(NULL);
  }

  debug("Device went offline at %lld. Reason code %d, attemping recovery.", (long long) went_offline, reason);

  restart_or_reboot();
}

int should_reboot()
{
  if (went_offline > 0)
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
  }

  if (network_restarted == 0) {
    network_restarted = 1;
    network_restart();
  }

  if (delay <= MAX_INTERVAL)
    delay *= 2;
}
