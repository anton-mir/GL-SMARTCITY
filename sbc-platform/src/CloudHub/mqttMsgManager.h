#ifndef MQTT_MSG_MANAGER_H
#define MQTT_MSG_MANAGER_H

#include <mutex>
#include <thread>
#include <list>
#include <map>

#include <unistd.h>
#include "mqttMsgInstance.h"
#include "hubConfigManager.h"

class MqttMsgManager
{
private:
    shared_ptr<thread> th;
    mutex childPidMutex;
    pid_t *waitChildPid;
    int *waitChildResult;
    uint32_t    maxParallelInstances_;
    list<shared_ptr<MqttMsgInstance> > msgInProgress;
    mutex   dataQueueMutex;
    shared_ptr<list<shared_ptr<ClusterDataGroup> > > dataQueue;
    list<shared_ptr<ClusterDataGroup> > internalDataQueue;
    mutex loopControlMutex;
    bool terminateRequestedFlag;
//    unsigned msgSendTimeout_;
    shared_ptr<HubConfigManager> cfg_;

private:
    MqttMsgManager();
    void requestTerminate();
    bool isTerminateRequested();
    void stopMgrThread();
    int runInstance();
    void moveDataQueueToInternal();
    void cleanCompletedMsgInstances();
    void terminateHangupInstances();

public:
    virtual ~MqttMsgManager();
    static shared_ptr<MqttMsgManager> create(shared_ptr<HubConfigManager> cfg);
    void addToDataQueue(shared_ptr<ClusterDataGroup> item);

//    void signalChild(pid_t pid, int result);
    void serverLoop();  // thread loop function

    void signalChildL2(pid_t pid, int result); // !!! don't execute this method from signal handler, use  signalChild !!!
};

typedef struct mqtt_term_res_st {
    pid_t pid;
    int result;
    MqttMsgManager *mgr;
} mqtt_term_res;

#endif
