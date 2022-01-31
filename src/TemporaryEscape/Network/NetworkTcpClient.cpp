#include "NetworkTcpClient.hpp"

#define CMP "NetworkTcpClient"

using namespace Engine;

Network::TcpClient::TcpClient(EventListener& listener, const std::string& address, int port) : Client(listener) {
    acceptor = Client::connect<TcpConnector>(address, port);
}

Network::TcpClient::~TcpClient() {
    acceptor.reset();
}
