#ifndef __CTIW_H_
#define __CTIW_H_
#include <stdbool.h>

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

struct nl80211_info_list {
  struct iw_info_entry *e;
  int len;
};

struct iw_info_entry {
  bool ac;
  bool five;
  int phy;
};

struct nl80211_stationlist {
  struct iw_stationlist_entry *s;
  int len;
};

struct iw_stationlist_entry {
  uint8_t mac[6];
  int16_t tx_bitrate;
  int8_t mcs;
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
  int32_t freq;
  char ssid[ESSID_MAX_SIZE+1];
  struct crypto_entry crypto;
  uint8_t signal;
  uint8_t quality;
  uint8_t quality_max;
};

struct iw_ops {
  const char *name;
  int (*channel)(const char *, int *);
  int (*txpower)(const char *, int *);
  int (*bitrate)(const char *, int *);
  int (*freq)(const char *, int *);
  int (*signal)(const char *, int *);
  int (*noise)(const char *, int *);
  int (*ssids)(char *, int *);
  int (*scan)(const char *, char *, int *);
  int (*quality)(int, int *);
  int (*quality_max)(int *);
  int (*ssid)(const char *, char *);
  int (*bssid)(const char *, char *);
  int (*encryption)(const char *, char *);
  int (*stations)(const char *, char *, int *);
  int (*info)(char *, int *);
  int (*disconnect)(char *);
};

const struct iw_ops nl80211_exec;
int nl80211_handler(struct nl80211_state *state, char *cmd, void *dev);
void format_encryption(struct crypto_entry *c, char *buf);

void perform_scan();

#endif
