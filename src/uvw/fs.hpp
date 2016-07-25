#pragma once


#include <utility>
#include <memory>
#include <string>
#include <chrono>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "util.hpp"


namespace uvw {


class Fs final: public Request<Fs, uv_fs_t> {
    static void fsCallback(uv_fs_t *req) {
        // TODO type
        /*
        Fs &fs = *(static_cast<Fs*>(req->data));

        auto ptr = res.shared_from_this();
        (void)ptr;

        res.reset();

        if(req->result) {
            res.publish(ErrorEvent{status});
        } else {
            res.publish(FsEvent{});
        }
        */
    }

    static void fsReadCallback(uv_fs_t *req) {
        // TODO - uv_fs_read callback
    }

    static void fsWriteCallback(uv_fs_t *req) {
        // TODO - uv_fs_write callback
    }

    static void fsStatCallback(uv_fs_t *req) {
        // TODO - uv_fs_stat callback
    }

    static void fsReadlinkCallback(uv_fs_t *req) {
        // TODO - uv_fs_readlink callback
    }

    using Request::Request;

public:
    using Time = std::chrono::seconds;

    using Flags = int;
    using Mode = int;
    using Offset = int64_t;

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

    void open(std::string path, Flags flags, Mode mode) {
        invoke(&uv_fs_open, parent(), get<uv_fs_t>(), path.data(), flags, mode, &fsCallback);
    }

    // TODO uv_fs_open (sync (cb null))
    // TODO uv_fs_read (sync (cb null)/async)

    void unlink(std::string path) {
        invoke(&uv_fs_unlink, parent(), get<uv_fs_t>(), path.data(), &fsCallback);
    }

    // TODO uv_fs_unlink (sync (cb null))
    // TODO uv_fs_write (sync (cb null)/async)

    void mkdir(std::string path, Mode mode) {
        invoke(&uv_fs_mkdir, parent(), get<uv_fs_t>(), path.data(), mode, &fsCallback);
    }

    // TODO uv_fs_mkdir (sync (cb null))

    void mkdtemp(std::string tpl, Mode mode) {
        invoke(&uv_fs_mkdtemp, parent(), get<uv_fs_t>(), tpl.data(), &fsCallback);
    }

    // TODO uv_fs_mkdtemp (sync (cb null))

    void rmdir(std::string path) {
        invoke(&uv_fs_rmdir, parent(), get<uv_fs_t>(), path.data(), &fsCallback);
    }

    // TODO uv_fs_rmdir (sync (cb null))

    void scandir(std::string path, Flags flags) {
        invoke(&uv_fs_scandir, parent(), get<uv_fs_t>(), path.data(), flags, &fsCallback);
    }

    // TODO uv_fs_scandir (sync (cb null))
    // TODO uv_fs_scandir_next (sync (cb null)/async)

    void stat(std::string path) {
        invoke(&uv_fs_stat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback);
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

    void rename(std::string old, std::string path) {
        invoke(&uv_fs_rename, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsCallback);
    }

    // TODO uv_fs_rename (sync (cb null))

    void fsync(FileHandle file) {
        invoke(&uv_fs_fsync, parent(), get<uv_fs_t>(), file, &fsCallback);
    }

    // TODO uv_fs_fsync (sync (cb null))

    void fdatasync(FileHandle file) {
        invoke(&uv_fs_fdatasync, parent(), get<uv_fs_t>(), file, &fsCallback);
    }

    // TODO uv_fs_fdatasync (sync (cb null))

    void ftruncate(FileHandle file, Offset offset) {
        invoke(&uv_fs_ftruncate, parent(), get<uv_fs_t>(), file, offset, &fsCallback);
    }

    // TODO uv_fs_ftruncate (sync (cb null))
    // TODO uv_fs_sendfile (sync (cb null)/async)

    void access(std::string path, Mode mode) {
        invoke(&uv_fs_access, parent(), get<uv_fs_t>(), path.data(), mode, &fsCallback);
    }

    // TODO uv_fs_access (sync (cb null))

    void chmod(std::string path, Mode mode) {
        invoke(&uv_fs_chmod, parent(), get<uv_fs_t>(), path.data(), mode, &fsCallback);
    }

    // TODO uv_fs_chmod (sync (cb null))

    void fchmod(FileHandle file, Mode mode) {
        invoke(&uv_fs_fchmod, parent(), get<uv_fs_t>(), file, mode, &fsCallback);
    }

    // TODO uv_fs_fchmod (sync (cb null))

    void utime(std::string path, Time atime, Time mtime) {
        invoke(&uv_fs_utime, parent(), get<uv_fs_t>(), path.data(), atime.count(), mtime.count(), &fsCallback);
    }

    // TODO uv_fs_utime (sync (cb null))

    void utime(FileHandle file, Time atime, Time mtime) {
        invoke(&uv_fs_futime, parent(), get<uv_fs_t>(), file, atime.count(), mtime.count(), &fsCallback);
    }

    // TODO uv_fs_futime (sync (cb null))

    void link(std::string old, std::string path) {
        invoke(&uv_fs_link, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsCallback);
    }

    // TODO uv_fs_link (sync (cb null))

    void symlink(std::string old, std::string path, Flags flags) {
        invoke(&uv_fs_symlink, parent(), get<uv_fs_t>(), old.data(), path.data(), flags, &fsCallback);
    }

    // TODO uv_fs_symlink (sync (cb null))

    void readlink(std::string path) {
        invoke(&uv_fs_readlink, parent(), get<uv_fs_t>(), path.data(), &fsReadlinkCallback);
    }

    // TODO uv_fs_readlink (sync (cb null))

    void realpath(std::string path) {
        invoke(&uv_fs_realpath, parent(), get<uv_fs_t>(), path.data(), &fsCallback);
    }

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
