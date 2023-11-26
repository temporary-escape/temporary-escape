#pragma once

#include <msgpack.hpp>

#define MSGPACK_FRIEND(T)                                                                                              \
    friend struct msgpack::MSGPACK_DEFAULT_API_NS::adaptor::convert<T>;                                                \
    friend struct msgpack::MSGPACK_DEFAULT_API_NS::adaptor::pack<T>;
