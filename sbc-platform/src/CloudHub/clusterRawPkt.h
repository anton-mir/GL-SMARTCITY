#ifndef CLUSTER_RAW_PKT_H
#define CLUSTER_RAW_PKT_H

#include <memory>
#include <string>
#include <inttypes.h>
#include <time.h>

using namespace std;

class ClusterPktTime
{
public:
    time_t seconds_;        //packet creation time sec.milisec
    unsigned mseconds_;     //
public:
    ClusterPktTime();
    ClusterPktTime(time_t seconds, unsigned mseconds);
    string toString();
    string secToString();
    string msecToString();

    bool operator== (const ClusterPktTime &x) {
        if (x.seconds_ == seconds_ && x.mseconds_ == mseconds_)
            return true;
        return false;
    }
    ClusterPktTime& operator+=(const ClusterPktTime& x) {
        this->seconds_ += x.seconds_;
        this->mseconds_ += x.mseconds_;
        if (this->mseconds_ >= 1000) {
            this->seconds_ += 1;
            this->mseconds_ -= 1000;
        }
        return *this;
    }
    bool operator> (const ClusterPktTime& x)
    {
        if (this->seconds_ > x.seconds_) return true;
        if (this->seconds_ == x.seconds_ && this->mseconds_ > x.mseconds_) return true;
        return false;
    }
    bool operator< (const ClusterPktTime& x)
    {
        if (this->seconds_ < x.seconds_) return true;
        if (this->seconds_ == x.seconds_ && this->mseconds_ < x.mseconds_) return true;
        return false;
    }

};
// ------------------------ //
class ClusterRawPkt
{
private:
    uint8_t *data_;
    size_t size_;
    ClusterPktTime pktTime;

private:
    ClusterRawPkt();
public:
    ~ClusterRawPkt();
    static shared_ptr<ClusterRawPkt> create(uint8_t *data, size_t size, time_t seconds, unsigned mseconds);
    const uint8_t* getData();
    ClusterPktTime getPktTime();
};

#endif
