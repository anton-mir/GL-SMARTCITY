#ifndef UDP_MSG_SENDER_HPP_
#define UDP_MSG_SENDER_HPP_

#include <boost/asio.hpp>

class UdpMsgSender {
private:
    const boost::asio::ip::address address;
    int port_number;

public:
    UdpMsgSender(const char* addr, int port_num);
    ~UdpMsgSender();

    bool send(const std::string& message) const;
};

#endif //UDP_MSG_SENDER_HPP_
