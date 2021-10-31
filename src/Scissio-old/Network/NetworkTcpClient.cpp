#include "NetworkTcpClient.hpp"

using namespace Scissio;

Network::TcpClient::TcpClient(const std::string& address, int port) {
    connect<TcpConnector>(address, port);
}
