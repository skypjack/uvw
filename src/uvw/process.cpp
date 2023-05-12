#ifdef UVW_AS_LIB
#    include "process.h"
#endif

#include <algorithm>
#include "config.h"

namespace uvw {

UVW_INLINE exit_event::exit_event(int64_t code, int sig) noexcept
    : status{code}, signal{sig} {}

UVW_INLINE void process_handle::exit_callback(uv_process_t *hndl, int64_t exit_status, int term_signal) {
    process_handle &process = *(static_cast<process_handle *>(hndl->data));
    process.publish(exit_event{exit_status, term_signal});
}

UVW_INLINE process_handle::process_handle(loop::token token, std::shared_ptr<loop> ref)
    : handle{token, std::move(ref)} {}

UVW_INLINE void process_handle::disable_stdio_inheritance() noexcept {
    uv_disable_stdio_inheritance();
}

UVW_INLINE bool process_handle::kill(int pid, int signum) noexcept {
    return (0 == uv_kill(pid, signum));
}

UVW_INLINE int process_handle::init() {
    // deferred initialization: libuv initializes process handles only when
    // uv_spawn is invoked and uvw stays true to the underlying library
    return 0;
}

UVW_INLINE int process_handle::spawn(const char *file, char **args, char **env) {
    uv_process_options_t po;

    po.exit_cb = &exit_callback;
    po.file = file;
    po.args = args;
    po.env = env;
    po.cwd = po_cwd.empty() ? nullptr : po_cwd.data();
    po.flags = static_cast<uv_process_flags>(po_flags);
    po.uid = po_uid;
    po.gid = po_gid;

    std::vector<uv_stdio_container_t> poStdio;
    poStdio.reserve(po_fd_stdio.size() + po_stream_stdio.size());
    poStdio.insert(poStdio.begin(), po_fd_stdio.cbegin(), po_fd_stdio.cend());
    poStdio.insert(poStdio.end(), po_stream_stdio.cbegin(), po_stream_stdio.cend());

    po.stdio_count = static_cast<decltype(po.stdio_count)>(poStdio.size());
    po.stdio = poStdio.data();

    // see init member function for more details
    leak_if(0);

    return uv_spawn(parent().raw(), raw(), &po);
}

UVW_INLINE int process_handle::kill(int signum) {
    return uv_process_kill(raw(), signum);
}

UVW_INLINE int process_handle::pid() noexcept {
    return raw()->pid;
}

UVW_INLINE process_handle &process_handle::cwd(const std::string &path) noexcept {
    po_cwd = path;
    return *this;
}

UVW_INLINE process_handle &process_handle::flags(process_flags flags) noexcept {
    po_flags = flags;
    return *this;
}

UVW_INLINE process_handle &process_handle::stdio(file_handle fd, stdio_flags flags) {
    auto fgs = static_cast<uv_stdio_flags>(flags);

    auto actual = uvw::file_handle{fd};

    auto it = std::find_if(po_fd_stdio.begin(), po_fd_stdio.end(), [actual](auto &&container) {
        return static_cast<const uvw::details::uv_type_wrapper<int>>(container.data.fd) == static_cast<const uvw::details::uv_type_wrapper<int>>(actual);
    });

    if(it == po_fd_stdio.cend()) {
        uv_stdio_container_t container;
        container.flags = fgs;
        container.data.fd = actual;
        po_fd_stdio.push_back(std::move(container));
    } else {
        it->flags = fgs;
        it->data.fd = actual;
    }

    return *this;
}

UVW_INLINE process_handle &process_handle::uid(uid_type id) {
    po_uid = id;
    return *this;
}

UVW_INLINE process_handle &process_handle::gid(gid_type id) {
    po_gid = id;
    return *this;
}

} // namespace uvw
