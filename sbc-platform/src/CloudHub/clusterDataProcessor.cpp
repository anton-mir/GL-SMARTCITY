#include <iostream>
#include <limits>

#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <QVariant>

typedef std::numeric_limits< double > dbl;


#include "clusterDataProcessor.h"
#include "mqttMsgParamsItem.h"

using namespace std;

ClusterDataProcessor::ClusterDataProcessor()
    : mqttMsgManager_{nullptr}
    , loopTerminateFlag {false}
    , storeEveryPkt_ { 1 } // store each new pkt
{

}

shared_ptr<ClusterDataProcessor>
ClusterDataProcessor::create(shared_ptr<MqttMsgManager> mqttMsgManager, shared_ptr<HubConfigManager> cfg)
{
    shared_ptr<ClusterDataProcessor> instance = shared_ptr<ClusterDataProcessor>(new ClusterDataProcessor);
    if (instance == nullptr) {
        cerr << "allocate cluster data processor failed" << endl;
        return nullptr;
    }
    // ClusterPktTime sendInterval, unsigned storeEveryPkt
    instance->mqttMsgManager_ = mqttMsgManager;
    instance->sendInterval_ = cfg->getDataProcessorGroupInterval();
    instance->storeEveryPkt_ = cfg->getDataProcessorStoreEveryPkt();
    instance->sendIntervalLights_ = cfg->getDataProcessorGroupIntervalLights();
    instance->storeEveryPktLights_ = cfg->getDataProcessorStoreEveryPktLights();
    instance->cfg_ = cfg;
    if (instance->runInstance() < 0)
        return nullptr;
    return instance;
}

void
ClusterDataProcessor::addDataToQueue(shared_ptr<ClusterRawPkt> newData)
{
    // cout << __PRETTY_FUNCTION__ << "add raw pkt: " << newData << endl;
//    unique_lock<mutex> lock(rawPacketsQueueMutex);
    unique_lock<mutex> lock(loopControlMutex);
    if (loopTerminateFlag)
        cout << "data processos into terminating state, packet skipped" << endl;
    else
        rawPacketsQueue.push_back(newData);
}

void
ClusterDataProcessor::loopTerminateRequest()
{
    unique_lock<mutex> lock(loopControlMutex);
    loopTerminateFlag = true;
}

void dataProcessosthLoop(ClusterDataProcessor *instance)
{
    instance->serverLoop();
}


int
ClusterDataProcessor::runInstance()
{
    th = shared_ptr<thread>(new thread(dataProcessosthLoop, this));
    if (th == nullptr) {
        cerr << "udp server thread create failed" << endl;
        return -1;
    }
    return 0;
}

bool ClusterDataProcessor::isLoopTerminateNeeded()
{
    unique_lock<mutex> lock(loopControlMutex);
    return loopTerminateFlag;
}

void ClusterDataProcessor::serverLoop()
{
    //int drop_pkt_num = 0;
    while(!isLoopTerminateNeeded()) {
        shared_ptr<ClusterRawPkt> cur_pkt;
        {
            unique_lock<mutex> lock(loopControlMutex);
            if (rawPacketsQueue.size() > 0) {
                cur_pkt = rawPacketsQueue.front();
                rawPacketsQueue.pop_front();
            }
            else
                cur_pkt = nullptr;
        }
        if (cur_pkt != nullptr) {
            // !!! need write implementation of data parsing !!! //
            QByteArray qData((const char*)cur_pkt.get()->getData());
            QJsonParseError jError;
            QJsonDocument jDoc = QJsonDocument::fromJson(qData, &jError);
            if (QJsonParseError::NoError != jError.error) {
                cout << "failed to parse json pkt data: " << cur_pkt.get() << endl;
            } else {
                // parse json pkt success
                QJsonObject jObject = jDoc.object();
                if ( !jObject.contains("id") ) {
                    cout << "pkt must contain field \"id\", drop pkt: " << cur_pkt.get() << endl;
                } else {
                    // id present into pkt

                    qlonglong id = jObject["id"].toVariant().toLongLong();
                    jObject.remove("id"); // todo: delete id and repack pkt to reduce mqtt pkt size
                    QJsonDocument jDoc2(jObject);
                    qData = jDoc2.toJson(QJsonDocument::Compact);
                    auto dataItem = ClusterDataItem::create(id, cur_pkt->getPktTime(), (uint8_t*)qData.data(), qData.size());
                    if (dataItem != nullptr) {
                        bool createNewGroupNeeded = false;
                        auto it = groupInProgress.find(id);
                        shared_ptr<ClusterDataGroup> curGroup;
                        if ( it != groupInProgress.end() ) {
                            curGroup = it->second;
                            auto dItemTime = dataItem->getObjTime();
                            if (curGroup->isTimeToSendGroup(dItemTime)) {
                                // if time to send group pkt happens
                                mqttMsgManager_->addToDataQueue(curGroup);
                                groupInProgress.erase(it); // may be this line not needed //
                                createNewGroupNeeded = true;
                            } else {
                                // if need add packet to current group
                                curGroup->pushBack(dataItem);
                            }
                        }
                        else // if first pkt for id
                            createNewGroupNeeded = true;
                        if (createNewGroupNeeded) {
                            int objType = cfg_->getObjTypeForObjId(id);
                            if (objType == -1) {
                                cout << "object type for id: " << id << " not found, packet dropped" << endl;
                            } else {
                                if (objType == MQTT_OBJ_TYPE_LIGTH)
                                    curGroup = ClusterDataGroup::create(sendIntervalLights_, storeEveryPktLights_, objType);
                                else
                                    curGroup = ClusterDataGroup::create(sendInterval_, storeEveryPkt_, objType);
                                if (curGroup == nullptr) {
                                    cerr << "failed to allocate new cluster group, pkt dropped" << endl;
                                } else { // if new group created
                                    curGroup->pushBack(dataItem);
                                    groupInProgress[id] = curGroup;
                                } // end curGroup == nullptr
                            }
                        } // end createNewGroupNeeded
                    } // end dataItem != nullptr

                } // end Object.contains("id")

            } // end QJsonParseError::NoError != jError.error
            //cout << __PRETTY_FUNCTION__ << " delete " << drop_pkt_num << " pkt data: " << cur_pkt.get() << endl;
            //drop_pkt_num++;
        } else { // cur_pkt is null
            // !!! need replace to wait condition variable!!! //
            // cout << __PRETTY_FUNCTION__ << " sleep" << endl;
            usleep(50000);
        } // end cur_pkt != null
    }
    cout << "data processor loop terminated" << endl;
}


