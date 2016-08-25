#ifndef __CTIW_H_
#define __CTIW_H_
#include <stdbool.h>
/* #include <ctype.h> */
/* #include <stdio.h> */
/* #include <stdlib.h> */

#define ESSID_MAX_SIZE 32

void format_mac(unsigned char *mac, char *buff);
void mac_addr_n2a(char *mac_addr, unsigned char *arg);

#define NONE   (1 << 0)
#define WEP40  (1 << 1)
#define TKIP   (1 << 2)
#define WRAP   (1 << 3)
#define CCMP   (1 << 4)
#define WEP104 (1 << 5)
#define AESOCB (1 << 6)
#define CKIP   (1 << 7)

#define MGMT_NONE    (1 << 0)
#define MGMT_8021x   (1 << 1)
#define MGMT_PSK     (1 << 2)

#define AUTH_OPEN     (1 << 0)
#define AUTH_SHARED   (1 << 1)

struct nl80211_state {
  struct nl_sock *nl_sock;
  int nl80211_id;
  struct nl_cache *nl_cache;
  struct genl_family *nl80211;
  struct genl_family *nlctrl;
};

struct nl80211_msg_conveyor {
  struct nl_msg *msg;
  struct nl_cb *cb;
};

struct nl80211_rssi {
  int16_t rate;
  int8_t  rssi;
};

struct nl80211_interface_stats {
  unsigned char *ssid;
  unsigned char bssid[7];
  int txpower;
};

struct nl80211_ssid_list {
  struct iw_ssid_entry *e;
  int len;
};

struct iw_ssid_entry {
  uint8_t mac[6];
  char ssid[ESSID_MAX_SIZE+1];
  char ifname[20];
  unsigned char *mode;
  int channel;
  int index;
  int phy;
};

struct nl80211_stationlist {
  struct iw_stationlist_entry *s;
  int len;
};

struct iw_stationlist_entry {
  uint8_t mac[6];
  int16_t tx_bitrate;
  int16_t rx_bitrate;
  int32_t inactive_time;
  unsigned long long rx_bytes_64;
  unsigned long long tx_bytes_64;
  uint32_t rx_bytes;
  uint32_t tx_bytes;
  unsigned long long beacon_rx;
  unsigned long long t_offset;
  int32_t tx_packets;
  int32_t rx_packets;
  int32_t tx_retries;
  int32_t tx_failed;
  int32_t conn_time;
  int32_t beacon_loss;
  int32_t expected_tput;
  int8_t signal;
  int8_t signal_avg;
  int8_t beacon_signal_avg;
  bool authorized;
  bool authenticated;
  bool associated;
  bool wmm;
  bool mfp;
  bool tdls;
  int preamble;
};

struct nl80211_scanlist {
  struct iw_scanlist_entry *s;
  int len;
};

struct node {
  int radio_2;
  int radio_5;
};

// From IWINFO
struct crypto_entry {
  uint8_t enabled;
  uint8_t wpa_version;
  uint8_t group_ciphers;
  uint8_t pair_ciphers;
  uint8_t auth_suites;
  uint8_t auth_algs;
};

struct iw_scanlist_entry {
  uint8_t mac[6];
  int32_t channel;
  int32_t age;
  char ssid[ESSID_MAX_SIZE+1];
  struct crypto_entry crypto;
  uint8_t signal;
  uint8_t quality;
  uint8_t quality_max;
};

struct iw_ops {
  const char *name;

  /* int (*probe)(const char *ifname); */
  /* int (*mode)(const char *, int *); */
  int (*channel)(const char *, int *);
  /* int (*frequency)(const char *, int *); */
  /* int (*frequency_offset)(const char *, int *); */
  int (*txpower)(const char *, int *);
  /* int (*txpower_offset)(const char *, int *); */
  int (*bitrate)(const char *, int *);
  int (*signal)(const char *, int *);
  int (*noise)(const char *, int *);
  int (*ssids)(char *, int *);
  int (*scan)(const char *, char *, int *);
  int (*quality)(int, int *);
  int (*quality_max)(int *);
  /* int (*mbssid_support)(const char *, int *); */
  /* int (*hwmodelist)(const char *, int *); */
  /* int (*htmodelist)(const char *, int *); */
  int (*ssid)(const char *, char *);
  int (*bssid)(const char *, char *);
  /* int (*country)(const char *, char *); */
  /* int (*hardware_id)(const char *, char *); */
  /* int (*hardware_name)(const char *, char *); */
  int (*encryption)(const char *, char *);
  int (*stations)(const char *, char *, int *);
  int (*info)(char *, int *);
  /* int (*phyname)(const char *, char *); */
  /* int (*assoclist)(const char *, char *, int *); */
  /* int (*txpwrlist)(const char *, char *, int *); */
  /* int (*scanlist)(const char *, char *, int *); */
  /* int (*freqlist)(const char *, char *, int *); */
  /* int (*countrylist)(const char *, char *, int *); */
  /* int (*lookup_phy)(const char *, char *); */
  /* void (*close)(void); */
};

/* static const struct ie_print wifiprinters[] = { */
/*   [1] = { "WPA", print_wifi_wpa, 2, 255, BIT(PRINT_SCAN), }, */
/*   [2] = { "WMM", print_wifi_wmm, 1, 255, BIT(PRINT_SCAN), }, */
/*   [4] = { "WPS", print_wifi_wps, 0, 255, BIT(PRINT_SCAN), }, */
/* }; */

/* static const struct ie_print ieprinters[] = { */
/*   [0] = { "SSID", print_ssid, 0, 32, BIT(PRINT_SCAN) | BIT(PRINT_LINK), }, */
/*   [1] = { "Supported rates", print_supprates, 0, 255, BIT(PRINT_SCAN), }, */
/*   [3] = { "DS Parameter set", print_ds, 1, 1, BIT(PRINT_SCAN), }, */
/*   [5] = { "TIM", print_tim, 4, 255, BIT(PRINT_SCAN), }, */
/*   [6] = { "IBSS ATIM window", print_ibssatim, 2, 2, BIT(PRINT_SCAN), }, */
/*   [7] = { "Country", print_country, 3, 255, BIT(PRINT_SCAN), }, */
/*   [11] = { "BSS Load", print_bss_load, 5, 5, BIT(PRINT_SCAN), }, */
/*   [32] = { "Power constraint", print_powerconstraint, 1, 1, BIT(PRINT_SCAN), }, */
/*   [35] = { "TPC report", print_tpcreport, 2, 2, BIT(PRINT_SCAN), }, */
/*   [42] = { "ERP", print_erp, 1, 255, BIT(PRINT_SCAN), }, */
/*   [45] = { "HT capabilities", print_ht_capa, 26, 26, BIT(PRINT_SCAN), }, */
/*   [47] = { "ERP D4.0", print_erp, 1, 255, BIT(PRINT_SCAN), }, */
/*   [74] = { "Overlapping BSS scan params", print_obss_scan_params, 14, 255, BIT(PRINT_SCAN), }, */
/*   [61] = { "HT operation", print_ht_op, 22, 22, BIT(PRINT_SCAN), }, */
/*   [62] = { "Secondary Channel Offset", print_secchan_offs, 1, 1, BIT(PRINT_SCAN), }, */
/*   [191] = { "VHT capabilities", print_vht_capa, 12, 255, BIT(PRINT_SCAN), }, */
/*   [192] = { "VHT operation", print_vht_oper, 5, 255, BIT(PRINT_SCAN), }, */
/*   [48] = { "RSN", print_rsn, 2, 255, BIT(PRINT_SCAN), }, */
/*   [50] = { "Extended supported rates", print_supprates, 0, 255, BIT(PRINT_SCAN), }, */
/*   [113] = { "MESH Configuration", print_mesh_conf, 7, 7, BIT(PRINT_SCAN), }, */
/*   [114] = { "MESH ID", print_ssid, 0, 32, BIT(PRINT_SCAN) | BIT(PRINT_LINK), }, */
/*   [127] = { "Extended capabilities", print_capabilities, 0, 255, BIT(PRINT_SCAN), }, */
/*   [107] = { "802.11u Interworking", print_interworking, 0, 255, BIT(PRINT_SCAN), }, */
/*   [108] = { "802.11u Advertisement", print_11u_advert, 0, 255, BIT(PRINT_SCAN), }, */
/*   [111] = { "802.11u Roaming Consortium", print_11u_rcon, 0, 255, BIT(PRINT_SCAN), }, */
/* }; */

const struct iw_ops nl80211_exec;
int nl80211_handler(struct nl80211_state *state, char *cmd, void *dev);
void format_encryption(struct crypto_entry *c, char *buf);

/* int scan(const struct iw_ops *iw, json_object *jscan_array, const struct iw_ssid_entry *e, int len, char *ssids); */
void perform_scan();

/* struct test_struct *curr; */
/* struct test_struct *head; */
/* void perform_scan(struct test_struct **head); */

#endif
