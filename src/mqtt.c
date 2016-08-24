#include <stdlib.h>
#include <mosquitto.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <json-c/json.h>
#include <json-c/json.h>
#include <stdio.h>
#include <unistd.h>
#include "collector.h"
#include "mqtt.h"
#include "helper.h"
#include "json.h"
/* #include "connections.h" */
#include <stdlib.h>
#include "options.h"
#include <string.h>
#include <stdio.h>
#include "dbg.h"
#include "socketman.h"
#include <wait.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

bool connected = false;
time_t m0=0;

/* char id[26]; */
char mac[17];

int dial_mqtt();

void status_topic(char *status, const char *topic) {
  strcpy(status, topic);
  strcat(status,"/");
  /* strcat(status, wan_mac()); */
}

void main_topic(char * topic, const char *name, const char *key) {
  debug("MAC: %s", options.mac);
  strcpy(topic, name);
  strcat(topic, "/");
  strcat(topic, key);
  strcat(topic, "/");
  strcat(topic, options.mac);
}

// Why?
void create_mqfifo(int a)
{
  FILE *fd;
  fd = fopen("/tmp/.mqfifo","w+");
  if(a==10)
  {
    fprintf(fd,"%s","true");
  }
  else
  {
    fprintf(fd,"%s","false");
  }
  fclose(fd);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
  if(!result) {
    connected = true;
    create_mqfifo(10);

    int k,n;
    k = strlen(options.key);
    n = strlen(options.topic);
    char topic[k+n+19];
    main_topic(topic, options.topic, options.key);

    /* int x=strlen(credentials.topic_status); */
    /* char status[x+19]; */
    /* status_topic(status, credentials.topic_status); */

    options.qos = 0;
    mosquitto_subscribe(mosq, NULL, topic, options.qos);

    /* char *status = "not-implemented"; */
    /* debug("Main topic is %s and status topic is %s", topic, status); */

    /* mosquitto_publish(mosq, 0, status, 1, "1", 0, false); */
    /* sleep(10); */
    /* mosquitto_publish(mosq, 0, topic, 1, "zzsimon simon simon simon isims simon sim on simsimon sim ", 0, false); */
  } else {
    create_mqfifo(0);
    if (1) {
      fprintf(stderr, "Connect failed\n");
    }
  }
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{

  if(message->payloadlen){
    /* printf("%s %s\n", message->topic, message->payload); */
  }else{
    printf("%s (null)\n", message->topic);
  }
  fflush(stdout);

  if(message->payloadlen) {
    /* debug("received message %s\n", (const char*)message->payload); */
    process_message((const char*)message->payload);
    /* debug("Running CMD: %s", msg); */
    /* free(msg); */
    fflush(stdout);
  }
}

/* mosq * */
/* void publish(json_object *array) { */
/*   /1* debug("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: %s", credentials); *1/ */
/* } */

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
  int i;
  debug("Subscribed (mid: %d): %d", mid, granted_qos[0]);

  for(i=1; i<qos_count; i++) {
    debug("WHHHHAT, %d", granted_qos[i]);
  }
}

void my_log_callback()
{
}

void my_disconnect_callback()
{
  connected = false;
  // Why?
  create_mqfifo(0);
  debug("Disconnected from %s server", options.host);
}

void mqtt_connect() {
  do
  {
    dial_mqtt();
    sleep(5);
  }
  while(!connected);
}

int dial_mqtt() {

  debug("Connecting to MQTT");
  char id[27];

  sprintf(id, "CUCUMBER-%s", options.mac);

  int keepalive = 60;
  bool clean_session = true;
  struct mosquitto *mosq = NULL;

  mosquitto_lib_init();
  mosq = mosquitto_new(id, clean_session, NULL);
  /* free(id); */

  /* debug("xxx: %s", id); */

  if(!mosq){
    fprintf(stderr, "M Error: Out of memory.\n");
    exit(1);
  }

  /* int x = strlen(options.topic_status); */
  /* char status[x+19]; */
  /* status_topic(status, credentials.topic_status); */

  if (options.tls) {
    debug("Connecting via encrypted channel");
    mosquitto_tls_opts_set(mosq,1,NULL,NULL);
    if (options.psk) {
      mosquitto_tls_set(mosq, "/etc/certs/cacrt.pem", "/etc/certs/", "/etc/certs/crt.pem", "/etc/certs/key.pem", NULL);
    } else {
      // Replace with the right directories //
      mosquitto_tls_set(mosq, "/etc/certs/cacrt.pem", "/etc/certs/", "", "", NULL);
    }
  }

  mosquitto_log_callback_set(mosq, my_log_callback);
  mosquitto_connect_callback_set(mosq, my_connect_callback);
  mosquitto_message_callback_set(mosq, my_message_callback);
  mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
  mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
  mosquitto_username_pw_set(mosq, options.username, options.password);
  /* mosquitto_will_set(mosq, status, 1, "0", 0, false); */

  /* debug("HOST: %s User: %s, Pass: %s", options.host, options.username, options.password); */
  if(mosquitto_connect(mosq, options.host, options.port, keepalive)) {
    debug("Unable to connect to %s server. Sleeping 5.", options.host);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return(0);
  }

  mosquitto_loop_forever(mosq, -1, 1);

  return(1);
}
