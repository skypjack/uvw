#pragma once


#include <utility>
#include <memory>
#include <string>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


class Fs final: public Request<Fs, uv_fs_t> {
    static void fsCallback(uv_fs_t *req) {
        // TODO
    }

    using Request::Request;

public:
    template<typename... Args>
    static std::shared_ptr<Fs> create(Args&&... args) {
        return std::shared_ptr<Fs>{new Fs{std::forward<Args>(args)...}};
    }

    ~Fs() {
        uv_fs_req_cleanup(get<uv_fs_t>());
    }

    void close(FileHandle file) {
        invoke(&uv_fs_close, parent(), get<uv_fs_t>(), file, &fsCallback);
    }

    // TODO uv_fs_close (sync (cb null))
    // TODO uv_fs_open (sync (cb null)/async)
    // TODO uv_fs_read (sync (cb null)/async)

    void unlink(std::string path) {
        invoke(&uv_fs_unlink, parent(), get<uv_fs_t>(), path.data(), &fsCallback);
    }

    // TODO uv_fs_unlink (sync (cb null))
    // TODO uv_fs_write (sync (cb null)/async)
    // TODO uv_fs_mkdir (sync (cb null)/async)
    // TODO uv_fs_mkdtemp (sync (cb null)/async)

    void rmdir(std::string path) {
        invoke(&uv_fs_rmdir, parent(), get<uv_fs_t>(), path.data(), &fsCallback);
    }

    // TODO uv_fs_rmdir (sync (cb null))
    // TODO uv_fs_scandir (sync (cb null)/async)
    // TODO uv_fs_scandir_next (sync (cb null)/async)

    void stat(std::string path) {
        invoke(&uv_fs_stat, parent(), get<uv_fs_t>(), path.data(), &fsCallback);
    }

    // TODO uv_fs_stat (sync (cb null))

    void fstat(FileHandle file) {
        invoke(&uv_fs_fstat, parent(), get<uv_fs_t>(), file, &fsCallback);
    }

    // TODO uv_fs_fstat (sync (cb null))

    void lstat(std::string path) {
        invoke(&uv_fs_lstat, parent(), get<uv_fs_t>(), path.data(), &fsCallback);
    }

    // TODO uv_fs_lstat (sync (cb null))
    // TODO uv_fs_rename (sync (cb null)/async)
    // TODO uv_fs_fsync (sync (cb null)/async)
    // TODO uv_fs_fdatasync (sync (cb null)/async)
    // TODO uv_fs_ftruncate (sync (cb null)/async)
    // TODO uv_fs_sendfile (sync (cb null)/async)
    // TODO uv_fs_access (sync (cb null)/async)
    // TODO uv_fs_chmod (sync (cb null)/async)
    // TODO uv_fs_fchmod (sync (cb null)/async)
    // TODO uv_fs_utime (sync (cb null)/async)
    // TODO uv_fs_futime (sync (cb null)/async)
    // TODO uv_fs_link (sync (cb null)/async)
    // TODO uv_fs_symlink (sync (cb null)/async)
    // TODO uv_fs_readlink (sync (cb null)/async)
    // TODO uv_fs_realpath (sync (cb null)/async)

    void chown(std::string path, Uid uid, Gid gid) {
        invoke(&uv_fs_chown, parent(), get<uv_fs_t>(), path.data(), uid, gid, &fsCallback);
    }

    // TODO uv_fs_chown (sync (cb null))

    void fchown(FileHandle file, Uid uid, Gid gid) {
        invoke(&uv_fs_fchown, parent(), get<uv_fs_t>(), file, uid, gid, &fsCallback);
    }

    // TODO uv_fs_fchown (sync (cb null))
};


}
