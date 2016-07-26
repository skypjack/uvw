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
    using Type = details::UVFsType;
    static constexpr details::UVFsType value = e;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::STAT>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::STAT>>
{
    using Type = details::UVFsType;
    static constexpr details::UVFsType value = details::UVFsType::STAT;

    TypedEvent(const Stat &s): fsStat{s} { }

    const Stat & stat() const noexcept { return fsStat; }

private:
    Stat fsStat;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::FSTAT>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::FSTAT>>
{
    using Type = details::UVFsType;
    static constexpr details::UVFsType value = details::UVFsType::FSTAT;

    TypedEvent(const Stat &s): fsStat{s} { }

    const Stat & stat() const noexcept { return fsStat; }

private:
    Stat fsStat;
};


template<>
struct TypedEvent<details::UVFsType, details::UVFsType::LSTAT>
        : Event<TypedEvent<details::UVFsType, details::UVFsType::LSTAT>>
{
    using Type = details::UVFsType;
    static constexpr details::UVFsType value = details::UVFsType::LSTAT;

    TypedEvent(const Stat &s): fsStat{s} { }

    const Stat & stat() const noexcept { return fsStat; }

private:
    Stat fsStat;
};


template<details::UVFsType e>
using FsEvent = TypedEvent<details::UVFsType, e>;


class Fs final: public Request<Fs, uv_fs_t> {
    template<details::UVFsType e>
    static void fsGenericCallback(uv_fs_t *req) {
        Fs &fs = *(static_cast<Fs*>(req->data));

        auto ptr = fs.shared_from_this();
        (void)ptr;

        fs.reset();

        if(req->result) {
            int err = req->result;
            fs.publish(ErrorEvent{err});
        } else {
            fs.publish(FsEvent<e>{});
        }
    }

    static void fsReadCallback(uv_fs_t *req) {
        // TODO - uv_fs_read callback
    }

    static void fsWriteCallback(uv_fs_t *req) {
        // TODO - uv_fs_write callback
    }

    template<details::UVFsType e>
    static void fsStatCallback(uv_fs_t *req) {
        Fs &fs = *(static_cast<Fs*>(req->data));

        auto ptr = fs.shared_from_this();
        (void)ptr;

        fs.reset();

        if(req->result) {
            int err = req->result;
            fs.publish(ErrorEvent{err});
        } else {
            fs.publish(FsEvent<e>{req->statbuf});
        }
    }

    static void fsReadlinkCallback(uv_fs_t *req) {
        // TODO - uv_fs_readlink callback
    }

    using Request::Request;

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
        invoke(&uv_fs_close, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::CLOSE>);
    }

    auto closeSync(FileHandle file) {
        auto err = uv_fs_close(parent(), get<uv_fs_t>(), file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::CLOSE>{});
    }

    void open(std::string path, int flags, int mode) {
        invoke(&uv_fs_open, parent(), get<uv_fs_t>(), path.data(), flags, mode, &fsGenericCallback<Type::OPEN>);
    }

    auto openSync(std::string path, int flags, int mode) {
        auto err = uv_fs_open(parent(), get<uv_fs_t>(), path.data(), flags, mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::OPEN>{});
    }

    // TODO uv_fs_read (sync (cb null)/async)

    void unlink(std::string path) {
        invoke(&uv_fs_unlink, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::UNLINK>);
    }

    auto unlinkSync(std::string path) {
        auto err = uv_fs_unlink(parent(), get<uv_fs_t>(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::UNLINK>{});
    }

    void write(FileHandle file, char *data, ssize_t len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(data, len) };
        invoke(&uv_fs_write, parent(), get<uv_fs_t>(), file, bufs, 1, offset, &fsWriteCallback);
    }

    void write(FileHandle file, std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        write(file, data.get(), len, offset);
    }

    auto writeSync(FileHandle file, char *data, ssize_t len, int64_t offset) {
        // TODO uv_fs_write (sync (cb null))
    }

    auto writeSync(FileHandle file, std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        // TODO uv_fs_write (sync (cb null))
    }

    void mkdir(std::string path, int mode) {
        invoke(&uv_fs_mkdir, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::MKDIR>);
    }

    auto mkdirSync(std::string path, int mode) {
        auto err = uv_fs_mkdir(parent(), get<uv_fs_t>(), path.data(), mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::MKDIR>{});
    }

    void mkdtemp(std::string tpl) {
        invoke(&uv_fs_mkdtemp, parent(), get<uv_fs_t>(), tpl.data(), &fsGenericCallback<Type::MKDTEMP>);
    }

    auto mkdtempSync(std::string tpl) {
        auto err = uv_fs_mkdtemp(parent(), get<uv_fs_t>(), tpl.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::MKDTEMP>{});
    }

    void rmdir(std::string path) {
        invoke(&uv_fs_rmdir, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::RMDIR>);
    }

    auto rmdirSync(std::string path) {
        auto err = uv_fs_rmdir(parent(), get<uv_fs_t>(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::RMDIR>{});
    }

    void scandir(std::string path, int flags) {
        invoke(&uv_fs_scandir, parent(), get<uv_fs_t>(), path.data(), flags, &fsGenericCallback<Type::SCANDIR>);
    }

    auto scandirSync(std::string path, int flags) {
        auto err = uv_fs_scandir(parent(), get<uv_fs_t>(), path.data(), flags, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::SCANDIR>{});
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
        invoke(&uv_fs_stat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback<Type::STAT>);
    }

    auto statSync(std::string path) {
        auto err = uv_fs_stat(parent(), get<uv_fs_t>(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::STAT>{get<uv_fs_t>()->statbuf});
    }

    void fstat(FileHandle file) {
        invoke(&uv_fs_fstat, parent(), get<uv_fs_t>(), file, &fsStatCallback<Type::FSTAT>);
    }

    auto fstatSync(FileHandle file) {
        auto err = uv_fs_fstat(parent(), get<uv_fs_t>(), file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FSTAT>{get<uv_fs_t>()->statbuf});
    }

    void lstat(std::string path) {
        invoke(&uv_fs_lstat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback<Type::LSTAT>);
    }

    auto lstatSync(std::string path) {
        auto err = uv_fs_lstat(parent(), get<uv_fs_t>(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::LSTAT>{get<uv_fs_t>()->statbuf});
    }

    void rename(std::string old, std::string path) {
        invoke(&uv_fs_rename, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::RENAME>);
    }

    auto renameSync(std::string old, std::string path) {
        auto err = uv_fs_rename(parent(), get<uv_fs_t>(), old.data(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::RENAME>{});
    }

    void fsync(FileHandle file) {
        invoke(&uv_fs_fsync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FSYNC>);
    }

    auto fsyncSync(FileHandle file) {
        auto err = uv_fs_fsync(parent(), get<uv_fs_t>(), file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FSYNC>{});
    }

    void fdatasync(FileHandle file) {
        invoke(&uv_fs_fdatasync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FDATASYNC>);
    }

    auto fdatasyncSync(FileHandle file) {
        auto err = uv_fs_fdatasync(parent(), get<uv_fs_t>(), file, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FDATASYNC>{});
    }

    void ftruncate(FileHandle file, int64_t offset) {
        invoke(&uv_fs_ftruncate, parent(), get<uv_fs_t>(), file, offset, &fsGenericCallback<Type::FTRUNCATE>);
    }

    auto ftruncateSync(FileHandle file, int64_t offset) {
        auto err = uv_fs_ftruncate(parent(), get<uv_fs_t>(), file, offset, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FTRUNCATE>{});
    }

    void sendfile(FileHandle out, FileHandle in, int64_t offset, size_t length) {
        invoke(&uv_fs_sendfile, parent(), get<uv_fs_t>(), out, in, offset, length, &fsGenericCallback<Type::SENDFILE>);
    }

    auto sendfileSync(FileHandle out, FileHandle in, int64_t offset, size_t length) {
        auto err = uv_fs_sendfile(parent(), get<uv_fs_t>(), out, in, offset, length, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::SENDFILE>{});
    }

    void access(std::string path, int mode) {
        invoke(&uv_fs_access, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::ACCESS>);
    }

    auto accessSync(std::string path, int mode) {
        auto err = uv_fs_access(parent(), get<uv_fs_t>(), path.data(), mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::ACCESS>{});
    }

    void chmod(std::string path, int mode) {
        invoke(&uv_fs_chmod, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::CHMOD>);
    }

    auto chmodSync(std::string path, int mode) {
        auto err = uv_fs_chmod(parent(), get<uv_fs_t>(), path.data(), mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::CHMOD>{});
    }

    void fchmod(FileHandle file, int mode) {
        invoke(&uv_fs_fchmod, parent(), get<uv_fs_t>(), file, mode, &fsGenericCallback<Type::FCHMOD>);
    }

    auto fchmodSync(FileHandle file, int mode) {
        auto err = uv_fs_fchmod(parent(), get<uv_fs_t>(), file, mode, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FCHMOD>{});
    }

    void utime(std::string path, Time atime, Time mtime) {
        invoke(&uv_fs_utime, parent(), get<uv_fs_t>(), path.data(), atime.count(), mtime.count(), &fsGenericCallback<Type::UTIME>);
    }

    auto utimeSync(std::string path, Time atime, Time mtime) {
        auto err = uv_fs_utime(parent(), get<uv_fs_t>(), path.data(), atime.count(), mtime.count(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::UTIME>{});
    }

    void futime(FileHandle file, Time atime, Time mtime) {
        invoke(&uv_fs_futime, parent(), get<uv_fs_t>(), file, atime.count(), mtime.count(), &fsGenericCallback<Type::FUTIME>);
    }

    auto futimeSync(FileHandle file, Time atime, Time mtime) {
        auto err = uv_fs_futime(parent(), get<uv_fs_t>(), file, atime.count(), mtime.count(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FUTIME>{});
    }

    void link(std::string old, std::string path) {
        invoke(&uv_fs_link, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::LINK>);
    }

    auto linkSync(std::string old, std::string path) {
        auto err = uv_fs_link(parent(), get<uv_fs_t>(), old.data(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::LINK>{});
    }

    void symlink(std::string old, std::string path, int flags) {
        invoke(&uv_fs_symlink, parent(), get<uv_fs_t>(), old.data(), path.data(), flags, &fsGenericCallback<Type::SYMLINK>);
    }

    auto symlinkSync(std::string old, std::string path, int flags) {
        auto err = uv_fs_symlink(parent(), get<uv_fs_t>(), old.data(), path.data(), flags, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::SYMLINK>{});
    }

    void readlink(std::string path) {
        invoke(&uv_fs_readlink, parent(), get<uv_fs_t>(), path.data(), &fsReadlinkCallback);
    }

    // TODO uv_fs_readlink (sync (cb null))

    void realpath(std::string path) {
        invoke(&uv_fs_realpath, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::REALPATH>);
    }

    auto realpathSync(std::string path) {
        auto err = uv_fs_realpath(parent(), get<uv_fs_t>(), path.data(), nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::REALPATH>{});
    }

    void chown(std::string path, Uid uid, Gid gid) {
        invoke(&uv_fs_chown, parent(), get<uv_fs_t>(), path.data(), uid, gid, &fsGenericCallback<Type::CHOWN>);
    }

    auto chownSync(std::string path, Uid uid, Gid gid) {
        auto err = uv_fs_chown(parent(), get<uv_fs_t>(), path.data(), uid, gid, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::CHOWN>{});
    }

    void fchown(FileHandle file, Uid uid, Gid gid) {
        invoke(&uv_fs_fchown, parent(), get<uv_fs_t>(), file, uid, gid, &fsGenericCallback<Type::FCHOWN>);
    }

    auto fchownSync(FileHandle file, Uid uid, Gid gid) {
        auto err = uv_fs_fchown(parent(), get<uv_fs_t>(), file, uid, gid, nullptr);
        return std::make_pair(ErrorEvent{err}, FsEvent<Type::FCHOWN>{});
    }
};


}
