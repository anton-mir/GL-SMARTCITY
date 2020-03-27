#ifndef MQTT_MSG_INSTANCE_H
#define MQTT_MSG_INSTANCE_H

#include "clusterDataGroup.h"
#include "hubConfigManager.h"

using namespace std;

class MqttMsgInstance
{
private:
    shared_ptr<ClusterDataGroup> dataItem_;
    pid_t   instancePid_;
    time_t timeout_;
    shared_ptr<HubConfigManager> cfg_;

private:
    MqttMsgInstance();
    int childFunction();
public:
    static shared_ptr<MqttMsgInstance> create(shared_ptr<ClusterDataGroup> dataItem, shared_ptr<HubConfigManager> cfg);
    pid_t getInstancePid();
    uint64_t getObjId();
    bool isOperationTimeout();
    void requestTerminate();
    bool isLigthsInstance();
};

#endif
