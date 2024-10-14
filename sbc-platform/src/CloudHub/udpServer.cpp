#include <iostream>
#include <thread>

#include <errno.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "udpServer.h"
#include "clusterRawPkt.h"


using namespace std;

UdpServer::UdpServer()
    : port_{0}
    , sockfd{-1}
    , sockfd6{-1}
    , th {nullptr}
    , loopTerminateFlag{false}
    , errorStateFlag{false}
{
    cout << __PRETTY_FUNCTION__ << " executed" << endl;
}

void udpSrvthLoop(UdpServer *srv)
{
    srv->serverLoop();
}



shared_ptr<UdpServer>
UdpServer::create(unsigned port, shared_ptr<ClusterDataProcessor> dataProcessor)
{
//    UdpServer* srv;

    if (port == 0 || port > 65534) {
        cerr << "invalid udp server port " << port << endl;
        return nullptr;
    }
    shared_ptr<UdpServer> srv = shared_ptr<UdpServer>(new UdpServer());
    if (srv == nullptr) {
        cerr << "udp server create failed" << endl;
        return nullptr;
    }
    if (srv->runInstance(port) < 0)
        return nullptr;
    srv->dataProcessor_ = dataProcessor;
    return srv;
}

int UdpServer::runInstance(unsigned port)
{
    // struct sockaddr_in servaddr;
    struct sockaddr_in6 servaddr6;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        cerr << "socket create failed, err:" << strerror(errno) << endl;
        return -1;
    }
    if ( (sockfd6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) {
        cerr << "socket6 create failed, err:" << strerror(errno) << endl;
        return -1;
    }
    /*
    memset(&servaddr, 0, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        cerr << "udp socket bind failed, err:" << strerror(errno) << endl;
        return -1;
    }
    */

    memset(&servaddr6, 0, sizeof(servaddr6));
    // Filling server information
    servaddr6.sin6_family    = AF_INET6; // IPv6
    servaddr6.sin6_addr = in6addr_any;
    servaddr6.sin6_port = htons(port);
    // Bind the socket with the server address
    if ( bind(sockfd6, (const struct sockaddr *)&servaddr6, sizeof(servaddr6)) < 0 ) {
        cerr << "udp socket bind ipv6 failed, err:" << strerror(errno) << endl;
        return -1;
    }
    this->port_ = port;

    th = shared_ptr<thread>(new thread(udpSrvthLoop, this));
    if (th == nullptr) {
        cerr << "udp server thread create failed" << endl;
        return -1;
    }


    return 0;
}

bool UdpServer::isLoopTerminateNeeded()
{
    unique_lock<mutex> lock(loopControlMutex);
    return loopTerminateFlag;
}

void UdpServer::loopTerminateRequest()
{
    unique_lock<mutex> lock(loopControlMutex);
    loopTerminateFlag = true;
}

void UdpServer::setErrorState()
{
    unique_lock<mutex> lock(loopControlMutex);
    errorStateFlag = true;
}
bool UdpServer::isErrorState()
{
    unique_lock<mutex> lock(loopControlMutex);
    return errorStateFlag;
}

int UdpServer::readUdpPkt(int sockf)
{
    int rsl;
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    //len = sizeof(sockaddr_in);
    //rsl = recvfrom(sockfd, buffer, MAX_PKT_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
    rsl = recv(sockf, buffer, MAX_PKT_SIZE, MSG_WAITALL);
    if (rsl < 0) {
        cerr << "udp server recv err: " << strerror(errno) << endl;
        setErrorState();
        return -1;
    }
    if (rsl == 0) {
        cerr << "udp server socket unexpected closed" << endl;
        setErrorState();
        return -1;
    }
    buffer[rsl] = 0x0;

    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }
//            if (1) { // print time
//                 printf("Current time: %" PRIdMAX ".%03ld seconds since the Epoch\n",
//                        (intmax_t)s, ms);
//            }
//            cout << "msg_num: " << msg_cnt << " data received: " << buffer << endl;
//            msg_cnt++;
    //very simple check json
    if (*buffer == '{' && buffer[rsl - 1] == '}') {
        shared_ptr<ClusterRawPkt> pkt = ClusterRawPkt::create(buffer, rsl, s, ms);
        dataProcessor_->addDataToQueue(pkt);
    }
    return 0;
}

void UdpServer::serverLoop()
{
    fd_set rfds;
    struct timeval tv;
    int rsl, max_fd;
    //int len;
    //struct sockaddr_in cliaddr;
//    int msg_cnt = 0;

    while (!isLoopTerminateNeeded()) {
//        cout << "udp serv loop" << endl;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        FD_SET(sockfd6, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        if (sockfd > sockfd6)
            max_fd = sockfd + 1;
        else
            max_fd = sockfd6 + 1;

        rsl = select(max_fd, &rfds, NULL, NULL, &tv);
        if (rsl < 0 && errno == EINTR) { // if EINTR, sleep and continue
            usleep(50000);
            continue;
        }
        if (rsl < 0) {
            cout << "select error, err: " << strerror(errno) << endl;
            setErrorState();
            break;
        }
        if (FD_ISSET(sockfd, &rfds)) {
            if (readUdpPkt(sockfd) < 0)
                break;
        }
        if (FD_ISSET(sockfd6, &rfds)) {
            if (readUdpPkt(sockfd6) < 0)
                break;
        }

    }
    printf("udp server loop terminated\n");

}

UdpServer::~UdpServer()
{
    cout << __PRETTY_FUNCTION__ << " executed" << endl;
    if (th != nullptr) {
        loopTerminateRequest();
        th->join();
    }
    if (sockfd6 > 0)
        close(sockfd6);
    if (sockfd > 0)
        close(sockfd);
    cout << __PRETTY_FUNCTION__ << " end" << endl;
}
