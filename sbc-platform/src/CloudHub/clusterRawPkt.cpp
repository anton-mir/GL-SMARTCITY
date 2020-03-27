#include <memory.h>
#include <iostream>
#include "clusterRawPkt.h"

using namespace std;

ClusterPktTime::ClusterPktTime()
    :seconds_{0}
    ,mseconds_{0}
{
}

ClusterPktTime::ClusterPktTime(time_t seconds, unsigned mseconds)
    : seconds_{seconds}
    , mseconds_{mseconds}
{
}

string
ClusterPktTime::toString()
{
    char tmpBuf[256];
    string retVal;
    snprintf(tmpBuf, sizeof(tmpBuf), "%lu.%03u", seconds_, mseconds_);
    retVal.append(tmpBuf);
    return retVal;
}

string
ClusterPktTime::secToString()
{
    char tmpBuf[256];
    string retVal;
    snprintf(tmpBuf, sizeof(tmpBuf), "%lu", seconds_);
    retVal.append(tmpBuf);
    return retVal;
}

string
ClusterPktTime::msecToString()
{
    char tmpBuf[256];
    string retVal;
    snprintf(tmpBuf, sizeof(tmpBuf), "%u", mseconds_);
    retVal.append(tmpBuf);
    return retVal;
}



// ------------------------ //

ClusterRawPkt::ClusterRawPkt()
    : data_ {nullptr}
    , size_ {0}
{

}

shared_ptr<ClusterRawPkt>
ClusterRawPkt::create(uint8_t *data, size_t size, time_t seconds, unsigned mseconds)
{
    shared_ptr<ClusterRawPkt> instance = shared_ptr<ClusterRawPkt>(new ClusterRawPkt);
    instance->data_ = new uint8_t[size + 1];
    if (instance->data_ == nullptr) {
        cerr << "allocate pkt buf failed" << endl;
        return nullptr;
    }
    memcpy(instance->data_, data, size);
    instance->data_[size] = 0x0; // add null string terminator
    instance->size_ = size;
    instance->pktTime = ClusterPktTime(seconds, mseconds);
    return instance;
}


ClusterRawPkt::~ClusterRawPkt()
{
    if (data_ != nullptr)
        delete[] data_;
}

const uint8_t*
ClusterRawPkt::getData()
{
    return data_;
}

ClusterPktTime
ClusterRawPkt::getPktTime()
{
    return pktTime;
}
