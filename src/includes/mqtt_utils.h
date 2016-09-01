#ifndef _MQTT_CONFIG_H
#define _MQTT_CONFIG_H

int client_id_generate(char *id);

void topic_id_generate(char * topic, const char *name, const char *key);

#endif
