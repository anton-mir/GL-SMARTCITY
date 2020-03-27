#include "hubConfigManagerWrapper.h"
#include "hubConfigManager.h"

uint64_t get_sbc_id_by_mqtt_id(char *mqtt_id, int *stat)
{
    shared_ptr<HubConfigManager> cfg = HubConfigManager::getLastInstance();
    return cfg->getSbcIdByMqttId(mqtt_id, stat);
}
