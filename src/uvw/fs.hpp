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
struct TypedEvent<details::UVFsType, details::UVFsType::READ>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::READ>>
{
    TypedEvent(const char *p, std::unique_ptr<const char[]> ptr, ssize_t l) noexcept
        : rPath{p}, dt{std::move(ptr)}, len{l}
    { }

    const char * path() const noexcept { return rPath; }
    const char * data() const noexcept { return dt.get(); }
    ssize_t length() const noexcept { return len; }

private:
    const char *rPath;
    std::unique_ptr<const char[]> dt;
    const ssize_t len;
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
        : rPath{p}, fsStat(s)
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
        : rPath{p}, fsStat(s)
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
        : rPath{p}, fsStat(s)
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


template<typename T>
class FsRequest: public Request<T, uv_fs_t> {
protected:
    template<details::UVFsType e>
    static void fsGenericCallback(uv_fs_t *req) {
        auto ptr = Request<T, uv_fs_t>::reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<e>{req->path}); }
    }

    template<details::UVFsType e>
    static void fsResultCallback(uv_fs_t *req) {
        auto ptr = Request<T, uv_fs_t>::reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<e>{req->path, req->result}); }
    }

    template<details::UVFsType e>
    static void fsStatCallback(uv_fs_t *req) {
        auto ptr = Request<T, uv_fs_t>::reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<e>{req->path, req->statbuf}); }
    }

    using Request<T, uv_fs_t>::Request;

    template<typename... Args>
    void cleanupAndInvoke(Args&&... args) {
        uv_fs_req_cleanup(this->template get<uv_fs_t>());
        this->invoke(std::forward<Args>(args)...);
    }

    template<typename F, typename... Args>
    void cleanupAndInvokeSync(F &&f, Args&&... args) {
        uv_fs_req_cleanup(this->template get<uv_fs_t>());
        std::forward<F>(f)(std::forward<Args>(args)..., nullptr);
    }

public:
    using Time = std::chrono::seconds;
    using Type = details::UVFsType;
    using EntryType = details::UVDirentTypeT;
    using Entry = std::pair<EntryType, std::string>;
};


class FileReq final: public FsRequest<FileReq> {
    static constexpr uv_file BAD_FD = -1;

    static void fsOpenCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));

        if(req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        } else {
            ptr->file = static_cast<uv_file>(req->result);
            ptr->publish(FsEvent<Type::OPEN>{req->path});
        }
    }

    static void fsCloseCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));

        if(req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        } else {
            ptr->file = BAD_FD;
            ptr->publish(FsEvent<Type::CLOSE>{req->path});
        }
    }

    static void fsReadCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<Type::READ>{req->path, std::move(ptr->data), req->result}); }
    }

    using FsRequest::FsRequest;

public:
    template<typename... Args>
    static std::shared_ptr<FileReq> create(Args&&... args) {
        return std::shared_ptr<FileReq>{new FileReq{std::forward<Args>(args)...}};
    }

    ~FileReq() noexcept {
        uv_fs_req_cleanup(get<uv_fs_t>());
    }

    void close() {
        cleanupAndInvoke(&uv_fs_close, parent(), get<uv_fs_t>(), file, &fsCloseCallback);
    }

    auto closeSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_close, parent(), req, file);
        if(req->result >= 0) { file = BAD_FD; }
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::CLOSE>{req->path});
    }

    void open(std::string path, int flags, int mode) {
        cleanupAndInvoke(&uv_fs_open, parent(), get<uv_fs_t>(), path.data(), flags, mode, &fsOpenCallback);
    }

    auto openSync(std::string path, int flags, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_open, parent(), req, path.data(), flags, mode);
        if(req->result >= 0) { file = static_cast<uv_file>(req->result); }
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::OPEN>{req->path});
    }

    void read(int64_t offset, unsigned int len) {
        data = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(data.get(), len);
        uv_buf_t bufs[] = { buffer };
        cleanupAndInvoke(&uv_fs_read, parent(), get<uv_fs_t>(), file, bufs, 1, offset, &fsReadCallback);
    }

    auto readSync(int64_t offset, unsigned int len) {
        data = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(data.get(), len);
        uv_buf_t bufs[] = { buffer };
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_read, parent(), req, file, bufs, 1, offset);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::READ>{req->path, std::move(data), req->result});
    }

    void write(std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        cleanupAndInvoke(&uv_fs_write, parent(), get<uv_fs_t>(), file, bufs, 1, offset, &fsResultCallback<Type::WRITE>);
    }

    auto writeSync(std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_write, parent(), get<uv_fs_t>(), file, bufs, 1, offset);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::WRITE>{req->path, req->result});
    }

    void fstat() {
        cleanupAndInvoke(&uv_fs_fstat, parent(), get<uv_fs_t>(), file, &fsStatCallback<Type::FSTAT>);
    }

    auto fstatSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fstat, parent(), req, file);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FSTAT>{req->path, req->statbuf});
    }

    void fsync() {
        cleanupAndInvoke(&uv_fs_fsync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FSYNC>);
    }

    auto fsyncSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fsync, parent(), req, file);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FSYNC>{req->path});
    }

    void fdatasync() {
        cleanupAndInvoke(&uv_fs_fdatasync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FDATASYNC>);
    }

    auto fdatasyncSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fdatasync, parent(), req, file);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FDATASYNC>{req->path});
    }

    void ftruncate(int64_t offset) {
        cleanupAndInvoke(&uv_fs_ftruncate, parent(), get<uv_fs_t>(), file, offset, &fsGenericCallback<Type::FTRUNCATE>);
    }

    auto ftruncateSync(int64_t offset) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_ftruncate, parent(), req, file, offset);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FTRUNCATE>{req->path});
    }

    void sendfile(FileHandle out, int64_t offset, size_t length) {
        cleanupAndInvoke(&uv_fs_sendfile, parent(), get<uv_fs_t>(), out, file, offset, length, &fsResultCallback<Type::SENDFILE>);
    }

    auto sendfileSync(FileHandle out, int64_t offset, size_t length) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_sendfile, parent(), req, out, file, offset, length);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::SENDFILE>{req->path, req->result});
    }

    void fchmod(int mode) {
        cleanupAndInvoke(&uv_fs_fchmod, parent(), get<uv_fs_t>(), file, mode, &fsGenericCallback<Type::FCHMOD>);
    }

    auto fchmodSync(int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fchmod, parent(), req, file, mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FCHMOD>{req->path});
    }

    void futime(Time atime, Time mtime) {
        cleanupAndInvoke(&uv_fs_futime, parent(), get<uv_fs_t>(), file, atime.count(), mtime.count(), &fsGenericCallback<Type::FUTIME>);
    }

    auto futimeSync(Time atime, Time mtime) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_futime, parent(), req, file, atime.count(), mtime.count());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FUTIME>{req->path});
    }

    void fchown(Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_fchown, parent(), get<uv_fs_t>(), file, uid, gid, &fsGenericCallback<Type::FCHOWN>);
    }

    auto fchownSync(Uid uid, Gid gid) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fchown, parent(), req, file, uid, gid);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FCHOWN>{req->path});
    }

    operator FileHandle() const noexcept { return file; }

private:
    std::unique_ptr<char[]> data{nullptr};
    uv_buf_t buffer{};
    uv_file file{BAD_FD};
};


class FsReq final: public FsRequest<FsReq> {
    using FsRequest::FsRequest;

    static void fsReadlinkCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr), req->result}); }
    }

public:
    template<typename... Args>
    static std::shared_ptr<FsReq> create(Args&&... args) {
        return std::shared_ptr<FsReq>{new FsReq{std::forward<Args>(args)...}};
    }

    ~FsReq() noexcept {
        uv_fs_req_cleanup(get<uv_fs_t>());
    }

    void unlink(std::string path) {
        cleanupAndInvoke(&uv_fs_unlink, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::UNLINK>);
    }

    auto unlinkSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_unlink, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::UNLINK>{req->path});
    }

    void mkdir(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_mkdir, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::MKDIR>);
    }

    auto mkdirSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_mkdir, parent(), req, path.data(), mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::MKDIR>{req->path});
    }

    void mkdtemp(std::string tpl) {
        cleanupAndInvoke(&uv_fs_mkdtemp, parent(), get<uv_fs_t>(), tpl.data(), &fsGenericCallback<Type::MKDTEMP>);
    }

    auto mkdtempSync(std::string tpl) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_mkdtemp, parent(), req, tpl.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::MKDTEMP>{req->path});
    }

    void rmdir(std::string path) {
        cleanupAndInvoke(&uv_fs_rmdir, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::RMDIR>);
    }

    auto rmdirSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_rmdir, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::RMDIR>{req->path});
    }

    void scandir(std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_scandir, parent(), get<uv_fs_t>(), path.data(), flags, &fsResultCallback<Type::SCANDIR>);
    }

    auto scandirSync(std::string path, int flags) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_scandir, parent(), req, path.data(), flags);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::SCANDIR>{req->path, req->result});
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
        cleanupAndInvokeSync(&uv_fs_stat, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::STAT>{req->path, req->statbuf});
    }

    void lstat(std::string path) {
        cleanupAndInvoke(&uv_fs_lstat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback<Type::LSTAT>);
    }

    auto lstatSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_lstat, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::LSTAT>{req->path, req->statbuf});
    }

    void rename(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_rename, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::RENAME>);
    }

    auto renameSync(std::string old, std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_rename, parent(), req, old.data(), path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::RENAME>{req->path});
    }

    void access(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_access, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::ACCESS>);
    }

    auto accessSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_access, parent(), req, path.data(), mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::ACCESS>{req->path});
    }

    void chmod(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_chmod, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::CHMOD>);
    }

    auto chmodSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_chmod, parent(), req, path.data(), mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::CHMOD>{req->path});
    }

    void utime(std::string path, Time atime, Time mtime) {
        cleanupAndInvoke(&uv_fs_utime, parent(), get<uv_fs_t>(), path.data(), atime.count(), mtime.count(), &fsGenericCallback<Type::UTIME>);
    }

    auto utimeSync(std::string path, Time atime, Time mtime) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_utime, parent(), req, path.data(), atime.count(), mtime.count());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::UTIME>{req->path});
    }

    void link(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_link, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::LINK>);
    }

    auto linkSync(std::string old, std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_link, parent(), req, old.data(), path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::LINK>{req->path});
    }

    void symlink(std::string old, std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_symlink, parent(), get<uv_fs_t>(), old.data(), path.data(), flags, &fsGenericCallback<Type::SYMLINK>);
    }

    auto symlinkSync(std::string old, std::string path, int flags) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_symlink, parent(), req, old.data(), path.data(), flags);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::SYMLINK>{req->path});
    }

    void readlink(std::string path) {
        cleanupAndInvoke(&uv_fs_readlink, parent(), get<uv_fs_t>(), path.data(), &fsReadlinkCallback);
    }

    auto readlinkSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_readlink, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr), req->result});
    }

    void realpath(std::string path) {
        cleanupAndInvoke(&uv_fs_realpath, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::REALPATH>);
    }

    auto realpathSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_realpath, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::REALPATH>{req->path});
    }

    void chown(std::string path, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_chown, parent(), get<uv_fs_t>(), path.data(), uid, gid, &fsGenericCallback<Type::CHOWN>);
    }

    auto chownSync(std::string path, Uid uid, Gid gid) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_chown, parent(), req, path.data(), uid, gid);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::CHOWN>{req->path});
    }
};


}
