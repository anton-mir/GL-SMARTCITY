#include "ParkingController.hpp"
#include "json.hpp"
#include "logger.h"
#include "restclient-cpp/restclient.h"

using json = nlohmann::json;

ParkingController::ParkingController(std::string parkingId)
    : m_parkingId(parkingId)
{
}

bool ParkingController::getParkingData()
{
    RestClient::Response res_location = RestClient::get(std::string(REQUEST_PARKING_INFO(m_parkingId)));
    if (res_location.code != 200)
        return false;

    if (!parseLocationResponse(res_location.body))
        return false;

    log_info("Successfully parsed location data of the parking");

    RestClient::Response res_spots = RestClient::get(std::string(REQUEST_PARKING_FREE_SPOTS(m_parkingId)));
    if (res_spots.code != 200)
        return false;

    m_count = parseParkingFreeSpotsResponse(res_spots.body);
    if (-1 == m_count)
        return false;

    log_info("Successfully parsed data about available parking spots");

    return true;
}

bool ParkingController::parseLocationResponse(const std::string& response)
{
    json value = json::parse(response.c_str());
    if (value.is_null()) {
        log_error("Bad response from Parking Server");
        return false;
    }
    try {
        value.at("lat_gps").get_to(m_lat);
        value.at("lon_gps").get_to(m_lon);
    } catch (std::exception& ex) {
        log_error("Not found objenct in JSON: %s", ex.what());
        return false;
    }

    return true;
}

int ParkingController::parseParkingFreeSpotsResponse(const std::string& response)
{
    json array = json::parse(response.c_str());
    if (array.is_null()) {
        log_error("Bad response from Parking Server");
        return -1;
    }

    std::string occupied_tag = "occupied";
    std::string schema_tag = "schemaPolygon";

    int count = 0;

    for (auto element : array) {
        if (!element[schema_tag.c_str()].is_null()) {
            bool occupied = true;
            element[occupied_tag.c_str()].get_to(occupied);
            if (!occupied)
                count++;
        }
    }

    return count;
}

double ParkingController::lat() const
{
    return m_lat;
}

double ParkingController::lon() const
{
    return m_lon;
}

unsigned int ParkingController::count() const
{
    return static_cast<unsigned int>(m_count);
}

void ParkingController::info() const
{
    log_info("%s :: parking area: id = %s Latitude  = %f Longitude = %f FreePlaces = %d", __func__, m_parkingId.c_str(), m_lat, m_lon, m_count);
}
