#define _GNU_SOURCE
#define BUFSIZE 16384
#define MYPROTO NETLINK_ROUTE
#define MYMGRP RTMGRP_IPV4_ROUTE

#ifndef _LINUX_IF_H
#define _LINUX_IF_H
#endif                          /* _LINUX_IF_H */

#include <stdio.h>
#include <unistd.h>
#include "dbg.h"
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <inttypes.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include "platform.h"
#include "network.h"
#include <compiler.h>

struct route_info {
  struct in_addr dstAddr;
  struct in_addr srcAddr;
  struct in_addr gateWay;
  char ifName[IF_NAMESIZE];
};

struct interface {
  int     index;
  int     flags;      /* IFF_UP etc. */
  long    speed;      /* Mbps; -1 is unknown */
  int     duplex;     /* DUPLEX_FULL, DUPLEX_HALF, or unknown */
  char    name[IF_NAMESIZE + 1];
};

// Much of this logic was taken from StackO and other linux forums
int readNlSock(int sockFd, char *bufPtr, __u32 seqNum, pid_t pId)
{
  struct nlmsghdr *nlHdr;
  int readLen = 0, msgLen = 0;

  do {
    /* Recieve response from the kernel */
    if ((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0) {
      perror("SOCK READ: ");
      return -1;
    }

    nlHdr = (struct nlmsghdr *) bufPtr;

    /* Check if the header is valid */
    if ((NLMSG_OK(nlHdr, readLen) == 0)
        || (nlHdr->nlmsg_type == NLMSG_ERROR)) {
      perror("Error in recieved packet");
      return -1;
    }

    /* Check if the its the last message */
    if (nlHdr->nlmsg_type == NLMSG_DONE) {
      break;
    } else {
      /* Else move the pointer to buffer appropriately */
      bufPtr += readLen;
      msgLen += readLen;
    }

    /* Check if its a multi part message */
    if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0) {
      /* return if its not */
      break;
    }
  } while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != (__u32)pId));
  return msgLen;
}

void parseRoutes(struct nlmsghdr *nlHdr,
    struct route_info *rtInfo,
    struct defaultRoute *dr
    )
{
  struct rtmsg *rtMsg;
  struct rtattr *rtAttr;
  int rtLen;

  rtMsg = (struct rtmsg *) NLMSG_DATA(nlHdr);

  /* If the route is not for AF_INET or does not belong to main routing table
     then return. */
  if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
    return;

  /* get the rtattr field */
  rtAttr = (struct rtattr *) RTM_RTA(rtMsg);
  rtLen = RTM_PAYLOAD(nlHdr);
  for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
    switch (rtAttr->rta_type) {
      case RTA_OIF:
        if_indextoname(*(int *) RTA_DATA(rtAttr), rtInfo->ifName);
        break;
      case RTA_GATEWAY:
        rtInfo->gateWay.s_addr= *(u_int *) RTA_DATA(rtAttr);
        break;
      case RTA_PREFSRC:
        rtInfo->srcAddr.s_addr= *(u_int *) RTA_DATA(rtAttr);
        break;
      case RTA_DST:
        rtInfo->dstAddr .s_addr= *(u_int *) RTA_DATA(rtAttr);
        break;
    }
  }

  if (rtInfo->dstAddr.s_addr == 0) {
    snprintf(dr->if_name, sizeof(dr->if_name), "%s", (char *) rtInfo->ifName);
    snprintf(dr->ip, sizeof(dr->ip), "%s", (char *)inet_ntoa(rtInfo->gateWay));
  }

  return;
}

struct defaultRoute route()
{
  struct defaultRoute dr = { 0 };
  struct nlmsghdr *nlMsg;
  struct route_info *rtInfo;
  char msgBuf[BUFSIZE];
  int sock, len;
  __u32 msgSeq = 0;

  if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
    debug("Socket creation failed, exiting.");
    close(sock);
    return dr;
  }

  memset(msgBuf, 0, BUFSIZE);

  nlMsg = (struct nlmsghdr *) msgBuf;

  nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  nlMsg->nlmsg_type = RTM_GETROUTE;

  nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
  nlMsg->nlmsg_seq = msgSeq++;
  nlMsg->nlmsg_pid = getpid();

  if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0) {
    debug("Write To Socket Failed...");
    close(sock);
    return dr;
  }

  if ((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {
    debug("Read From Socket Failed...");
    close(sock);
    return dr;
  }

  rtInfo = (struct route_info *) malloc(sizeof(struct route_info));

  if (rtInfo == NULL) {
    close(sock);
    return dr;
  }

  for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
    memset(rtInfo, 0, sizeof(struct route_info));
    parseRoutes(nlMsg, rtInfo, &dr);
  }
  free(rtInfo);
  close(sock);

  return dr;
}

void recover_network() {
  // Not implemented yet
}

void restart_network() {
#ifdef __OPENWRT__
    system("/etc/init.d/network restart");
    sleep(1);
    system("/etc/init.d/firewall restart");
#elif defined __linux
    system("/etc/init.d/network restart");
#endif
}

static int get_interface_common(const int fd, struct ifreq *const ifr, struct interface *const info)
{
  struct ethtool_cmd  cmd;
  int                 result;

  if (ioctl(fd, SIOCGIFFLAGS, ifr) == -1)
    info->flags = 0;
  else
    info->flags = ifr->ifr_flags;

  ifr->ifr_data = (void *)&cmd;
  cmd.cmd = ETHTOOL_GSET; /* "Get settings" */
  if (ioctl(fd, SIOCETHTOOL, ifr) == -1) {
    info->speed = -1L;
    info->duplex = DUPLEX_UNKNOWN;
  } else {
    info->speed = ethtool_cmd_speed(&cmd);
    info->duplex = cmd.duplex;
  }

  do {
    result = close(fd);
  } while (result == -1 && errno == EINTR);
  if (result == -1)
    return errno;

  return 0;
}

int get_interface_by_name(const char *const name, struct interface *const info)
{
  int             socketfd, result;
  struct ifreq    ifr;

  if (!name || !*name || !info)
    return errno = EINVAL;

  socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (socketfd == -1)
    return errno;

  strncpy(ifr.ifr_name, name, IF_NAMESIZE);
  if (ioctl(socketfd, SIOCGIFINDEX, &ifr) == -1) {
    do {
      result = close(socketfd);
    } while (result == -1 && errno == EINTR);
    return errno = ENOENT;
  }

  info->index = ifr.ifr_ifindex;
  strncpy(info->name, name, IF_NAMESIZE);
  info->name[IF_NAMESIZE] = '\0';

  /* https://lists.samba.org/archive/samba-technical/2016-March/112887.html */
  return get_interface_common(socketfd, &ifr, info);
}

int monitor_interface(char *nic)
{
  struct interface    iface;
  int                 status = 0;

  if (get_interface_by_name(nic, &iface) != 0)
    return 0;

  printf("%s: Interface %d", iface.name, iface.index);
  if (iface.flags & IFF_UP)
    printf(", up");
  if (iface.duplex == DUPLEX_FULL)
    printf(", full duplex");
  else
    if (iface.duplex == DUPLEX_HALF)
      printf(", half duplex");
  if (iface.speed > 0)
    printf(", %ld Mbps", iface.speed);
  printf("\n");

  return status;
}

/* http://www.mail-archive.com/davinci-linux-open-source@linux.davincidsp.com/msg05405/netlink_test.c * */
int open_netlink()
{
  int sock = socket(AF_NETLINK,SOCK_RAW,MYPROTO);
  struct sockaddr_nl addr;

  memset((void *)&addr, 0, sizeof(addr));

  if (sock<0)
    return sock;
  addr.nl_family = AF_NETLINK;
  addr.nl_pid = getpid();
  addr.nl_groups = RTMGRP_LINK|RTMGRP_IPV4_IFADDR|RTMGRP_IPV6_IFADDR;
  if (bind(sock,(struct sockaddr *)&addr,sizeof(addr))<0) {
    close(sock);
    return -1;
  }
  return sock;
}


int read_event(int sockint, int (*msg_handler)(struct sockaddr_nl *,
      struct nlmsghdr *))
{
  int status;
  int ret = 0;
  char buf[4096];
  struct iovec iov = { buf, sizeof buf };
  struct sockaddr_nl snl;
  struct msghdr msg = { (void*)&snl, sizeof snl, &iov, 1, NULL, 0, 0};
  struct nlmsghdr *h;

  status = recvmsg(sockint, &msg, 0);

  if(status < 0)
  {
    /* Socket non-blocking so bail out once we have read everything */
    if (errno == EWOULDBLOCK || errno == EAGAIN)
      return ret;

    /* Anything else is an error */
    printf("read_netlink: Error recvmsg: %d\n", status);
    perror("read_netlink: Error: ");
    return status;
  }

  if(status == 0)
  {
    printf("read_netlink: EOF\n");
    return ret;
  }

  /* We need to handle more than one message per 'recvmsg' */
  for(h = (struct nlmsghdr *) buf; NLMSG_OK (h, (unsigned int)status);
      h = NLMSG_NEXT (h, status))
  {
    /* Finish reading */
    if (h->nlmsg_type == NLMSG_DONE)
      return ret;

    /* Message is some kind of error */
    if (h->nlmsg_type == NLMSG_ERROR)
    {
      printf("read_netlink: Message is an error - decode TBD\n");
      return -1; // Error
    }

    /* Call message handler */
    if(msg_handler)
    {
      ret = (*msg_handler)(&snl, h);
      if(ret < 0)
      {
        printf("read_netlink: Message hander error %d\n", ret);
        return ret;
      }
    }
    else
    {
      printf("read_netlink: Error NULL message handler\n");
      return -1;
    }
  }

  return ret;
}

/*static int netlink_link_state(UNUSED(struct sockaddr_nl *nl), struct nlmsghdr *msg)
{
  struct ifinfomsg *ifi;

  ifi = NLMSG_DATA(msg);
  char ifname[1024];if_indextoname(ifi->ifi_index,ifname);

  printf("netlink_link_state: Link %s %s\n",
      //(ifi->ifi_flags & IFF_RUNNING)?"Up":"Down");
    ifname,(ifi->ifi_flags & IFF_UP)?"Up":"Down");
  return 0;
}*/

/*static int msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg)
{
  struct ifinfomsg *ifi=NLMSG_DATA(msg);
  struct ifaddrmsg *ifa=NLMSG_DATA(msg);
  char ifname[1024];
  switch (msg->nlmsg_type)
  {
    case RTM_NEWADDR:
      if_indextoname(ifi->ifi_index,ifname);
      printf("msg_handler: RTM_NEWADDR : %s\n",ifname);
      break;
    case RTM_DELADDR:
      if_indextoname(ifi->ifi_index,ifname);
      printf("msg_handler: RTM_DELADDR : %s\n",ifname);
      break;
    case RTM_NEWLINK:
      if_indextoname(ifa->ifa_index,ifname);
      printf("msg_handler: RTM_NEWLINK\n");
      netlink_link_state(nl, msg);
      break;
    case RTM_DELLINK:
      if_indextoname(ifa->ifa_index,ifname);
      printf("msg_handler: RTM_DELLINK : %s\n",ifname);
      break;
    default:
      printf("msg_handler: Unknown netlink nlmsg_type %d\n",
          msg->nlmsg_type);
      break;
  }
  return 0;
}*/


void interface_ip(char *interface, char *wan_ip, size_t len)
{
  struct ifaddrs *ifaddr, *ifa;
  int family, n;
  struct sockaddr_in *sa;
  char *addr;

  if (getifaddrs(&ifaddr) == -1) {
    return;
  }

  for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
    if (ifa->ifa_addr == NULL)
      continue;

    family = ifa->ifa_addr->sa_family;

    if (family == AF_INET) {
      int rc = strcmp(ifa->ifa_name, interface);
      if (rc == 0) {
        sa = (struct sockaddr_in *) ifa->ifa_addr;
        addr = inet_ntoa(sa->sin_addr);
        snprintf(wan_ip, len, "%s", (char *) addr);
        break;
      }
    }
  }

  freeifaddrs(ifaddr);
}

struct InterfaceStats stats(char *interface)
{
  struct rtnl_link *link;
  struct nl_sock *socket;
  uint64_t kbytes_in = 0, kbytes_out = 0, rxerrors = 0, txerrors = 0;

  socket = nl_socket_alloc();
  nl_connect(socket, NETLINK_ROUTE);

  if (rtnl_link_get_kernel(socket, 0, interface, &link) >= 0) {
    kbytes_in = rtnl_link_get_stat(link, RTNL_LINK_RX_BYTES);
    kbytes_out = rtnl_link_get_stat(link, RTNL_LINK_TX_BYTES);
    rxerrors = rtnl_link_get_stat(link, RTNL_LINK_RX_ERRORS);
    txerrors = rtnl_link_get_stat(link, RTNL_LINK_TX_ERRORS);

    rtnl_link_put(link);
  }

  struct InterfaceStats r = { kbytes_out, kbytes_in, txerrors, rxerrors };
  nl_socket_free(socket);
  return r;
}
