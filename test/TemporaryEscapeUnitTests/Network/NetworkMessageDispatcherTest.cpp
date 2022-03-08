#include "../Common.hpp"
#include <TemporaryEscape/Network/Packet.hpp>

#define TAG "[NetworkMessageDispatch]"

/*struct MessageA {
    int a{123};

    MSGPACK_DEFINE_ARRAY(a);
};

REGISTER_MESSAGE(MessageA);

struct MessageB {
    std::string b{"Hello"};

    MSGPACK_DEFINE_ARRAY(b);
};

REGISTER_MESSAGE(MessageB);

TEST_CASE("Dispatch message into the correct callback", TAG) {
    Network::MessageDispatcher<bool> dispatcher{};

    bool dispatched[2] = {false, false};

    dispatcher.add<MessageA>([&](bool val, MessageA message) {
        REQUIRE(val == true);
        REQUIRE(message.a == 123);

        dispatched[0] = true;
    });

    dispatcher.add<MessageB>([&](bool val, MessageB message) {
        REQUIRE(val == false);
        REQUIRE(message.b == "Hello");

        dispatched[1] = true;
    });

    Network::Packet packet;
    msgpack::pack(packet.data, MessageA{});
    packet.id = Network::getMessageId<MessageA>();

    dispatcher.dispatch(true, packet);
    REQUIRE(dispatched[0] == true);

    packet = Network::Packet{};
    msgpack::pack(packet.data, MessageB{});
    packet.id = Network::getMessageId<MessageB>();

    dispatcher.dispatch(false, packet);
    REQUIRE(dispatched[1] == true);
}*/
