#pragma once


#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "event.hpp"
#include "handle.hpp"
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


namespace details {


enum class UVProcessFlags: std::underlying_type_t<uv_process_flags> {
    PROCESS_SETUID = UV_PROCESS_SETUID,
    PROCESS_SETGID = UV_PROCESS_SETGID,
    PROCESS_WINDOWS_VERBATIM_ARGUMENTS = UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS,
    PROCESS_DETACHED = UV_PROCESS_DETACHED,
    PROCESS_WINDOWS_HIDE = UV_PROCESS_WINDOWS_HIDE
};


enum class UVStdIOFlags: std::underlying_type_t<uv_stdio_flags> {
    IGNORE = UV_IGNORE,
    CREATE_PIPE = UV_CREATE_PIPE,
    INHERIT_FD = UV_INHERIT_FD,
    INHERIT_STREAM = UV_INHERIT_STREAM,
    READABLE_PIPE = UV_READABLE_PIPE,
    WRITABLE_PIPE = UV_WRITABLE_PIPE
};


}


struct ExitEvent: Event<ExitEvent> { };


class ProcessHandle final: public Handle<ProcessHandle, uv_process_t> {
    static void exitCallback(uv_process_t *handle, int64_t exitStatus, int termSignal) {
        ProcessHandle &process = *(static_cast<ProcessHandle*>(handle->data));
        process.publish(ExitEvent{});
    }

    using Handle::Handle;

public:
    using Process = details::UVProcessFlags;
    using StdIO = details::UVStdIOFlags;

    template<typename... Args>
    static std::shared_ptr<ProcessHandle> create(Args&&... args) {
        return std::shared_ptr<ProcessHandle>{new ProcessHandle{std::forward<Args>(args)...}};
    }

    static void disableStdIOInheritance() noexcept {
        uv_disable_stdio_inheritance();
    }

    static bool kill(int pid, int signum) noexcept {
        return (0 == uv_kill(pid, signum));
    }

    bool init() const noexcept { return true; }

    void spawn(const char *file, char **args, char **env = nullptr) {
        uv_process_options_t po;

        po.exit_cb = &exitCallback;

        po.file = file;
        po.args = args;
        po.env = env;

        po.cwd = poCwd.data();
        po.flags = poFlags;
        po.stdio_count = poStdio.size();
        po.stdio = poStdio.data();
        po.uid = poUid;
        po.gid = poGid;

        invoke(&uv_spawn, parent(), get<uv_process_t>(), &po);
    }

    void kill(int signum) {
        invoke(&uv_process_kill, get<uv_process_t>(), signum);
    }

    int pid() noexcept {
        return get<uv_process_t>()->pid;
    }

    ProcessHandle& cwd(std::string &path) noexcept {
        poCwd = path;
        return *this;
    }

    ProcessHandle& flags(Flags<Process> flags) noexcept {
        poFlags = flags;
        return *this;
    }

    template<typename T, typename U>
    ProcessHandle& stdio(Flags<StdIO> flags, StreamHandle<T, U> &stream) {
        uv_stdio_container_t container;
        Flags<StdIO>::Type fgs = flags;
        container.flags = static_cast<uv_stdio_flags>(fgs);
        container.data.stream = stream.template get<uv_stream_t>();
        poStdio.push_back(std::move(container));
        return *this;
    }

    template<typename T>
    ProcessHandle& stdio(Flags<StdIO> flags, FileHandle fd) {
        uv_stdio_container_t container;
        Flags<StdIO>::Type fgs = flags;
        container.flags = static_cast<uv_stdio_flags>(fgs);
        container.data.fd = fd;
        poStdio.push_back(std::move(container));
        return *this;
    }

    ProcessHandle& uid(Uid id) {
        poUid = id;
        return *this;
    }

    ProcessHandle& gid(Gid id) {
        poGid = id;
        return *this;
    }

private:
    std::string poCwd;
    Flags<Process> poFlags;
    std::vector<uv_stdio_container_t> poStdio;
    Uid poUid;
    Gid poGid;
};


}
