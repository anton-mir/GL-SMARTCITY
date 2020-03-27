#ifndef MQTT_CIOTC_H
#define MQTT_CIOTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#define MQTT_OBJ_TYPE_CAR   1
#define MQTT_OBJ_TYPE_LIGTH 2

#define DEF_SBC_LIGTHS_MQTT_ID  "mqtt-007"

int sbc_mqtt_main(int argc, char **argv, unsigned obj_type, unsigned rport, char *rhost, uint64_t sbc_id);

#ifdef __cplusplus
}
#endif

#endif
