QT += core
QT -= gui

CONFIG += c++11
TARGET = cloud_hub
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    udpServer.cpp \
    clusterDataProcessor.cpp \
    mqttMsgInstance.cpp \
    mqttMsgManager.cpp \
    clusterDataItem.cpp \
    clusterDataGroup.cpp \
    clusterRawPkt.cpp \
    sbc_mqtt_ciotc.c \
    mqttMsgParamsItem.cpp \
    hubConfigManager.cpp \
    hubConfigManagerWrapper.cpp

HEADERS += \
    udpServer.h \
    clusterDataProcessor.h \
    mqttMsgInstance.h \
    mqttMsgManager.h \
    clusterDataItem.h \
    clusterDataGroup.h \
    clusterRawPkt.h \
    sbc_mqtt_ciotc.h \
    mqttMsgParamsItem.h \
    hubConfigManager.h \
    hubConfigManagerWrapper.h

LIBS += -lssl -lcrypto -lpaho-mqtt3cs -ljwt -ljansson
QMAKE_CXXFLAGS += -DDEBUG


