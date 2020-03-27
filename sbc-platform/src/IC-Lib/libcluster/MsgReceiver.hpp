#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace boost::system;
using namespace boost::asio;
using boost::asio::ip::udp;

class MsgReceiver {
private:
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    const boost::asio::ip::address address;
    int port_number;
    boost::array<char, 1024> recv_buffer_;
    void (*m_callback)(const char* buffer);

    void start_receive();
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);

public:
    void setCallback(void (*Callback)(const char* buffer)) { m_callback = Callback; }
    MsgReceiver(boost::asio::io_service& io_service, const char* addr, int port_num);
    void stop();
    void flushInternalBuffer();
};
