#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <mosquitto.h>
/* #include "mqtt.h" */
#include "helper.h"
#include "options.h"
#include "dbg.h"
#include <time.h>
#include <message.h>

bool connected = false;
time_t m0=0;

int dial_mqtt();

void main_topic(char * topic, const char *name, const char *key) {
  strcpy(topic, name);
  strcat(topic, "/");
  strcat(topic, key);
  strcat(topic, "/");
  strcat(topic, options.mac);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
  if(!result) {
    connected = true;

    int k,n;
    k = strlen(options.key);
    n = strlen(options.topic);
    char topic[k+n+19];
    main_topic(topic, options.topic, options.key);

    options.qos = 0;
    mosquitto_subscribe(mosq, NULL, topic, options.qos);

    debug("Main topic is %s and status topic is %s", topic, options.status_topic);

    if (options.status_topic)
      mosquitto_publish(mosq, 0, options.status_topic, 1, "1", 0, false);
  }
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{

  if (message->payloadlen) {
    /* printf("%s %s\n", message->topic, message->payload); */
  } else {
    printf("%s (null)\n", message->topic);
  }
  fflush(stdout);

  if(message->payloadlen)
    process_message((const char*)message->payload);
}

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
  /* create_mqfifo(0); */
  debug("Disconnected from %s server", options.mqtt_host);
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

  if(!mosq){
    fprintf(stderr, "M Error: Out of memory.\n");
    exit(1); // Or not, re-run
  }

  if (options.tls) {
    debug("Connecting via encrypted channel");
    if (strcmp(options.cacrt, "") == 0) {
      debug("[ERROR] Missing ca file");
    }
    mosquitto_tls_opts_set(mosq, 1, NULL, NULL);
    if (options.psk) {
      // Not in use
      /* mosquitto_tls_set(
       * mosq, "/tmp/cacrt.pem",
       * "/tmp/",
       * "/etc/certs/crt.pem",
       * "/etc/certs/key.pem",
       * NULL); */
    } else {
      mosquitto_tls_set(mosq, options.cacrt, NULL, NULL, NULL, NULL);
    }
  }

  mosquitto_log_callback_set(mosq, my_log_callback);
  mosquitto_connect_callback_set(mosq, my_connect_callback);
  mosquitto_message_callback_set(mosq, my_message_callback);
  mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
  mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
  mosquitto_username_pw_set(mosq, options.username, options.password);
  mosquitto_will_set(mosq, options.status_topic, 1, "0", 0, false);

  if(mosquitto_connect(mosq, options.mqtt_host, options.port, keepalive)) {
    debug("Unable to connect to %s server. Sleeping 5.", options.mqtt_host);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return(0);
  }

  mosquitto_loop_forever(mosq, -1, 1);

  return(1);
}
