#ifndef CLUSTER_DATA_GROUP_H
#define CLUSTER_DATA_GROUP_H

#include <list>
#include "clusterDataItem.h"

using namespace std;
class ClusterDataGroup
{
private:
    uint64_t    objId_; // sbc objId
    ClusterPktTime firstItemTime;
    ClusterPktTime expairedTime;
    list<shared_ptr<ClusterDataItem>> items;
    unsigned storeEveryPkt_;
    unsigned skipLeft;
    unsigned objType_;
private:
    ClusterDataGroup();
public:

    virtual ~ClusterDataGroup();
    /**
      groupTimeInterval - time interval for group packets
      storeEveryPkt - store every number pkt (e.g. 1 for store each new packet, 10 for store each 10 packet from udp server)
     **/
    static shared_ptr<ClusterDataGroup> create(ClusterPktTime groupTimeInterval, unsigned storeEveryPkt, unsigned objType);
    bool isTimeToSendGroup(ClusterPktTime &time);
    void pushBack(shared_ptr<ClusterDataItem> item);
    uint64_t getObjId();
    // caller must get mqttObjId from mqttMsgParamsItem before, this need because sbc platfomrm don't support 64bit id at this time
    shared_ptr<string> toString(shared_ptr<string> mqttObjId);
    unsigned getObjType();

};

#endif
