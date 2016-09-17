#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <mosquitto.h>
#include "helper.h"
#include "options.h"
#include "dbg.h"
#include <time.h>
#include <message.h>
#include "mqtt_utils.h"

bool connected = false;
time_t m0=0;

int dial_mqtt();

void read_message()
{

}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
  if(!result) {
    connected = true;

    int k,n;
    k = strlen(options.key);
    n = strlen(options.topic);
    char topic[k+n+19];
    topic_id_generate(topic, options.topic, options.key);

    options.qos = 2;
    mosquitto_subscribe(mosq, NULL, topic, options.qos);

    if (strcmp(options.status_topic, "") != 0)
      mosquitto_publish(mosq, 0, options.status_topic, 1, "1", 1, false);
  }
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
  if (message->payloadlen)
    debug("Inbound message recieved");

  char id[100];
  char cmd[10000];
  id[0] = '\0';
  cmd[0] = '\0';

  process_message((const char*)message->payload, cmd, id);
  if (id[0] != '\0') {
    mosquitto_publish(mosq, 0, options.status_topic, strlen(id), id, 1, false);
    debug("READ: %s", id);
  }
  if (cmd[0] != '\0')
    process_cmd(cmd, id);
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
  int i;
  debug("Subscribed (mid: %d): %d", mid, granted_qos[0]);

  for(i=1; i<qos_count; i++) {
    debug("QOS: %d", granted_qos[i]);
  }
}

void *reconnect(void *x)
{
  while(!connected) {
    debug("Unable to connect to %s. Sleeping 15", options.mqtt_host);
    sleep(15);
    if (!dial_mqtt()) {
      debug("Reconnected to %s, yay!", options.mqtt_host);
      break;
    }
  }
  return NULL;
}

void my_disconnect_callback()
{
  connected = false;
  debug("MQTT Lost connected to %s", options.mqtt_host);
}

void mqtt_connect() {
  if (strcmp(options.mqtt_host, "") != 0)
  {
    int rc = dial_mqtt();

    if (rc) {
      pthread_t conn_thread;
      if(pthread_create(&conn_thread, NULL, reconnect, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return;
      }

      /* if(pthread_join(conn_thread, NULL)) { */
      /*   fprintf(stderr, "Error joining thread\n"); */
      /*   return; */
      /* } */
    };
  } else {
    debug("No MQTT host, skipping connect.");
  }
  return;
}

int dial_mqtt()
{
  debug("Connecting to MQTT...");
  char id[27];

  client_id_generate(id);

  int keepalive = 60;
  bool clean_session = true;
  struct mosquitto *mosq = NULL;

  mosq = mosquitto_new(id, clean_session, NULL);
  if(!mosq){
    switch(errno){
      case ENOMEM:
        fprintf(stderr, "Error: Out of memory.\n");
        break;
      case EINVAL:
        fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
        break;
    }
    mosquitto_lib_cleanup();
    return 1;
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

  /* mosquitto_log_callback_set(mosq, my_log_callback); */
  mosquitto_connect_callback_set(mosq, my_connect_callback);
  mosquitto_message_callback_set(mosq, my_message_callback);
  mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
  mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
  mosquitto_username_pw_set(mosq, options.username, options.password);

  if (strcmp(options.status_topic, "") != 0)
    mosquitto_will_set(mosq, options.status_topic, 1, "0", 2, false);

  int rc = mosquitto_connect_async(mosq, options.mqtt_host, options.port, keepalive);
  if (rc) {
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return(rc);
  }

  rc = mosquitto_loop_start(mosq);

  if(rc == MOSQ_ERR_NO_CONN){
    rc = 0;
    connected = 1;
  }
  if(rc){
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
  }
  return rc;
}
