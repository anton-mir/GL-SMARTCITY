#ifndef CLUSTER_DATA_ITEM_H
#define CLUSTER_DATA_ITEM_H

#include <inttypes.h>
#include <memory>

#include "clusterRawPkt.h"

using namespace std;

class ClusterDataItem
{
private:
    uint64_t objId_;
    uint8_t* data_;   // data without object id
    size_t  size_;
    ClusterPktTime   pktTime_;

private:
    ClusterDataItem();

public:
    ~ClusterDataItem();
    static shared_ptr<ClusterDataItem> create(uint64_t objId, ClusterPktTime pktTime, uint8_t *data, size_t dataSize);
    uint64_t getObjId();
    ClusterPktTime getObjTime();
    shared_ptr<string> toString(shared_ptr<string> mqttObjId);
};

#endif
