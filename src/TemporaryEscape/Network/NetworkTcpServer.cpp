#include "NetworkTcpServer.hpp"

#define CMP "NetworkTcpServer"

using namespace Engine;

Network::TcpServer::TcpServer(EventListener& listener, const int port) : Server(listener) {
    acceptor = Server::bind<TcpAcceptor>(port);
}

Network::TcpServer::~TcpServer() {
    acceptor->close();
    acceptor.reset();
}
