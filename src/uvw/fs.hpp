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


namespace details {


enum class UVFsType: std::underlying_type_t<uv_fs_type> {
    UNKNOWN = UV_FS_UNKNOWN,
    CUSTOM = UV_FS_CUSTOM,
    OPEN = UV_FS_OPEN,
    CLOSE = UV_FS_CLOSE,
    READ = UV_FS_READ,
    WRITE = UV_FS_WRITE,
    SENDFILE = UV_FS_SENDFILE,
    STAT = UV_FS_STAT,
    LSTAT = UV_FS_LSTAT,
    FSTAT = UV_FS_FSTAT,
    FTRUNCATE = UV_FS_FTRUNCATE,
    UTIME = UV_FS_UTIME,
    FUTIME = UV_FS_FUTIME,
    ACCESS = UV_FS_ACCESS,
    CHMOD = UV_FS_CHMOD,
    FCHMOD = UV_FS_FCHMOD,
    FSYNC = UV_FS_FSYNC,
    FDATASYNC = UV_FS_FDATASYNC,
    UNLINK = UV_FS_UNLINK,
    RMDIR = UV_FS_RMDIR,
    MKDIR = UV_FS_MKDIR,
    MKDTEMP = UV_FS_MKDTEMP,
    RENAME = UV_FS_RENAME,
    SCANDIR = UV_FS_SCANDIR,
    LINK = UV_FS_LINK,
    SYMLINK = UV_FS_SYMLINK,
    READLINK = UV_FS_READLINK,
    CHOWN = UV_FS_CHOWN,
    FCHOWN = UV_FS_FCHOWN,
    REALPATH = UV_FS_REALPATH
};


enum class UVDirentTypeT: std::underlying_type_t<uv_dirent_type_t> {
    UNKNOWN = UV_DIRENT_UNKNOWN,
    FILE = UV_DIRENT_FILE,
    DIR = UV_DIRENT_DIR,
    LINK = UV_DIRENT_LINK,
    FIFO = UV_DIRENT_FIFO,
    SOCKET = UV_DIRENT_SOCKET,
    CHAR = UV_DIRENT_CHAR,
    BLOCK = UV_DIRENT_BLOCK
};


}


template<details::UVFsType e>
struct TypedEvent<details::UVFsType, e>
        : Event<TypedEvent<details::UVFsType, e>>
{
    TypedEvent(const char *p) noexcept
        : rPath{p}
    { }

    const char * path() const noexcept { return rPath; }

private:
    const char *rPath;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::OPEN>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::OPEN>>
{
    TypedEvent(const char *p, ssize_t desc) noexcept
        : rPath{p}, fd{desc}
    { }

    const char * path() const noexcept { return rPath; }
    FileHandle file() const noexcept { return fd; }

private:
    const char *rPath;
    const FileHandle fd;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::WRITE>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::WRITE>>
{
    TypedEvent(const char *p, ssize_t s) noexcept
        : rPath{p}, sz{s}
    { }

    const char * path() const noexcept { return rPath; }
    ssize_t amount() const noexcept { return sz; }

private:
    const char *rPath;
    const ssize_t sz;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::SENDFILE>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::SENDFILE>>
{
    TypedEvent(const char *p, ssize_t s) noexcept
        : rPath{p}, sz{s}
    { }

    const char * path() const noexcept { return rPath; }
    ssize_t amount() const noexcept { return sz; }

private:
    const char *rPath;
    const ssize_t sz;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::STAT>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::STAT>>
{
    TypedEvent(const char *p, const Stat &s) noexcept
        : rPath{p}, fsStat{s}
    { }

    const char * path() const noexcept { return rPath; }
    const Stat & stat() const noexcept { return fsStat; }

private:
    const char *rPath;
    Stat fsStat;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::FSTAT>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::FSTAT>>
{
    TypedEvent(const char *p, const Stat &s) noexcept
        : rPath{p}, fsStat{s}
    { }

    const char * path() const noexcept { return rPath; }
    const Stat & stat() const noexcept { return fsStat; }

private:
    const char *rPath;
    Stat fsStat;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::LSTAT>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::LSTAT>>
{
    TypedEvent(const char *p, const Stat &s) noexcept
        : rPath{p}, fsStat{s}
    { }

    const char * path() const noexcept { return rPath; }
    const Stat & stat() const noexcept { return fsStat; }

private:
    const char *rPath;
    Stat fsStat;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::SCANDIR>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::SCANDIR>>
{
    TypedEvent(const char *p, ssize_t s) noexcept
        : rPath{p}, sz{s}
    { }

    const char * path() const noexcept { return rPath; }
    ssize_t amount() const noexcept { return sz; }

private:
    const char *rPath;
    const ssize_t sz;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::READLINK>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::READLINK>>
{
    explicit TypedEvent(const char *p, const char *d, ssize_t l) noexcept
        : rPath{p}, dt{d}, len{l}
    { }

    const char * path() const noexcept { return rPath; }
    const char * data() const noexcept { return dt; }
    ssize_t length() const noexcept { return len; }

private:
    const char *rPath;
    const char *dt;
    const ssize_t len;
};


template<details::UVFsType e>
using FsEvent = TypedEvent<details::UVFsType, e>;


class Fs final: public Request<Fs, uv_fs_t> {
    template<details::UVFsType e>
    static void fsGenericCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<e>{req->path}); }
    }

    static void fsReadCallback(uv_fs_t *req) {
        // TODO - uv_fs_read callback
    }

    template<details::UVFsType e>
    static void fsResultCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<e>{req->path, req->result}); }
    }

    template<details::UVFsType e>
    static void fsStatCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<e>{req->path, req->statbuf}); }
    }

    static void fsReadlinkCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr), req->result}); }
    }

    using Request::Request;

    template<typename... Args>
    auto cleanupAndInvoke(Args&&... args) {
        uv_fs_req_cleanup(get<uv_fs_t>());
        return invoke(std::forward<Args>(args)...);
    }

    template<typename F, typename... Args>
    auto cleanupAndInvokeSync(F &&f, Args&&... args) {
        uv_fs_req_cleanup(get<uv_fs_t>());
        return std::forward<F>(f)(std::forward<Args>(args)...);
    }

public:
    using Time = std::chrono::seconds;
    using Type = details::UVFsType;
    using EntryType = details::UVDirentTypeT;
    using Entry = std::pair<EntryType, std::string>;

    template<typename... Args>
    static std::shared_ptr<Fs> create(Args&&... args) {
        return std::shared_ptr<Fs>{new Fs{std::forward<Args>(args)...}};
    }

    ~Fs() {
        uv_fs_req_cleanup(get<uv_fs_t>());
    }

    void close(FileHandle file) {
        cleanupAndInvoke(&uv_fs_close, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::CLOSE>);
    }

    auto closeSync(FileHandle file) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_close, parent(), req, file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::CLOSE>{req->path});
    }

    void open(std::string path, int flags, int mode) {
        cleanupAndInvoke(&uv_fs_open, parent(), get<uv_fs_t>(), path.data(), flags, mode, &fsResultCallback<Type::OPEN>);
    }

    auto openSync(std::string path, int flags, int mode) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_open, parent(), req, path.data(), flags, mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::OPEN>{req->path, req->result});
    }

    void read(FileHandle file, int64_t offset, unsigned int len) {
        // TODO uv_fs_read (async)
    }

    auto readSync(FileHandle file, int64_t offset, unsigned int len) {
        // TODO uv_fs_read (sync (cb null))
    }

    void unlink(std::string path) {
        cleanupAndInvoke(&uv_fs_unlink, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::UNLINK>);
    }

    auto unlinkSync(std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_unlink, parent(), req, path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::UNLINK>{req->path});
    }

    void write(FileHandle file, std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        cleanupAndInvoke(&uv_fs_write, parent(), get<uv_fs_t>(), file, bufs, 1, offset, &fsResultCallback<Type::WRITE>);
    }

    auto writeSync(FileHandle file, std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        auto req = get<uv_fs_t>();
        auto err = uv_fs_write(parent(), get<uv_fs_t>(), file, bufs, 1, offset, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::WRITE>{req->path, req->result});
    }

    void mkdir(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_mkdir, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::MKDIR>);
    }

    auto mkdirSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        auto err = uv_fs_mkdir(parent(), req, path.data(), mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::MKDIR>{req->path});
    }

    void mkdtemp(std::string tpl) {
        cleanupAndInvoke(&uv_fs_mkdtemp, parent(), get<uv_fs_t>(), tpl.data(), &fsGenericCallback<Type::MKDTEMP>);
    }

    auto mkdtempSync(std::string tpl) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_mkdtemp, parent(), req, tpl.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::MKDTEMP>{req->path});
    }

    void rmdir(std::string path) {
        cleanupAndInvoke(&uv_fs_rmdir, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::RMDIR>);
    }

    auto rmdirSync(std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_rmdir, parent(), req, path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::RMDIR>{req->path});
    }

    void scandir(std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_scandir, parent(), get<uv_fs_t>(), path.data(), flags, &fsResultCallback<Type::SCANDIR>);
    }

    auto scandirSync(std::string path, int flags) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_scandir, parent(), req, path.data(), flags, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::SCANDIR>{req->path, req->result});
    }

    std::pair<bool, Entry> scandirNext() {
        uv_dirent_t dirent;
        std::pair<bool, Entry> ret{false, { EntryType::UNKNOWN, "" }};
        auto res = uv_fs_scandir_next(get<uv_fs_t>(), &dirent);

        if(UV_EOF != res) {
            ret.second.first = static_cast<EntryType>(dirent.type);
            ret.second.second = dirent.name;
            ret.first = true;
        }

        return ret;
    }

    void stat(std::string path) {
        cleanupAndInvoke(&uv_fs_stat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback<Type::STAT>);
    }

    auto statSync(std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_stat, parent(), req, path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::STAT>{req->path, req->statbuf});
    }

    void fstat(FileHandle file) {
        cleanupAndInvoke(&uv_fs_fstat, parent(), get<uv_fs_t>(), file, &fsStatCallback<Type::FSTAT>);
    }

    auto fstatSync(FileHandle file) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_fstat, parent(), req, file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FSTAT>{req->path, req->statbuf});
    }

    void lstat(std::string path) {
        cleanupAndInvoke(&uv_fs_lstat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback<Type::LSTAT>);
    }

    auto lstatSync(std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_lstat, parent(), req, path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::LSTAT>{req->path, req->statbuf});
    }

    void rename(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_rename, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::RENAME>);
    }

    auto renameSync(std::string old, std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_rename, parent(), req, old.data(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::RENAME>{req->path});
    }

    void fsync(FileHandle file) {
        cleanupAndInvoke(&uv_fs_fsync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FSYNC>);
    }

    auto fsyncSync(FileHandle file) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_fsync, parent(), req, file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FSYNC>{req->path});
    }

    void fdatasync(FileHandle file) {
        cleanupAndInvoke(&uv_fs_fdatasync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FDATASYNC>);
    }

    auto fdatasyncSync(FileHandle file) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_fdatasync, parent(), req, file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FDATASYNC>{req->path});
    }

    void ftruncate(FileHandle file, int64_t offset) {
        cleanupAndInvoke(&uv_fs_ftruncate, parent(), get<uv_fs_t>(), file, offset, &fsGenericCallback<Type::FTRUNCATE>);
    }

    auto ftruncateSync(FileHandle file, int64_t offset) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_ftruncate, parent(), req, file, offset, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FTRUNCATE>{req->path});
    }

    void sendfile(FileHandle out, FileHandle in, int64_t offset, size_t length) {
        cleanupAndInvoke(&uv_fs_sendfile, parent(), get<uv_fs_t>(), out, in, offset, length, &fsResultCallback<Type::SENDFILE>);
    }

    auto sendfileSync(FileHandle out, FileHandle in, int64_t offset, size_t length) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_sendfile, parent(), req, out, in, offset, length, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::SENDFILE>{req->path, req->result});
    }

    void access(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_access, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::ACCESS>);
    }

    auto accessSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_access, parent(), req, path.data(), mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::ACCESS>{req->path});
    }

    void chmod(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_chmod, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::CHMOD>);
    }

    auto chmodSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_chmod, parent(), req, path.data(), mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::CHMOD>{req->path});
    }

    void fchmod(FileHandle file, int mode) {
        cleanupAndInvoke(&uv_fs_fchmod, parent(), get<uv_fs_t>(), file, mode, &fsGenericCallback<Type::FCHMOD>);
    }

    auto fchmodSync(FileHandle file, int mode) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_fchmod, parent(), req, file, mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FCHMOD>{req->path});
    }

    void utime(std::string path, Time atime, Time mtime) {
        cleanupAndInvoke(&uv_fs_utime, parent(), get<uv_fs_t>(), path.data(), atime.count(), mtime.count(), &fsGenericCallback<Type::UTIME>);
    }

    auto utimeSync(std::string path, Time atime, Time mtime) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_utime, parent(), req, path.data(), atime.count(), mtime.count(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::UTIME>{req->path});
    }

    void futime(FileHandle file, Time atime, Time mtime) {
        cleanupAndInvoke(&uv_fs_futime, parent(), get<uv_fs_t>(), file, atime.count(), mtime.count(), &fsGenericCallback<Type::FUTIME>);
    }

    auto futimeSync(FileHandle file, Time atime, Time mtime) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_futime, parent(), req, file, atime.count(), mtime.count(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FUTIME>{req->path});
    }

    void link(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_link, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::LINK>);
    }

    auto linkSync(std::string old, std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_link, parent(), req, old.data(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::LINK>{req->path});
    }

    void symlink(std::string old, std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_symlink, parent(), get<uv_fs_t>(), old.data(), path.data(), flags, &fsGenericCallback<Type::SYMLINK>);
    }

    auto symlinkSync(std::string old, std::string path, int flags) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_symlink, parent(), req, old.data(), path.data(), flags, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::SYMLINK>{req->path});
    }

    void readlink(std::string path) {
        cleanupAndInvoke(&uv_fs_readlink, parent(), get<uv_fs_t>(), path.data(), &fsReadlinkCallback);
    }

    auto readlinkSync(std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_readlink, parent(), req, path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr), req->result});
    }

    void realpath(std::string path) {
        cleanupAndInvoke(&uv_fs_realpath, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::REALPATH>);
    }

    auto realpathSync(std::string path) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_realpath, parent(), req, path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::REALPATH>{req->path});
    }

    void chown(std::string path, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_chown, parent(), get<uv_fs_t>(), path.data(), uid, gid, &fsGenericCallback<Type::CHOWN>);
    }

    auto chownSync(std::string path, Uid uid, Gid gid) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_chown, parent(), req, path.data(), uid, gid, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::CHOWN>{req->path});
    }

    void fchown(FileHandle file, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_fchown, parent(), get<uv_fs_t>(), file, uid, gid, &fsGenericCallback<Type::FCHOWN>);
    }

    auto fchownSync(FileHandle file, Uid uid, Gid gid) {
        auto req = get<uv_fs_t>();
        auto err = cleanupAndInvokeSync(&uv_fs_fchown, parent(), req, file, uid, gid, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FCHOWN>{req->path});
    }
};


}
