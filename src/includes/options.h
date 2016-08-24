#ifndef OPTIONS_STRUCT
#define OPTIONS_STRUCT
struct _options {
  char username[128];
  char password[128];
  char topic[128];
  char key[128];
  char config[128];
  char host[128];
  char url[128];
  char mac[18];
  char mac_file[128];
  char *id;
  int timeout;
  int port;
  int qos;
  int sleep;
  int no_scan;
  int no_survey;
  int tls;
  int psk; // Not the right name;
  int random;
  int initialized;
  char token[128];
  char machine[100];
} options;
#endif
