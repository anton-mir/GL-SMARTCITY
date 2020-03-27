#include <iostream>
#include "clusterDataGroup.h"

ClusterDataGroup::ClusterDataGroup()
    : objId_ {0xFFFFFFFFFFFFFFFF}
    , storeEveryPkt_ {1}
    , skipLeft {0}
{
    cout << __PRETTY_FUNCTION__ << " executed" << endl;
}

ClusterDataGroup::~ClusterDataGroup()
{
    cout << __PRETTY_FUNCTION__ << " executed" << endl;
}

shared_ptr<ClusterDataGroup> ClusterDataGroup::create(ClusterPktTime groupTimeInterval, unsigned storeEveryPkt, unsigned objType)
{
    cout << __PRETTY_FUNCTION__ << " executed" << endl;
    shared_ptr<ClusterDataGroup> instance = shared_ptr<ClusterDataGroup>(new ClusterDataGroup);
    if (instance == nullptr) {
        cerr << "allocate cluster data group failed" << endl;
        return nullptr;
    }
    instance->expairedTime = groupTimeInterval; // temporary store time interval to expaired time
    instance->storeEveryPkt_ = storeEveryPkt;
    instance->objType_ = objType;

    return instance;
}

bool
ClusterDataGroup::isTimeToSendGroup(ClusterPktTime &time)
{
    if (items.size() != 0 && time > expairedTime)
        return true;
    return false;
}

void
ClusterDataGroup::pushBack(shared_ptr<ClusterDataItem> item)
{
    if (items.size() == 0) {
        firstItemTime = item->getObjTime();
        objId_ = item->getObjId();
        //calculate expaired time
        expairedTime += firstItemTime;
    }
    if (skipLeft == 0) {
        items.push_back(item);
        skipLeft = storeEveryPkt_ - 1;
    } else // if need skip packet, reduce skip counter
        skipLeft--;

}

uint64_t
ClusterDataGroup::getObjId()
{
    return objId_;
}


shared_ptr<string>
ClusterDataGroup::toString(shared_ptr<string> mqttObjId)
{
    shared_ptr<string> outData = shared_ptr<string>(new string);
    string tmpStr;
    char tmpBuf[256];

    snprintf(tmpBuf, sizeof(tmpBuf), "%zu", items.size());
    tmpBuf[sizeof(tmpBuf) - 1] = 0x0;

    // packet header disabled for reduce cloud parsing time //
    // by Denis request 2019.03.29 //
//    outData->append("{\"id\": \"");
//    outData->append(mqttObjId->c_str());
//    outData->append("\"");
//    outData->append(",\"cnt\":");
//    outData->append(tmpBuf);
//    outData->append(",\"data\": [ ");
    outData->append("[");
    for (auto it = items.begin(); it != items.end(); it++) {
        if (it != items.begin())
            outData->append(",");
            outData->append(*((*it)->toString(mqttObjId)));
    }
    outData->append("]");
//    outData->append(" ]}");
    return outData;
}


unsigned
ClusterDataGroup::getObjType()
{
    return objType_;
}

