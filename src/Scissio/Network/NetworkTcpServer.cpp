#include "NetworkTcpServer.hpp"

#define CMP "NetworkTcpServer"

using namespace Scissio;

Network::TcpServer::TcpServer(EventListener& listener, const int port) : Server(listener) {
    acceptor = Server::bind<TcpAcceptor>(port);
}

Network::TcpServer::~TcpServer() {
    acceptor->close();
}
