#include <stdbool.h>
#include "compiler.h"

/* These are a mix of functions from IW
   Many thanks for helping get this going
   */

#ifndef __IW_H
#define __IW_H

#define ETH_ALEN 6

/* #include <ctype.h> */
/* #include <netlink/genl/genl.h> */
/* #include <netlink/genl/family.h> */
/* #include <netlink/genl/ctrl.h> */

#define WLAN_CAPABILITY_ESS                     (1<<0)
#define WLAN_CAPABILITY_IBSS                    (1<<1)
#define WLAN_CAPABILITY_CF_POLLABLE             (1<<2)
#define WLAN_CAPABILITY_CF_POLL_REQUEST         (1<<3)
#define WLAN_CAPABILITY_PRIVACY                 (1<<4)
#define WLAN_CAPABILITY_SHORT_PREAMBLE          (1<<5)
#define WLAN_CAPABILITY_PBCC                    (1<<6)
#define WLAN_CAPABILITY_CHANNEL_AGILITY         (1<<7)
#define WLAN_CAPABILITY_SPECTRUM_MGMT           (1<<8)
#define WLAN_CAPABILITY_QOS                     (1<<9)
#define WLAN_CAPABILITY_SHORT_SLOT_TIME         (1<<10)
#define WLAN_CAPABILITY_APSD                    (1<<11)
#define WLAN_CAPABILITY_RADIO_MEASURE           (1<<12)
#define WLAN_CAPABILITY_DSSS_OFDM               (1<<13)
#define WLAN_CAPABILITY_DEL_BACK                (1<<14)
#define WLAN_CAPABILITY_IMM_BACK                (1<<15)
#define WLAN_CAPABILITY_DMG_TYPE_MASK           (3<<0)
#define WLAN_CAPABILITY_DMG_TYPE_IBSS           (1<<0) /* Tx by: STA */
#define WLAN_CAPABILITY_DMG_TYPE_PBSS           (2<<0) /* Tx by: PCP */
#define WLAN_CAPABILITY_DMG_TYPE_AP             (3<<0) /* Tx by: AP */
#define WLAN_CAPABILITY_DMG_CBAP_ONLY           (1<<2)
#define WLAN_CAPABILITY_DMG_CBAP_SOURCE         (1<<3)
#define WLAN_CAPABILITY_DMG_PRIVACY             (1<<4)
#define WLAN_CAPABILITY_DMG_ECPAC               (1<<5)
#define WLAN_CAPABILITY_DMG_SPECTRUM_MGMT       (1<<8)
#define WLAN_CAPABILITY_DMG_RADIO_MEASURE       (1<<12)

#define BIT(x) (1ULL<<(x))

void mac_addr_n2a(char *mac_addr, unsigned char *arg);

UNUSED(static int error_handler(UNUSED(struct sockaddr_nl *nla), struct nlmsgerr *err, void *arg));
static int error_handler(UNUSED(struct sockaddr_nl *nla), struct nlmsgerr *err, void *arg) {
  // Callback for errors.
  printf("error_handler() called. %d\n", err->error);
  int *ret = arg;
  *ret = err->error;
  return NL_STOP;
}

enum print_ie_type {
  PRINT_SCAN,
  PRINT_LINK,
};

// For family_handler() and nl_get_multicast_id().
struct handler_args {
  const char *group;
  int id;
};

struct scan_params {
  bool unknown;
  enum print_ie_type type;
  bool show_both_ie_sets;
};

UNUSED(static int finish_handler(UNUSED(struct nl_msg *msg), void *arg));
static int finish_handler(UNUSED(struct nl_msg *msg), void *arg) {
  // Callback for NL_CB_FINISH.
  int *ret = arg;
  *ret = 0;
  return NL_SKIP;
}

UNUSED(static int ack_handler(UNUSED(struct nl_msg *msg), void *arg));
static int ack_handler(UNUSED(struct nl_msg *msg), void *arg) {
  // Callback for NL_CB_ACK.
  int *ret = arg;
  *ret = 0;
  return NL_STOP;
}

int nl_get_multicast_id(struct nl_sock *sock, const char *family, const char *group);

void print_ssid_escaped(const uint8_t len, const uint8_t *data);

/*static void print_capa_dmg(__u16 capa)
{
  switch (capa & WLAN_CAPABILITY_DMG_TYPE_MASK) {
    case WLAN_CAPABILITY_DMG_TYPE_AP:
      printf(" DMG_ESS");
      break;
    case WLAN_CAPABILITY_DMG_TYPE_PBSS:
      printf(" DMG_PCP");
      break;
    case WLAN_CAPABILITY_DMG_TYPE_IBSS:
      printf(" DMG_IBSS");
      break;
  }

  if (capa & WLAN_CAPABILITY_DMG_CBAP_ONLY)
    printf(" CBAP_Only");
  if (capa & WLAN_CAPABILITY_DMG_CBAP_SOURCE)
    printf(" CBAP_Src");
  if (capa & WLAN_CAPABILITY_DMG_PRIVACY)
    printf(" Privacy");
  if (capa & WLAN_CAPABILITY_DMG_ECPAC)
    printf(" ECPAC");
  if (capa & WLAN_CAPABILITY_DMG_SPECTRUM_MGMT)
    printf(" SpectrumMgmt");
  if (capa & WLAN_CAPABILITY_DMG_RADIO_MEASURE)
    printf(" RadioMeasure");
}*/

/*static void print_capa_non_dmg(__u16 capa)
{
  if (capa & WLAN_CAPABILITY_ESS)
    printf(" ESS");
  if (capa & WLAN_CAPABILITY_IBSS)
    printf(" IBSS");
  if (capa & WLAN_CAPABILITY_CF_POLLABLE)
    printf(" CfPollable");
  if (capa & WLAN_CAPABILITY_CF_POLL_REQUEST)
    printf(" CfPollReq");
  if (capa & WLAN_CAPABILITY_PRIVACY)
    printf(" Privacy");
  if (capa & WLAN_CAPABILITY_SHORT_PREAMBLE)
    printf(" ShortPreamble");
  if (capa & WLAN_CAPABILITY_PBCC)
    printf(" PBCC");
  if (capa & WLAN_CAPABILITY_CHANNEL_AGILITY)
    printf(" ChannelAgility");
  if (capa & WLAN_CAPABILITY_SPECTRUM_MGMT)
    printf(" SpectrumMgmt");
  if (capa & WLAN_CAPABILITY_QOS)
    printf(" QoS");
  if (capa & WLAN_CAPABILITY_SHORT_SLOT_TIME)
    printf(" ShortSlotTime");
  if (capa & WLAN_CAPABILITY_APSD)
    printf(" APSD");
  if (capa & WLAN_CAPABILITY_RADIO_MEASURE)
    printf(" RadioMeasure");
  if (capa & WLAN_CAPABILITY_DSSS_OFDM)
    printf(" DSSS-OFDM");
  if (capa & WLAN_CAPABILITY_DEL_BACK)
    printf(" DelayedBACK");
  if (capa & WLAN_CAPABILITY_IMM_BACK)
    printf(" ImmediateBACK");
}*/

#endif
