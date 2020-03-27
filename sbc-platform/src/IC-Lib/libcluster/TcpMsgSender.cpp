#include <boost/array.hpp>

#include "TcpMsgSender.hpp"
#include "logger.h"

using boost::asio::ip::tcp;

TcpMsgSender::TcpMsgSender(const char* addr, int port_num)
    : address(boost::asio::ip::address_v4::from_string(addr))
    , port_number(port_num)
{
}

TcpMsgSender::~TcpMsgSender()
{
}

std::string TcpMsgSender::sendWithResponse(const char* data) const
{
    boost::asio::io_service io;
    std::string result;

    tcp::resolver resolver(io);
    tcp::resolver::query query(address.to_string(), std::to_string(port_number));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io);
    boost::asio::connect(socket, endpoint_iterator);

    std::string msg = data;

    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(msg), ignored_error);

    boost::array<char, 128> buffer;
    boost::system::error_code error;
    size_t len = socket.read_some(boost::asio::buffer(buffer), error);

    log_debug("read [%d] %s", len, buffer.data());

    if (len > 0) {
        result = buffer.data();
    }

    if (error) {
        throw boost::system::system_error(error); // Some other error.
    }

    socket.close();
    return result;
}

void TcpMsgSender::send(const char* data) const
{
    boost::asio::io_service io;
    std::string result;

    tcp::resolver resolver(io);
    tcp::resolver::query query(address.to_string(), std::to_string(port_number));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io);
    boost::asio::connect(socket, endpoint_iterator);

    std::string msg = data;

    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(msg), ignored_error);

    socket.close();
}
