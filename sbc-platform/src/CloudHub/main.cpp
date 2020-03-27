#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <memory.h>

#include <iostream>

#include "mqttMsgManager.h"
#include "clusterDataProcessor.h"
#include "udpServer.h"
#include "hubConfigManager.h"

using namespace std;

volatile sig_atomic_t gSigQueue = 0;
static pid_t *child_a0 = NULL;
static int *rsl_a0 = NULL;
static int cnt_a0 = 0;
static pid_t *child_a1 = NULL;
static int *rsl_a1 = NULL;
static int cnt_a1 = 0;
static int max_alen = 0;

shared_ptr<MqttMsgManager> mqttMsgManager = nullptr;

// SIGCHLD handler. //
static void sigchld_hdl (int sig)
{
    std::cout<<"seig start"<<std::endl;
    pid_t curPid;
    int rsl;
    if (sig != SIGCHLD) // if not sigchld, thomesing wrong
        return;
    /* Wait for all dead processes.
     * We use a non-blocking call to be sure this signal handler will not
     * block if a child was cleaned up in another part of the program. */
    while (1) {
        pid_t *child_ac;
        int *rsl_ac;
        int cnt_ac;

        curPid=waitpid(-1, &rsl, WNOHANG);
        if (curPid <= 0)
            break;
        if (gSigQueue == 0) {
            child_ac = child_a0;
            rsl_ac = rsl_a0;
            cnt_ac = cnt_a0;
        } else {
            child_ac = child_a1;
            rsl_ac = rsl_a1;
            cnt_ac = cnt_a1;
        }

        if (WIFEXITED(rsl))
            rsl = WEXITSTATUS(rsl);
        else
            rsl = EXIT_FAILURE;

        if (cnt_ac >= max_alen) {
            cout << "failed to store child result for pid=" << curPid << endl;
        } else {
            child_ac[cnt_ac] = curPid;
            rsl_ac[cnt_ac] = rsl;
            cnt_ac++;
            if (gSigQueue == 0)
                cnt_a0 = cnt_ac;
            else
                cnt_a1 = cnt_ac;
        }
    }
    std::cout<<"seig end"<<std::endl;
}

void usage(char *app_name)
{
    cout << "Usage: app_binary config_file" << endl;
    cout << "Example:" << endl;
    cout << app_name << " ./config.json" << endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    struct sigaction act;

    if (argc < 1)
        usage( (char*)"cloud_hub");
    if (argc != 2)
        usage(argv[0]);
    cout << "trap0" << endl;
    shared_ptr<HubConfigManager> cfg = HubConfigManager::create(argv[1]);
    if (cfg == nullptr)
        return EXIT_FAILURE;
#ifndef DEBUG
    std::cout.setstate(std::ios::failbit) ;
#endif
    cout << "trap1" << endl;
    max_alen = cfg->getMqttMsgParallelInstances() * 2; // max ended child process for one iteration is 2*max parallel instances
    child_a0 = (pid_t*)malloc(max_alen*sizeof(pid_t));
    rsl_a0 = (int*)malloc(max_alen*sizeof(int));
    cnt_a0 = 0;
    child_a1 = (pid_t*)malloc(max_alen*sizeof(pid_t));
    rsl_a1 = (int*)malloc(max_alen*sizeof(int));
    cnt_a1 = 0;
    if (child_a0 == NULL || rsl_a0 == NULL || child_a1 == NULL || rsl_a1 == NULL) {
        cerr << "failed to allocate child process memory" << endl;
        exit(EXIT_FAILURE);
    }



    //mqttMsgManager = shared_ptr<MqttMsgManager>(MqttMsgManager::create(MGR_PARALLEL_INSTANCES_DEFAULT));
    mqttMsgManager = shared_ptr<MqttMsgManager>
            (MqttMsgManager::create( cfg ));
    if (mqttMsgManager == nullptr) {
        cerr << "create mqtt message manager failed" << endl;
        return EXIT_FAILURE;
    }
    cout << "trap2" << endl;
    // after create manager register sigchld
    memset (&act, 0, sizeof(act));
    act.sa_handler = sigchld_hdl;
    if (sigaction(SIGCHLD, &act, 0)) {
        cerr << "register SIGCHLD handler failed" << endl;
        return EXIT_FAILURE;
    }

    shared_ptr<ClusterDataProcessor> dataProcessor(ClusterDataProcessor::create(mqttMsgManager, cfg));
    if (dataProcessor == nullptr) {
        cerr << "create cluster data processor failed" << endl;
        return EXIT_FAILURE;
    }
    shared_ptr<UdpServer> udpSrv = UdpServer::create(40883, dataProcessor); //5050
    if (udpSrv == nullptr) {
        cerr << "create udp server failed" << endl;
        return EXIT_FAILURE;
    }
    while (1) {
        pid_t *child_ac;
        int *rsl_ac;
        int cnt_ac;

        if (gSigQueue == 0) {
            gSigQueue = 1;
            child_ac = child_a0;
            rsl_ac = rsl_a0;
            cnt_ac = cnt_a0;

        } else {
            gSigQueue = 0;
            child_ac = child_a1;
            rsl_ac = rsl_a1;
            cnt_ac = cnt_a1;
        }
        for (int i=0; i < cnt_ac; i++) {
            //if (mqttMsgManager != nullptr)
            mqttMsgManager->signalChildL2(child_ac[i], rsl_ac[i]);
        }
        if (gSigQueue == 0)
            cnt_a1 = 0;
        else
            cnt_a0 = 0;

        // if udp server loop error break man loop
        if (udpSrv->isErrorState())
            break;
//        cout << "main hello" << endl;
        usleep(100000); // need to 2 times less then MqttMsgManager server loop
    }
    mqttMsgManager = nullptr;   // tell to signal hanler, that mqttMsgManager absent
    cout << "before exit" << endl;
    return EXIT_SUCCESS;
}

