/*
 * =====================================================================================
 *
 *       Filename:  cluster.c
 *
 *    Description:  Instrument Cluster application
 *
 *        Version:  1.0
 *        Created:  20.03.17 19:45:45
 *       Revision:  02.2020
 *       Compiler:  gcc
 *
 *   Organization:  GlobalLogic
 *
 * =====================================================================================
 */

#include <cerrno>
#include <clocale>
#include <mutex>
#include <cstring>
#include <thread>

#include "MsgReceiver.hpp"
#include "ParkingController.hpp"
#include "TcpMsgSender.hpp"
#include "UdpMsgSender.hpp"
#include "cluster.h"
#include "json.hpp"
#include "logger.h"

using nlohmann::json;

#define SKIN_MAX_LENGTH 32

#define REGREQ_CMD_NAME "RegReq"
#define REGREQ_MAX_LENGTH (1000)

static io_service vwsReceiverService;
static std::shared_ptr<MsgReceiver> vwsReceiver = nullptr;
static std::shared_ptr<UdpMsgSender> vwsSender = nullptr;
static std::shared_ptr<UdpMsgSender> scsSender = nullptr;
static std::shared_ptr<UdpMsgSender> hubSender = nullptr;

struct cluster_ctx {
    struct cluster_config c = {};
    char skin[SKIN_MAX_LENGTH] = {};
};

static std::shared_ptr<cluster_ctx> ctx;

static struct cluster_state {
    double acceleration;
    double course;
    double latitude;
    double longitude;
    double speed;
    double rpm;
    int laneid;
    int gear;
    bool special;
} state;

static struct cluster_state_car {
    double acceleration;
    double course;
    double latitude;
    double longitude;
    double speed;
    double rpm;
    double temp;
    double humidity;
    double co2;
    double tvoc;
    int laneid;
    int gear;
    bool special;
} state_car;

static struct cluster_state_box {
    uint8_t device_id;
    uint8_t device_working_status;
    uint32_t device_message_counter;
    char device_type[19];
    char device_description[500];
    char message_date_time[24];
    double latitude;
    double longitude;
    double altitude;
    double temp_internal;
    double temp;
    double humidity;
    double co2;
    double tvoc;
    double pressure;
    double co;
    double co_temp;
    double co_hum;
    double co_err;
    double no2;
    double no2_temp;
    double no2_hum;
    double no2_err;
    double so2;
    double so2_temp;
    double so2_hum;
    double so2_err;
    double o3;
    double o3_temp;
    double o3_hum;
    double o3_err;
    double hcho;
    double pm2_5;
    double pm10;
} state_box;

static std::mutex state_mutex;

static void state_parse(const json& j)
{
    log_info("Packet parse started\n");

    json state_node = j["state"];
    json airc_device_info_node = j["airc_device_data"];

    if (state_node.is_null())
    {
        log_info("Packet state is NULL!\n");
        return;
    }

    log_info("cluster: parse message");
    std::lock_guard<std::mutex> lock(state_mutex);
    if(j["type"] == "Car") {
        try {
            state_node.at("acceleration").get_to(state.acceleration);
            state_node.at("course").get_to(state.course);
            state_node.at("latitude").get_to(state.latitude);
            state_node.at("longitude").get_to(state.longitude);
            state_node.at("speed").get_to(state.speed);
            state_node.at("rpm").get_to(state.rpm);
            state_node.at("special").get_to(state.special);
            state_node.at("gear").get_to(state.gear);
            state_node.at("laneid").get_to(state.laneid);
        } catch (std::exception &ex) {
            log_error("Problem with read package from eserver: %s", ex.what());
        }
    }
    else if(j["type"] == "AirC_Car") {
        try {
            state_node.at("acceleration").get_to(state_car.acceleration);
            state_node.at("course").get_to(state_car.course);
            state_node.at("latitude").get_to(state_car.latitude);
            state_node.at("longitude").get_to(state_car.longitude);
            state_node.at("speed").get_to(state_car.speed);
            state_node.at("rpm").get_to(state_car.rpm);
            state_node.at("special").get_to(state_car.special);
            state_node.at("gear").get_to(state_car.gear);
            state_node.at("laneid").get_to(state_car.laneid);
            state_node.at("temp").get_to(state_car.temp);
            state_node.at("humidity").get_to(state_car.humidity);
            state_node.at("co2").get_to(state_car.co2);
            state_node.at("tvoc").get_to(state_car.tvoc);
        } catch (std::exception &ex) {
            log_error("Problem with read package from eserver: %s", ex.what());
        }
    }
    else if(j["type"] == "AirC_Box") {
        try {
            log_info("Parsing AirC_Box packet...");
            std::string airc_type, airc_description, airc_date_time;

            // AirC device information
            airc_device_info_node.at("id").get_to(state_box.device_id);
            airc_device_info_node.at("status").get_to(state_box.device_working_status);
            airc_device_info_node.at("message_counter").get_to(state_box.device_message_counter);

            airc_device_info_node.at("type").get_to(airc_type);
            strncpy(state_box.device_type, airc_type.c_str(), 19);
            airc_device_info_node.at("description").get_to(airc_description);
            strncpy(state_box.device_description, airc_description.c_str(), 500);
            airc_device_info_node.at("date_time").get_to(airc_date_time);
            strncpy(state_box.message_date_time, airc_date_time.c_str(), 24);
            airc_device_info_node.at("latitude").get_to(state_box.latitude);
            airc_device_info_node.at("longitude").get_to(state_box.longitude);
            airc_device_info_node.at("altitude").get_to(state_box.altitude);

            // Sensor data
            state_node.at("temp_internal").get_to(state_box.temp_internal);
            state_node.at("temp").get_to(state_box.temp);
            state_node.at("humidity").get_to(state_box.humidity);
            state_node.at("co2").get_to(state_box.co2);
            state_node.at("tvoc").get_to(state_box.tvoc);
            state_node.at("pressure").get_to(state_box.pressure);
            state_node.at("co").get_to(state_box.co);
            state_node.at("co_temp").get_to(state_box.co_temp);
            state_node.at("co_hum").get_to(state_box.co_hum);
            state_node.at("co_err").get_to(state_box.co_err);
            state_node.at("no2").get_to(state_box.no2);
            state_node.at("no2_temp").get_to(state_box.no2_temp);
            state_node.at("no2_hum").get_to(state_box.no2_hum);
            state_node.at("no2_err").get_to(state_box.no2_err);
            state_node.at("so2").get_to(state_box.so2);
            state_node.at("so2_temp").get_to(state_box.so2_temp);
            state_node.at("so2_hum").get_to(state_box.so2_hum);
            state_node.at("so2_err").get_to(state_box.so2_err);
            state_node.at("o3").get_to(state_box.o3);
            state_node.at("o3_temp").get_to(state_box.o3_temp);
            state_node.at("o3_hum").get_to(state_box.o3_hum);
            state_node.at("o3_err").get_to(state_box.o3_err);
            state_node.at("hcho").get_to(state_box.hcho);
            state_node.at("pm2_5").get_to(state_box.pm2_5);
            state_node.at("pm10").get_to(state_box.pm10);

            log_info("== Airc_Box message start ==");
            log_info(j.dump().c_str());
            log_info("== Airc_Box message end ==");
        } catch (std::exception &ex) {
            log_error("AirC_Box: read error [%s]", ex.what());
        }
    }
}

static notifier_cb event_update;

static int filter_pkt(const char* pkt)
{
    json value = json::parse(pkt);

    if (value.is_null())
    {
        log_error("Unable to parse vws paket %s <<END", pkt);
        return -EINVAL;
    }

    std::strcpy(ctx->skin, value.at("skin").get<std::string>().c_str());

    log_info("Packet parse started\n");

    state_parse(value);

    return 0;
}

void onVwsMessageReceived(const char* buffer)
{
    int err;

    if (!buffer) {
        return;
    }

    err = filter_pkt(buffer);

    if (!err) {

        log_debug("Sending message to SCS...");
        scsSender->send(buffer);
        log_debug("Sending message to CloudHUB...");
        hubSender->send(buffer);

        if (event_update) {
            event_update();
        }
    }
    vwsReceiver->flushInternalBuffer();
}

int cluster_init(struct cluster_config* c)
{
    setlocale(LC_NUMERIC, "en_US.utf8");
    initLogger();

    int err;

    ctx = std::make_shared<cluster_ctx>();
    if (!ctx) {
        err = -ENOMEM;
        log_error("No memory");
        return err;
    }
    memcpy(&ctx->c, c, sizeof(struct cluster_config));

    /* Send register request to VWS */
    TcpMsgSender vwsRegisterSender(c->vws_address, c->vws_register_port);

    char data[REGREQ_MAX_LENGTH];
    std::snprintf(data, REGREQ_MAX_LENGTH, R"({ "cmd": "%s", "id": %d, "ip": "%s", "port": %d })",
        REGREQ_CMD_NAME, c->id, c->bind_address, c->port);

    vwsRegisterSender.send(data);

    vwsSender = std::make_shared<UdpMsgSender>(c->vws_address, c->vws_control_messages_port);
    if (nullptr == vwsSender) {
        err = -ENOMEM;
        log_error("vwsSender pointer is null.");
        return err;
    }
    scsSender = std::make_shared<UdpMsgSender>(c->scs_address, c->scs_port);
    if (nullptr == scsSender) {
        err = -ENOMEM;
        log_error("scsSender pointer is null.");
        return err;
    }
    hubSender = std::make_shared<UdpMsgSender>(c->hub_address, c->hub_port);
    if (nullptr == hubSender) {
        err = -ENOMEM;
        log_error("hubSender pointer is null.");
        return err;
    }

    return 0;
}

void subscribeVwsReceiver(void (*Callback)(const char*))
{
    try {
        vwsReceiver = std::make_shared<MsgReceiver>(vwsReceiverService, ctx->c.bind_address, ctx->c.port);
        vwsReceiver->setCallback(Callback);
        vwsReceiverService.run();
    }

    catch (std::exception& e) {
        log_error(e.what());
        throw;
    }
}

int cluster_run()
{
    std::thread([] {
        subscribeVwsReceiver(onVwsMessageReceived);
    }).detach();

    return 0;
}

int cluster_register_notifier(notifier_cb callback)
{
    event_update = callback;
    return 0;
}
