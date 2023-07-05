#pragma once

#include "message.hpp"
#include "peer.hpp"
#include <functional>

namespace Engine::Network {
class ENGINE_API Peer;

class ENGINE_API Dispatcher {
public:
    using PeerPtr = std::shared_ptr<Peer>;

    template <typename F> struct Traits;

    template <typename C, typename R, typename T> struct Traits<R (C::*)(const PeerPtr&, T) const> {
        using Ret = R;
        using Arg = T;
    };

    using Handler = std::function<void(const PeerPtr&, uint64_t, ObjectHandlePtr)>;
    using HandlerMap = std::unordered_map<uint64_t, Handler>;

    explicit Dispatcher(ErrorHandler& errorHandler);
    virtual ~Dispatcher() = default;

    /**
     * Registers a new handler that accepts some message type.
     * The handler function must accept `const std::shared_ptr<Network::Peer>&` as the first argument.
     * The second argument is the request message type. This must be unique. Only one handler per one message
     * can exist.
     * The function can return either a response message, indicating that some data should be sent back.
     * Or it can return void, indicating that nothing is returned back to the sender of the request.
     *
     * @tparam Fn The raw lambda function type. This will be auto deduced. No need to explicitly provide it.
     * @param fn The lambda function as the handler.
     */
    template <typename Fn> void addHandler(Fn fn) {
        using Req = typename Traits<decltype(&Fn::operator())>::Arg;
        using Res = typename Traits<decltype(&Fn::operator())>::Ret;
        const auto hash = Detail::MessageHelper<Req>::hash;
        HandlerFactory<Res, Req>::create(handlers, hash, std::forward<Fn>(fn));
    }

    /**
     * Registers a new handler that accepts some message type.
     * The handler function must accept `const std::shared_ptr<Network::Peer>&` as the first argument.
     * The second argument is the request message type. This must be unique. Only one handler per one message
     * can exist.
     * The function can return either a response message, indicating that some data should be sent back.
     * Or it can return void, indicating that nothing is returned back to the sender of the request.
     *
     * @tparam C The class that contains the handler.
     * @tparam R Return type of the handler. It can be either some message (response) or void.
     * @tparam T Message (request) type of the handler.
     * @param instance Pointer to the class instance that has the handler.
     * @param fn Pointer to the function.
     */
    template <typename C, typename R, typename T> void addHandler(C* instance, R (C::*fn)(const PeerPtr&, T)) {
        const auto hash = Detail::MessageHelper<T>::hash;
        HandlerFactory<R, T>::create(
            handlers, hash, [instance, fn](const PeerPtr& peer, T m) { (instance->*fn)(peer, std::move(m)); });
    }

    template <typename C, typename R, typename T>
    void addHandler(C* instance, R (C::*fn)(const PeerPtr&, T), const std::string_view& name) {
        const auto hash = Detail::getMessageHash(name);
        HandlerFactory<R, T>::create(
            handlers, hash, [instance, fn](const PeerPtr& peer, T m) { (instance->*fn)(peer, std::move(m)); });
    }

    /**
     * Dispatches a Msgpack object to some handler.
     *
     * @warning Do not call this method. This is an internal method only to be used by the Peer class internally.
     *
     * @param peer Shared pointer to the peer.
     * @param id Unique message ID that is used to route the message to the correct handler.
     * @param reqId Request ID of this message, indicating that the sender is invoking a request.
     * @param oh The raw Msgpack object handle that needs to be converted into the correct handler message type.
     */
    virtual void dispatch(const PeerPtr& peer, uint64_t id, uint64_t reqId, ObjectHandlePtr oh);

    /**
     * This function is executed every time there is some work to be done.
     * Such work can be a request message that needs to execute some handler function,
     * or some callback function from a request.
     * If you wish to use multiple threads, one for network I/O and one (or more) for handling the messages,
     * you can override this function and forward the function fn to any thread you wish to use.
     * This could also be used to synchronize with the main thread (rendering thread in a game, for example).
     *
     * @param fn The function that wraps the work needed to execute some handler or some callback request function.
     */
    virtual void postDispatch(std::function<void()> fn) = 0;

private:
    template <typename Res, typename Req> struct HandlerFactory {
        static void create(HandlerMap& handlers, const uint64_t hash, std::function<Res(const PeerPtr&, Req)> fn) {
            // Sanity check
            const auto check = handlers.find(hash);
            if (check != handlers.end()) {
                throw std::runtime_error("The type of this message has already been registered");
            }

            handlers[hash] = [fn = std::move(fn)](const PeerPtr& peer, const uint64_t reqId, ObjectHandlePtr oh) {
                Req req{};
                oh->get().via.array.ptr[1].convert(req);

                Res res = fn(peer, std::move(req));
                peer->send<Res>(res, reqId, true);
            };
        }
    };

    template <typename Req> struct HandlerFactory<void, Req> {
        static void create(HandlerMap& handlers, const uint64_t hash, std::function<void(const PeerPtr&, Req)> fn) {
            // Sanity check
            const auto check = handlers.find(hash);
            if (check != handlers.end()) {
                throw std::runtime_error("The type of this message has already been registered");
            }

            handlers[hash] = [fn = std::move(fn)](const PeerPtr& peer, const uint64_t reqId, ObjectHandlePtr oh) {
                (void)reqId;

                Req req{};
                oh->get().via.array.ptr[1].convert(req);

                fn(peer, std::move(req));
            };
        }
    };

    ErrorHandler& errorHandler;
    HandlerMap handlers;
};

template <> struct Dispatcher::HandlerFactory<void, RawMessage> {
    static void create(HandlerMap& handlers, const uint64_t hash, std::function<void(const PeerPtr&, RawMessage)> fn) {
        // Sanity check
        const auto check = handlers.find(hash);
        if (check != handlers.end()) {
            throw std::runtime_error("The type of this message has already been registered");
        }

        handlers[hash] = [fn = std::move(fn)](const PeerPtr& peer, const uint64_t reqId, ObjectHandlePtr oh) {
            (void)reqId;
            fn(peer, RawMessage{std::move(oh)});
        };
    }
};
} // namespace Engine::Network
