#pragma once


#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


class Pipe final: public Stream<Pipe, uv_pipe_t> {
    using Stream::Stream;

public:
    enum class Pending: std::underlying_type_t<uv_handle_type> {
        UNKNOWN = UV_UNKNOWN_HANDLE,
        PIPE = UV_NAMED_PIPE,
        TCP = UV_TCP,
        UDP = UV_UDP
    };

    template<typename... Args>
    static std::shared_ptr<Pipe> create(Args&&... args) {
        return std::shared_ptr<Pipe>{new Pipe{std::forward<Args>(args)...}};
    }

    bool init(bool ipc = false) { return initialize<uv_pipe_t>(&uv_pipe_init, ipc); }

    void open(FileHandle file) {
        invoke(&uv_pipe_open, get<uv_pipe_t>(), file);
    }

    void bind(std::string name) {
        invoke(&uv_pipe_bind, get<uv_pipe_t>(), name.data());
    }

    void connect(std::string name) {
        auto listener = [ptr = shared_from_this()](const auto &event, details::Connect &) {
            ptr->publish(event);
        };

        auto connect = loop().resource<details::Connect>();
        connect->once<ErrorEvent>(listener);
        connect->once<ConnectEvent>(listener);
        connect->connect(&uv_pipe_connect, get<uv_pipe_t>(), name.data());
    }

    std::string sock() const noexcept { return details::path(&uv_pipe_getsockname, get<uv_pipe_t>()); }
    std::string peer() const noexcept { return details::path(&uv_pipe_getpeername, get<uv_pipe_t>()); }

    void pending(int count) noexcept { uv_pipe_pending_instances(get<uv_pipe_t>(), count); }
    int pending() noexcept { return uv_pipe_pending_count(get<uv_pipe_t>()); }

    Pending receive() noexcept {
        auto type = uv_pipe_pending_type(get<uv_pipe_t>());

        switch(type) {
        case UV_NAMED_PIPE:
            return Pending::PIPE;
        case UV_TCP:
            return Pending::TCP;
        case UV_UDP:
            return Pending::UDP;
        default:
            return Pending::UNKNOWN;
        }
    }
};


}
