/*
 * =====================================================================================
 *
 *       Filename:  cluster.h
 *
 *    Description:  Instrument Cluster application
 *
 *        Version:  1.0
 *        Created:  20.03.17 20:03:13
 *       Revision:  02.2020
 *       Compiler:  gcc
 *
 *   Organization:  GlobalLogic
 *
 * =====================================================================================
 */
#pragma once

#include <netinet/in.h>
#include <string>

struct cluster_config {
    char scs_address[INET_ADDRSTRLEN];
    char scs_mask[INET_ADDRSTRLEN];
    int scs_port;
    char vws_address[INET_ADDRSTRLEN];
    int vws_control_messages_port;
    int vws_register_port;
    int vws_heartbeat_port;
    char bind_address[INET_ADDRSTRLEN];
    int port;
    char hub_address[INET_ADDRSTRLEN];
    int hub_port;

    int id; // temporarly. Will be move to handshake between VWS<->cluster
};

typedef void (*notifier_cb)(void);

int cluster_init(struct cluster_config* c);
int cluster_run();
int cluster_register_notifier(notifier_cb callback);
