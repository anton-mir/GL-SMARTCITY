#ifndef HUB_CONFIG_MANAGER_H
#define HUB_CONFIG_MANAGER_H

#include <memory>
#include <mutex>
#include <list>

#include "inttypes.h"
#include "mqttMsgParamsItem.h"
#include "clusterRawPkt.h"

// !!!! FORBIDDEN TO USE HubConfigManager into signal processing function !!! //
using namespace std;

class ClusterPktTime;
//class MqttMsgParamsItem;

class HubConfigManager
{
private:
    mutex instanceAccessMutex;
    unsigned mqttMsgParallelInstances;  // max parrallel processes for send mqtt messages
    unsigned mqttMsgOperationTimeout;        // send mqtt message timeout into seconds
    ClusterPktTime  dataProcessorGroupInterval; // time interval for grouping data
    unsigned dataProcessorStoreEveryPkt;    // store every
    unsigned udpServerPort;
    list<shared_ptr<MqttMsgParamsItem> > objMqttParams;
    unsigned mqttMsgOperationTimeoutLights;        // send mqtt message timeout into seconds for ligths objects
    ClusterPktTime  dataProcessorGroupIntervalLights; // time interval for grouping data, lights objects
    unsigned dataProcessorStoreEveryPktLights;    // store every, lights objects

private:
    HubConfigManager();
public:
    static shared_ptr<HubConfigManager> create(char *configFile);
    unsigned getMqttMsgParallelInstances();
    unsigned getMqttMsgSendTimeout();
    ClusterPktTime getDataProcessorGroupInterval();
    unsigned getDataProcessorStoreEveryPkt();
    unsigned getUdpServerPort();
    shared_ptr<MqttMsgParamsItem> getMqttParamsForObjId(uint64_t  objId);
    int getObjTypeForObjId(uint64_t  objId);   // if objId not found return -1
    unsigned getMqttMsgSendTimeoutLights();
    ClusterPktTime getDataProcessorGroupIntervalLights();
    unsigned getDataProcessorStoreEveryPktLights();

    static shared_ptr<HubConfigManager> getLastInstance();
    uint64_t getSbcIdByMqttId(char *mqttId, int *result);






};

#endif
