#include <ctype.h>
#include <net/if.h>
#include <string.h>
#include "dbg.h"
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <json-c/json.h>
#include <fnmatch.h>
#include <limits.h>
#include <glob.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include "nl80211.h"
#include <errno.h>
#include "iw.h"
#include "phy.h"
#include "utils.h"
#include <compiler.h>

#define min(x, y) ((x) < (y)) ? (x) : (y)
#define ARRAY_SIZE(ar) (sizeof(ar)/sizeof(ar[0]))

static struct nl80211_state *nlstate = NULL;
static int (*registered_handler)(struct nl_msg *, void *);
static void *registered_handler_data;
static unsigned char ms_oui[3] = { 0x00, 0x50, 0xf2 };
static unsigned char ieee80211_oui[3] = { 0x00, 0x0f, 0xac };

static int * nl80211_send(
    struct nl80211_msg_conveyor *cv,
    int (*cb_func)(struct nl_msg *, void *), void *cb_arg
    )
{
  int err = 1;
  nl_cb_set(cv->cb, NL_CB_VALID, NL_CB_CUSTOM, cb_func, cb_arg);

  nl_send_auto_complete(nlstate->nl_sock, cv->msg);

  nl_cb_set(cv->cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
  nl_cb_err(cv->cb,               NL_CB_CUSTOM, error_handler, &err);
  nl_cb_set(cv->cb, NL_CB_ACK,    NL_CB_CUSTOM, ack_handler, &err);

  while (err > 0)
    nl_recvmsgs(nlstate->nl_sock, cv->cb);

  return 0;
}

static int family_handler(struct nl_msg *msg, void *arg)
{
  struct handler_args *grp = arg;
  struct nlattr *tb[CTRL_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *mcgrp;
  int rem_mcgrp;

  nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

  if (!tb[CTRL_ATTR_MCAST_GROUPS]) return NL_SKIP;

  nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {  // This is a loop.
    struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

    nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp), nla_len(mcgrp), NULL);

    if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] || !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]) continue;
    if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]), grp->group,
          nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]))) {
      continue;
    }

    grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
    break;
  }

  return NL_SKIP;
}

// From http://sourcecodebrowser.com/iw/0.9.14/genl_8c.html.
int nl_get_multicast_id(struct nl_sock *sock, const char *family, const char *group)
{
  struct nl_msg *msg;
  struct nl_cb *cb;
  int ret, ctrlid;
  struct handler_args grp = { .group = group, .id = -ENOENT, };

  msg = nlmsg_alloc();
  if (!msg) return -ENOMEM;

  cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (!cb) {
    ret = -ENOMEM;
    goto out_fail_cb;
  }

  ctrlid = genl_ctrl_resolve(sock, "nlctrl");

  genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);

  ret = -ENOBUFS;
  NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

  ret = nl_send_auto_complete(sock, msg);
  if (ret < 0) goto out;

  ret = 1;

  nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ret);
  nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &ret);
  nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, family_handler, &grp);

  while (ret > 0) nl_recvmsgs(sock, cb);

  if (ret == 0) ret = grp.id;

nla_put_failure:
out:
  nl_cb_put(cb);
out_fail_cb:
  nlmsg_free(msg);
  return ret;
}

// From http://git.kernel.org/cgit/linux/kernel/git/jberg/iw.git/tree/util.c.
void mac_addr_n2a(char *mac_addr, unsigned char *arg)
{
  int i, l;

  l = 0;
  for (i = 0; i < 6; i++) {
    if (i == 0) {
      sprintf(mac_addr+l, "%02X", arg[i]);
      l += 2;
    } else {
      sprintf(mac_addr+l, ":%02X", arg[i]);
      l += 3;
    }
  }
}

char *channel_width_name(enum nl80211_chan_width width)
{
  switch (width) {
    case NL80211_CHAN_WIDTH_20_NOHT:
      return "20 MHz (no HT)";
    case NL80211_CHAN_WIDTH_20:
      return "20 MHz";
    case NL80211_CHAN_WIDTH_40:
      return "40 MHz";
    case NL80211_CHAN_WIDTH_80:
      return "80 MHz";
    case NL80211_CHAN_WIDTH_80P80:
      return "80+80 MHz";
    case NL80211_CHAN_WIDTH_160:
      return "160 MHz";
    case NL80211_CHAN_WIDTH_5:
      return "5 MHz";
    case NL80211_CHAN_WIDTH_10:
      return "10 MHz";
    default:
      return NULL;
  }
}

static void nl80211_close(void)
{
  if (nlstate)
  {
    if (nlstate->nl_sock)
      nl_socket_free(nlstate->nl_sock);

    if (nlstate->nl_cache)
      nl_cache_free(nlstate->nl_cache);

    free(nlstate);
    nlstate = NULL;
  }
}

static void nl80211_free(struct nl80211_msg_conveyor *cv)
{
  if (cv)
  {
    if (cv->cb)
      nl_cb_put(cv->cb);

    if (cv->msg)
      nlmsg_free(cv->msg);

    cv->cb  = NULL;
    cv->msg = NULL;
  }
}

void register_handler(int (*handler)(struct nl_msg *, void *), void *data)
{
  registered_handler = handler;
  registered_handler_data = data;
}

int valid_handler(struct nl_msg *msg, UNUSED(void *arg))
{
  if (registered_handler)
    return registered_handler(msg, registered_handler_data);

  return NL_OK;
}

static int no_seq_check(UNUSED(struct nl_msg *msg), UNUSED(void *arg)) {
  // Callback for NL_CB_SEQ_CHECK.
  return NL_OK;
}

int ieee80211_frequency_to_channel(int freq)
{
  /* see 802.11-2007 17.3.8.3.2 and Annex J */
  if (freq == 2484)
    return 14;
  else if (freq < 2484)
    return (freq - 2407) / 5;
  else if (freq >= 4910 && freq <= 4980)
    return (freq - 4000) / 5;
  else if (freq <= 45000) /* DMG band lower limit */
    return (freq - 5000) / 5;
  else if (freq >= 58320 && freq <= 64800)
    return (freq - 56160) / 2160;
  else
    return 0;
}

char * format_enc_suites(int suites)
{
  static char str[64] = { 0 };
  char *pos = str;

  if (suites & MGMT_PSK)
    pos += sprintf(pos, "PSK/");

  if (suites & MGMT_8021x)
    pos += sprintf(pos, "802.1X/");

  if (!suites || (suites & MGMT_NONE))
    pos += sprintf(pos, "NONE/");

  if ((pos - str) > 0)
    *(pos - 1) = 0;

  return str;
}

char * format_enc_ciphers(int ciphers)
{
  static char str[128] = { 0 };
  char *pos = str;

  if (ciphers & WEP40)
    pos += sprintf(pos, "WEP-40, ");

  if (ciphers & WEP104)
    pos += sprintf(pos, "WEP-104, ");

  if (ciphers & TKIP)
    pos += sprintf(pos, "TKIP, ");

  if (ciphers & CCMP)
    pos += sprintf(pos, "CCMP, ");

  if (ciphers & WRAP)
    pos += sprintf(pos, "WRAP, ");

  if (ciphers & AESOCB)
    pos += sprintf(pos, "AES-OCB, ");

  if (ciphers & CKIP)
    pos += sprintf(pos, "CKIP, ");

  if (!ciphers || (ciphers & NONE))
    pos += sprintf(pos, "NONE, ");

  if ((pos - str) >= 2)
    *(pos - 2) = 0;

  return str;
}

void format_encryption(struct crypto_entry *c, char *buf)
{
  if (!c)
  {
    sprintf(buf, "unknown");
  }
  else if (c->enabled)
  {
    if (c->auth_algs && !c->wpa_version)
    {
      if ((c->auth_algs & AUTH_OPEN) &&
          (c->auth_algs & AUTH_SHARED))
      {
        sprintf(buf, "WEP Open/Shared (%s)",
            format_enc_ciphers(c->pair_ciphers));
      }
      else if (c->auth_algs & AUTH_OPEN)
      {
        sprintf(buf, "WEP Open System (%s)",
            format_enc_ciphers(c->pair_ciphers));
      }
      else if (c->auth_algs & AUTH_SHARED)
      {
        sprintf(buf, "WEP Shared Auth (%s)",
            format_enc_ciphers(c->pair_ciphers));
      }
    }

    else if (c->wpa_version)
    {
      switch (c->wpa_version) {
        case 3:
          sprintf(buf, "WPA/WPA2 %s (%s)",
              format_enc_suites(c->auth_suites),
              format_enc_ciphers(c->pair_ciphers | c->group_ciphers));
          break;

        case 2:
          sprintf(buf, "WPA2 %s (%s)",
              format_enc_suites(c->auth_suites),
              format_enc_ciphers(c->pair_ciphers | c->group_ciphers));
          break;

        case 1:
          sprintf(buf, "WPA %s (%s)",
              format_enc_suites(c->auth_suites),
              format_enc_ciphers(c->pair_ciphers | c->group_ciphers));
          break;
      }
    }
    else
    {
      sprintf(buf, "none");
    }
  }
  else
  {
    sprintf(buf, "none");
  }

}

// From IWINFO
void iwinfo_parse_rsn(struct crypto_entry *c, uint8_t *data, uint8_t len,
    uint8_t defcipher, uint8_t defauth)
{
  uint16_t i, count;

  data += 2;
  len -= 2;

  if (!memcmp(data, ms_oui, 3))
    c->wpa_version += 1;
  else if (!memcmp(data, ieee80211_oui, 3))
    c->wpa_version += 2;

  if (len < 4)
  {
    c->group_ciphers |= defcipher;
    c->pair_ciphers  |= defcipher;
    c->auth_suites   |= defauth;
    return;
  }

  if (!memcmp(data, ms_oui, 3) || !memcmp(data, ieee80211_oui, 3))
  {
    switch (data[3])
    {
      case 1: c->group_ciphers |= WEP40;  break;
      case 2: c->group_ciphers |= TKIP;   break;
      case 4: c->group_ciphers |= CCMP;   break;
      case 5: c->group_ciphers |= WEP104; break;
      case 6:  /* AES-128-CMAC */ break;
      default: /* proprietary */  break;
    }
  }

  data += 4;
  len -= 4;

  if (len < 2)
  {
    c->pair_ciphers |= defcipher;
    c->auth_suites  |= defauth;
    return;
  }

  count = data[0] | (data[1] << 8);
  if (2 + (count * 4) > len)
    return;

  for (i = 0; i < count; i++)
  {
    if (!memcmp(data + 2 + (i * 4), ms_oui, 3) ||
        !memcmp(data + 2 + (i * 4), ieee80211_oui, 3))
    {
      switch (data[2 + (i * 4) + 3])
      {
        case 1: c->pair_ciphers |= WEP40;  break;
        case 2: c->pair_ciphers |= TKIP;   break;
        case 4: c->pair_ciphers |= CCMP;   break;
        case 5: c->pair_ciphers |= WEP104; break;
        case 6:  /* AES-128-CMAC */ break;
        default: /* proprietary */  break;
      }
    }
  }

  data += 2 + (count * 4);
  len -= 2 + (count * 4);

  if (len < 2)
  {
    c->auth_suites |= defauth;
    return;
  }

  count = data[0] | (data[1] << 8);
  if (2 + (count * 4) > len)
    return;

  for (i = 0; i < count; i++)
  {
    if (!memcmp(data + 2 + (i * 4), ms_oui, 3) ||
        !memcmp(data + 2 + (i * 4), ieee80211_oui, 3))
    {
      switch (data[2 + (i * 4) + 3])
      {
        case 1: c->auth_suites |= MGMT_8021x; break;
        case 2: c->auth_suites |= MGMT_PSK;   break;
        case 3:  /* FT/IEEE 802.1X */                 break;
        case 4:  /* FT/PSK */                         break;
        case 5:  /* IEEE 802.1X/SHA-256 */            break;
        case 6:  /* PSK/SHA-256 */                    break;
        default: /* proprietary */                    break;
      }
    }
  }

  data += 2 + (count * 4);
  len -= 2 + (count * 4);
}

static void nl80211_info_elements(struct nlattr **bss,
    struct iw_scanlist_entry *s)
{
  int ielen = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
  unsigned char *ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
  int len;

  while (ielen >= 2 && ielen >= ie[1])
  {
    switch (ie[0])
    {
      case 0: /* SSID */
        len = min(ie[1], ESSID_MAX_SIZE);
        memcpy(s->ssid, ie + 2, len);
        s->ssid[len] = 0;
        break;

      case 48: /* RSN */
        iwinfo_parse_rsn(&s->crypto, ie + 2, ie[1],
            CCMP, MGMT_8021x);
        break;

      case 221: /* Vendor */
        if (ie[1] >= 4 && !memcmp(ie + 2, ms_oui, 3) && ie[5] == 1)
          iwinfo_parse_rsn(&s->crypto, ie + 6, ie[1] - 4,
              TKIP, MGMT_PSK);
        break;
    }

    ielen -= ie[1] + 2;
    ie += ie[1] + 2;
  }
}

// From IW
/*static char *get_chain_signal(struct nlattr *attr_list)
{
  struct nlattr *attr;
  static char buf[64];
  char *cur = buf;
  int i = 0, rem;
  const char *prefix;

  if (!attr_list)
    return "";

  nla_for_each_nested(attr, attr_list, rem) {
    if (i++ > 0)
      prefix = ", ";
    else
      prefix = "[";

    cur += snprintf(cur, sizeof(buf) - (cur - buf), "%s%d", prefix,
        (int8_t) nla_get_u8(attr));
  }

  if (i)
    snprintf(cur, sizeof(buf) - (cur - buf), "] ");

  return buf;
}*/

void parse_bitrate(struct nlattr *bitrate_attr, int16_t *buf)
{
  int rate = 0;
  struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
  static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
    [NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
    [NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
    [NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
    [NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
    [NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
  };

  if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
        bitrate_attr, rate_policy)) {
    return;
  }

  if (rinfo[NL80211_RATE_INFO_BITRATE])
    rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
  if (rate > 0)
    rate = rate / 10;

  *buf = rate;
}

void parse_mcs(struct nlattr *bitrate_attr, int8_t *buf)
{

  int mcs = 0;
  struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
  static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
    [NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
    [NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
    [NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
    [NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
    [NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
  };

  if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
        bitrate_attr, rate_policy)) {
    return;
  }

  if (rinfo[NL80211_RATE_INFO_MCS])
    mcs = nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]);

  *buf = mcs;
}

int nl80211_init(void)
{
  int err;

  if (!nlstate)
  {
    nlstate = malloc(sizeof(struct nl80211_state));
    if (!nlstate) {
      err = -ENOMEM;
      goto err;
    }

    memset(nlstate, 0, sizeof(*nlstate));

    nlstate->nl_sock = nl_socket_alloc();
    if (!nlstate->nl_sock) {
      debug("Failed to allocate netlink socket.");
      err = -ENOMEM;
      goto err;
    }

    if (genl_connect(nlstate->nl_sock)) {
      debug("Failed to connect to generic netlink.");
      err = -ENOLINK;
      goto err;
    }

    nlstate->nl80211_id = genl_ctrl_resolve(nlstate->nl_sock, "nl80211");
    if (nlstate->nl80211_id < 0) {
      debug("nl80211 not found.");
      err = -ENOENT;
      goto err;
    }

    nl_socket_set_buffer_size(nlstate->nl_sock, 8192, 8192);
  }
  return 0;

err:
  nl80211_close();
  return err;
}

static struct nl80211_msg_conveyor * nl80211_new(int cmd, int flags)
{
  static struct nl80211_msg_conveyor cv;

  struct nl_msg *msg = nlmsg_alloc();

  if (!msg) {
    debug("Failed to allocate netlink message.");
    goto err;
  }

  struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (!cb) {
    debug("Failed to allocate netlink callback.");
    goto err;
  }

  genlmsg_put(msg, 0, 0, nlstate->nl80211_id, 0, flags, cmd, 0);

  cv.msg = msg;
  cv.cb  = cb;

  return &cv;

err:
  if (msg)
    nlmsg_free(msg);

  return NULL;
}

static struct nl80211_msg_conveyor * nl80211_msg(const char *ifname, int cmd, int flags)
{
  signed long long devidx = 0;
  struct nl80211_msg_conveyor *cv;

  if (nl80211_init() < 0) {
    return NULL;
  }

  if (ifname) {
    devidx = if_nametoindex(ifname);
    if (devidx <= 0) {
      debug("Interface not found. %s (%lld)", ifname, devidx);
      return NULL;
    }
  }

  cv = nl80211_new(cmd, flags);
  if (!cv) {
    return NULL;
  }

  if (devidx > 0) {
    NLA_PUT_U32(cv->msg, NL80211_ATTR_IFINDEX, devidx);
  }

  return cv;

nla_put_failure:
  nl80211_free(cv);
  return NULL;
}

static int get_link_freq(struct nl_msg *msg, void *arg)
{
  int32_t *freq = arg;
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
  static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
    [NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
    [NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
  };

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
      genlmsg_attrlen(gnlh, 0), NULL);

  if (!tb_msg[NL80211_ATTR_SURVEY_INFO]) {
    debug("survey data missing!");
    return NL_SKIP;
  }

  if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX,
        tb_msg[NL80211_ATTR_SURVEY_INFO],
        survey_policy)) {
    debug("failed to parse nested attributes!");
    return NL_SKIP;
  }

  if (!sinfo[NL80211_SURVEY_INFO_FREQUENCY]) {
    return NL_SKIP;
  }

  if (sinfo[NL80211_SURVEY_INFO_IN_USE] && !*freq) {
    *freq = (int32_t)nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
  }
  return NL_SKIP;

}

static int get_link_noise(struct nl_msg *msg, void *arg)
{
  int8_t *noise = arg;
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
  static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
    [NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
    [NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
  };

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
      genlmsg_attrlen(gnlh, 0), NULL);

  if (!tb_msg[NL80211_ATTR_SURVEY_INFO]) {
    debug("survey data missing!");
    return NL_SKIP;
  }

  if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX,
        tb_msg[NL80211_ATTR_SURVEY_INFO],
        survey_policy)) {
    debug("failed to parse nested attributes!");
    return NL_SKIP;
  }

  if (!sinfo[NL80211_SURVEY_INFO_NOISE]) {
    return NL_SKIP;
  }

  if (sinfo[NL80211_SURVEY_INFO_IN_USE] && !*noise) {
    *noise = (int8_t)nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);
  }
  return NL_SKIP;

}

int nl80211_get_noise(const char *ifname, int *buf)
{
  int8_t noise;
  struct nl80211_msg_conveyor *req;

  req = nl80211_msg(ifname, NL80211_CMD_GET_SURVEY, NLM_F_DUMP);
  if (req)
  {
    noise = 0;
    nl80211_send(req, get_link_noise, &noise);
    nl80211_free(req);

    if (noise)
    {
      *buf = noise;
      return 1;
    }
  }

  return 0;
}

int nl80211_get_freq(const char *ifname, int *buf)
{
  int32_t freq;
  struct nl80211_msg_conveyor *req;

  req = nl80211_msg(ifname, NL80211_CMD_GET_SURVEY, NLM_F_DUMP);
  if (req)
  {
    freq = 0;
    nl80211_send(req, get_link_freq, &freq);
    nl80211_free(req);

    if (freq)
    {
      *buf = freq;
      return 1;
    }
  }

  return 0;
}

static int get_bssid(struct nl_msg *msg, void *arg)
{

  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct nl80211_interface_stats *is = arg;

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
      genlmsg_attrlen(gnlh, 0), NULL);

  if (is->ssid && (tb_msg[NL80211_ATTR_SSID])) {
    memcpy(is->ssid, nla_data(tb_msg[NL80211_ATTR_SSID]), nla_len(tb_msg[NL80211_ATTR_SSID]));
    is->ssid[nla_len(tb_msg[NL80211_ATTR_SSID])] = '\0';
    return NL_SKIP;
  }

  if (tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) {
    uint32_t txp = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]);
    is->txpower = txp;
    return NL_SKIP;
  }

  if (tb_msg[NL80211_ATTR_MAC]) {
    memcpy(is->bssid, nla_data(tb_msg[NL80211_ATTR_MAC]), nla_len(tb_msg[NL80211_ATTR_MAC]));
    is->bssid[nla_len(tb_msg[NL80211_ATTR_MAC])] = '\0';
    return NL_SKIP;
  }

  return NL_SKIP;
}

static int nl80211_signal_cb(struct nl_msg *msg, void *arg)
{
  int8_t dbm;
  int16_t mbit;
  struct nl80211_rssi *rr = arg;

  struct nlattr *tb[NL80211_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
  struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
  /* struct nlattr *binfo[NL80211_STA_BSS_PARAM_MAX + 1]; */
  static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
    [NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
    [NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
    [NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
    [NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
    [NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
    [NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
    [NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
    [NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
  };
  static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
    [NL80211_RATE_INFO_BITRATE]      = { .type = NLA_U16  },
    [NL80211_RATE_INFO_MCS]          = { .type = NLA_U8   },
    [NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
    [NL80211_RATE_INFO_SHORT_GI]     = { .type = NLA_FLAG },
  };

  /* static struct nla_policy bss_policy[NL80211_STA_BSS_PARAM_MAX + 1] = { */
  /*   [NL80211_STA_BSS_PARAM_CTS_PROT] = { .type = NLA_FLAG }, */
  /*   [NL80211_STA_BSS_PARAM_SHORT_PREAMBLE] = { .type = NLA_FLAG }, */
  /*   [NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME] = { .type = NLA_FLAG }, */
  /*   [NL80211_STA_BSS_PARAM_DTIM_PERIOD] = { .type = NLA_U8 }, */
  /*   [NL80211_STA_BSS_PARAM_BEACON_INTERVAL] = { .type = NLA_U16 }, */
  /* }; */

  nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
      genlmsg_attrlen(gnlh, 0), NULL);

  if (!tb[NL80211_ATTR_STA_INFO]) {
    debug("sta stats missing!");
    return NL_SKIP;
  }
  if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
        tb[NL80211_ATTR_STA_INFO],
        stats_policy)) {
    debug("failed to parse nested attributes!");
    return NL_SKIP;
  }

  if (sinfo[NL80211_STA_INFO_SIGNAL]) {
    dbm = nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);
    rr->rssi = rr->rssi ? (int8_t)((rr->rssi + dbm) / 2) : dbm;
  }

  if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
    if (!nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
          sinfo[NL80211_STA_INFO_TX_BITRATE],
          rate_policy))
    {
      if (rinfo[NL80211_RATE_INFO_BITRATE])
      {
        mbit = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
        rr->rate = rr->rate ? (int16_t)((rr->rate + mbit) / 2) : mbit;
      }
    }
  }
  /* char buf[100]; */

  /* if (sinfo[NL80211_STA_INFO_BSS_PARAM]) { */
  /*   if (nla_parse_nested(binfo, NL80211_STA_BSS_PARAM_MAX, */
  /*         sinfo[NL80211_STA_INFO_BSS_PARAM], */
  /*         bss_policy)) { */
  /*     fprintf(stderr, "failed to parse nested bss parameters!\n"); */
  /*   } else { */
  /*     char *delim = ""; */
  /*     printf("\n\tbss flags:\t"); */
  /*     if (binfo[NL80211_STA_BSS_PARAM_CTS_PROT]) { */
  /*       printf("CTS-protection"); */
  /*       delim = " "; */
  /*     } */
  /*     if (binfo[NL80211_STA_BSS_PARAM_SHORT_PREAMBLE]) { */
  /*       printf("%sshort-preamble", delim); */
  /*       delim = " "; */
  /*     } */
  /*     if (binfo[NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME]) */
  /*       printf("%sshort-slot-time", delim); */
  /*     printf("\n\tdtim period:\t%d", */
  /*         nla_get_u8(binfo[NL80211_STA_BSS_PARAM_DTIM_PERIOD])); */
  /*     printf("\n\tbeacon int:\t%d", */
  /*         nla_get_u16(binfo[NL80211_STA_BSS_PARAM_BEACON_INTERVAL])); */
  /*     printf("\n"); */
  /*   } */
  /* } */

  return NL_SKIP;
}

static int get_stations(struct nl_msg *msg, void *arg)
{
  struct nlattr *tb[NL80211_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
  struct nl80211_sta_flag_update *sta_flags;

  struct nl80211_stationlist *sl = arg;

  static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
    [NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
    [NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
    [NL80211_STA_INFO_RX_BYTES64] = { .type = NLA_U64 },
    [NL80211_STA_INFO_TX_BYTES64] = { .type = NLA_U64 },
    [NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
    [NL80211_STA_INFO_BEACON_RX] = { .type = NLA_U64},
    [NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
    [NL80211_STA_INFO_T_OFFSET] = { .type = NLA_U64 },
    [NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
    [NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
    [NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
    [NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
    [NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
    [NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 },
    [NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
    [NL80211_STA_INFO_BEACON_LOSS] = { .type = NLA_U32},
    [NL80211_STA_INFO_RX_DROP_MISC] = { .type = NLA_U64},
    [NL80211_STA_INFO_STA_FLAGS] =
    { .minlen = sizeof(struct nl80211_sta_flag_update) },
    [NL80211_STA_INFO_LOCAL_PM] = { .type = NLA_U32},
    [NL80211_STA_INFO_PEER_PM] = { .type = NLA_U32},
    [NL80211_STA_INFO_NONPEER_PM] = { .type = NLA_U32},
    [NL80211_STA_INFO_CHAIN_SIGNAL] = { .type = NLA_NESTED },
    [NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = { .type = NLA_NESTED },
    [NL80211_STA_INFO_TID_STATS] = { .type = NLA_NESTED },
    [NL80211_STA_INFO_BSS_PARAM] = { .type = NLA_NESTED },
    [NL80211_STA_INFO_RX_DURATION] = { .type = NLA_U64 },
  };

  nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
      genlmsg_attrlen(gnlh, 0), NULL);

  if (!tb[NL80211_ATTR_STA_INFO]) {
    debug("sta stats missing!");
    return NL_SKIP;
  }
  if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
        tb[NL80211_ATTR_STA_INFO],
        stats_policy)) {
    debug("failed to parse nested attributes!");
    return NL_SKIP;
  }

  memset(sl->s, 0, sizeof(*sl->s));

  if (tb[NL80211_ATTR_MAC]) {
    memcpy(sl->s->mac, nla_data(tb[NL80211_ATTR_MAC]), 6);
  }

  if (sinfo[NL80211_STA_INFO_INACTIVE_TIME])
    sl->s->inactive_time = nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]);

  if (sinfo[NL80211_STA_INFO_RX_BYTES64])
    sl->s->rx_bytes = (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_RX_BYTES64]);

  else if (sinfo[NL80211_STA_INFO_RX_BYTES])
    sl->s->rx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]);

  if (sinfo[NL80211_STA_INFO_RX_PACKETS])
    sl->s->rx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]);

  if (sinfo[NL80211_STA_INFO_TX_BYTES64]) {
    sl->s->tx_bytes_64 = (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_TX_BYTES64]);
  }
  else if (sinfo[NL80211_STA_INFO_TX_BYTES])
    sl->s->tx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]);

  if (sinfo[NL80211_STA_INFO_TX_PACKETS])
    sl->s->tx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]);

  if (sinfo[NL80211_STA_INFO_TX_RETRIES])
    sl->s->tx_retries = nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]);

  if (sinfo[NL80211_STA_INFO_TX_FAILED])
    sl->s->tx_failed = nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]);

  if (sinfo[NL80211_STA_INFO_BEACON_LOSS])
    sl->s->beacon_loss = nla_get_u32(sinfo[NL80211_STA_INFO_BEACON_LOSS]);

  if (sinfo[NL80211_STA_INFO_BEACON_RX])
    sl->s->beacon_rx = (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_BEACON_RX]);

  if (sinfo[NL80211_STA_INFO_SIGNAL])
    sl->s->signal = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);

  if (sinfo[NL80211_STA_INFO_SIGNAL_AVG])
    sl->s->signal_avg = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]);

  if (sinfo[NL80211_STA_INFO_BEACON_SIGNAL_AVG])
    sl->s->beacon_signal_avg = nla_get_u8(sinfo[NL80211_STA_INFO_BEACON_SIGNAL_AVG]);

  if (sinfo[NL80211_STA_INFO_T_OFFSET])
    sl->s->t_offset = (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_T_OFFSET]);

  if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
    int16_t buf = 0;
    parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], &buf);
    sl->s->tx_bitrate = buf;
  }

  if (sinfo[NL80211_STA_INFO_RX_BITRATE]) {
    int16_t buf = 0;
    parse_bitrate(sinfo[NL80211_STA_INFO_RX_BITRATE], &buf);
    sl->s->rx_bitrate = buf;

    int8_t buff = 0;
    parse_mcs(sinfo[NL80211_STA_INFO_TX_BITRATE], &buff);
    sl->s->mcs = buff;
  }

  if (sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]) {
    uint32_t thr;

    thr = nla_get_u32(sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]);
    thr = thr * 1000 / 1024;
    sl->s->expected_tput = thr;
  }

  if (sinfo[NL80211_STA_INFO_STA_FLAGS]) {
    sta_flags = (struct nl80211_sta_flag_update *)
      nla_data(sinfo[NL80211_STA_INFO_STA_FLAGS]);

    if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
      if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHORIZED))
        sl->s->authorized = true;
      else
        sl->s->authorized = false;
    }

    if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
      if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHENTICATED))
        sl->s->authenticated = true;
      else
        sl->s->authenticated = true;
    }

    if (sta_flags->mask & BIT(NL80211_STA_FLAG_ASSOCIATED)) {
      if (sta_flags->set & BIT(NL80211_STA_FLAG_ASSOCIATED))
        sl->s->associated = true;
      else
        sl->s->associated = false;
    }

    if (sta_flags->mask & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
      if (sta_flags->set & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE))
        // Short
        sl->s->preamble = 1;
      else
        // Long
        sl->s->preamble = 2;
    }

    if (sta_flags->mask & BIT(NL80211_STA_FLAG_WME)) {
      if (sta_flags->set & BIT(NL80211_STA_FLAG_WME))
        sl->s->wmm = true;
      else
        sl->s->wmm = false;
    }

    if (sta_flags->mask & BIT(NL80211_STA_FLAG_MFP)) {
      if (sta_flags->set & BIT(NL80211_STA_FLAG_MFP))
        sl->s->mfp = true;
      else
        sl->s->mfp = false;
    }

    if (sta_flags->mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
      if (sta_flags->set & BIT(NL80211_STA_FLAG_TDLS_PEER))
        sl->s->tdls = true;
      else
        sl->s->tdls = false;
    }
  }

  if (sinfo[NL80211_STA_INFO_CONNECTED_TIME])
    sl->s->conn_time = nla_get_u32(sinfo[NL80211_STA_INFO_CONNECTED_TIME]);

  sl->s++;
  sl->len++;
  return NL_SKIP;
}

static int get_ssids(struct nl_msg *msg, void *arg)
{
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct nl80211_ssid_list *sl = arg;

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
      genlmsg_attrlen(gnlh, 0), NULL);

  if (sl) {
    memset(sl->e, 0, sizeof(*sl->e));

    if (tb_msg[NL80211_ATTR_WIPHY]) {
      unsigned int thiswiphy = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
      sl->e->phy = thiswiphy;
    }

    if (tb_msg[NL80211_ATTR_IFNAME]) {
      char *ifname = nla_data(tb_msg[NL80211_ATTR_IFNAME]);
      memcpy(sl->e->ifname, ifname, 10);
    }

    if (tb_msg[NL80211_ATTR_SSID]) {
      memcpy(sl->e->ssid, nla_data(tb_msg[NL80211_ATTR_SSID]), nla_len(tb_msg[NL80211_ATTR_SSID]));
      sl->e->ssid[nla_len(tb_msg[NL80211_ATTR_SSID])] = '\0';
    }

    if (tb_msg[NL80211_ATTR_WIPHY_FREQ]) {
      uint32_t freq = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FREQ]);

      int channel = ieee80211_frequency_to_channel(freq);
      sl->e->channel = channel;
    }

    sl->e++;
    sl->len++;
  }

  return NL_SKIP;
}

static void nl80211_signal(const char *ifname, struct nl80211_rssi *r)
{
  struct dirent;
  struct nl80211_msg_conveyor *req;

  r->rssi = 0;
  r->rate = 0;

  req = nl80211_msg(ifname, NL80211_CMD_GET_STATION, NLM_F_DUMP);

  if (req)
  {
    nl80211_send(req, nl80211_signal_cb, r);
    nl80211_free(req);
  }

}

int nl80211_get_signal(const char *ifname, int *buf)
{
  struct nl80211_rssi r;
  nl80211_signal(ifname, &r);

  if (r.rssi)
  {
    *buf = r.rssi;
    return 1;
  }

  return 0;
}

static int get_scan(struct nl_msg *msg, void *arg)
{
  int8_t rssi;
  struct nlattr *tb[NL80211_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *bss[NL80211_BSS_MAX + 1];
  char mac_addr[20];
  struct nl80211_scanlist *sl = arg;
  uint16_t caps = 0;

  static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
    [NL80211_BSS_TSF] = { .type = NLA_U64 },
    [NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
    [NL80211_BSS_BSSID] = { },
    [NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
    [NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
    [NL80211_BSS_INFORMATION_ELEMENTS] = { },
    [NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
    [NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
    [NL80211_BSS_STATUS] = { .type = NLA_U32 },
    [NL80211_BSS_SEEN_MS_AGO] = { .type = NLA_U32 },
    [NL80211_BSS_BEACON_IES] = { },
  };

  // Parse and error check.
  nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
  if (!tb[NL80211_ATTR_BSS]) {
    printf("bss info missing!\n");
    return NL_SKIP;
  }
  if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy)) {
    printf("failed to parse nested attributes!\n");
    return NL_SKIP;
  }
  if (!bss[NL80211_BSS_BSSID]) return NL_SKIP;
  if (!bss[NL80211_BSS_INFORMATION_ELEMENTS]) return NL_SKIP;


  memset(sl->s, 0, sizeof(*sl->s));
  mac_addr_n2a(mac_addr, nla_data(bss[NL80211_BSS_BSSID]));

  memcpy(sl->s->mac, nla_data(bss[NL80211_BSS_BSSID]), 6);

  uint32_t freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
  sl->s->channel = ieee80211_frequency_to_channel(freq);
  sl->s->freq = freq;

  if (bss[NL80211_BSS_CAPABILITY])
    caps = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);

  if (caps & (1<<4))
    sl->s->crypto.enabled = 1;

  if (bss[NL80211_BSS_SIGNAL_MBM]) {
    sl->s->signal =
      (uint8_t)((int32_t)nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]) / 100);

    rssi = sl->s->signal - 0x100;
    if (rssi < -110)
      rssi = -110;
    else if (rssi > -40)
      rssi = -40;

    sl->s->quality = (rssi + 110);
    sl->s->quality_max = 70;
  }

  if (bss[NL80211_BSS_SEEN_MS_AGO]) {
    int age = nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]);
    sl->s->age = age;
  }

  if (bss[NL80211_BSS_INFORMATION_ELEMENTS])
    nl80211_info_elements(bss, sl->s);

  sl->s++;
  sl->len++;

  return NL_SKIP;
}

static const char *ifmodes[NL80211_IFTYPE_MAX + 1] = {
  "unspecified",
  "IBSS",
  "managed",
  "AP",
  "AP/VLAN",
  "WDS",
  "monitor",
  "mesh point",
  "P2P-client",
  "P2P-GO",
  "P2P-device",
  "outside context of a BSS",
};

static char modebuf[100];

const char *iftype_name(enum nl80211_iftype iftype)
{
  if (iftype <= NL80211_IFTYPE_MAX && ifmodes[iftype])
    return ifmodes[iftype];
  sprintf(modebuf, "Unknown mode (%d)", iftype);
  return modebuf;
}

// From IW. Not in use.
void print_ht_capability(__u16 cap)
{
#define PRINT_HT_CAP(_cond, _str) \
  do { \
    if (_cond) \
    printf("\t\t\t" _str "\n"); \
  } while (0)

  printf("\t\tCapabilities: 0x%02x\n", cap);

  PRINT_HT_CAP((cap & BIT(0)), "RX LDPC");
  PRINT_HT_CAP((cap & BIT(1)), "HT20/HT40");
  PRINT_HT_CAP(!(cap & BIT(1)), "HT20");

  PRINT_HT_CAP(((cap >> 2) & 0x3) == 0, "Static SM Power Save");
  PRINT_HT_CAP(((cap >> 2) & 0x3) == 1, "Dynamic SM Power Save");
  PRINT_HT_CAP(((cap >> 2) & 0x3) == 3, "SM Power Save disabled");

  PRINT_HT_CAP((cap & BIT(4)), "RX Greenfield");
  PRINT_HT_CAP((cap & BIT(5)), "RX HT20 SGI");
  PRINT_HT_CAP((cap & BIT(6)), "RX HT40 SGI");
  PRINT_HT_CAP((cap & BIT(7)), "TX STBC");

  PRINT_HT_CAP(((cap >> 8) & 0x3) == 0, "No RX STBC");
  PRINT_HT_CAP(((cap >> 8) & 0x3) == 1, "RX STBC 1-stream");
  PRINT_HT_CAP(((cap >> 8) & 0x3) == 2, "RX STBC 2-streams");
  PRINT_HT_CAP(((cap >> 8) & 0x3) == 3, "RX STBC 3-streams");

  PRINT_HT_CAP((cap & BIT(10)), "HT Delayed Block Ack");

  PRINT_HT_CAP(!(cap & BIT(11)), "Max AMSDU length: 3839 bytes");
  PRINT_HT_CAP((cap & BIT(11)), "Max AMSDU length: 7935 bytes");

  PRINT_HT_CAP((cap & BIT(12)), "DSSS/CCK HT40");
  PRINT_HT_CAP(!(cap & BIT(12)), "No DSSS/CCK HT40");


  PRINT_HT_CAP((cap & BIT(14)), "40 MHz Intolerant");

  PRINT_HT_CAP((cap & BIT(15)), "L-SIG TXOP protection");
#undef PRINT_HT_CAP
}

// Not working, prints 4 times. Should sort loop.
static int print_info(struct nl_msg *msg, void *arg)
{
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
  //struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
  struct nl80211_info_list *sl = arg;

  /*static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
    [NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
    [NL80211_FREQUENCY_ATTR_DISABLED] = { .type = NLA_FLAG },
    [NL80211_FREQUENCY_ATTR_NO_IR] = { .type = NLA_FLAG },
    [__NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
    [NL80211_FREQUENCY_ATTR_RADAR] = { .type = NLA_FLAG },
    [NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32 },
  };*/


  struct nlattr *nl_band;
  int rem_band;
  static int64_t phy_id = -1;
  static int last_band = -1;

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
      genlmsg_attrlen(gnlh, 0), NULL);

  if (tb_msg[NL80211_ATTR_WIPHY]) {
    if (nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]) != phy_id)
      last_band = -1;

    phy_id = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
    sl->e->phy = phy_id;
  }

  if (tb_msg[NL80211_ATTR_WIPHY_BANDS]) {
    nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band) {
      last_band = nl_band->nla_type;
      if (last_band == 1) {
        sl->e->five = true;
      }

      nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band),
          nla_len(nl_band), NULL);

      if (tb_band[NL80211_BAND_ATTR_HT_CAPA]) {
        /* __u16 cap = nla_get_u16(tb_band[NL80211_BAND_ATTR_HT_CAPA]); */
        /* print_ht_capability(cap); */
      }

      if (tb_band[NL80211_BAND_ATTR_VHT_CAPA] &&
          tb_band[NL80211_BAND_ATTR_VHT_MCS_SET]) {
        sl->e->ac = true;
      }

      // Maybe use this later
      /* if (tb_band[NL80211_BAND_ATTR_FREQS]) { */
      /*   if (!band_had_freq) { */
      /*     printf("\t\tFrequencies:\n"); */
      /*     band_had_freq = true; */
      /*   } */
      /*   nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq) { */
      /*     uint32_t freq; */
      /*     nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX, nla_data(nl_freq), */
      /*         nla_len(nl_freq), freq_policy); */
      /*     if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ]) */
      /*       continue; */
      /*     freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]); */
      /*     printf("\t\t\t* %d MHz [%d]", freq, ieee80211_frequency_to_channel(freq)); */

      /*     if (tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] && */
      /*         !tb_freq[NL80211_FREQUENCY_ATTR_DISABLED]) */
      /*       printf(" (%.1f dBm)", 0.01 * nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER])); */
      /*   } */
      /* } */
    }
  }

  // This isn't getting the channel widths, something missing.
  if (0 && tb_msg[NL80211_ATTR_INTERFACE_COMBINATIONS]) {
    struct nlattr *nl_combi;
    int rem_combi;
    bool have_combinations = false;

    nla_for_each_nested(nl_combi, tb_msg[NL80211_ATTR_INTERFACE_COMBINATIONS], rem_combi) {
      static struct nla_policy iface_combination_policy[NUM_NL80211_IFACE_COMB] = {
        [NL80211_IFACE_COMB_LIMITS] = { .type = NLA_NESTED },
        [NL80211_IFACE_COMB_MAXNUM] = { .type = NLA_U32 },
        [NL80211_IFACE_COMB_STA_AP_BI_MATCH] = { .type = NLA_FLAG },
        [NL80211_IFACE_COMB_NUM_CHANNELS] = { .type = NLA_U32 },
        [NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS] = { .type = NLA_U32 },
      };
      struct nlattr *tb_comb[NUM_NL80211_IFACE_COMB];
      static struct nla_policy iface_limit_policy[NUM_NL80211_IFACE_LIMIT] = {
        [NL80211_IFACE_LIMIT_TYPES] = { .type = NLA_NESTED },
        [NL80211_IFACE_LIMIT_MAX] = { .type = NLA_U32 },
      };
      struct nlattr *tb_limit[NUM_NL80211_IFACE_LIMIT];
      struct nlattr *nl_limit;
      int err, rem_limit;
      bool comma = false;

      if (!have_combinations) {
        printf("\tvalid interface combinations:\n");
        have_combinations = true;
      }

      err = nla_parse_nested(tb_comb, MAX_NL80211_IFACE_COMB,
          nl_combi, iface_combination_policy);
      if (err || !tb_comb[NL80211_IFACE_COMB_LIMITS] ||
          !tb_comb[NL80211_IFACE_COMB_MAXNUM] ||
          !tb_comb[NL80211_IFACE_COMB_NUM_CHANNELS]) {
        printf(" <failed to parse>\n");
        goto broken_combination;
      }

      nla_for_each_nested(nl_limit, tb_comb[NL80211_IFACE_COMB_LIMITS], rem_limit) {

        err = nla_parse_nested(tb_limit, MAX_NL80211_IFACE_LIMIT,
            nl_limit, iface_limit_policy);
        if (err || !tb_limit[NL80211_IFACE_LIMIT_TYPES]) {
          printf("<failed to parse>\n");
          goto broken_combination;
        }

        if (comma)
          printf(", ");
        comma = true;
        printf("#{");

      }

      if (tb_comb[NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS]) {
        unsigned long widths = nla_get_u32(tb_comb[NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS]);

        if (widths) {
          int width;
          bool first = true;

          printf(", radar detect widths: {");
          for (width = 0; width < 32; width++)
            if (widths & (1 << width)) {
              printf("%s %s",
                  first ? "":",",
                  channel_width_name(width));
              first = false;
            }
        }
      }
broken_combination:
      ;
    }

    if (!have_combinations)
      printf("\tinterface combinations are not supported\n");
  }

  sl->e++;
  sl->len++;
  return NL_SKIP;
}

int nl80211_get_bssid(const char *ifname, char *buff)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_interface_stats is;

  is.ssid = NULL;

  req = nl80211_msg(ifname, NL80211_CMD_GET_INTERFACE, 0);
  if (req)
  {
    nl80211_send(req, get_bssid, &is);
    nl80211_free(req);
    if (is.bssid[0])
    {
      sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
          is.bssid[1], is.bssid[2], is.bssid[3],
          is.bssid[4], is.bssid[5], is.bssid[6]);

      return 1;
    }
  }
  return 0;
}

int nl80211_get_ssid(const char *ifname, char *buff)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_interface_stats is;

  is.ssid = (unsigned char *)buff;
  *buff = 0;

  req = nl80211_msg(ifname, NL80211_CMD_GET_INTERFACE, 0);
  if (req)
  {
    nl80211_send(req, get_bssid, &is);
    nl80211_free(req);
    return 1;
  }

  return 0;
}

int nl80211_get_txpower(const char *ifname, int *buff)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_interface_stats is;

  is.ssid = NULL;
  is.bssid[0] = 0;
  *buff = 0;

  req = nl80211_msg(ifname, NL80211_CMD_GET_INTERFACE, 0);
  if (req)
  {
    nl80211_send(req, get_bssid, &is);
    nl80211_free(req);
    if (is.txpower > 1)
      *buff = is.txpower / 100;
    return 1;
  }

  return 0;
}

int nl80211_get_bitrate(const char *ifname, int *buf)
{
  struct nl80211_rssi r;
  nl80211_signal(ifname, &r);

  if (r.rate)
  {
    *buf = (r.rate * 100);
    return 1;
  }

  return 0;
}

// From iwinfo.
int nl80211_get_quality(int signal, int *buf)
{
  if (signal >= 0)
  {
    *buf = signal;
  }

  /*  The cfg80211 wext compat layer assumes a signal range
   *  of -110 dBm to -40 dBm, the quality value is derived
   *  by adding 110 to the signal level */
  else
  {
    if (signal < -110)
      signal = -110;
    else if (signal > -40)
      signal = -40;

    *buf = (signal + 110);
  }

  return 1;
}

int nl80211_get_quality_max(int *buf)
{
  *buf = 70;
  return 1;
}

int nl80211_get_encryption(UNUSED(const char *ifname), UNUSED(char *buf))
{
  // Not implemented
  return 1;
}

int nl80211_get_ssids(char *buf, int *len)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_ssid_list sl = { .e = (struct iw_ssid_entry *)buf };

  req = nl80211_msg(NULL, NL80211_CMD_GET_INTERFACE, NLM_F_DUMP);
  if (req)
  {
    nl80211_send(req, get_ssids, &sl);
    nl80211_free(req);
  }

  *len = sl.len * sizeof(struct iw_ssid_entry);
  return *len ? 1 : 0;
}

int nl80211_get_ssids_basic(char *buf, int *len)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_ssid_list sl = { .e = (struct iw_ssid_entry *)buf };

  req = nl80211_msg(NULL, NL80211_CMD_GET_INTERFACE, NLM_F_DUMP);
  if (req)
  {
    nl80211_send(req, get_ssids, &sl);
    nl80211_free(req);
  }

  *len = sl.len * sizeof(struct iw_ssid_entry);
  return *len ? 0 : -1;
}

int nl80211_get_stations(const char *ifname, char *buf, int *len)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_stationlist sl = { .s = (struct iw_stationlist_entry *)buf };

  req = nl80211_msg(ifname, NL80211_CMD_GET_STATION, NLM_F_DUMP);
  if (req)
  {
    nl80211_send(req, get_stations, &sl);
    nl80211_free(req);
  }

  *len = sl.len * sizeof(struct iw_stationlist_entry);
  return *len ? 0 : -1;
}

int nl80211_get_info(char *buf, int *len)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_info_list sl = { .e = (struct iw_info_entry *)buf };

  req = nl80211_msg(NULL, NL80211_CMD_GET_WIPHY, NLM_F_DUMP);
  if (req)
  {

    nl80211_send(req, print_info, &sl);
    nl80211_free(req);
  }

  *len = sl.len * sizeof(struct iw_info_entry);
  return *len ? 1 : 0;
}

struct trigger_results {
  int done;
  int aborted;
};

static int callback_trigger(struct nl_msg *msg, void *arg) {
  // Called by the kernel when the scan is done or has been aborted.
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct trigger_results *results = arg;

  if (gnlh->cmd == NL80211_CMD_SCAN_ABORTED) {
    printf("Scan aborted!\n");
    results->done = 1;
    results->aborted = 1;
  } else if (gnlh->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
    printf("Got scan results.\n");
    results->done = 1;
    results->aborted = 0;
  }  // else probably an uninteresting multicast message.

  return NL_SKIP;
}

int do_scan_trigger()
{
  struct trigger_results results = { .done = 0, .aborted = 0 };
  struct nl_msg *msg;
  struct nl_cb *cb;
  struct nl_msg *ssids_to_scan;
  int err;
  int mcid = nl_get_multicast_id(nlstate->nl_sock, "nl80211", "scan");

  nl_socket_add_membership(nlstate->nl_sock, mcid);

  msg = nlmsg_alloc();
  if (!msg) {
    printf("ERROR: Failed to allocate netlink message for msg.\n");
    return -ENOMEM;
  }
  ssids_to_scan = nlmsg_alloc();
  if (!ssids_to_scan) {
    printf("ERROR: Failed to allocate netlink message for ssids_to_scan.\n");
    nlmsg_free(msg);
    return -ENOMEM;
  }
  cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (!cb) {
    printf("ERROR: Failed to allocate netlink callbacks.\n");
    nlmsg_free(msg);
    nlmsg_free(ssids_to_scan);
    return -ENOMEM;
  }

  nla_put(ssids_to_scan, 1, 0, "");
  nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS, ssids_to_scan);
  nlmsg_free(ssids_to_scan);

  nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, callback_trigger, &results);
  nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
  nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
  nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);
  nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);

  printf("Waiting for scan to complete...\n");

  // Needs error check
  while (!results.done)
    nl_recvmsgs(nlstate->nl_sock, cb);

  printf("Scan is done.\n");

  nlmsg_free(msg);
  nl_cb_put(cb);
  return 0;
}

int nl80211_run_scan(const char *ifname, char *buf, int *len)
{
  struct nl80211_msg_conveyor *req;
  struct nl80211_scanlist sl = { .s = (struct iw_scanlist_entry *)buf };

  debug("Starting the scan....");
  req = nl80211_msg(ifname, NL80211_CMD_TRIGGER_SCAN, 0);
  if (req)
  {
    nl80211_send(req, NULL, NULL);
    nl80211_free(req);
  }

  int err = do_scan_trigger();
  if (err != 0) {
    printf("do_scan_trigger() failed with %d.\n", err);
    return err;
  }

  req = nl80211_msg(ifname, NL80211_CMD_GET_SCAN, NLM_F_DUMP);
  if (req)
  {
    nl80211_send(req, get_scan, &sl);
    nl80211_free(req);
  }

  *len = sl.len * sizeof(struct iw_scanlist_entry);
  return *len ? 1 : 0;
}

int nl80211_disconnect(char *buf)
{
  debug("Disconnecting client %s", buf);
  struct nl80211_msg_conveyor *req;

  req = nl80211_msg("wlan1", NL80211_CMD_DEL_STATION, 0);
  if (req)
  {

    unsigned char mac_addr[ETH_ALEN];

    if (mac_addr_a2n(mac_addr, buf)) {
      debug("invalid mac address");
      return 2;
    }
    NLA_PUT(req->msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);
    NLA_PUT_U8(req->msg, NL80211_ATTR_MGMT_SUBTYPE, 10);

    nl80211_send(req, NULL, NULL);
    nl80211_free(req);
    return 0;
  }

nla_put_failure:
  return 1;
}

const struct iw_ops nl80211_exec = {
  .name           = "nl80211",
  .txpower        = nl80211_get_txpower,
  .bitrate        = nl80211_get_bitrate,
  .signal         = nl80211_get_signal,
  .noise          = nl80211_get_noise,
  .freq           = nl80211_get_freq,
  .ssids          = nl80211_get_ssids,
  .quality        = nl80211_get_quality,
  .quality_max    = nl80211_get_quality_max,
  .ssid           = nl80211_get_ssid,
  .bssid          = nl80211_get_bssid,
  .scan           = nl80211_run_scan,
  .encryption     = nl80211_get_encryption,
  .stations       = nl80211_get_stations,
  .info           = nl80211_get_info,
  .disconnect     = nl80211_disconnect
};
