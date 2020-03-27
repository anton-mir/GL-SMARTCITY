#ifndef MQTT_MSG_PARAMS_ITEM_H
#define MQTT_MSG_PARAMS_ITEM_H

#include <memory>
#include <string>
#include <inttypes.h>

#include "sbc_mqtt_ciotc.h"

using namespace std;
class MqttMsgParamsItem
{
public:
    uint64_t sbcObjId;
    shared_ptr<string> deviceId;
    shared_ptr<string> registryId;
    shared_ptr<string> projectId;
    shared_ptr<string> keyPath;
    shared_ptr<string> algorithm;
    shared_ptr<string> rootPath;
    shared_ptr<string> region;
    int objType;
    unsigned rport;
    shared_ptr<string> rhost;
private:
    MqttMsgParamsItem();

public:
    static shared_ptr<MqttMsgParamsItem> create(uint64_t newSbcObjId, shared_ptr<string> newDeviceId, shared_ptr<string> newRegistryId,
                shared_ptr<string> newProjectId, shared_ptr<string> newKeyPath, shared_ptr<string> newAlgorithm,
                shared_ptr<string> newRootPath, shared_ptr<string> newRegion, int newObjType);
    static shared_ptr<MqttMsgParamsItem> createL(uint64_t newSbcObjId, shared_ptr<string> newDeviceId, shared_ptr<string> newRegistryId,
                shared_ptr<string> newProjectId, shared_ptr<string> newKeyPath, shared_ptr<string> newAlgorithm,
                shared_ptr<string> newRootPath, shared_ptr<string> newRegion, int newObjType, shared_ptr<string> newRHost, unsigned newRPort);


};

#endif
