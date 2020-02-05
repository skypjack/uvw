#include "pipe.h"

namespace uvw {

    bool PipeHandle::init() {
        return initialize(&uv_pipe_init, ipc);
    }

    void PipeHandle::open(FileHandle file) {
        invoke(&uv_pipe_open, get(), file);
    }

    void PipeHandle::bind(std::string name) {
        invoke(&uv_pipe_bind, get(), name.data());
    }

    void PipeHandle::connect(std::string name) {
        auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
            ptr->publish(event);
        };

        auto connect = loop().resource<details::ConnectReq>();
        connect->once<ErrorEvent>(listener);
        connect->once<ConnectEvent>(listener);
        connect->connect(&uv_pipe_connect, get(), name.data());
    }

    std::string PipeHandle::sock() const noexcept {
        return details::tryRead(&uv_pipe_getsockname, get());
    }

    std::string PipeHandle::peer() const noexcept {
        return details::tryRead(&uv_pipe_getpeername, get());
    }

    void PipeHandle::pending(int count) noexcept {
        uv_pipe_pending_instances(get(), count);
    }

    int PipeHandle::pending() noexcept {
        return uv_pipe_pending_count(get());
    }

    HandleType PipeHandle::receive() noexcept {
        HandleCategory category = uv_pipe_pending_type(get());
        return Utilities::guessHandle(category);
    }

    bool PipeHandle::chmod(Flags<Chmod> flags) noexcept {
        return (0 == uv_pipe_chmod(get(), flags));
    }
}
