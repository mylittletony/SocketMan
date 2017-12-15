#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"
#include <sys/types.h>
#include <unistd.h>
#include <mosquitto.h>

int client_id_generate(char *id)
{
  int len;

  len = strlen("/-") + 6 + strlen(options.mac);
  snprintf(id, len, "%dx%s", getpid(), options.mac);
  if(strlen(id) > MOSQ_MQTT_ID_MAX_LENGTH){
    id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
  }
  return MOSQ_ERR_SUCCESS;
}

void topic_id_generate(char * topic, const char *name, const char *key)
{
  strcpy(topic, "sub/");
  strcat(topic, name);
  strcat(topic, "/");
  strcat(topic, key);
  strcat(topic, "/");
  strcat(topic, options.mac);
}
