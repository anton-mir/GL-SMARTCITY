#include <iostream>
#include <memory.h>

#include "clusterDataItem.h"

ClusterDataItem::ClusterDataItem()
    : objId_{ 0xFFFFFFFFFFFFFFFF}
    , data_{ nullptr}
{

}
ClusterDataItem::~ClusterDataItem()
{
//    cout << __PRETTY_FUNCTION__ << " executed" << endl;
    if (data_ != nullptr)
        delete[] data_;
}

shared_ptr<ClusterDataItem>
ClusterDataItem::create(uint64_t objId, ClusterPktTime pktTime, uint8_t *data, size_t dataSize)
{

    shared_ptr<ClusterDataItem> item = shared_ptr<ClusterDataItem>(new ClusterDataItem);
    if (item == nullptr) {
        cerr << "allocate cluster data instance failed" << endl;
        return nullptr;
    }
    item->data_ = new uint8_t[ dataSize + 1 ];
    if (item->data_ == nullptr) {
        cerr << "allocate item data failed" << endl;
        return nullptr;
    }
    memcpy(item->data_, data, dataSize);
    item->data_[dataSize] = 0x0;
    item->size_ = dataSize;
    item->objId_ = objId;
    item->pktTime_ = pktTime;
    return item;
}

uint64_t
ClusterDataItem::getObjId()
{
    return objId_;
}

ClusterPktTime
ClusterDataItem::getObjTime()
{
    return pktTime_;
}

shared_ptr<string>
ClusterDataItem::toString(shared_ptr<string> mqttObjId)
{
    shared_ptr<string> outData = shared_ptr<string>(new string);
    // this operation reduce parsing time into cluster //
    // by Denis request 2019.03.29 //
    outData->append("{\"id\":\"");
    outData->append(mqttObjId->c_str());
    outData->append("\",\"timestp\":");
    outData->append(pktTime_.secToString());
    outData->append(",\"msec\":");
    outData->append(pktTime_.msecToString());
    outData->append(",");
    outData->append((char*)data_+1);
    //outData->append((char*)data_);
    // end //
    return outData;
}
