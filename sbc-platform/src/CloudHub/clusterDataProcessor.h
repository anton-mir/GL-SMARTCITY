#ifndef CLUSTER_DATA_PROCESSOR_H
#define CLUSTER_DATA_PROCESSOR_H

#include <map>
#include <list>
#include <mutex>
#include <thread>

#include <inttypes.h>
#include "mqttMsgManager.h"
#include "clusterDataGroup.h"
#include "clusterRawPkt.h"
#include "hubConfigManager.h"

using namespace std;

class ClusterDataProcessor {

private:
    ClusterPktTime sendInterval_;
    shared_ptr<MqttMsgManager> mqttMsgManager_;
    map<uint64_t, std::shared_ptr<ClusterDataGroup> > groupInProgress;
    mutex loopControlMutex;
//    mutex rawPacketsQueueMutex;
    list<shared_ptr<ClusterRawPkt> > rawPacketsQueue;
    shared_ptr<thread> th;
    bool loopTerminateFlag;
    unsigned storeEveryPkt_;
    shared_ptr<HubConfigManager> cfg_;
    ClusterPktTime sendIntervalLights_;
    unsigned storeEveryPktLights_;


private:
    ClusterDataProcessor();
    void loopTerminateRequest();
    int runInstance();


public:
    static shared_ptr<ClusterDataProcessor> create(shared_ptr<MqttMsgManager> mqttMsgManager, shared_ptr<HubConfigManager> cfg);
    void addDataToQueue(shared_ptr<ClusterRawPkt> newData); // new data into json format so no need data size

    bool isLoopTerminateNeeded();
    void serverLoop();  // thread loop function


};

#endif
