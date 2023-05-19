#ifdef UVW_AS_LIB
#    include "util.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE passwd_info::passwd_info(std::shared_ptr<uv_passwd_t> pwd)
    : value{pwd} {}

UVW_INLINE std::string passwd_info::username() const noexcept {
    return ((value && value->username) ? value->username : "");
}

UVW_INLINE decltype(uv_passwd_t::uid) passwd_info::uid() const noexcept {
    return (value ? value->uid : decltype(uv_passwd_t::uid){});
}

UVW_INLINE decltype(uv_passwd_t::gid) passwd_info::gid() const noexcept {
    return (value ? value->gid : decltype(uv_passwd_t::gid){});
}

UVW_INLINE std::string passwd_info::shell() const noexcept {
    return ((value && value->shell) ? value->shell : "");
}

UVW_INLINE std::string passwd_info::homedir() const noexcept {
    return ((value && value->homedir) ? value->homedir : "");
}

UVW_INLINE passwd_info::operator bool() const noexcept {
    return static_cast<bool>(value);
}

UVW_INLINE uts_name::uts_name(std::shared_ptr<uv_utsname_t> init)
    : uname{init} {}

UVW_INLINE std::string uts_name::sysname() const noexcept {
    return uname ? uname->sysname : "";
}

UVW_INLINE std::string uts_name::release() const noexcept {
    return uname ? uname->release : "";
}

UVW_INLINE std::string uts_name::version() const noexcept {
    return uname ? uname->version : "";
}

UVW_INLINE std::string uts_name::machine() const noexcept {
    return uname ? uname->machine : "";
}

namespace details {

UVW_INLINE void common_alloc_callback(uv_handle_t *, std::size_t suggested, uv_buf_t *buf) {
    auto size = static_cast<unsigned int>(suggested);
    *buf = uv_buf_init(new char[size], size);
}

UVW_INLINE sockaddr ip_addr(const char *addr, unsigned int port) {
    if(sockaddr_in addr_in; uv_ip4_addr(addr, port, &addr_in) == 0) {
        return reinterpret_cast<const sockaddr &>(addr_in);
    } else if(sockaddr_in6 addr_in6; uv_ip6_addr(addr, port, &addr_in6) == 0) {
        return reinterpret_cast<const sockaddr &>(addr_in6);
    }

    return {};
}

UVW_INLINE socket_address sock_addr(const sockaddr_in &addr) {
    if(char name[details::DEFAULT_SIZE]; uv_ip4_name(&addr, name, details::DEFAULT_SIZE) == 0) {
        return socket_address{std::string{name}, ntohs(addr.sin_port)};
    }

    return socket_address{};
}

UVW_INLINE socket_address sock_addr(const sockaddr_in6 &addr) {
    if(char name[details::DEFAULT_SIZE]; uv_ip6_name(&addr, name, details::DEFAULT_SIZE) == 0) {
        return socket_address{std::string{name}, ntohs(addr.sin6_port)};
    }

    return socket_address{};
}

UVW_INLINE socket_address sock_addr(const sockaddr &addr) {
    if(addr.sa_family == AF_INET) {
        return sock_addr(reinterpret_cast<const sockaddr_in &>(addr));
    } else if(addr.sa_family == AF_INET6) {
        return sock_addr(reinterpret_cast<const sockaddr_in6 &>(addr));
    }

    return socket_address{};
}

UVW_INLINE socket_address sock_addr(const sockaddr_storage &storage) {
    if(storage.ss_family == AF_INET) {
        return sock_addr(reinterpret_cast<const sockaddr_in &>(storage));
    } else if(storage.ss_family == AF_INET6) {
        return sock_addr(reinterpret_cast<const sockaddr_in6 &>(storage));
    }

    return socket_address{};
}

} // namespace details

UVW_INLINE pid_type utilities::os::pid() noexcept {
    return uv_os_getpid();
}

UVW_INLINE pid_type utilities::os::ppid() noexcept {
    return uv_os_getppid();
}

UVW_INLINE std::string utilities::os::homedir() noexcept {
    return details::try_read(&uv_os_homedir);
}

UVW_INLINE std::string utilities::os::tmpdir() noexcept {
    return details::try_read(&uv_os_tmpdir);
}

UVW_INLINE std::string utilities::os::env(const std::string &name) noexcept {
    return details::try_read(&uv_os_getenv, name.c_str());
}

UVW_INLINE bool utilities::os::env(const std::string &name, const std::string &value) noexcept {
    return (0 == (value.empty() ? uv_os_unsetenv(name.c_str()) : uv_os_setenv(name.c_str(), value.c_str())));
}

UVW_INLINE std::string utilities::os::hostname() noexcept {
    return details::try_read(&uv_os_gethostname);
}

UVW_INLINE uts_name utilities::os::uname() noexcept {
    auto ptr = std::make_shared<uv_utsname_t>();
    uv_os_uname(ptr.get());
    return ptr;
}

UVW_INLINE passwd_info utilities::os::passwd() noexcept {
    auto deleter = [](uv_passwd_t *passwd) {
        uv_os_free_passwd(passwd);
        delete passwd;
    };

    std::shared_ptr<uv_passwd_t> ptr{new uv_passwd_t, std::move(deleter)};
    uv_os_get_passwd(ptr.get());
    return ptr;
}

UVW_INLINE int utilities::os::priority(pid_type pid) {
    int prio = 0;

    if(uv_os_getpriority(pid, &prio)) {
        prio = UV_PRIORITY_LOW + 1;
    }

    return prio;
}

UVW_INLINE bool utilities::os::priority(pid_type pid, int prio) {
    return 0 == uv_os_setpriority(pid, prio);
}

UVW_INLINE handle_type utilities::guess_handle(handle_category category) noexcept {
    switch(category) {
    case UV_ASYNC:
        return handle_type::ASYNC;
    case UV_CHECK:
        return handle_type::CHECK;
    case UV_FS_EVENT:
        return handle_type::FS_EVENT;
    case UV_FS_POLL:
        return handle_type::FS_POLL;
    case UV_HANDLE:
        return handle_type::HANDLE;
    case UV_IDLE:
        return handle_type::IDLE;
    case UV_NAMED_PIPE:
        return handle_type::PIPE;
    case UV_POLL:
        return handle_type::POLL;
    case UV_PREPARE:
        return handle_type::PREPARE;
    case UV_PROCESS:
        return handle_type::PROCESS;
    case UV_STREAM:
        return handle_type::STREAM;
    case UV_TCP:
        return handle_type::TCP;
    case UV_TIMER:
        return handle_type::TIMER;
    case UV_TTY:
        return handle_type::TTY;
    case UV_UDP:
        return handle_type::UDP;
    case UV_SIGNAL:
        return handle_type::SIGNAL;
    case UV_FILE:
        return handle_type::FILE;
    default:
        return handle_type::UNKNOWN;
    }
}

UVW_INLINE handle_type utilities::guess_handle(file_handle file) noexcept {
    handle_category category = uv_guess_handle(file);
    return guess_handle(category);
}

UVW_INLINE std::vector<cpu_info> utilities::cpu() noexcept {
    std::vector<cpu_info> cpuinfos;

    uv_cpu_info_t *infos;
    int count;

    if(0 == uv_cpu_info(&infos, &count)) {
        for(int next = 0; next < count; ++next) {
            cpuinfos.push_back({infos[next].model, infos[next].speed, infos[next].cpu_times});
        }

        uv_free_cpu_info(infos, count);
    }

    return cpuinfos;
}

UVW_INLINE std::vector<interface_address> utilities::interface_addresses() noexcept {
    std::vector<interface_address> interfaces;

    uv_interface_address_t *ifaces{nullptr};
    int count{0};

    if(0 == uv_interface_addresses(&ifaces, &count)) {
        for(int next = 0; next < count; ++next) {
            interface_address iface_addr;

            iface_addr.name = ifaces[next].name;
            std::copy(ifaces[next].phys_addr, (ifaces[next].phys_addr + 6), iface_addr.physical);
            iface_addr.internal = ifaces[next].is_internal == 0 ? false : true;

            if(ifaces[next].address.address4.sin_family == AF_INET) {
                iface_addr.address = details::sock_addr(ifaces[next].address.address4);
                iface_addr.netmask = details::sock_addr(ifaces[next].netmask.netmask4);
            } else if(ifaces[next].address.address4.sin_family == AF_INET6) {
                iface_addr.address = details::sock_addr(ifaces[next].address.address6);
                iface_addr.netmask = details::sock_addr(ifaces[next].netmask.netmask6);
            }

            interfaces.push_back(std::move(iface_addr));
        }

        uv_free_interface_addresses(ifaces, count);
    }

    return interfaces;
}

UVW_INLINE std::string utilities::index_to_name(unsigned int index) noexcept {
    return details::try_read(&uv_if_indextoname, index);
}

UVW_INLINE std::string utilities::index_to_iid(unsigned int index) noexcept {
    return details::try_read(&uv_if_indextoiid, index);
}

UVW_INLINE bool utilities::replace_allocator(malloc_func_type malloc_func, realloc_func_type realloc_func, calloc_func_type calloc_func, free_func_type free_func) noexcept {
    return (0 == uv_replace_allocator(malloc_func, realloc_func, calloc_func, free_func));
}

UVW_INLINE std::array<double, 3> utilities::load_average() noexcept {
    std::array<double, 3> avg;
    uv_loadavg(avg.data());
    return avg;
}

UVW_INLINE char **utilities::setup_args(int argc, char **argv) {
    return uv_setup_args(argc, argv);
}

UVW_INLINE std::string utilities::process_title() {
    std::size_t size = details::DEFAULT_SIZE;
    char buf[details::DEFAULT_SIZE];
    std::string str{};

    if(0 == uv_get_process_title(buf, size)) {
        str.assign(buf, size);
    }

    return str;
}

UVW_INLINE bool utilities::process_title(const std::string &title) {
    return (0 == uv_set_process_title(title.c_str()));
}

UVW_INLINE uint64_t utilities::total_memory() noexcept {
    return uv_get_total_memory();
}

UVW_INLINE uint64_t utilities::constrained_memory() noexcept {
    return uv_get_constrained_memory();
}

UVW_INLINE uint64_t utilities::available_memory() noexcept {
    return uv_get_available_memory();
}

UVW_INLINE double utilities::uptime() noexcept {
    double ret;

    if(0 != uv_uptime(&ret)) {
        ret = 0;
    }

    return ret;
}

UVW_INLINE resource_usage utilities::rusage() noexcept {
    resource_usage ru;
    auto err = uv_getrusage(&ru);
    return err ? resource_usage{} : ru;
}

UVW_INLINE timespec64 utilities::gettime(clock_id source) noexcept {
    timespec64 ts;
    auto err = uv_clock_gettime(static_cast<uv_clock_id>(source), &ts);
    return err ? timespec64{} : ts;
}

UVW_INLINE uint64_t utilities::hrtime() noexcept {
    return uv_hrtime();
}

UVW_INLINE std::string utilities::path() noexcept {
    return details::try_read(&uv_exepath);
}

UVW_INLINE std::string utilities::cwd() noexcept {
    return details::try_read(&uv_cwd);
}

UVW_INLINE bool utilities::chdir(const std::string &dir) noexcept {
    return (0 == uv_chdir(dir.data()));
}

UVW_INLINE timeval64 utilities::time_of_day() noexcept {
    uv_timeval64_t ret;
    uv_gettimeofday(&ret);
    return ret;
}

UVW_INLINE void utilities::sleep(unsigned int msec) noexcept {
    uv_sleep(msec);
}

UVW_INLINE unsigned int utilities::available_parallelism() noexcept {
    return uv_available_parallelism();
}

} // namespace uvw
