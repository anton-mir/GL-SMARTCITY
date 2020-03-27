/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  Instrument Cluster application
 *
 *        Version:  1.0
 *        Created:  14.03.17 20:29:49
 *       Revision:  02.2020
 *       Compiler:  gcc
 *
  *   Organization:  GlobalLogic
 *
 * =====================================================================================
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <thread>

#include "libcluster/cluster.h"
#include "libcluster/json.hpp"
#include "libcluster/logger.h"

#define DEFAULT_CLUSTER_CONFIG "config.json"

using nlohmann::json;

void status_event()
{
    log_info("STATUS EVENT");
}

int parse_config(const char* filename, struct cluster_config* config)
{
    FILE* fp;
    struct stat filestatus = {};
    int file_size;
    char* file_contents;
    json cfg_root = nullptr;

    if (stat(filename, &filestatus) != 0) {
        log_error("File %s not found", filename);
        return -ENOENT;
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if (file_contents == nullptr) {
        log_error("Memory error: unable to allocate %d bytes", file_size);
        return -ENOMEM;
    }

    fp = fopen(filename, "rt");
    if (fp == nullptr) {
        log_error("Unable to open %s", filename);
        fclose(fp);
        free(file_contents);
        return -EACCES;
    }
    if (fread(file_contents, file_size, 1, fp) != 1) {
        log_error("Unable t read content of %s", filename);
        fclose(fp);
        free(file_contents);
        return -EIO;
    }
    fclose(fp);

    cfg_root = json::parse(file_contents);
    if (cfg_root.is_null()) {
        log_error("Failed to parse the contents of the config file %s", filename);
        return 1;
    }

    log_info("Here is what has been found in the config file:  %s ", cfg_root.dump().c_str());

    try {
        cfg_root.at("id").get_to(config->id);
        std::strcpy(config->bind_address, cfg_root.at("bind_address").get<std::string>().c_str());
        cfg_root.at("port").get_to(config->port);
        std::strcpy(config->scs_address, cfg_root.at("scs_address").get<std::string>().c_str());
        std::strcpy(config->scs_mask, cfg_root.at("scs_mask").get<std::string>().c_str());
        cfg_root.at("scs_port").get_to(config->scs_port);
        std::strcpy(config->vws_address, cfg_root.at("vws_address").get<std::string>().c_str());
        cfg_root.at("vws_control_messages_port").get_to(config->vws_control_messages_port);
        cfg_root.at("vws_register_port").get_to(config->vws_register_port);
        cfg_root.at("vws_heartbeat_port").get_to(config->vws_heartbeat_port);
        std::strcpy(config->hub_address, cfg_root.at("hub_address").get<std::string>().c_str());
        cfg_root.at("hub_port").get_to(config->hub_port);
    } catch (std::exception& ex) {
        log_error("Failed to populate cluster config with the data from the config file %s.", ex.what());
        return -1;
    }

    log_info("Cluster config: 'id'=[%d]; 'bind_address'=[%s]; 'port'=[%d]; 'scs_address'=[%s]; 'scs_mask'=[%s]; 'scs_port'=[%d];'hub_address'=[%s]; 'hub_port'=[%d]",
            config->id, config->bind_address, config->port, config->scs_address, config->scs_mask, config->scs_port, config->hub_address, config->hub_port);

    free(file_contents);
    return 0;
}

void usage(char* progname)
{
    log_info("Usage: \n"
             "\n"
             "  %s [-h] [config file]\n"
             "\n"
             "",
        progname);
    exit(1);
}

int main(int argc, char* argv[])
{
    int err;
    struct cluster_config config = {};
    if ((argc > 1) && !strcmp(argv[1], "-h")) {
        usage(argv[0]);
    }
    err = parse_config(((argc > 1) ? argv[1] : DEFAULT_CLUSTER_CONFIG), &config);
    if (err) {
        return err;
    }
    err = cluster_init(&config);
    if (err) {
        return err;
    }

    cluster_register_notifier(status_event);
    cluster_run();
    while (1)
    {
        using namespace std::chrono;
        std::this_thread::sleep_for(nanoseconds(1U));
    }
}
