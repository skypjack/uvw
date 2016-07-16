#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


class Shutdown final: public Request<Shutdown> {
    static void shutdownCallback(uv_shutdown_t *req, int status) {
        Shutdown &shutdown = *(static_cast<Shutdown*>(req->data));

        auto ptr = shutdown.shared_from_this();
        (void)ptr;

        shutdown.reset();

        if(status) {
            shutdown.publish(ErrorEvent{status});
        } else {
            shutdown.publish(ShutdownEvent{});
        }
    }

    explicit Shutdown(std::shared_ptr<Loop> ref)
        : Request{RequestType<uv_shutdown_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<Shutdown> create(Args&&... args) {
        return std::shared_ptr<Shutdown>{new Shutdown{std::forward<Args>(args)...}};
    }

    template<typename T>
    void shutdown(Resource<T> &res) noexcept {
        if(0 == invoke(&uv_shutdown, get<uv_shutdown_t>(), res.template get<uv_stream_t>(), &shutdownCallback)) {
            leak();
        }
    }
};


}
