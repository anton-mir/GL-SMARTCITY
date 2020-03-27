#include <iostream>
#include <QFile>
#include "mqttMsgParamsItem.h"

MqttMsgParamsItem::MqttMsgParamsItem()
{

}

//shared_ptr<MqttMsgParamsItem>
//MqttMsgParamsItem::create(uint64_t sbcObjId)
shared_ptr<MqttMsgParamsItem>
MqttMsgParamsItem::create(uint64_t newSbcObjId, shared_ptr<string> newDeviceId, shared_ptr<string> newRegistryId,
                shared_ptr<string> newProjectId, shared_ptr<string> newKeyPath, shared_ptr<string> newAlgorithm,
                shared_ptr<string> newRootPath, shared_ptr<string> newRegion, int newObjType)
{
    shared_ptr<MqttMsgParamsItem> instance = shared_ptr<MqttMsgParamsItem>(new MqttMsgParamsItem());
    if (newDeviceId->size() == 0) {
        cout << "empty mqtt deviceId, element dropped sbcObjId: " << newSbcObjId;
        return nullptr;
    }
    if (newRegistryId->size() == 0) {
        cout << "empty mqtt registryId, element dropped sbcObjId: " << newSbcObjId;
        return nullptr;
    }
    if (newProjectId->size() == 0) {
        cout << "empty mqtt projectId, element dropped sbcObjId: " << newSbcObjId;
        return nullptr;
    }
    if (newKeyPath->size() == 0 || !QFile::exists(QString(newKeyPath->c_str()))) {
        cout << "key file not exists, element dropped objId: " << newSbcObjId << ", file: " << newKeyPath->c_str() << endl;
        return nullptr;
    }
    if (newAlgorithm->size() == 0) {
        cout << "empty mqtt algorythm, use default RS256, sbcObjId: " << newSbcObjId;
        newAlgorithm->append("RS256");
    }
    if (newRootPath->size() == 0 || !QFile::exists(QString(newRootPath->c_str()))) {
        cout << "root cert file not exists, element dropped objId: " << newSbcObjId << ", file: " << newRootPath->c_str() << endl;
        return nullptr;
    }
    if (newRegion->size() == 0) {
        cout << "empty mqtt region, element dropped sbcObjId: " << newSbcObjId;
        return nullptr;
    }
    instance->sbcObjId = newSbcObjId;
    instance->deviceId = newDeviceId;
    instance->registryId = newRegistryId;
    instance->projectId = newProjectId;
    instance->keyPath = newKeyPath;
    instance->algorithm = newAlgorithm;
    instance->rootPath = newRootPath;
    instance->region = newRegion;
    instance->objType = newObjType;

    return instance;
}

shared_ptr<MqttMsgParamsItem>
MqttMsgParamsItem::createL(uint64_t newSbcObjId, shared_ptr<string> newDeviceId, shared_ptr<string> newRegistryId,
            shared_ptr<string> newProjectId, shared_ptr<string> newKeyPath, shared_ptr<string> newAlgorithm,
            shared_ptr<string> newRootPath, shared_ptr<string> newRegion, int newObjType, shared_ptr<string> newRHost, unsigned newRPort)
{
    shared_ptr<MqttMsgParamsItem> instance = MqttMsgParamsItem::create(newSbcObjId, newDeviceId, newRegistryId, newProjectId, newKeyPath, newAlgorithm, newRootPath, newRegion, newObjType);
    if (instance == nullptr)
        return nullptr;
    instance->rport = newRPort;
    instance->rhost = newRHost;
    return instance;
}
