/*
 * =====================================================================================
 *
 *       Filename:  socketman.c
 *
 *    Description:
 *
 *        Version:  2.0
 *        Created:  01/08/16 00:05:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tony and Friends,
 *   Organization:  Cucumber Tony Limited
 *
 * =====================================================================================
 */
#include <unistd.h>
#include <json-c/json.h>
#include <getopt.h>
#include "dbg.h"
#include "helper.h"
#include <syslog.h>
#include <stdio.h>
#include "options.h"
#include "collector.h"
#include "boot.h"
#include <string.h>
#include <signal.h>
#include "mqtt.h"
#include "platform.h"
#include <sys/wait.h>
#include <curl/curl.h>
#include "mqtt.h"

int verbose_flag, cpid;
/* volatile sig_atomic_t g_eflag = 0; */
/* volatile sig_atomic_t g_hupflag = 1; */

void sig_handler(int signo) {
  /*   kill(parent, SIGTERM); */
  /*   kill(0, SIGTERM); */
  /*   debug("received  EXIT or KILL signal going to exit \n"); */
  /*   openlog("Socketman", LOG_PID|LOG_CONS, LOG_USER); */
  /*   syslog(LOG_INFO, "Received signal for pause or kill now "); */
  /*   closelog(); */
  exit(0);
}

void handle_signal(int signal) {
  const char *signal_name;
  sigset_t pending;

  switch (signal) {
    case SIGHUP:
      signal_name = "SIGHUP";
      break;
    case SIGUSR1:
      signal_name = "SIGUSR1";
      break;
    case SIGINT:
      /* mqtt_alarm = 1; */
      printf("Caught SIGINT, exiting now\n");
      sleep(1);
      exit(0);
    case SIGTERM:
      /* mqtt_alarm = 1; */
      printf("Caught SIGTERM, exiting now\n");
      sleep(1);
      exit(0);
    default:
      fprintf(stderr, "Caught wrong signal: %d\n", signal);
      return;
  }

  /*
   * Please note that printf et al. are NOT safe to use in signal handlers.
   * Look for async safe functions.
   */
  printf("Caught %s, sleeping for ~3 seconds\n"
      "Try sending another SIGHUP / SIGINT / SIGALRM "
      "(or more) meanwhile\n", signal_name);

  // So what did you send me while I was asleep?
  sigpending(&pending);
  if (sigismember(&pending, SIGHUP)) {
    printf("A SIGHUP is waiting\n");
  }
  if (sigismember(&pending, SIGUSR1)) {
    printf("A SIGUSR1 is waiting\n");
  }

  printf("Done handling %s\n\n", signal_name);
}

void validate_options()
{
  if (strlen(options.config) == 0) {
    debug("Use a config file, not arguments.");
    if (strlen(options.username) == 0) {
      debug("MQTT username is required");
      exit(EXIT_FAILURE);
    }

    if (strlen(options.password) == 0) {
      debug("MQTT password is required");
      exit(EXIT_FAILURE);
    }

    if (strlen(options.topic) == 0) {
      debug("MQTT topic is required");
      exit(EXIT_FAILURE);
    }

    if (strlen(options.mqtt_host) == 0) {
      debug("MQTT host is required");
      exit(EXIT_FAILURE);
    }

    if (strlen(options.key) == 0) {
      debug("MQTT key is required");
      exit(EXIT_FAILURE);
    }

    if (options.port == 0) {
      debug("MQTT port is required");
      exit(EXIT_FAILURE);
    }

    if (strlen(options.url) == 0) {
      debug("API URI is required");
      exit(EXIT_FAILURE);
    }

    if (strlen(options.mac) == 0) {
      readlineToBuffer(options.mac_file, options.mac);
      if (strlen(options.mac) == 0) {
        debug("No mac address found in file");
        exit(EXIT_FAILURE);
      } else {
        debug("Setting MAC to: %s", options.mac);
      }
    }
  }
}

/* static void child_handler(int sig) */
/* { */
/*   pid_t pid; */
/*   int status; */

/*   debug("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); */
/* } */

int main( int argc,char **argv)
{
  int c;

  /* struct sigaction sa; */
  /* sigemptyset(&sa.sa_mask); */
  /* sa.sa_flags = 0; */
  /* sa.sa_handler = child_handler; */

  /* sigaction(SIGCHLD, &sa, NULL); */

  /* if (signal(SIGHUP, sig_handler) == SIG_ERR) */
  /*   debug("n't catch SIGHUP\n"); */
  /* if (signal(SIGSEGV, sig_handler) == SIG_ERR) */
  /*   printf("n't catch SIGSEGV\n"); */
  /* if (signal(SIGINT, sig_handler) == SIG_ERR) */
  /*   printf("n't catch SIGINTV\n"); */
  /* if (signal(SIGTERM, sig_handler) == SIG_ERR) */
  /*   printf("n't catch SIGTERM\n"); */
  /* if (signal(SIGTSTP,sig_handler) == SIG_ERR) */
  /*   printf("n't catch SIGTSTP\n"); */
  /* if (signal(SIGBUS,sig_handler) == SIG_ERR) */
  /*   printf("n't catch SIGBUS\n"); */
  /* if (signal(SIGABRT,sig_handler) == SIG_ERR) */
  /*   printf("n't catch SIGABRT\n"); */

  /* signal(SIGCHLD, SIG_IGN); /1*  Silently (and portably) reap children. *1/ */

  struct sigaction sa;

  printf("My pid is: %d\n", getpid());

  sa.sa_handler = &handle_signal;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);

  // Intercept SIGHUP and SIGINT
  if (sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGHUP"); // Should not happen
  }

  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGUSR1"); // Should not happen
  }

  // Will always fail, SIGKILL is intended to force kill your process
  if (sigaction(SIGKILL, &sa, NULL) == -1) {
    perror("Cannot handle SIGKILL"); // Will always happen
    printf("You can never handle SIGKILL anyway...\n");
  }

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGINT"); // Should not happen
  }

  // Implement better args
  while(1)
  {
    static struct option long_options[]=
    {
      {"debug",        no_argument,          &verbose_flag,  1 },
      {"help",         no_argument,          0,              'h'},
      {"username",     required_argument,    0,              'u'},
      {"noscan",       no_argument,          0,              'e'},
      {"nosurvey",     no_argument,          0,              'x'},
      {"password",     required_argument,    0,              'p'},
      {"sleep",        required_argument,    0,              's'},
      {"port",         required_argument,    0,              'P'},
      {"qos",          required_argument,    0,              'q'},
      {"topic",        required_argument,    0,              't'},
      {"key",          required_argument,    0,              'k'},
      {"host",         required_argument,    0,              'H'},
      {"url",          required_argument,    0,              'a'},
      {"mac",          required_argument,    0,              'm'},
      {"mac-file",     required_argument,    0,              'w'},
      {"tls",          no_argument,          &options.tls,   1  },
      {"psk",          required_argument,    0,              'k'},
      {"token",        required_argument,    0,              'T'},
      {"config",       required_argument,    0,              'C'},
      {0,0,0,0}
    };

    int option_index = 0;
    c = getopt_long_only(argc,argv,"h:u:p:e:x:s:P:qt:k:H:a:m:f:k:T:C",long_options, &option_index);

    if (c == -1)
      break;

    switch(c)
    {
      case 0:

        if (long_options[option_index].flag != 0)
          break;
        if (optarg)
          printf (" with arg %s", optarg);
        printf ("\n");
        break;

      case 'u':
        strcpy(options.username, optarg);
        break;

      case 'e':
        options.no_scan = 1;
        break;

      case 'x':
        options.no_survey = 1;
        break;

      case 'p':
        strcpy(options.password, optarg);
        break;

      case 's':
        options.sleep = atoi(optarg);
        break;

      case 'P':
        options.port = atoi(optarg);
        break;

      case 'H':
        strcpy(options.mqtt_host, optarg);
        break;

      case 'l':
        options.tls = atoi(optarg);
        break;

      case 'm':
        strcpy(options.mac, optarg);
        break;

      case 'w':
        strcpy(options.mac_file, optarg);
        break;

      case 't':
        strcpy(options.topic, optarg);
        break;

      case 'a':
        strcpy(options.url, optarg);
        break;

      case 'k':
        strcpy(options.key, optarg);
        break;

        /* case 'q': */
        /*   qos = atoi(optarg); */
        /*   break; */

      case 'C':
        strcpy(options.config, optarg);
        break;

      case 'T':
        strcpy(options.token, optarg);
        break;

      case 'h':
        printf("\n------------------SocketMan 3.0\n");
        exit(0);
        break;

      case '?':
        break;
      default:
        printf("No arguments sent, ignoring\n");
    }
  }

  validate_options();

  if (strcmp(OS, "OPENWRT") == 0)
    debug("I am an OpenWRT box, yay!");

  boot();

  return 1;
}
