#include "NetworkAcceptor.hpp"

#define CMP "NetworkAcceptor"

using namespace Scissio;

Network::Acceptor::Acceptor(EventListener& listener) : listener(listener) {
}

void Network::Acceptor::eventPacket(const StreamPtr& stream, Packet packet) {
    const auto packetId = packet.id;

    try {
        listener.eventPacket(stream, std::move(packet));
    } catch (std::exception& e) {
        Log::e(CMP, "Failed to accept packet id: {}", packetId);
        backtrace(e);
    }
}

void Network::Acceptor::eventConnect(const StreamPtr& stream) {
    try {
        listener.eventConnect(stream);
    } catch (std::exception& e) {
        Log::e(CMP, "Failed to connect stream");
        backtrace(e);
    }
}

void Network::Acceptor::eventDisconnect(const StreamPtr& stream) {
    try {
        listener.eventDisconnect(stream);
    } catch (std::exception& e) {
        Log::e(CMP, "Failed to disconnect stream");
        backtrace(e);
    }
}
