#ifndef OPTIONS_STRUCT
#define OPTIONS_STRUCT
struct _options {
  char username[20];
  char password[20];
  char topic[128];
  char key[128];
  char config[128];
  char mqtt_host[128];
  char api_url[128];
  char stats_url[128];
  char backup_stats_url[128];
  char boot_url[128];
  char health_url[128];
  char boot_cmd[128];
  char mac[18];
  char cacrt[18];
  char mac_file[128];
  char init[128];
  char cache[28];
  char archive[28];
  char *id;
  int ping_interval;
  int noping;
  int timeout;
  int nocache;
  int port;
  int debug;
  int insecure;
  int qos;
  /* int heartbeat; */
  int rest;
  int reboot;
  int health_port;
  int monitor;
  int sleep;
  int scan;
  int survey;
  int tls;
  int psk; // Not the right name;
  int random;
  int initialized;
  char token[128];
  char machine[100];
} options;
#endif
