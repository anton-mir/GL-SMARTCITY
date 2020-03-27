#include "MsgReceiver.hpp"
#include "logger.h"

MsgReceiver::MsgReceiver(boost::asio::io_service& io_service, const char* addr, int port_num)
    : socket_(io_service)
    , address(boost::asio::ip::address_v4::from_string(addr))
    , port_number(port_num)
{
    error_code error;
    socket_.open(udp::v4(), error);

    if (!error) {
        socket_.set_option(udp::socket::reuse_address(true));
        socket_.bind(udp::endpoint(address, port_number));
        start_receive();
    }
}

void MsgReceiver::start_receive()
{
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&MsgReceiver::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void MsgReceiver::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (bytes_transferred <= 0 && !error) {
        log_warning("Received Null-size message.");
        start_receive();
        return;
    }
    log_info("Received message with len = %d.", bytes_transferred);

    if (NULL != m_callback) {
        m_callback(recv_buffer_.data());
    }

    start_receive();
}

void MsgReceiver::stop()
{
    error_code error;
    socket_.shutdown(udp::socket::shutdown_send, error);
    if (error) {
        log_error(error.message().c_str());
    }

    socket_.close(error);
    if (error) {
        log_error(error.message().c_str());
    }
}

void MsgReceiver::flushInternalBuffer()
{
    recv_buffer_.fill(0);
}
