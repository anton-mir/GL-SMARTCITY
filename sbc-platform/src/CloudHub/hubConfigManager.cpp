#include <iostream>
#include <QFile>
#include <QJsonDocument>
#include <QIODevice>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>

#include <sys/resource.h>

#include "mqttMsgParamsItem.h"
#include "clusterRawPkt.h"
#include "hubConfigManager.h"
#include "mqttMsgManager.h"

static shared_ptr<HubConfigManager> lastInstance;

#define DEF_UDP_SERVER_PORT 40883
#define MAX_UDP_SERVER_PORT 65535
#define MIN_UDP_SERVER_PORT 1

#define DEF_DATA_PROCESSOR_STORE_EVERY_PKT  10
#define MIN_DATA_PROCESSOR_STORE_EVERY_PKT  1
#define MAX_DATA_PROCESSOR_STORE_EVERY_PKT  1000
#define DEF_DATA_PROCESSOR_STORE_EVERY_PKT_LIGHTS   1

#define MIN_DATA_PROC_GROUP_INTERVAL_SEC    0
#define MIN_DATA_PROC_GROUP_INTERVAL_MSEC   500
#define MAX_DATA_PROC_GROUP_INTERVAL_SEC    120
#define MAX_DATA_PROC_GROUP_INTERVAL_MSEC   0


#define DEF_MQTT_MSG_SEND_TIMEOUT       7
#define MAX_MQTT_MSG_SENT_TIMEOUT       120
#define MIN_MQTT_MSG_SENT_TIMEOUT       2
#define DEF_MQTT_MSG_SEND_TIMEOUT_LIGHTS    4


#define DEF_DATA_PROCESSOR_GROUP_TIME_SEC   2
#define DEF_DATA_PROCESSOR_GROUP_TIME_MSEC  0

#define DEF_DATA_PROCESSOR_LIGHTS_GROUP_TIME_SEC    1
#define DEF_DATA_PROCESSOR_LIGHTS_GROUP_TIME_MSEC   500

#define DEF_MGR_PARALLEL_INSTANCES  1
//#define MGR_MAX_PARALLEL_INSTANCES      1000
#define MGR_MIN_PARALLEL_INSTANCES      1



HubConfigManager::HubConfigManager()
    : mqttMsgParallelInstances {DEF_MGR_PARALLEL_INSTANCES}
    , mqttMsgOperationTimeout {DEF_MQTT_MSG_SEND_TIMEOUT}
    , dataProcessorGroupInterval { ClusterPktTime(DEF_DATA_PROCESSOR_GROUP_TIME_SEC, DEF_DATA_PROCESSOR_GROUP_TIME_MSEC) }
    , dataProcessorStoreEveryPkt {DEF_DATA_PROCESSOR_STORE_EVERY_PKT}
    , udpServerPort {DEF_UDP_SERVER_PORT}
    , mqttMsgOperationTimeoutLights {DEF_MQTT_MSG_SEND_TIMEOUT_LIGHTS}
    , dataProcessorGroupIntervalLights { ClusterPktTime(DEF_DATA_PROCESSOR_LIGHTS_GROUP_TIME_SEC, DEF_DATA_PROCESSOR_LIGHTS_GROUP_TIME_MSEC) }
    , dataProcessorStoreEveryPktLights { DEF_DATA_PROCESSOR_STORE_EVERY_PKT_LIGHTS }
{

}

shared_ptr<HubConfigManager>
HubConfigManager::create(char *configFile)
{
    if (!QFile::exists(QString(configFile))) {
        cerr << "config file not exists" << endl;
        return nullptr;
    }
    shared_ptr<HubConfigManager> instance = shared_ptr<HubConfigManager>(new HubConfigManager);
    unique_lock<mutex> lock(instance->instanceAccessMutex);
    QFile cfgFile(configFile);
    if(!cfgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        cerr << "open config file failed, file path: " << configFile << endl;
        return nullptr;
    }
    lastInstance = instance;
    QByteArray data = cfgFile.readAll();
    cfgFile.close();
    QJsonParseError error;
    QJsonDocument jDoc = QJsonDocument::fromJson(data, &error);
    if(QJsonParseError::NoError != error.error) {
        cerr << "parse config file failed, error: " << error.errorString().toLocal8Bit().constData() << endl;
        return nullptr;
    }
    QJsonObject jObj = jDoc.object();
    // ----------------------- //
    if (!jObj.contains("mqtt_msg_parallel_instances"))
            cout << "mqtt_msg_parallel_instances not found, use default value: " << DEF_MGR_PARALLEL_INSTANCES << endl;
    else {
        struct rlimit rl;
        instance->mqttMsgParallelInstances = jObj["mqtt_msg_parallel_instances"].toInt();
        getrlimit(RLIMIT_NPROC, &rl);
        // cout << "cur limit: " << rl.rlim_cur << endl;
        if (rl.rlim_cur < instance->mqttMsgParallelInstances) {
            cout << "Warning: current instance limit is: " << rl.rlim_cur << endl;
            instance->mqttMsgParallelInstances = rl.rlim_cur;
        } else if (instance->mqttMsgParallelInstances == 0) // min parralel must be great then 0
            instance->mqttMsgParallelInstances = MGR_MIN_PARALLEL_INSTANCES;
    }
    // ----------------------- //
    if (!jObj.contains("mqtt_msg_operation_timeout"))
        cout << "mqtt_msg_operation_timeout not found, use default value: " << DEF_MQTT_MSG_SEND_TIMEOUT << endl;
    else {
        instance->mqttMsgOperationTimeout = jObj["mqtt_msg_operation_timeout"].toInt();
        if (instance->mqttMsgOperationTimeout > MAX_MQTT_MSG_SENT_TIMEOUT) {
            cout << "max allowed mqtt_msg_operation_timeout is " << MAX_MQTT_MSG_SENT_TIMEOUT << endl;
            instance->mqttMsgOperationTimeout = MAX_MQTT_MSG_SENT_TIMEOUT;
        } else if (instance->mqttMsgOperationTimeout < MIN_MQTT_MSG_SENT_TIMEOUT) {
            cout << "min allowed mqtt_msg_operation_timeout is " << MIN_MQTT_MSG_SENT_TIMEOUT << endl;
            instance->mqttMsgOperationTimeout = MIN_MQTT_MSG_SENT_TIMEOUT;
        }
    }
    // ----------------------- //
    if (!jObj.contains("mqtt_ligths_operation_timeout"))
        cout << "mqtt_ligths_operation_timeout not found, use default value: " << DEF_MQTT_MSG_SEND_TIMEOUT_LIGHTS << endl;
    else {
        instance->mqttMsgOperationTimeout = jObj["mqtt_ligths_operation_timeout"].toInt();
        if (instance->mqttMsgOperationTimeout > MAX_MQTT_MSG_SENT_TIMEOUT) {
            cout << "max allowed mqtt_ligths_operation_timeout is " << MAX_MQTT_MSG_SENT_TIMEOUT << endl;
            instance->mqttMsgOperationTimeout = MAX_MQTT_MSG_SENT_TIMEOUT;
        } else if (instance->mqttMsgOperationTimeout < MIN_MQTT_MSG_SENT_TIMEOUT) {
            cout << "min allowed mqtt_ligths_operation_timeout is " << MIN_MQTT_MSG_SENT_TIMEOUT << endl;
            instance->mqttMsgOperationTimeout = MIN_MQTT_MSG_SENT_TIMEOUT;
        }
    }
    // ----------------------- //
    if (!jObj.contains("data_processos_group_intervel"))
        cout << "data_processos_group_intervel not found, use default value: " << DEF_DATA_PROCESSOR_GROUP_TIME_SEC << "." << DEF_DATA_PROCESSOR_GROUP_TIME_MSEC << endl;
    else {
        QJsonValueRef jVal = jObj["data_processos_group_intervel"];
        QJsonObject jSubObj = jVal.toObject();
        if (!jSubObj.contains("sec") || !jSubObj.contains("msec")) {
            printf("data_processos_group_intervel must have fields: sec, msec\n");
            return nullptr;
        } else {
            ClusterPktTime tmpTime;
            instance->dataProcessorGroupInterval.seconds_ = jSubObj["sec"].toInt();
            instance->dataProcessorGroupInterval.mseconds_ = jSubObj["msec"].toInt();
            tmpTime = ClusterPktTime(MIN_DATA_PROC_GROUP_INTERVAL_SEC, MIN_DATA_PROC_GROUP_INTERVAL_MSEC);
            if (instance->dataProcessorGroupInterval < tmpTime) {
                cout << "min allowed data_processos_group_intervel is " << MIN_DATA_PROC_GROUP_INTERVAL_SEC << "." << MIN_DATA_PROC_GROUP_INTERVAL_MSEC << endl;
                instance->dataProcessorGroupInterval.seconds_ = MIN_DATA_PROC_GROUP_INTERVAL_SEC;
                instance->dataProcessorGroupInterval.mseconds_ = MIN_DATA_PROC_GROUP_INTERVAL_MSEC;
            } else {
                tmpTime = ClusterPktTime(MAX_DATA_PROC_GROUP_INTERVAL_SEC, MAX_DATA_PROC_GROUP_INTERVAL_MSEC);
                if (instance->dataProcessorGroupInterval > tmpTime) {
                    cout << "max allowed data_processos_group_intervel is " << MAX_DATA_PROC_GROUP_INTERVAL_SEC << "." << MAX_DATA_PROC_GROUP_INTERVAL_MSEC << endl;
                    instance->dataProcessorGroupInterval.seconds_ = MAX_DATA_PROC_GROUP_INTERVAL_SEC;
                    instance->dataProcessorGroupInterval.mseconds_ = MAX_DATA_PROC_GROUP_INTERVAL_MSEC;
                }
            }
        }
    }

    // ----------------------- //
    if (!jObj.contains("data_lights_group_intervel"))
        cout << "data_lights_group_intervel not found, use default value: " << DEF_DATA_PROCESSOR_GROUP_TIME_SEC << "." << DEF_DATA_PROCESSOR_GROUP_TIME_MSEC << endl;
    else
    {
        QJsonValueRef jVal = jObj["data_lights_group_intervel"];
        QJsonObject jSubObj = jVal.toObject();
        if (!jSubObj.contains("sec") || !jSubObj.contains("msec")) {
            printf("data_lights_group_intervel must have fields: sec, msec\n");
            return nullptr;
        }
        else
        {
            ClusterPktTime tmpTime;
            instance->dataProcessorGroupIntervalLights.seconds_ = jSubObj["sec"].toInt();
            instance->dataProcessorGroupIntervalLights.mseconds_ = jSubObj["msec"].toInt();
            tmpTime = ClusterPktTime(MIN_DATA_PROC_GROUP_INTERVAL_SEC, MIN_DATA_PROC_GROUP_INTERVAL_MSEC);

            if (instance->dataProcessorGroupIntervalLights < tmpTime)
            {
                cout << "min allowed data_lights_group_intervel is " << MIN_DATA_PROC_GROUP_INTERVAL_SEC << "." << MIN_DATA_PROC_GROUP_INTERVAL_MSEC << endl;
                instance->dataProcessorGroupIntervalLights.seconds_ = MIN_DATA_PROC_GROUP_INTERVAL_SEC;
                instance->dataProcessorGroupIntervalLights.mseconds_ = MIN_DATA_PROC_GROUP_INTERVAL_MSEC;
            }
            else
            {
                tmpTime = ClusterPktTime(MAX_DATA_PROC_GROUP_INTERVAL_SEC, MAX_DATA_PROC_GROUP_INTERVAL_MSEC);
                if (instance->dataProcessorGroupIntervalLights > tmpTime) {
                    cout << "max allowed data_lights_group_intervel is " << MAX_DATA_PROC_GROUP_INTERVAL_SEC << "." << MAX_DATA_PROC_GROUP_INTERVAL_MSEC << endl;
                    instance->dataProcessorGroupIntervalLights.seconds_ = MAX_DATA_PROC_GROUP_INTERVAL_SEC;
                    instance->dataProcessorGroupIntervalLights.mseconds_ = MAX_DATA_PROC_GROUP_INTERVAL_MSEC;
                }
            }
        }
    }

    // ----------------------- //
    if (!jObj.contains("data_processor_store_every_pkt"))
        cout << "data_processor_store_every_pkt not found, use default value: " << DEF_DATA_PROCESSOR_STORE_EVERY_PKT << endl;
    else
    {
        instance->dataProcessorStoreEveryPkt = jObj["data_processor_store_every_pkt"].toInt();
        if (instance->dataProcessorStoreEveryPkt > MAX_DATA_PROCESSOR_STORE_EVERY_PKT) {
            cout << "max allowed data_processor_store_every_pkt is " << MAX_DATA_PROCESSOR_STORE_EVERY_PKT << endl;
            instance->dataProcessorStoreEveryPkt = MAX_DATA_PROCESSOR_STORE_EVERY_PKT;
        } else if (instance->dataProcessorStoreEveryPkt < MIN_DATA_PROCESSOR_STORE_EVERY_PKT) {
            cout << "min allowed data_processor_store_every_pkt is " << MIN_DATA_PROCESSOR_STORE_EVERY_PKT << endl;
            instance->dataProcessorStoreEveryPkt = MIN_DATA_PROCESSOR_STORE_EVERY_PKT;
        }
    }

    // ----------------------- //
    if (!jObj.contains("data_processor_ligths_store_every_pkt"))
        cout << "data_processor_ligths_store_every_pkt not found, use default value: " << DEF_DATA_PROCESSOR_STORE_EVERY_PKT << endl;
    else
    {
        instance->dataProcessorStoreEveryPktLights = jObj["data_processor_ligths_store_every_pkt"].toInt();
        if (instance->dataProcessorStoreEveryPktLights > MAX_DATA_PROCESSOR_STORE_EVERY_PKT) {
            cout << "max allowed data_processor_ligths_store_every_pkt is " << MAX_DATA_PROCESSOR_STORE_EVERY_PKT << endl;
            instance->dataProcessorStoreEveryPktLights = MAX_DATA_PROCESSOR_STORE_EVERY_PKT;
        } else if (instance->dataProcessorStoreEveryPktLights < MIN_DATA_PROCESSOR_STORE_EVERY_PKT) {
            cout << "min allowed data_processor_ligths_store_every_pkt is " << MIN_DATA_PROCESSOR_STORE_EVERY_PKT << endl;
            instance->dataProcessorStoreEveryPktLights = MIN_DATA_PROCESSOR_STORE_EVERY_PKT;
        }
    }

    // ----------------------- //
    if (!jObj.contains("port"))
    {
        cout << "MQTT [port] value not found in config, using default: " << DEF_UDP_SERVER_PORT << endl;
    }
    else
    {
        instance->udpServerPort = jObj["data_processor_store_every_pkt"].toInt();
        if (instance->udpServerPort > MAX_UDP_SERVER_PORT)
        {
            cout << "max allowed port is " << MAX_UDP_SERVER_PORT << endl;
            instance->udpServerPort = MAX_UDP_SERVER_PORT;
        }
        else if (instance->udpServerPort < MIN_UDP_SERVER_PORT)
        {
            cout << "min allowed port is " << MIN_UDP_SERVER_PORT << endl;
            instance->udpServerPort = MIN_UDP_SERVER_PORT;
        }
    }

    // ----------------------- //
    // objMqttParams
    if (!jObj.contains("mqtt_group_param"))
    {
        cout << "mqtt_group_param absent" << endl;
        return nullptr;
    }
    QJsonValueRef jVal = jObj["mqtt_group_param"];
    QJsonObject jSubObj = jVal.toObject();

    shared_ptr<string> registryId;
    if (!jSubObj.contains("registry_id"))
    {
        cout << "mqtt_group_param.registry_id absent" << endl;
        return nullptr;
    }
    else
    {
        registryId = shared_ptr<string>(new string(jSubObj["registry_id"].toString().trimmed().toUtf8().constData()));
        if (registryId->size() == 0) {
            cout << "mqtt_group_param.registry_id empty" << endl;
            return nullptr;
        }
    }

    // -------- //
    shared_ptr<string> projectId;
    if (!jSubObj.contains("project_id"))
    {
        cout << "mqtt_group_param.project_id absent" << endl;
        return nullptr;
    }
    else
    {
        projectId = shared_ptr<string>(new string(jSubObj["project_id"].toString().trimmed().toUtf8().constData()));
        if (projectId->size() == 0) {
            cout << "mqtt_group_param.project_id empty" << endl;
            return nullptr;
        }
    }

    // -------- //
    shared_ptr<string> rootPath;
    if (!jSubObj.contains("root_cert"))
    {
        cout << "mqtt_group_param.root_cert absent" << endl;
        return nullptr;
    }
    else
    {
        rootPath = shared_ptr<string>(new string(jSubObj["root_cert"].toString().trimmed().toUtf8().constData()));

        if (rootPath->size() == 0) {
            cout << "mqtt_group_param.root_cert empty" << endl;
            return nullptr;
        }

        if (!QFile::exists(rootPath->c_str()))
        {
            cout << "mqtt_group_param.root_cert: file not exists: " << rootPath->c_str() << endl;
            return nullptr;
        }
    }

    // -------- //
    shared_ptr<string> region;
    if (!jSubObj.contains("region"))
    {
        cout << "mqtt_group_param.region absent" << endl;
        return nullptr;
    }
    else
    {
    region = shared_ptr<string>(new string(jSubObj["region"].toString().trimmed().toUtf8().constData()));
        if (region->size() == 0)
        {
            cout << "mqtt_group_param.region empty" << endl;
            return nullptr;
        }
    }

    // -------- //
    if (!jSubObj.contains("objects"))
    {
        cout << "mqtt_group_param.objects absent" << endl;
        return nullptr;
    }
    jVal = jSubObj["objects"];
    QJsonArray jMqttObj = jVal.toArray();

    if ( jMqttObj.count() == 0)
    {
        cout << "mqtt_group_param.objects is empty" << endl;
        return nullptr;
    }

    for (int lp=0; lp < jMqttObj.count(); lp++)
    {
        shared_ptr<string> deviceId;
        shared_ptr<string> keyPath;
        shared_ptr<string> algorithm;
        string strObjType;
        int objType;
        shared_ptr<string> rhost;
        unsigned rport;
        qlonglong sbc_id;
        QJsonObject mObj = jMqttObj[lp].toObject();

        cout << "Processing config object " << lp << " ... ";

        if (!mObj.contains("mqtt_id"))
        {
            cout << "object[" << lp << "] skipped, mqtt_id absent" << endl;
            continue;
        }

        deviceId = shared_ptr<string>(new string(mObj["mqtt_id"].toString().trimmed().toUtf8().constData()));

        if (deviceId->size() == 0)
        {
            cout << "object[" << lp << "] skipped, mqtt_id empty" << endl;
            continue;
        }

        // ---- //
        if (!mObj.contains("sbc_id"))
        {
            cout << "object[" << lp << "] skipped, sbc_id absent" << endl;
            continue;
        }
        sbc_id =  mObj["sbc_id"].toVariant().toLongLong();

        // ---- //
        if (!mObj.contains("key_path"))
        {
            cout << "object[" << lp << "] skipped, key_path absent" << endl;
            continue;
        }
        keyPath = shared_ptr<string>(new string(mObj["key_path"].toString().trimmed().toUtf8().constData()));

        if (keyPath->size() == 0)
        {
            cout << "object[" << lp << "] skipped, key_path empty" << endl;
            continue;
        }

        if (!QFile::exists(keyPath->c_str()))
        {
            cout << "object[" << lp << "] skipped, key_path file not exists: " << keyPath->c_str() << endl;
            continue;
        }

        // ---- //

        if (!mObj.contains("key_algorytm"))
        {
            cout << "object[" << lp << "] skipped, key_algorytm absent" << endl;
            continue;
        }

        algorithm = shared_ptr<string>(new string(mObj["key_algorytm"].toString().trimmed().toUtf8().constData()));

        if (algorithm->size() == 0)
        {
            cout << "object[" << lp << "] skipped, key_algorytm empty" << endl;
            continue;
        }
        // ---- //
        if (mObj.contains("type"))
        {
            strObjType = mObj["type"].toString().toUpper().toUtf8().data();

            if (strObjType == "LIGHTS")
            {
                objType = MQTT_OBJ_TYPE_LIGTH;
                if (!mObj.contains("rhost"))
                {
                    cout << "lights[" << lp << "] skipped, rhost absent." << endl;
                    continue;
                }
                rhost = shared_ptr<string>(new string(mObj["rhost"].toString().trimmed().toUtf8().data()));
                if (rhost->size() == 0)
                {
                    cout << "lights[" << lp << "] skipped, rhost empty." << endl;
                    continue;
                }
                if (!mObj.contains("rport"))
                {
                    cout << "lights[" << lp << "] skipped, rport absent." << endl;
                    continue;
                }
                rport = mObj["rport"].toInt();
                if (rport < MIN_UDP_SERVER_PORT || rport > MAX_UDP_SERVER_PORT)
                {
                    cout << "lights[" << lp << "] skipped, rport invalid value: " << rport << endl;
                    continue;
                }
            }
            else if (strObjType == "AIRC_BOX")
            {
                cout << "Using AirC_Box type.";
                objType = MQTT_OBJ_TYPE_AIRC_BOX;
            }
            else if (strObjType == "CAR")
            {
                cout << "Using Car type.";
                objType = MQTT_OBJ_TYPE_CAR;
            }
            else
            {
                cout << "unknow type: " << mObj["type"].toString().toUpper().toUtf8().data() << ", use default type: car" << endl;
                objType = MQTT_OBJ_TYPE_CAR;
            }

        }
        else
        {
            cout << "absent object type, use default type: car" << endl;
            objType = MQTT_OBJ_TYPE_CAR;
        }

        // ---- //
        shared_ptr<MqttMsgParamsItem> mqttInstance = nullptr;
        if (objType == MQTT_OBJ_TYPE_LIGTH)
            mqttInstance = MqttMsgParamsItem::createL(sbc_id, deviceId, registryId, projectId, keyPath, algorithm, rootPath, region, objType, rhost, rport);
        else if (objType == MQTT_OBJ_TYPE_AIRC_BOX)
            mqttInstance = MqttMsgParamsItem::create(sbc_id, deviceId, registryId, projectId, keyPath, algorithm, rootPath, region, objType);
        else if (objType == MQTT_OBJ_TYPE_CAR)
            mqttInstance = MqttMsgParamsItem::create(sbc_id, deviceId, registryId, projectId, keyPath, algorithm, rootPath, region, objType);

        if (mqttInstance != nullptr)
            instance->objMqttParams.push_back(mqttInstance);

        cout << " Object processed.\n";
    }
    // -------- //
    if (instance->objMqttParams.empty() ) {
        cout << "no one valid entry into mqtt_group_param.objects" << endl;
        return nullptr;
    }
    // ----------------------- //


    // ------------------------------------------------------------------- //
    // ----------------------- test values ------------------------------- //

//    {
//    shared_ptr<string> deviceId1 = shared_ptr<string>(new string("mqtt-001"));
//    shared_ptr<string> deviceId2 = shared_ptr<string>(new string("mqtt-002"));
//    shared_ptr<string> deviceId3 = shared_ptr<string>(new string("mqtt-003"));
//    shared_ptr<string> deviceId10 = shared_ptr<string>(new string("mqtt-0010"));
//
//    shared_ptr<string> registryId = shared_ptr<string>(new string("smartcity"));
//    shared_ptr<string> projectId = shared_ptr<string>(new string("sunlit-precinct-233015"));
//    shared_ptr<string> keyPath = shared_ptr<string>(new string("./rsa_private.pem"));
//    shared_ptr<string> algorithm = shared_ptr<string>(new string("RS256"));
//    shared_ptr<string> rootPath = shared_ptr<string>(new string("./roots.pem"));
//    shared_ptr<string> region = shared_ptr<string>(new string("europe-west1"));

//    instance->objMqttParams.push_back(MqttMsgParamsItem::create(1001, deviceId1, registryId, projectId, keyPath, algorithm, rootPath, region));
//    instance->objMqttParams.push_back(MqttMsgParamsItem::create(1002, deviceId2, registryId, projectId, keyPath, algorithm, rootPath, region));
//    instance->objMqttParams.push_back(MqttMsgParamsItem::create(1003, deviceId3, registryId, projectId, keyPath, algorithm, rootPath, region));
//    }

    // ------------------------------------------------------------------- //

    lastInstance = instance;
    return instance;
}

unsigned
HubConfigManager::getMqttMsgParallelInstances()
{
    unique_lock<mutex> lock(instanceAccessMutex);
    return mqttMsgParallelInstances;
}

unsigned
HubConfigManager::getMqttMsgSendTimeout()
{
    unique_lock<mutex> lock(instanceAccessMutex);
    return mqttMsgOperationTimeout;
}


ClusterPktTime
HubConfigManager::getDataProcessorGroupInterval()
{
    unique_lock<mutex> lock(instanceAccessMutex);
    return dataProcessorGroupInterval;
}

unsigned
HubConfigManager::getDataProcessorStoreEveryPkt()
{
    unique_lock<mutex> lock(instanceAccessMutex);
    return dataProcessorStoreEveryPkt;
}

shared_ptr<MqttMsgParamsItem>
HubConfigManager::getMqttParamsForObjId(uint64_t  objId)
{
    unique_lock<mutex> lock(instanceAccessMutex);
    for (auto it = objMqttParams.begin(); it != objMqttParams.end(); it++)
        if ( (*it)->sbcObjId == objId )
            return *it;
    return nullptr;
}

int
HubConfigManager::getObjTypeForObjId(uint64_t  objId)
{
    unique_lock<mutex> lock(instanceAccessMutex);
    for (auto it = objMqttParams.begin(); it != objMqttParams.end(); it++)
        if ( (*it)->sbcObjId == objId )
            return (*it)->objType;
    return -1;
}


unsigned
HubConfigManager::getMqttMsgSendTimeoutLights()
{
    unique_lock<mutex> lock(instanceAccessMutex);
    return mqttMsgOperationTimeoutLights;
}


ClusterPktTime
HubConfigManager::getDataProcessorGroupIntervalLights()
{
    unique_lock<mutex> lock(instanceAccessMutex);
    return dataProcessorGroupIntervalLights;
}

unsigned
HubConfigManager::getDataProcessorStoreEveryPktLights()
{
    unique_lock<mutex> lock(instanceAccessMutex);
    return dataProcessorStoreEveryPktLights;
}

uint64_t
HubConfigManager::getSbcIdByMqttId(char *mqttId, int *result)
{
    uint64_t ret_val;
    unique_lock<mutex> lock(instanceAccessMutex);
    auto it = objMqttParams.begin();

    for (; it != objMqttParams.end(); it++)
    {
        if ((*it)->deviceId->compare(mqttId) == 0)
            break;
    }
    if (it == objMqttParams.end())
    {
        if (result != nullptr)
            *result = -1;
        ret_val = 0;
    }
    else
    {
        if (result != nullptr)
            *result = 0;
        ret_val = (*it)->sbcObjId;
    }
    return ret_val;
}

shared_ptr<HubConfigManager>
HubConfigManager::getLastInstance()
{
    return lastInstance;
}
