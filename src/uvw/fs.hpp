#pragma once


#include <utility>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


class Fs final: public Request<Fs> {
    explicit Fs(std::shared_ptr<Loop> ref)
        : Request{RequestType<uv_fs_t>{}, std::move(ref)}
    { }

public:
    template<typename... Args>
    static std::shared_ptr<Fs> create(Args&&... args) {
        return std::shared_ptr<Fs>{new Fs{std::forward<Args>(args)...}};
    }

    ~Fs() {
        uv_fs_req_cleanup(get<uv_fs_t>());
    }

    // TODO uv_fs_close
    // TODO uv_fs_open
    // TODO uv_fs_read
    // TODO uv_fs_unlink
    // TODO uv_fs_write
    // TODO uv_fs_mkdir
    // TODO uv_fs_mkdtemp
    // TODO uv_fs_rmdir
    // TODO uv_fs_scandir
    // TODO uv_fs_scandir_next
    // TODO uv_fs_stat
    // TODO uv_fs_fstat
    // TODO uv_fs_lstat
    // TODO uv_fs_rename
    // TODO uv_fs_fsync
    // TODO uv_fs_fdatasync
    // TODO uv_fs_ftruncate
    // TODO uv_fs_sendfile
    // TODO uv_fs_access
    // TODO uv_fs_chmod
    // TODO uv_fs_fchmod
    // TODO uv_fs_utime
    // TODO uv_fs_futime
    // TODO uv_fs_link
    // TODO uv_fs_symlink
    // TODO uv_fs_readlink
    // TODO uv_fs_realpath
    // TODO uv_fs_chown
    // TODO uv_fs_fchown
};


}
