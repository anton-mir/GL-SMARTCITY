#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <thread>
#include <mutex>
#include <inttypes.h>

#include "clusterDataProcessor.h"

#define MAX_PKT_SIZE    65536

using namespace std;

class UdpServer
{
private:
    UdpServer();
    void loopTerminateRequest();
    int runInstance(unsigned port);
    void setErrorState();
    int readUdpPkt(int sockf);

private:
    unsigned port_;
    int sockfd;
    int sockfd6;
    uint8_t buffer[MAX_PKT_SIZE];
    shared_ptr<thread> th;
    mutex loopControlMutex;
    bool loopTerminateFlag;
    bool errorStateFlag;
    shared_ptr<ClusterDataProcessor> dataProcessor_;


public:
    virtual ~UdpServer();
    static shared_ptr<UdpServer> create(unsigned port, shared_ptr<ClusterDataProcessor> dataProcessor);
    bool isLoopTerminateNeeded();
    void serverLoop();  // thread loop function
    bool isErrorState();


};

#endif
