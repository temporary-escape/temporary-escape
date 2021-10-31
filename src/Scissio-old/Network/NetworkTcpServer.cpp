#include "NetworkTcpServer.hpp"

using namespace Scissio;

Network::TcpServer::TcpServer(const int port) {
    bind<TcpAcceptor>(port);
}
