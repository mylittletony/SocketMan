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
#include "system.h"

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif

/// CONFIG ///
int port = 53;
char *hostname = "health.cucumberwifi.io";
/// CONFIG ///

void flag(char *error) {
  // What to do?
  // Not sure where to log
}

int open_socket(char *ip)
{
  debug("CHECKING THE SOCKET!");
  struct sockaddr_in address;
  short int sock = -1;
  fd_set fdset;
  struct timeval tv;

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(ip);
  address.sin_port = htons(port);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);

  connect(sock, (struct sockaddr *)&address, sizeof(address));

  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  tv.tv_sec =  3;
  tv.tv_usec = 0;

  if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1)
  {
    int so_error;
    socklen_t len = sizeof so_error;
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error == 0) {
      return 1;
    }
  }

  flag("SOCKET");
  close(sock);
  return 0;
}

int connection_check()
{
  struct addrinfo *result;
  /* struct addrinfo *res; */
  struct in_addr addr;

  int error;

  error = getaddrinfo(hostname, NULL, NULL, &result);
  if (error != 0)
  {
    flag("DNS");
    fprintf(stderr, "DNS Lookup Failed: %s\n", gai_strerror(error));
    return 0;
  }

  addr.s_addr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;

  printf("\nUsing %s for internet check\n", inet_ntoa(addr));

  freeaddrinfo(result);

  return(open_socket(inet_ntoa(addr)));
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

void recover_network() {
  // Not implemented yet
  /* if (strcmp(OS, "OPENWRT") == 0); */
  /* if (strcmp(OS, "LINUX") == 0); */
}
