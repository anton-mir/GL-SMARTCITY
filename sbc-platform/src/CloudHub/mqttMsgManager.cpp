#include <iostream>

#include <string.h>
#include <errno.h>

#include "mqttMsgManager.h"
#include "hubConfigManager.h"
#include "sbc_mqtt_ciotc.h"

using namespace std;

MqttMsgManager::MqttMsgManager()
    : waitChildPid{nullptr}
    , waitChildResult{nullptr}
//    , maxParallelInstances_ {MGR_MIN_PARALLEL_INSTANCES}
    , dataQueue {nullptr}
    , terminateRequestedFlag {false}
//    , msgSendTimeout_ {DEF_MQTT_MSG_SEND_TIMEOUT}
    , cfg_ {nullptr}

{

}

MqttMsgManager::~MqttMsgManager()
{
    stopMgrThread();
    if (waitChildResult != nullptr) {
        delete[] waitChildResult;
        waitChildResult = nullptr;
    }
    if (waitChildPid != nullptr) {
        delete[] waitChildPid;
        waitChildPid = nullptr;
    }
}

void
MqttMsgManager::requestTerminate()
{
unique_lock<mutex> lock(loopControlMutex);
terminateRequestedFlag = true;
}

bool
MqttMsgManager::isTerminateRequested()
{
unique_lock<mutex> lock(loopControlMutex);
return terminateRequestedFlag;
}

void
MqttMsgManager::stopMgrThread()
{
    requestTerminate();
    th->join();
}


shared_ptr<MqttMsgManager>
MqttMsgManager::create(shared_ptr<HubConfigManager> cfg) // unsigned maxParallelInstances, unsigned msgSendTimeout
{
    struct timespec spec;
    int rsl;
    rsl = clock_gettime(CLOCK_REALTIME, &spec);
    if (rsl < 0) {
        cerr << "gettime failed, error:" << strerror(errno) << endl;
        return nullptr;
    }
    shared_ptr<MqttMsgManager> instance = shared_ptr<MqttMsgManager>(new MqttMsgManager);
    if (instance == nullptr) {
        cerr << "allocate mqtt message manager failed" << endl;
        return nullptr;
    }
    instance->maxParallelInstances_ = cfg->getMqttMsgParallelInstances();
    instance->cfg_ = cfg;
//    instance->msgSendTimeout_ = cfg->getMqttMsgSendTimeout();

    instance->waitChildPid = new pid_t[instance->maxParallelInstances_];
    if (instance->waitChildPid == nullptr) {
        cerr << "mqtt message manager: allocate child pid array failed" << endl;
        return nullptr;
    }
    instance->waitChildResult = new int[instance->maxParallelInstances_];
    if (instance->waitChildResult == nullptr) {
        cerr << "mqtt message manager: allocate child result array failed" << endl;
        return nullptr;
    }
    instance->dataQueue = shared_ptr<list<shared_ptr<ClusterDataGroup> > >(new list<shared_ptr<ClusterDataGroup> >);
    if (instance->dataQueue == nullptr) {
        cerr << "mqtt message manager: allocate data queue failed" << endl;
        return nullptr;
    }
    if (instance->runInstance() < 0) {
        cerr << "mqtt message manager: run main thread failed" << endl;
        return nullptr;
    }
    return instance;
}


void
MqttMsgManager::addToDataQueue(shared_ptr<ClusterDataGroup> item)
{
    unique_lock<mutex> lock(loopControlMutex);
    if (dataQueue == nullptr)
        dataQueue = shared_ptr<list<shared_ptr<ClusterDataGroup> > >(new list<shared_ptr<ClusterDataGroup> >);
    if (dataQueue == nullptr)
        cerr << "mqtt message manager: allocate data queue failed, group data dpopped for objId: " << item->getObjId() << endl;
    else
        dataQueue->push_back(item);
}



void mqttMsgMgrthLoop(MqttMsgManager *instance)
{
    instance->serverLoop();
}


int
MqttMsgManager::runInstance()
{
    th = shared_ptr<thread>(new thread(mqttMsgMgrthLoop, this));
    if (th == nullptr) {
        cerr << "mqtt msg manager thread create failed" << endl;
        return -1;
    }
    return 0;
}

void termChildTh(mqtt_term_res *res)
{
    if (res == nullptr)
        return;
    res->mgr->signalChildL2(res->pid, res->result);
    delete res;
}

/*
void
MqttMsgManager::signalChild(pid_t pid, int result)
{
    mqtt_term_res *res;
    cout << "child terminate signaling, pid: " << pid << " result: " << result << endl;
    res = new mqtt_term_res;
    res->pid = pid;
    res->result = result;
    res->mgr = this;
    thread t(termChildTh, res);
    t.detach();


}
*/

void
MqttMsgManager::signalChildL2(pid_t pid, int result)
{
    cout << "child terminate l2 sig, pid: " << pid << " result: " << result << endl;
    unique_lock<mutex> lock(childPidMutex);
    for (unsigned lp = 0; lp < maxParallelInstances_; lp++) {
        if (waitChildPid[lp] == 0) {
            waitChildResult[lp] = result;
            waitChildPid[lp] = pid;
            break;
        }
    }
}

void
MqttMsgManager::moveDataQueueToInternal()
{
    shared_ptr<list<shared_ptr<ClusterDataGroup> > > newGDataList;

    // copy data ptr queue to temp storage //
    {
    unique_lock<mutex> lock(loopControlMutex);
    newGDataList = dataQueue;
    dataQueue = nullptr;
    dataQueue = shared_ptr<list<shared_ptr<ClusterDataGroup> > >(new list<shared_ptr<ClusterDataGroup> >);
    if (dataQueue == nullptr)
        cerr << "Warning: allocate new group data list failed, try this operation later" << endl;
    }
    // now tmpGDataList contain a new packet group //
    if (newGDataList == nullptr || newGDataList->size() == 0) // nothing to do
    {
        printf("No msg\r\n");
        std::cout << "NO msg" << std::endl;
        return;
    }
    else
    {
        cout << "New message" << endl;
    }
    for (auto nIt = newGDataList->begin(); nIt != newGDataList->end(); nIt++) {
        auto it = internalDataQueue.begin();
        for (; it != internalDataQueue.end(); it++) {
            if ((*it)->getObjId() == (*nIt)->getObjId()) {
                *it = *nIt; // replace group data value if current object already present into internal queue
                break;
            }
        }
        if (it == internalDataQueue.end())  // if new objId
            internalDataQueue.push_back(*nIt);
    }

}

void
MqttMsgManager::cleanCompletedMsgInstances()
{
    unique_lock<mutex> lock(childPidMutex);
    cout << __PRETTY_FUNCTION__ << " began" << endl;
    if (msgInProgress.size() == 0) {
        // if no messages in progress present, need clear wait pid list, if not empty
        // in generaly this is not normal state, but for continue stable work make it
        for (unsigned lp = 0; lp < maxParallelInstances_; lp++)
            if (waitChildPid[lp] != 0)
                waitChildPid[lp] = 0;
    } else {
        for (unsigned lp = 0; lp < maxParallelInstances_; lp++) {
            if ( waitChildPid[lp] != 0) { // if this slot occuped by terminated child
                pid_t curPid = waitChildPid[lp];
                int rsl = waitChildResult[lp];
                auto it = msgInProgress.begin();
                for (; it != msgInProgress.end(); it ++) {
                    if ( curPid == (*it)->getInstancePid())
                        break;
                }
                if (it == msgInProgress.end())
                    cerr << "unknown child pid detected: " << curPid << endl;
                else {
                    if (rsl != EXIT_SUCCESS)
                        cout << "failed to store group data, objId: " << (*it)->getObjId() << endl;
                    msgInProgress.erase(it);
                    cout << "bababu" << endl;
                }
                waitChildPid[lp] = 0; // clear processed pid into pid list
            }
        }
    }
    cout << __PRETTY_FUNCTION__ << " end" << endl;
}

void
MqttMsgManager::terminateHangupInstances()
{
    //unique_lock<mutex> lock(childPidMutex);
    for (auto it = msgInProgress.begin(); it != msgInProgress.end(); it++)
        if ( !(*it)->isLigthsInstance() && (*it)->isOperationTimeout())
        //if ( (*it)->isOperationTimeout())
            (*it)->requestTerminate();
}


//only for teststand 2019.04.22 //
// run only one mqttMsgInstance for ligths//
static int isLigthsPresent = 0;
void
MqttMsgManager::serverLoop()
{
     std::cout << __PRETTY_FUNCTION__ << " loop started" << std::endl;
     printf("loop started\r");
    while (!isTerminateRequested()) {

        // printf("%s(): loop\n", __PRETTY_FUNCTION__);

        moveDataQueueToInternal();
        terminateHangupInstances();
        //printf("tr2\n");
        //cout << __PRETTY_FUNCTION__ << " before cleanCompletedMsgInstances" << endl;
        cleanCompletedMsgInstances();
        if (msgInProgress.size() < maxParallelInstances_) {
            // if free run slots for msg present
            for (auto it = internalDataQueue.begin(); it != internalDataQueue.end(); it ++) {
                auto rIt = msgInProgress.begin();
                for (; rIt != msgInProgress.end(); rIt++)
                    if ((*it)->getObjId() == (*rIt)->getObjId())
                        break;
                if (rIt == msgInProgress.end()) {
                    // if current object not in send msg state and free slot for send present
                    shared_ptr<MqttMsgInstance> newMsgInstance = nullptr;
                    if (!((*it)->getObjType() == MQTT_OBJ_TYPE_LIGTH && isLigthsPresent == 1)) {
                     newMsgInstance = MqttMsgInstance::create(*it, cfg_);
                    if (newMsgInstance == nullptr)
                        cerr << "create mqtt message instance failed, data group will be dropped, objId: " << (*it)->getObjId() << endl;
                    }
                    if (newMsgInstance != nullptr) {
                        if ((*it)->getObjType() == MQTT_OBJ_TYPE_LIGTH)
                            isLigthsPresent = 1;
                        msgInProgress.push_back(newMsgInstance);
                        it = internalDataQueue.erase(it);
                        if (msgInProgress.size() >= maxParallelInstances_)
                            break;
                    }
                }
                // cout << __PRETTY_FUNCTION__ << " tr8" << endl;
            }
        } // end msgInProgress.size() < maxParallelInstances_
        usleep(200000);
    }
    cout << "mqtt message manager: main thread loop ended" << endl;
}

