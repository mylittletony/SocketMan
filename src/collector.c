#include <unistd.h>
#include <resolv.h>
#include <netinet/tcp.h>
#include <time.h>
#include "monitor.h"
#include <json-c/json.h>
#include "options.h"
#include "network.h"
#include "system.h"
#include "helper.h"
#include "phy.h"
#include "dbg.h"
#include "cleaner.h"
#include "dhcp.h"
#include "utils.h"
#include <ctype.h>
#include "http.h"

time_t last_collect = 0;
int collected;

struct radio_list *curr, *head;

struct radio_list
{
  int val;
  char *ifname;
  struct radio_list *next;
};

int should_collect() {
  time_t now = time(NULL);
  int diff = now - last_collect;
  if (last_collect == 0 || diff >= options.heartbeat) {
    last_collect = time(NULL);
    return 1;
  } else {
    debug("Running now for %d seconds", diff);
  }
  return 0;
}

void format_ssids(const struct iw_ops *iw,
    struct iw_ssid_entry *e,
    json_object *jssids, int len)
{

  int noise, signal = 0, quality, quality_max, bitrate;
  //int txpower, channel;
  static char bssid[18] = { 0 };
  char ssid[ESSID_MAX_SIZE+1] = { 0 };
  char *interface = e->ifname;

  json_object *jiface = json_object_new_string(interface);
  json_object_object_add(jssids, "interface", jiface);

  json_object *jchannel = json_object_new_int(e->channel);
  json_object_object_add(jssids, "channel", jchannel);

  if (iw->noise(interface, &noise)) {
    json_object *jnoise = json_object_new_int(noise);
    json_object_object_add(jssids, "noise", jnoise);
  }

  if (len > 0) {
    if (iw->signal(interface, &signal)) {
      json_object *jsignal = json_object_new_int(signal);
      json_object_object_add(jssids, "signal", jsignal);
    }
    // THE QUALITY CAN BE NULL SOMETIMES - IE. MON INTERFACE //
    if (signal > 0 && iw->quality(signal, &quality)) {
      json_object *jquality = json_object_new_int(quality);
      json_object_object_add(jssids, "quality", jquality);
    }
  }

  if (iw->bitrate(interface, &bitrate)) {
    json_object *jbitrate = json_object_new_int(bitrate);
    json_object_object_add(jssids, "bitrate", jbitrate);
  }

  if (iw->quality_max(&quality_max)) {
    json_object *jquality_max = json_object_new_int(quality_max);
    json_object_object_add(jssids, "quality_max", jquality_max);
  }

  if (iw->bssid(interface, bssid)) {
    json_object *jbssid = json_object_new_string(bssid);
    json_object_object_add(jssids, "bssid", jbssid);
  }

  if (iw->ssid(interface, ssid)) {
    json_object *jssid = json_object_new_string(ssid);
    json_object_object_add(jssids, "ssid", jssid);
  }

  // Problem on Debian!
  /* if (iw->txpower(interface, &txpower)) { */
  /*   json_object *jtxpower = json_object_new_int(txpower); */
  /*   json_object_object_add(jssids, "txpower", jtxpower); */
  /* } */

  // Not complete
  /* if (iw->encryption(interface, encryption)) { */
  /*   json_object *jencryption = json_object_new_string(encryption); */
  /*   json_object_object_add(*jssids, "encryption", jencryption); */
  /* } */
}

void format_stations(const char *ssid,
    const char *ifname, struct iw_stationlist_entry *s, json_object *jstations
    )
{
  static char buf[18];

  json_object *jiface = json_object_new_string(ifname);
  json_object_object_add(jstations, "interface", jiface);

  json_object *jssid = json_object_new_string(ssid);
  json_object_object_add(jstations, "ssid", jssid);

  format_bssid(s->mac, buf);

  json_object *jmac = json_object_new_string(buf);
  json_object_object_add(jstations, "mac", jmac);

  json_object *jinactive = json_object_new_int(s->inactive_time);
  json_object_object_add(jstations, "inactive_time", jinactive);

  json_object *jrx_packets = json_object_new_double(s->rx_packets);
  json_object_object_add(jstations, "rx_packets", jrx_packets);

  json_object *jtx_packets = json_object_new_double(s->tx_packets);
  json_object_object_add(jstations, "tx_packets", jtx_packets);

  if (s->rx_bytes_64 >0) {
    json_object *jrx_bytes = json_object_new_double(s->rx_bytes_64);
    json_object_object_add(jstations, "rx_bytes", jrx_bytes);
  }
  else {
    json_object *jrx_bytes = json_object_new_double(s->rx_bytes);
    json_object_object_add(jstations, "rx_bytes", jrx_bytes);
  }

  if (s->tx_bytes_64 >0) {
    json_object *jtx_bytes = json_object_new_double(s->tx_bytes_64);
    json_object_object_add(jstations, "tx_bytes", jtx_bytes);
  } else {
    json_object *jtx_bytes = json_object_new_double(s->tx_bytes);
    json_object_object_add(jstations, "tx_bytes", jtx_bytes);
  }

  json_object *jbeacon_rx = json_object_new_double(s->beacon_rx);
  json_object_object_add(jstations, "beacon_rx", jbeacon_rx);

  json_object *jtx_retries = json_object_new_int(s->tx_retries);
  json_object_object_add(jstations, "tx_retries", jtx_retries);

  json_object *jtx_failed = json_object_new_int(s->tx_failed);
  json_object_object_add(jstations, "tx_failed", jtx_failed);

  json_object *jbeacon_loss = json_object_new_int(s->beacon_loss);
  json_object_object_add(jstations, "beacon_loss", jbeacon_loss);

  json_object *jsignal = json_object_new_int(s->signal);
  json_object_object_add(jstations, "signal", jsignal);

  json_object *jsignal_avg = json_object_new_int(s->signal_avg);
  json_object_object_add(jstations, "signal_avg", jsignal_avg);

  if (s->beacon_signal_avg > 0) {
    json_object *jbeacon_signal_avg = json_object_new_int(s->beacon_signal_avg);
    json_object_object_add(jstations, "beacon_signal_avg", jbeacon_signal_avg);
  }

  if (s->t_offset > 0) {
    json_object *jt_offset = json_object_new_double(s->t_offset);
    json_object_object_add(jstations, "t_offset", jt_offset);
  }

  json_object *jtx_bitrate = json_object_new_int(s->tx_bitrate);
  json_object_object_add(jstations, "tx_bitrate", jtx_bitrate);

  json_object *jrx_bitrate = json_object_new_int(s->rx_bitrate);
  json_object_object_add(jstations, "rx_bitrate", jrx_bitrate);

  json_object *jauthorized = json_object_new_boolean(s->authorized);
  json_object_object_add(jstations, "authorized", jauthorized);

  json_object *jauthenticated = json_object_new_boolean(s->authenticated);
  json_object_object_add(jstations, "authenticated", jauthenticated);

  json_object *jassociated = json_object_new_boolean(s->associated);
  json_object_object_add(jstations, "associated", jassociated);

  json_object *jwmm = json_object_new_boolean(s->wmm);
  json_object_object_add(jstations, "wmm", jwmm);

  json_object *jmfp = json_object_new_boolean(s->mfp);
  json_object_object_add(jstations, "mfp", jmfp);

  json_object *jtdls = json_object_new_boolean(s->tdls);
  json_object_object_add(jstations, "tdls", jtdls);

  json_object *jpreamble = json_object_new_int(s->preamble);
  json_object_object_add(jstations, "preamble", jpreamble);

  json_object *jconn_time = json_object_new_int(s->conn_time);
  json_object_object_add(jstations, "conn_time", jconn_time);

  json_object *jexpected_tput = json_object_new_int(s->expected_tput);
  json_object_object_add(jstations, "expected_tput", jexpected_tput);
}

void format_scan(struct iw_scanlist_entry *s, json_object *jscan)
{
  char enc[512];
  static char buf[18];

  format_bssid(s->mac, buf);

  json_object *jbssid = json_object_new_string(buf);
  json_object_object_add(jscan, "mac", jbssid);

  json_object *jchannel = json_object_new_int(s->channel);
  json_object_object_add(jscan, "channel", jchannel);

  json_object *jessid = json_object_new_string(s->ssid);
  json_object_object_add(jscan, "ssid", jessid);

  format_encryption(&s->crypto, enc);

  json_object *jcrypto = json_object_new_string(enc);
  json_object_object_add(jscan, "encryption", jcrypto);

  if (s->signal > 0) {
    int signal = s->signal - 0x100;
    json_object *jsignal = json_object_new_int(signal);
    json_object_object_add(jscan, "signal", jsignal);

    json_object *jquality = json_object_new_int(s->quality);
    json_object_object_add(jscan, "quality", jquality);

    json_object *jquality_max = json_object_new_int(s->quality_max);
    json_object_object_add(jscan, "quality_max", jquality_max);
  }

  json_object *jage = json_object_new_int(s->age);
  json_object_object_add(jscan, "last_seen", jage);
}

struct radio_list* create_list(struct iw_ssid_entry *e)
{
  struct radio_list *ptr = malloc(sizeof(struct radio_list));
  if(NULL == ptr)
  {
    printf("\n Node creation failed \n");
    return NULL;
  }
  ptr->val = e->phy;
  ptr->ifname = e->ifname;
  ptr->next = NULL;

  head = curr = ptr;
  return ptr;
}

struct radio_list* add_to_list(struct iw_ssid_entry *e)
{
  if(NULL == head)
  {
    return (create_list(e));
  }

  struct radio_list *ptr = (struct radio_list*)malloc(sizeof(struct radio_list));
  if(NULL == ptr)
  {
    printf("\n Node creation failed \n");
    return NULL;
  }
  ptr->val = e->phy;
  ptr->ifname = e->ifname;
  ptr->next = NULL;

  curr->next = ptr;
  curr = ptr;

  return ptr;
}

void run_interface_scan(json_object *jiface_array,
    json_object *jstations_array, json_object *jscan_array
    )
{

  head = NULL;
  curr = NULL;

  int len = 0;
  int x, i, ii, len_a, xx;
  char ssids[1024];
  char buf_a[1024];
  const struct iw_ops *iw;
  struct iw_ssid_entry *e;
  struct iw_stationlist_entry *st;

  // Check chipset / drivers required
  iw = &nl80211_exec;

  /* char mac[20]; */
  /* strcpy(mac, "insert mac"); */
  /* iw->disconnect(mac); */
  // Can cause mem. leak if no SSIDS
  iw->ssids(ssids, &len);

  for (i = 0, x = 1; i < len; i += sizeof(struct iw_ssid_entry), x++)
  {
    e = (struct iw_ssid_entry *) &ssids[i];

    /* printf("Collecting Data for SSID %s (phy %d)\n", e->ifname, e->phy); */

    iw->stations(e->ifname, buf_a, &len_a);
    for (ii = 0, xx = 1; ii < len_a; ii += sizeof(struct iw_stationlist_entry), xx++)
    {
      st = (struct iw_stationlist_entry *) &buf_a[ii];
      json_object *jstations = json_object_new_object();
      format_stations(e->ssid, e->ifname, st, jstations);
      json_object_array_add(jstations_array, jstations);
    }
    debug("%d clients connected to %s", xx-1, e->ifname);

    json_object *jssids = json_object_new_object();
    format_ssids(iw, e, jssids, len_a);
    json_object_array_add(jiface_array, jssids);

    int ret = strcmp(e->ifname, "mon0");
    if (ret != 0 && !options.no_survey)
      add_to_list(e);
  }

  // Needs scan logic built in
  if (!options.no_survey) {
    int alen = 0;
    char buf_s[4096];
    int len_s;
    static int myArray[2] = {100,100};
    struct radio_list *ptr = head;
    struct iw_scanlist_entry *sc;

    i = 0, x = 0;

    struct radio_list *holdMe = NULL;
    struct radio_list *freeMe = ptr;

    while(ptr != NULL)
    {
      if (in_array(ptr->val, myArray, 2) == 0) {
        myArray[alen] = ptr->val;
        alen++;
        len_s = 0;
        buf_s[0] = '\0';

        printf("Scanning on %s %d\n", ptr->ifname, ptr->val);
        if(iw->scan(ptr->ifname, buf_s, &len_s)) {
          for (i = 0, x = 1; i < len_s; i += sizeof(struct iw_scanlist_entry), x++)
          {
            sc = (struct iw_scanlist_entry *) &buf_s[i];
            json_object *jscan = json_object_new_object();
            format_scan(sc, jscan);
            json_object_array_add(jscan_array, jscan);
          }
        }
      }
      ptr = ptr->next;
    }

    while(freeMe != NULL) {
      holdMe = freeMe->next;
      free(freeMe);
      freeMe = holdMe;
    }
    free(ptr);
  }

  if (0) { // Not implemented
    struct iw_info_entry *il;
    char bufff[1024];
    int len, i, x;
    if (iw->info(bufff, &len)) {
      for (i = 0, x = 1; i < len; i += sizeof(struct iw_info_entry), x++)
      {
        il = (struct iw_info_entry *) &bufff[i];
        debug("s: %d", il->phy);
      }
    }
  }
}

void format_splash(json_object *jsplash_array)
{

  const struct splash_ops *splash;
  struct splash_list *clients = NULL;

  splash = &splash_exec;
  splash->clients(&clients);

  if (clients != NULL) {
    struct splash_list *holdMe = NULL;
    struct splash_list *freeMe = clients;

    while (clients->next != NULL) {
      json_object *jsplash = json_object_new_object();

      json_object *jmac = json_object_new_string(clients->mac);
      json_object_object_add(jsplash, "mac", jmac);

      json_object *jip = json_object_new_string(clients->ip);
      json_object_object_add(jsplash, "ip", jip);

      json_object *jcstate = json_object_new_string(clients->client_state);
      json_object_object_add(jsplash, "client_state", jcstate);

      json_object *jastate = json_object_new_string(clients->auth_state);
      json_object_object_add(jsplash, "auth_state", jastate);

      json_object_array_add(jsplash_array, jsplash);

      clients = clients->next;
    }

    while(freeMe != NULL) {
      holdMe = freeMe->next;
      free(freeMe);
      freeMe = holdMe;
    }
  }
}

void format_dhcp(json_object *jdhcp_array)
{

  const struct dhcp_ops *dhcp;
  struct dhcp_list *clients = NULL;

  dhcp = &dhcp_exec;
  dhcp->clients(&clients);

  if (clients != NULL) {

    struct dhcp_list *holdMe = NULL;
    struct dhcp_list *freeMe = clients;

    while (clients->next != NULL) {
      json_object *jdhcp = json_object_new_object();

      json_object *jmac = json_object_new_string(clients->mac);
      json_object_object_add(jdhcp, "mac", jmac);

      json_object *jip = json_object_new_string(clients->ip);
      json_object_object_add(jdhcp, "ip", jip);

      json_object *jname = json_object_new_string(clients->name);
      json_object_object_add(jdhcp, "name", jname);

      json_object_array_add(jdhcp_array, jdhcp);

      clients = clients->next;
    }

    while(freeMe != NULL) {
      holdMe = freeMe->next;
      free(freeMe);
      freeMe = holdMe;
    }
  }
}

void collect_data(int online)
{
  struct timespec tstart={0,0}, tend={0,0};
  clock_gettime(CLOCK_MONOTONIC, &tstart);

  debug("Collecting the device stats");

  char rx[21], tx[21], wan_ip[21] = "";
  json_object *jobj = json_object_new_object();
  json_object *jattr = json_object_new_object();

  struct defaultRoute dr = route();

  if (strcmp(dr.if_name, "") != 0) {
    interface_ip(dr.if_name, wan_ip, sizeof(wan_ip));

    struct InterfaceStats istats = stats(dr.if_name);
    sprintf(tx, "%" PRIu64, istats.tx);
    sprintf(rx, "%" PRIu64, istats.rx);
  }

  char machine[100];
  machine[0] = '\0';
  machine_type(machine, sizeof(machine));

  struct SystemInfo info = system_info();

  if (options.no_scan != 1)
  {
    debug("Running WiFi Collection");

    json_object *jiface_array = json_object_new_array();
    json_object *jstations_array = json_object_new_array();
    json_object *jscan_array = json_object_new_array();

    run_interface_scan(jiface_array, jstations_array, jscan_array);

    json_object_object_add(jobj, "ssids", jiface_array);
    json_object_object_add(jobj, "survey", jscan_array);
    json_object_object_add(jobj, "stations", jstations_array);
  }

#ifdef __OPENWRT__
  char firmware[20];
  readlineToBuffer("/etc/openwrt_version", firmware);

  // Should we save to options ? //
  json_object *jfirmware = json_object_new_string(firmware);
  json_object_object_add(jattr, "firmware", jfirmware);
#endif

  json_object *jserial = json_object_new_string("simon says");
  json_object_object_add(jattr, "serial", jserial);

  if (strcmp(dr.if_name, "") != 0) {
    json_object *jwan_name = json_object_new_string(dr.if_name);
    json_object_object_add(jattr, "wan_name", jwan_name);
  }

  if (strcmp(dr.ip, "") != 0) {
    json_object *jgateway = json_object_new_string(dr.ip);
    json_object_object_add(jattr, "wan_gateway", jgateway);
  }

  if (strlen(wan_ip) > 0) {
    json_object *jwanip = json_object_new_string(wan_ip);
    json_object_object_add(jattr, "wan_ip", jwanip);
  }

  if (machine[0] !='\0') {
    // Save to the options to prevent future lookups
    strcpy(options.machine, machine);
    json_object *jmachine = json_object_new_string(machine);
    json_object_object_add(jattr, "machine_type", jmachine);
  }

  json_object *jtx = json_object_new_string(tx);
  json_object_object_add(jattr, "tx_bytes", jtx);

  json_object *jrx = json_object_new_string(rx);
  json_object_object_add(jattr, "rx_bytes", jrx);

  json_object *juptime = json_object_new_int(info.uptime);
  json_object_object_add(jattr, "uptime", juptime);

  json_object *jload_1 = json_object_new_double(info.load_1);
  json_object_object_add(jattr, "load_1", jload_1);

  json_object *jload_5 = json_object_new_double(info.load_5);
  json_object_object_add(jattr, "load_5", jload_5);

  json_object *jload_15 = json_object_new_double(info.load_15);
  json_object_object_add(jattr, "load_15", jload_15);

  json_object *jtotalram = json_object_new_double(info.totalram);
  json_object_object_add(jattr, "total_ram", jtotalram);

  json_object *jfreeram = json_object_new_double(info.freeram);
  json_object_object_add(jattr, "free_ram", jfreeram);

  json_object *jprocs = json_object_new_int(info.procs);
  json_object_object_add(jattr, "procs", jprocs);

  // Reason code ! //
  bool bonline = online ? true : false;
  json_object *jonline = json_object_new_boolean(bonline);
  json_object_object_add(jattr, "online", jonline);

  time_t now = time(NULL);
  json_object *jcreated_at = json_object_new_int(now);
  json_object_object_add(jattr, "created_at", jcreated_at);

  json_object *jdhcp_array = json_object_new_array();
  format_dhcp(jdhcp_array);
  json_object_object_add(jobj, "dhcp", jdhcp_array);

  json_object *jsplash_array = json_object_new_array();
  format_splash(jsplash_array);
  json_object_object_add(jobj, "splash", jsplash_array);

  // MISSING!!!!!!!
  // INTERFACES
  // CAPS
  // MQTT STATUS

  json_object_object_add(jobj, "device", jattr);

  clock_gettime(CLOCK_MONOTONIC, &tend);
  printf("Stats collection finished in %.5f seconds\n",
      ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
      ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));

  // Should try and post, even if not online //
  if (post(jobj)) {
    /* if success / online */
    // Lookin' good
  } else {
    // Cache
  }

  run_cleanup(info);
  json_object_put(jobj);
}

void cache_data() {
  debug("save the datas!");
}

void post_data() {
  debug("sending the datas!");
}

void collect_and_send_data(int online)
{
  if (should_collect())
    collect_data(online);
}
