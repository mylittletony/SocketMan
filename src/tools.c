#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include "options.h"
#include "dbg.h"
#include "platform.h"
#include "network.h"
#include "system.h"
#include <compiler.h>

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif

int open_socket(char *ip, int port)
{
  int error = 0; // Socket error
  struct sockaddr_in address;
  short int sock = -1;
  fd_set fdset;
  struct timeval tv;
  int so_keepalive = 0;

  sock = socket(PF_INET, SOCK_STREAM , 0);
  if (sock < 0)
    return -1;

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(ip);
  address.sin_port = htons(port);

  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  tv.tv_sec =  3;
  tv.tv_usec = 0;

  int yes = 1;
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &so_keepalive, sizeof(so_keepalive));

  if (connect(sock, (struct sockaddr *)&address , sizeof(address)) < 0)
    error = -1;

  char *message = "HELLO";
  if ((error == 0) && (send(sock , message , strlen(message) , 0) < 0))
    error = 180;

  close(sock);
  return error;
}

int health_check(char *url, int port)
{
  struct addrinfo *result;
  struct in_addr addr;
  int error;

  char *t = "DNS";
  if (port > 53) {
    t = "WEB";
  }

  error = getaddrinfo(url, NULL, NULL, &result);
  if (error != 0)
  {
    debug("%s Lookup Failed: %s", t, gai_strerror(error));
    return -1;
  }

  addr.s_addr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;
  debug("\nUsing %s for %s check", inet_ntoa(addr), t);
  freeaddrinfo(result);

  return open_socket(inet_ntoa(addr), port);
}

// Connection Check tests the route, DNS and IP and returns a
// value greater than 0 if it was a (semi) success.
// FAIL = 0, IP = 1, IP+DNS = 4, IP+HTTP = 6, IP+DNS+HTTP = 9
// Use 1,3,5 primary number logic to get to this result
int connection_check()
{
  int result = 0;

  struct timespec tstart={0,0}, tend={0,0};
  if (options.debug) {
    clock_gettime(CLOCK_MONOTONIC, &tstart);
  }

  // Check IP - return if we do not have an IP - result 0
  struct defaultRoute dr = route();
  if (strlen(dr.ip) == 0) {
    return result;
  }
  result = 1;

  // Run DNS check
  int dns = health_check(options.health_url, options.health_port);
  if (dns >= 0) {
    result = result + 3;
  }

  // Check 80 - change to customisable value
  int web = health_check("www.google.com", 80);
  if (web >= 0) {
    result = result + 5;
  }

  if (options.debug) {
    clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("Connection check finished in %.5f seconds\n",
        ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
        ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
  }

  return result;
}

int copy_file(char *from, char *to)
{
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);

  if (fd_from < 0)
    return 0;

  fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, 644);
  if (fd_to < 0)
    goto out_error;

  while (nread = read(fd_from, buf, sizeof buf), nread > 0)
  {
    char *out_ptr = buf;
    ssize_t nwritten;

    do {
      nwritten = write(fd_to, out_ptr, nread);

      if (nwritten >= 0)
      {
        nread -= nwritten;
        out_ptr += nwritten;
      }
      else if (errno != EINTR)
      {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0)
  {
    if (close(fd_to) < 0)
    {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);
    return 1;
  }

out_error:
  saved_errno = errno;

  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);

  errno = saved_errno;
  return 0;
}

void flag(UNUSED(char *error)) {
  // What to do?
  // Not sure where to log
}

