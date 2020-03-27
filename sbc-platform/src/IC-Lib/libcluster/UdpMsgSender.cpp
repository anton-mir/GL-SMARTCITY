#include "UdpMsgSender.hpp"
#include "logger.h"

using namespace boost::system;
using namespace boost::asio;
using boost::asio::ip::udp;

UdpMsgSender::UdpMsgSender(const char* addr, int port_num)
    : address(boost::asio::ip::address_v4::from_string(addr))
    , port_number(port_num)
{
}

UdpMsgSender::~UdpMsgSender()
{
}

bool UdpMsgSender::send(const std::string& message) const
{
    io_service io_service;
    ip::udp::socket socket(io_service);
    error_code error;

    // Create the remote endpoint using the destination ip address and
    // the target port number.  This is not a broadcast
    auto remote = ip::udp::endpoint(address, static_cast<unsigned short>(port_number));
    log_debug("Send to %s:%d.", remote.address().to_string().c_str(), remote.port());

    try {

        // Open the socket, socket's destructor will
        // automatically close it.
        socket.open(boost::asio::ip::udp::v4());

        // And send the string... (synchronous / blocking)
        socket.send_to(buffer(message), remote, 0, error);
        if (error) {
            log_error(error.message().c_str());
        }

    } catch (const boost::system::system_error& ex) {
        // Exception thrown!
        log_error("Udp sender error: %s", ex.what());
        return false;
    }

    return true;
}
