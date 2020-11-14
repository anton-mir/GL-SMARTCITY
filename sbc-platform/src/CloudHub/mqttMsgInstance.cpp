#include <iostream>

#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "mqttMsgInstance.h"
#include "mqttMsgParamsItem.h"
#include "sbc_mqtt_ciotc.h"


MqttMsgInstance::MqttMsgInstance()
    : dataItem_ {nullptr}
    , instancePid_ { -1 }
    , timeout_ {0}
{

}

shared_ptr<MqttMsgInstance>
MqttMsgInstance::create(shared_ptr<ClusterDataGroup> dataItem, shared_ptr<HubConfigManager> cfg)
{
    struct timespec spec;

    if (dataItem == nullptr) {
        cerr << "mqtt msq item: failed, null data group detected" << endl;
        return nullptr;
    }
    shared_ptr<MqttMsgInstance> instance = shared_ptr<MqttMsgInstance>(new MqttMsgInstance);
    instance->dataItem_ = dataItem;
    clock_gettime(CLOCK_REALTIME, &spec);
    instance->timeout_ = spec.tv_sec + cfg->getMqttMsgSendTimeout(); // calculate operation timeout
    instance->cfg_ = cfg;
    pid_t pid = fork();
    if (pid == (-1)) {
        cerr << "mqtt msg instance create child sub process failed, objId:" << dataItem->getObjId() << endl;
        return nullptr;
    }
    if (pid > 0) { // if parent process
        instance->instancePid_ = pid;
        return instance;
    }
    // if child process prepare and run mqtt message

    exit(instance->childFunction());
}

pid_t
MqttMsgInstance::getInstancePid()
{
    return instancePid_;

}

uint64_t
MqttMsgInstance::getObjId()
{
    return dataItem_->getObjId();
}

bool
MqttMsgInstance::isOperationTimeout()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    if (timeout_ >= spec.tv_sec)
        return false;
    return true;
}

void MqttMsgInstance::requestTerminate()
{
    cout << "mqtt msg instance: send kill to pid: " << instancePid_ << endl;
    if (instancePid_ > 0)
        kill(instancePid_, SIGKILL);
}

int
MqttMsgInstance::childFunction()
{
    shared_ptr<string> msgText;
    char* exec_params[17];
    char temp_app_name[] = "./sbccloudhub";
    char *rhost;
    char tmpMqttId[256];

    exec_params[16] = nullptr;
    //shared_ptr<MqttMsgParamsItem> msgParams = MqttMsgParamsItem::create(dataItem_->getObjId());
    shared_ptr<MqttMsgParamsItem> msgParams = cfg_->getMqttParamsForObjId(dataItem_->getObjId());
    if (msgParams == nullptr) {
        cout << "get message parameters failed, objId: " << dataItem_->getObjId() << endl;
        exit(EXIT_FAILURE);
    }
    msgText = dataItem_->toString(msgParams->deviceId);

    exec_params[0] = temp_app_name;
    exec_params[1] = (char*)msgText->c_str();
    exec_params[2] = (char*)"--deviceid";
    if (isLigthsInstance()) {
        strncpy(tmpMqttId,DEF_SBC_LIGTHS_MQTT_ID, sizeof(tmpMqttId));
        exec_params[3] = tmpMqttId;
    } else {
        exec_params[3] = (char*)msgParams->deviceId->c_str();
    }
    exec_params[4] = (char*)"--registryid";
    exec_params[5] = (char*)msgParams->registryId->c_str();
    exec_params[6] = (char*)"--projectid";
    exec_params[7] = (char*)msgParams->projectId->c_str();
    exec_params[8] = (char*)"--keypath";
    exec_params[9] = (char*)msgParams->keyPath->c_str();
    exec_params[10] = (char*)"--algorithm";
    exec_params[11] = (char*)msgParams->algorithm->c_str();
    exec_params[12] = (char*)"--rootpath";
    exec_params[13] = (char*)msgParams->rootPath->c_str();
    exec_params[14] = (char*)"--region";
    exec_params[15] = (char*)msgParams->region->c_str();

//    // !!! need only for debug !!! //
    cout << msgParams->deviceId->c_str() << endl;
//    cout << "exec cmd: ";
//    for (unsigned i = 0; ; i++) {
//        if (exec_params[i] == nullptr)
//            break;
//        cout << exec_params[i] << " ";
//    }
//    cout << endl;
//    // !!! need only for debug !!! //
    if (msgParams->rhost == nullptr || msgParams->rhost->size() == 0)
        rhost = NULL;
    else
        rhost = (char*)msgParams->rhost->c_str();
    return sbc_mqtt_main(16, exec_params, dataItem_->getObjType(), msgParams->rport, rhost, msgParams->sbcObjId);

}



bool MqttMsgInstance::isLigthsInstance()
{
    if (dataItem_->getObjType() == MQTT_OBJ_TYPE_LIGTH)
        return true;
    return false;
}


