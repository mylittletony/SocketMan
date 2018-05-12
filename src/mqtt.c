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
#include <compiler.h>
#include <json-c/json.h>
#include "notify.h"

#define M 10
#define N 100

int counter = 0;
bool certs_checked = false;
struct mosquitto *mosq = NULL;
bool connected = false;
time_t m0=0;
char mqtt_id[27];
static int last_mid = -1;
static int last_mid_sent = -1;
static int mid_sent = 0;
static int mode = 0;
int mqtt_fails = 0;
static bool disconnect_sent = false;
void check_message_sent(int);
int dial_mqtt();

void read_message()
{
}

void my_connect_callback(struct mosquitto *mosq, UNUSED(void *userdata), int result)
{
  // Why?
  if(result) {
    return;
  }

  // On connect, send a message to the MQTT broker
  connected = true;

  int k,n;
  k = strlen(options.key);
  n = strlen(options.topic);
  char topic[k+n+19];
  topic_id_generate(topic, options.topic, options.key);

  options.qos = 1;
  char t[128];
  strcat(t, options.topic);

  mosquitto_subscribe(mosq, NULL, topic, options.qos);

  if (strcmp(options.topic, "") == 0) {
    return;
  }

  json_object *jobj = json_object_new_object();
  json_object *jmeta = json_object_new_object();

  // refactor
  json_object_object_add(jobj, "app", json_object_new_string("socketman"));
  json_object_object_add(jobj, "timestamp", json_object_new_int(time(NULL)));
  json_object_object_add(jobj, "event_type", json_object_new_string("CONNECT"));
  json_object_object_add(jmeta, "online", json_object_new_string("1"));
  json_object_object_add(jmeta, "client_id", json_object_new_string(mqtt_id));
  json_object_object_add(jobj, "meta", jmeta);

  const char *resp = json_object_to_json_string(jobj);

  // refactor and de-dup
  char topic_a[128];

  // Status at the front is just for status / online / offline updates
  strcpy(topic_a, "status/");
  strcat(topic_a, options.topic);
  strcat(topic_a, "/");
  strcat(topic_a, options.key);
  strcat(topic_a, "/");
  strcat(topic_a, options.mac);

  mosquitto_publish(mosq, 0, topic_a, strlen(resp), resp, 1, false);
  json_object_put(jobj);
}

static char *rand_string(char *str, char *prefix, size_t size)
{
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK0123456789...";
  if (size) {
    strcpy(str, prefix);
    --size;
    int im, n;
    for (n = strlen(prefix); n < N && im < M; ++n) {
      int key = rand() % (int) (sizeof charset - 1);
      str[n] = charset[key];
    }
    str[size] = '\0';
  }
  return str;
}

void disconnect() {
  if (connected) {
    debug("Disconnecting and cleaning up.");
    mqtt_fails=0;
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    connected = 0;
  }
}

void delivered(struct mosquitto *mosq, char *mid)
{
  char delivery[117];
  strcpy(delivery, "delivery/");
  strcat(delivery, options.topic);
  strcat(delivery, "/");
  strcat(delivery, options.key);
  strcat(delivery, "/");
  strcat(delivery, options.mac);

  // Needs check if running, check time, check live cmd.

  json_object *jobj = json_object_new_object();
  json_object *jmeta = json_object_new_object();

  json_object_object_add(jobj, "id", json_object_new_string(mid));
  json_object_object_add(jobj, "app", json_object_new_string("socketman"));
  json_object_object_add(jobj, "timestamp", json_object_new_int(time(NULL)));
  json_object_object_add(jobj, "event_type", json_object_new_string("DELIVERED"));
  json_object_object_add(jmeta, "delivered", json_object_new_boolean(1));
  json_object_object_add(jobj, "meta", jmeta);

  const char *report = json_object_to_json_string(jobj);

  mosquitto_publish(mosq, 0, delivery, strlen(report), report, 1, false);
}

// Refactor whole function
void my_message_callback(struct mosquitto *mosq, UNUSED(void *userdata), const struct mosquitto_message *message)
{
  if (!message->payloadlen) {
    debug("Error decoding JSON, not processing payload.");
    return;
  }

  time_t now = time(NULL);
  debug("Message received at %lld", (long long)now);

  if (!message->payloadlen) {
    return;
  }

  char type[10];
  char mid[36+1];
  char cmd[strlen(message->payload)+1];

  mid[0] = '\0';
  cmd[0] = '\0';

  // Unmarshalls the payload into logical parts
  process_message((const char*)message->payload, cmd, mid, type, strlen(message->payload)+1);

  // just in case we recv. a blank message
  if (mid[0] == '\0') {
    rand_string(mid, "sm_", 44);
  }

  // What type of message are we?
  char msg_type [36+1];
  strncpy(msg_type, mid, 3);
  msg_type[3] = 0;

  // Runs special commands, based on the type of request
  run_special(type);

  // If payload missing, do nothing!
  if (cmd[0] == '\0') {
    debug("Payload missing CMD or ID, exiting.");
    return;
  }

  // Save output to file if debug flag is set
  if (options.debug) {
    save_config("/tmp/.configs", cmd);
  }

  // Message processing
  debug("Running payload");

  struct timespec tstart={0,0}, tend={0,0};
  clock_gettime(CLOCK_MONOTONIC, &tstart);

  FILE *fp;
  int response = -1;
  char buffer[51200];
  buffer[0] = '\0';

  fp = popen(cmd, "r");
  if (fp != NULL) {
    /* response = 0; */
    if (strcmp(msg_type, "msg") == 0) {
      debug("Processing a message");
      memset(buffer, '\0', sizeof(buffer));
      fread(buffer, sizeof(char), sizeof(char) * sizeof(buffer), fp);
    }
    pclose(fp);
  }

  clock_gettime(CLOCK_MONOTONIC, &tend);
  debug("Payload finished in %.5f seconds",
      ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
      ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));

  if (options.debug) {
    debug("%s", buffer);
  }

  // Send delivery notification
  delivered(mosq, mid);

  if (strlen(buffer) == 0) {
    strcpy(buffer, "DNE");
  }

  // Will publish the job back to CT via REST API
  if (options.rest) {
    cmd_notify(response, mid, buffer);
    return;
  }

  // Don't publish messages that aren't messages
  if (strcmp(msg_type, "msg") != 0) {
    return;
  }

  json_object *jobj = json_object_new_object();
  json_object *jmeta = json_object_new_object();

  json_object_object_add(jobj, "id", json_object_new_string(mid));
  json_object_object_add(jobj, "app", json_object_new_string("socketman"));
  json_object_object_add(jobj, "timestamp", json_object_new_int(time(NULL)));
  json_object_object_add(jobj, "event_type", json_object_new_string("PROCESSED"));
  json_object_object_add(jmeta, "msg", json_object_new_string(buffer));

  // Should include a flag for the status of the job, maybe it fails.
  json_object_object_add(jobj, "meta", jmeta);
  const char *report = json_object_to_json_string(jobj);

  char pub[112];
  strcpy(pub, "pub/");
  strcat(pub, options.topic);
  strcat(pub, "/");
  strcat(pub, options.key);
  strcat(pub, "/");
  strcat(pub, options.mac);

  // Worth checking the connection (refactor) //
  int publish_message(const char *report, char *topic) {
    return mosquitto_publish(mosq, 0, topic, strlen(report), report, 1, false);
  }

  int ret = publish_message(report, pub);
  if (ret != MOSQ_ERR_SUCCESS) {
    int i;
    for (i = 0; i < 5; i++) {
      int sl = ((i*2)+1);
      debug("Failed to send, retrying (%d) after %d second", i+1, sl);
      sleep(sl);

      ret = publish_message(report, pub);
      if (ret == MOSQ_ERR_SUCCESS) {
        break;
      }
    }
  }

  json_object_put(jobj);

  check_message_sent(ret);

  if (ret == MOSQ_ERR_SUCCESS) {
    debug("Message published!");
    return;
  }

  debug("XX Message not published!! XX");
}

void my_subscribe_callback(UNUSED(struct mosquitto *mosq), UNUSED(void *userdata), int mid, int qos_count, const int *granted_qos)
{
  debug("Connected to broker QoS %d", granted_qos[0]);
}

void mqtt_connect() {
  disconnect();
  if (strcmp(options.mqtt_host, "") == 0) {
    debug("No MQTT host, skipping connect.");
    return;
  }

  dial_mqtt();
}

void check_message_sent(int ret) {
  if (ret != MOSQ_ERR_SUCCESS) {
    mqtt_fails++;
    debug("Message failed to send. Code: %d (%d)", ret, mqtt_fails);
  } else if (mqtt_fails != 0) {
    mqtt_fails=0;
  }

  if (ret != MOSQ_ERR_SUCCESS && mqtt_fails >= 3 && connected == 1) {
    debug("Reconnecting after %d failed pings", mqtt_fails);
    mqtt_fails=0;
    mqtt_connect();
  }
}

void my_disconnect_callback(UNUSED(struct mosquitto *mosq), UNUSED(void *userdata), UNUSED(int rc))
{
  counter++;
  debug("Lost connection with broker");

  // Checks once after 60s, then will only check every 120s
  if ((counter > 20 && !certs_checked) || counter > 120) {
    certs_checked = true;
    if (counter > 120)
      counter = 0;
    int check = check_certificates();
    if (check < 0) {
      debug("Restarting MQTT, new certificates installed");
      mqtt_connect();
    }
  }
}

void ping_mqtt()
{
  debug("Ping!");

  int k,n;
  k = strlen(options.key);
  n = strlen(options.topic);
  char topic[k+n+19];
  topic_id_generate(topic, options.topic, options.key);

  if (strcmp(options.topic, "") == 0) {
    return;
  }

  json_object *jobj = json_object_new_object();

  json_object_object_add(jobj, "timestamp", json_object_new_int(time(NULL)));
  json_object_object_add(jobj, "app", json_object_new_string("socketman"));
  json_object_object_add(jobj, "event_type", json_object_new_string("PING"));
  const char *resp = json_object_to_json_string(jobj);

  char topic_a[128];

  strcpy(topic_a, "status/");
  strcat(topic_a, options.topic);
  strcat(topic_a, "/");
  strcat(topic_a, options.key);
  strcat(topic_a, "/");
  strcat(topic_a, options.mac);

  int ret = mosquitto_publish(mosq, &mid_sent, topic_a, strlen(resp), resp, 2, false);
  json_object_put(jobj);

  check_message_sent(ret);
}

int dial_mqtt()
{
  mosquitto_lib_init();
  debug("Connecting to broker...");

  client_id_generate(mqtt_id);

  int keepalive = 10;
  bool clean_session = true;

  mosq = mosquitto_new(mqtt_id, clean_session, NULL);
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

  if (options.tls == true) {
    if (strcmp(options.cacrt, "") == 0) {
      debug("[ERROR] Missing ca file");
    }
    mosquitto_tls_opts_set(mosq, 1, NULL, NULL);
    mosquitto_tls_set(mosq, options.cacrt, NULL, NULL, NULL, NULL);
  }

  mosquitto_connect_callback_set(mosq, my_connect_callback);
  mosquitto_message_callback_set(mosq, my_message_callback);
  mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
  mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
  /* mosquitto_max_inflight_messages_set(mosq, 0); */
  mosquitto_username_pw_set(mosq, options.username, options.password);

  if (strcmp(options.topic, "") != 0) {

    json_object *jobj = json_object_new_object();
    json_object *jmeta = json_object_new_object();

    json_object_object_add(jobj, "app", json_object_new_string("socketman"));
    json_object_object_add(jobj, "timestamp", json_object_new_int(time(NULL)));
    json_object_object_add(jobj, "event_type", json_object_new_string("CONNECT"));
    json_object_object_add(jmeta, "online", json_object_new_string("0"));
    json_object_object_add(jmeta, "client_id", json_object_new_string(mqtt_id));
    json_object_object_add(jobj, "meta", jmeta);

    const char *report = json_object_to_json_string(jobj);

    // Refactor, de-dup after testing
    char topic[128];
    strcpy(topic, "status/");
    strcat(topic, options.topic);
    strcat(topic, "/");
    strcat(topic, options.key);
    strcat(topic, "/");
    strcat(topic, options.mac);

    // Set the last will on the broker
    mosquitto_will_set(mosq, topic, strlen(report), report, 1, false);
    json_object_put(jobj);
  }

  int rc = mosquitto_connect_async(mosq, options.mqtt_host, options.port, keepalive);
  if (rc) {
    disconnect();
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
