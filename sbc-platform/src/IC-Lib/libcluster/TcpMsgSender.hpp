#ifndef TCP_MSG_SENDER_HPP_
#define TCP_MSG_SENDER_HPP_

#include <boost/asio.hpp>

class TcpMsgSender {
private:
    const boost::asio::ip::address address;
    int port_number;

public:
    TcpMsgSender(const char* addr, int port_num);
    virtual ~TcpMsgSender();

    std::string sendWithResponse(const char* data) const;
    void send(const char* data) const;
};

#endif //TCP_MSG_SENDER_HPP_
