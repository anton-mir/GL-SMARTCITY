#ifndef PARKINGCONTROLLER_HPP_
#define PARKINGCONTROLLER_HPP_

#include <string>

#define REQUEST_PARKING_INFO(id) "https://pl.pub.globallogic.com/parking/api/v1/parkings/" + id
#define REQUEST_PARKING_FREE_SPOTS(id) "https://pl.pub.globallogic.com/parking/api/v1/parkings/" + id + "/spots"

class ParkingController {
public:
    ParkingController(const std::string parkingId);
    bool getParkingData();
    unsigned int count() const;
    double lon() const;
    double lat() const;
    void info() const;

private:
    bool parseLocationResponse(const std::string& response);
    int parseParkingFreeSpotsResponse(const std::string& response);

    double m_lon, m_lat = 0;
    int m_count = 0;
    std::string m_parkingId;
};

#endif // PARKINGCONTROLLER_HPP_
