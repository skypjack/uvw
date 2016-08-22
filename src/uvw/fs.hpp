#pragma once


#include <utility>
#include <memory>
#include <string>
#include <chrono>
#include <uv.h>
#include "event.hpp"
#include "request.hpp"
#include "misc.hpp"


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


/**
 * @brief Default FsEvent event.
 *
 * Available types are:
 *
 * * FsRequest::Type::UNKNOWN
 * * FsRequest::Type::CUSTOM
 * * FsRequest::Type::OPEN
 * * FsRequest::Type::CLOSE
 * * FsRequest::Type::READ
 * * FsRequest::Type::WRITE
 * * FsRequest::Type::SENDFILE
 * * FsRequest::Type::STAT
 * * FsRequest::Type::LSTAT
 * * FsRequest::Type::FSTAT
 * * FsRequest::Type::FTRUNCATE
 * * FsRequest::Type::UTIME
 * * FsRequest::Type::FUTIME
 * * FsRequest::Type::ACCESS
 * * FsRequest::Type::CHMOD
 * * FsRequest::Type::FCHMOD
 * * FsRequest::Type::FSYNC
 * * FsRequest::Type::FDATASYNC
 * * FsRequest::Type::UNLINK
 * * FsRequest::Type::RMDIR
 * * FsRequest::Type::MKDIR
 * * FsRequest::Type::MKDTEMP
 * * FsRequest::Type::RENAME
 * * FsRequest::Type::SCANDIR
 * * FsRequest::Type::LINK
 * * FsRequest::Type::SYMLINK
 * * FsRequest::Type::READLINK
 * * FsRequest::Type::CHOWN
 * * FsRequest::Type::FCHOWN
 * * FsRequest::Type::REALPATH
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs.html#c.uv_fs_type)
 * for further details.
 */
template<details::UVFsType e>
struct FsEvent: Event<FsEvent<e>> {
    FsEvent(const char *p) noexcept: rPath{p} { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

private:
    const char *rPath;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::READ.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::READ>
        : Event<FsEvent<details::UVFsType::READ>>
{
    FsEvent(const char *p, std::unique_ptr<const char[]> ptr, ssize_t l) noexcept
        : rPath{p}, dt{std::move(ptr)}, len{l}
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets the data read from the given path.
     * @return A bunch of data read from the given path.
     */
    const char * data() const noexcept { return dt.get(); }

    /**
     * @brief Gets the amount of data read from the given path.
     * @return The amount of data read from the given path.
     */
    ssize_t length() const noexcept { return len; }

private:
    const char *rPath;
    std::unique_ptr<const char[]> dt;
    const ssize_t len;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::WRITE.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::WRITE>
        : Event<FsEvent<details::UVFsType::WRITE>>
{
    FsEvent(const char *p, ssize_t s) noexcept
        : rPath{p}, sz{s}
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets the amount of data written to the given path.
     * @return The amount of data written to the given path.
     */
    ssize_t amount() const noexcept { return sz; }

private:
    const char *rPath;
    const ssize_t sz;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::SENDFILE.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::SENDFILE>
        : Event<FsEvent<details::UVFsType::SENDFILE>>
{
    FsEvent(const char *p, ssize_t s) noexcept
        : rPath{p}, sz{s}
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets the amount of data transferred.
     * @return The amount of data transferred.
     */
    ssize_t amount() const noexcept { return sz; }

private:
    const char *rPath;
    const ssize_t sz;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::STAT.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::STAT>
        : Event<FsEvent<details::UVFsType::STAT>>
{
    FsEvent(const char *p, const Stat &s) noexcept
        : rPath{p}, fsStat(s)
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets an initialized instance of Stat.
     * @return An initialized instance of Stat.
     */
    const Stat & stat() const noexcept { return fsStat; }

private:
    const char *rPath;
    Stat fsStat;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::FSTAT.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::FSTAT>
        : Event<FsEvent<details::UVFsType::FSTAT>>
{
    FsEvent(const char *p, const Stat &s) noexcept
        : rPath{p}, fsStat(s)
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets an initialized instance of Stat.
     * @return An initialized instance of Stat.
     */
    const Stat & stat() const noexcept { return fsStat; }

private:
    const char *rPath;
    Stat fsStat;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::LSTAT.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::LSTAT>
        : Event<FsEvent<details::UVFsType::LSTAT>>
{
    FsEvent(const char *p, const Stat &s) noexcept
        : rPath{p}, fsStat(s)
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets an initialized instance of Stat.
     * @return An initialized instance of Stat.
     */
    const Stat & stat() const noexcept { return fsStat; }

private:
    const char *rPath;
    Stat fsStat;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::SCANDIR.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::SCANDIR>
        : Event<FsEvent<details::UVFsType::SCANDIR>>
{
    FsEvent(const char *p, ssize_t s) noexcept
        : rPath{p}, sz{s}
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets the number of directory entries selected.
     * @return The number of directory entries selected.
     */
    ssize_t amount() const noexcept { return sz; }

private:
    const char *rPath;
    const ssize_t sz;
};


/**
 * @brief FsEvent event specialization for FsRequest::Type::READLINK.
 *
 * It will be emitted by FsReq and/or FileReq according with their
 * functionalities.
 */
template<>
struct FsEvent<details::UVFsType::READLINK>
        : Event<FsEvent<details::UVFsType::READLINK>>
{
    explicit FsEvent(const char *p, const char *d, ssize_t l) noexcept
        : rPath{p}, dt{d}, len{l}
    { }

    /**
     * @brief Gets the path affecting the request.
     * @return The path affecting the request.
     */
    const char * path() const noexcept { return rPath; }

    /**
     * @brief Gets the data read from the given path.
     * @return A bunch of data read from the given path.
     */
    const char * data() const noexcept { return dt; }

    /**
     * @brief Gets the amount of data read from the given path.
     * @return The amount of data read from the given path.
     */
    ssize_t length() const noexcept { return len; }

private:
    const char *rPath;
    const char *dt;
    const ssize_t len;
};


/**
 * @brief Base class for FsReq and/or FileReq.
 *
 * Not directly instantiable, should not be used by the users of the library.
 */
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


/**
 * @brief The FileReq request.
 *
 * Cross-platform sync and async filesystem operations.<br/>
 * All file operations are run on the threadpool.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs.html)
 * for further details.
 */
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
    /**
     * @brief Creates a new file request.
     * @param args A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created request.
     */
    template<typename... Args>
    static std::shared_ptr<FileReq> create(Args&&... args) {
        return std::shared_ptr<FileReq>{new FileReq{std::forward<Args>(args)...}};
    }

    ~FileReq() noexcept {
        uv_fs_req_cleanup(get<uv_fs_t>());
    }

    /**
     * @brief Async [close](http://linux.die.net/man/2/close).
     *
     * Emit a `FsEvent<FileReq::Type::CLOSE>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     */
    void close() {
        cleanupAndInvoke(&uv_fs_close, parent(), get<uv_fs_t>(), file, &fsCloseCallback);
    }

    /**
     * @brief Sync [close](http://linux.die.net/man/2/close).
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::CLOSE> }`.
     */
    auto closeSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_close, parent(), req, file);
        if(req->result >= 0) { file = BAD_FD; }
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::CLOSE>{req->path});
    }

    /**
     * @brief Async [open](http://linux.die.net/man/2/open).
     *
     * Emit a `FsEvent<FileReq::Type::OPEN>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path A valid path name for a file.
     * @param flags Flags, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     */
    void open(std::string path, int flags, int mode) {
        cleanupAndInvoke(&uv_fs_open, parent(), get<uv_fs_t>(), path.data(), flags, mode, &fsOpenCallback);
    }

    /**
     * @brief Sync [open](http://linux.die.net/man/2/open).
     * @param path A valid path name for a file.
     * @param flags Flags, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::OPEN> }`.
     */
    auto openSync(std::string path, int flags, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_open, parent(), req, path.data(), flags, mode);
        if(req->result >= 0) { file = static_cast<uv_file>(req->result); }
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::OPEN>{req->path});
    }

    /**
     * @brief Async [read](http://linux.die.net/man/2/preadv).
     *
     * Emit a `FsEvent<FileReq::Type::READ>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param offset Offset, as described in the official documentation.
     * @param len Length, as described in the official documentation.
     */
    void read(int64_t offset, unsigned int len) {
        data = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(data.get(), len);
        uv_buf_t bufs[] = { buffer };
        cleanupAndInvoke(&uv_fs_read, parent(), get<uv_fs_t>(), file, bufs, 1, offset, &fsReadCallback);
    }

    /**
     * @brief Sync [read](http://linux.die.net/man/2/preadv).
     * @param offset Offset, as described in the official documentation.
     * @param len Length, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::READ> }`.
     */
    auto readSync(int64_t offset, unsigned int len) {
        data = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(data.get(), len);
        uv_buf_t bufs[] = { buffer };
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_read, parent(), req, file, bufs, 1, offset);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::READ>{req->path, std::move(data), req->result});
    }

    /**
     * @brief Async [write](http://linux.die.net/man/2/pwritev).
     *
     * Emit a `FsEvent<FileReq::Type::WRITE>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param data The data to be written.
     * @param len The lenght of the submitted data.
     * @param offset Offset, as described in the official documentation.
     */
    void write(std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        cleanupAndInvoke(&uv_fs_write, parent(), get<uv_fs_t>(), file, bufs, 1, offset, &fsResultCallback<Type::WRITE>);
    }

    /**
     * @brief Sync [write](http://linux.die.net/man/2/pwritev).
     * @param data The data to be written.
     * @param len The lenght of the submitted data.
     * @param offset Offset, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::WRITE> }`.
     */
    auto writeSync(std::unique_ptr<char[]> data, ssize_t len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(data.get(), len) };
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_write, parent(), get<uv_fs_t>(), file, bufs, 1, offset);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::WRITE>{req->path, req->result});
    }

    /**
     * @brief Async [fstat](http://linux.die.net/man/2/fstat).
     *
     * Emit a `FsEvent<FileReq::Type::FSTAT>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     */
    void fstat() {
        cleanupAndInvoke(&uv_fs_fstat, parent(), get<uv_fs_t>(), file, &fsStatCallback<Type::FSTAT>);
    }

    /**
     * @brief Sync [fstat](http://linux.die.net/man/2/fstat).
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::FSTAT> }`.
     */
    auto fstatSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fstat, parent(), req, file);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FSTAT>{req->path, req->statbuf});
    }

    /**
     * @brief Async [fsync](http://linux.die.net/man/2/fsync).
     *
     * Emit a `FsEvent<FileReq::Type::FSYNC>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     */
    void fsync() {
        cleanupAndInvoke(&uv_fs_fsync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FSYNC>);
    }

    /**
     * @brief Sync [fsync](http://linux.die.net/man/2/fsync).
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::FSYNC> }`.
     */
    auto fsyncSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fsync, parent(), req, file);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FSYNC>{req->path});
    }

    /**
     * @brief Async [fdatasync](http://linux.die.net/man/2/fdatasync).
     *
     * Emit a `FsEvent<FileReq::Type::FDATASYNC>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     */
    void fdatasync() {
        cleanupAndInvoke(&uv_fs_fdatasync, parent(), get<uv_fs_t>(), file, &fsGenericCallback<Type::FDATASYNC>);
    }

    /**
     * @brief Sync [fdatasync](http://linux.die.net/man/2/fdatasync).
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::FDATASYNC> }`.
     */
    auto fdatasyncSync() {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fdatasync, parent(), req, file);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FDATASYNC>{req->path});
    }

    /**
     * @brief Async [ftruncate](http://linux.die.net/man/2/ftruncate).
     *
     * Emit a `FsEvent<FileReq::Type::FTRUNCATE>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param offset Offset, as described in the official documentation.
     */
    void ftruncate(int64_t offset) {
        cleanupAndInvoke(&uv_fs_ftruncate, parent(), get<uv_fs_t>(), file, offset, &fsGenericCallback<Type::FTRUNCATE>);
    }

    /**
     * @brief Sync [ftruncate](http://linux.die.net/man/2/ftruncate).
     * @param offset Offset, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::FTRUNCATE> }`.
     */
    auto ftruncateSync(int64_t offset) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_ftruncate, parent(), req, file, offset);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FTRUNCATE>{req->path});
    }

    /**
     * @brief Async [sendfile](http://linux.die.net/man/2/sendfile).
     *
     * Emit a `FsEvent<FileReq::Type::SENDFILE>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param out A valid instance of FileHandle.
     * @param offset Offset, as described in the official documentation.
     * @param length Length, as described in the official documentation.
     */
    void sendfile(FileHandle out, int64_t offset, size_t length) {
        cleanupAndInvoke(&uv_fs_sendfile, parent(), get<uv_fs_t>(), out, file, offset, length, &fsResultCallback<Type::SENDFILE>);
    }

    /**
     * @brief Sync [sendfile](http://linux.die.net/man/2/sendfile).
     * @param out A valid instance of FileHandle.
     * @param offset Offset, as described in the official documentation.
     * @param length Length, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::SENDFILE> }`.
     */
    auto sendfileSync(FileHandle out, int64_t offset, size_t length) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_sendfile, parent(), req, out, file, offset, length);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::SENDFILE>{req->path, req->result});
    }

    /**
     * @brief Async [fchmod](http://linux.die.net/man/2/fchmod).
     *
     * Emit a `FsEvent<FileReq::Type::FCHMOD>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param mode Mode, as described in the official documentation.
     */
    void fchmod(int mode) {
        cleanupAndInvoke(&uv_fs_fchmod, parent(), get<uv_fs_t>(), file, mode, &fsGenericCallback<Type::FCHMOD>);
    }

    /**
     * @brief Sync [fchmod](http://linux.die.net/man/2/fchmod).
     * @param mode Mode, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::FCHMOD> }`.
     */
    auto fchmodSync(int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fchmod, parent(), req, file, mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FCHMOD>{req->path});
    }

    /**
     * @brief Async [futime](http://linux.die.net/man/2/futime).
     *
     * Emit a `FsEvent<FileReq::Type::FUTIME>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param atime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     * @param mtime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     */
    void futime(Time atime, Time mtime) {
        cleanupAndInvoke(&uv_fs_futime, parent(), get<uv_fs_t>(), file, atime.count(), mtime.count(), &fsGenericCallback<Type::FUTIME>);
    }

    /**
     * @brief Sync [futime](http://linux.die.net/man/2/futime).
     * @param atime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     * @param mtime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::FUTIME> }`.
     */
    auto futimeSync(Time atime, Time mtime) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_futime, parent(), req, file, atime.count(), mtime.count());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FUTIME>{req->path});
    }

    /**
     * @brief Async [fchown](http://linux.die.net/man/2/fchown).
     *
     * Emit a `FsEvent<FileReq::Type::FCHOWN>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     */
    void fchown(Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_fchown, parent(), get<uv_fs_t>(), file, uid, gid, &fsGenericCallback<Type::FCHOWN>);
    }

    /**
     * @brief Sync [fchown](http://linux.die.net/man/2/fchown).
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::FCHOWN> }`.
     */
    auto fchownSync(Uid uid, Gid gid) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_fchown, parent(), req, file, uid, gid);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::FCHOWN>{req->path});
    }

    /**
     * @brief Cast operator to FileHandle.
     *
     * Cast operator to an internal representation of the underlying file
     * handle.
     *
     * @return A valid instance of FileHandle (the descriptor can be invalid).
     */
    operator FileHandle() const noexcept { return file; }

private:
    std::unique_ptr<char[]> data{nullptr};
    uv_buf_t buffer{};
    uv_file file{BAD_FD};
};


/**
 * @brief The FsReq request.
 *
 * Cross-platform sync and async filesystem operations.<br/>
 * All file operations are run on the threadpool.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs.html)
 * for further details.
 */
class FsReq final: public FsRequest<FsReq> {
    static void fsReadlinkCallback(uv_fs_t *req) {
        auto ptr = reserve(reinterpret_cast<uv_req_t*>(req));
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr), req->result}); }
    }

    using FsRequest::FsRequest;

public:
    /**
     * @brief Creates a new file request.
     * @param args A pointer to the loop from which the handle generated.
     * @return A pointer to the newly created request.
     */
    template<typename... Args>
    static std::shared_ptr<FsReq> create(Args&&... args) {
        return std::shared_ptr<FsReq>{new FsReq{std::forward<Args>(args)...}};
    }

    ~FsReq() noexcept {
        uv_fs_req_cleanup(get<uv_fs_t>());
    }

    /**
     * @brief Async [unlink](http://linux.die.net/man/2/unlink).
     *
     * Emit a `FsEvent<FileReq::Type::UNLINK>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     */
    void unlink(std::string path) {
        cleanupAndInvoke(&uv_fs_unlink, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::UNLINK>);
    }

    /**
     * @brief Sync [unlink](http://linux.die.net/man/2/unlink).
     * @param path Path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::UNLINK> }`.
     */
    auto unlinkSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_unlink, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::UNLINK>{req->path});
    }

    /**
     * @brief Async [mkdir](http://linux.die.net/man/2/mkdir).
     *
     * Emit a `FsEvent<FileReq::Type::MKDIR>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     */
    void mkdir(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_mkdir, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::MKDIR>);
    }

    /**
     * @brief Sync [mkdir](http://linux.die.net/man/2/mkdir).
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::MKDIR> }`.
     */
    auto mkdirSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_mkdir, parent(), req, path.data(), mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::MKDIR>{req->path});
    }

    /**
     * @brief Async [mktemp](http://linux.die.net/man/3/mkdtemp).
     *
     * Emit a `FsEvent<FileReq::Type::MKDTEMP>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param tpl Template, as described in the official documentation.
     */
    void mkdtemp(std::string tpl) {
        cleanupAndInvoke(&uv_fs_mkdtemp, parent(), get<uv_fs_t>(), tpl.data(), &fsGenericCallback<Type::MKDTEMP>);
    }

    /**
     * @brief Sync [mktemp](http://linux.die.net/man/3/mkdtemp).
     * @param tpl Template, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::MKDTEMP> }`.
     */
    auto mkdtempSync(std::string tpl) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_mkdtemp, parent(), req, tpl.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::MKDTEMP>{req->path});
    }

    /**
     * @brief Async [rmdir](http://linux.die.net/man/2/rmdir).
     *
     * Emit a `FsEvent<FileReq::Type::RMDIR>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     */
    void rmdir(std::string path) {
        cleanupAndInvoke(&uv_fs_rmdir, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::RMDIR>);
    }

    /**
     * @brief Sync [rmdir](http://linux.die.net/man/2/rmdir).
     * @param path Path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::RMDIR> }`.
     */
    auto rmdirSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_rmdir, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::RMDIR>{req->path});
    }

    /**
     * @brief Async [scandir](http://linux.die.net/man/3/scandir).
     *
     * Emit a `FsEvent<FileReq::Type::SCANDIR>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     * @param flags Flags, as described in the official documentation.
     */
    void scandir(std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_scandir, parent(), get<uv_fs_t>(), path.data(), flags, &fsResultCallback<Type::SCANDIR>);
    }

    /**
     * @brief Sync [scandir](http://linux.die.net/man/3/scandir).
     * @param path Path, as described in the official documentation.
     * @param flags Flags, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::SCANDIR> }`.
     */
    auto scandirSync(std::string path, int flags) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_scandir, parent(), req, path.data(), flags);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::SCANDIR>{req->path, req->result});
    }

    /**
     * @brief Gets entries populated with the next directory entry data.
     *
     * Returns instances of Entry, that is an alias for a pair where:
     *
     * * The first parameter indicates the entry type (see below).
     * * The second parameter is a `std::string` that contains the actual value.
     *
     * Available entry types are:
     *
     * * FsReq::EntryType::UNKNOWN
     * * FsReq::EntryType::FILE
     * * FsReq::EntryType::DIR
     * * FsReq::EntryType::LINK
     * * FsReq::EntryType::FIFO
     * * FsReq::EntryType::SOCKET
     * * FsReq::EntryType::CHAR
     * * FsReq::EntryType::BLOCK
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/fs.html#c.uv_dirent_t)
     * for further details.
     *
     * @return A pair where:
     *
     * * The first parameter is a boolean value that indicates if the current
     * entry is still valid.
     * * The second parameter is an instance of Entry (see above).
     */
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

    /**
     * @brief Async [stat](http://linux.die.net/man/2/stat).
     *
     * Emit a `FsEvent<FileReq::Type::STAT>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     */
    void stat(std::string path) {
        cleanupAndInvoke(&uv_fs_stat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback<Type::STAT>);
    }

    /**
     * @brief Sync [stat](http://linux.die.net/man/2/stat).
     * @param path Path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::STAT> }`.
     */
    auto statSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_stat, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::STAT>{req->path, req->statbuf});
    }

    /**
     * @brief Async [lstat](http://linux.die.net/man/2/lstat).
     *
     * Emit a `FsEvent<FileReq::Type::LSTAT>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     */
    void lstat(std::string path) {
        cleanupAndInvoke(&uv_fs_lstat, parent(), get<uv_fs_t>(), path.data(), &fsStatCallback<Type::LSTAT>);
    }

    /**
     * @brief Sync [lstat](http://linux.die.net/man/2/lstat).
     * @param path Path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::LSTAT> }`.
     */
    auto lstatSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_lstat, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::LSTAT>{req->path, req->statbuf});
    }

    /**
     * @brief Async [rename](http://linux.die.net/man/2/rename).
     *
     * Emit a `FsEvent<FileReq::Type::RENAME>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     */
    void rename(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_rename, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::RENAME>);
    }

    /**
     * @brief Sync [rename](http://linux.die.net/man/2/rename).
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::RENAME> }`.
     */
    auto renameSync(std::string old, std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_rename, parent(), req, old.data(), path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::RENAME>{req->path});
    }

    /**
     * @brief Async [access](http://linux.die.net/man/2/access).
     *
     * Emit a `FsEvent<FileReq::Type::ACCESS>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     */
    void access(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_access, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::ACCESS>);
    }

    /**
     * @brief Sync [access](http://linux.die.net/man/2/access).
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::ACCESS> }`.
     */
    auto accessSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_access, parent(), req, path.data(), mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::ACCESS>{req->path});
    }

    /**
     * @brief Async [chmod](http://linux.die.net/man/2/chmod).
     *
     * Emit a `FsEvent<FileReq::Type::CHMOD>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     */
    void chmod(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_chmod, parent(), get<uv_fs_t>(), path.data(), mode, &fsGenericCallback<Type::CHMOD>);
    }

    /**
     * @brief Sync [chmod](http://linux.die.net/man/2/chmod).
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::CHMOD> }`.
     */
    auto chmodSync(std::string path, int mode) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_chmod, parent(), req, path.data(), mode);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::CHMOD>{req->path});
    }

    /**
     * @brief Async [utime](http://linux.die.net/man/2/utime).
     *
     * Emit a `FsEvent<FileReq::Type::UTIME>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     * @param atime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     * @param mtime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     */
    void utime(std::string path, Time atime, Time mtime) {
        cleanupAndInvoke(&uv_fs_utime, parent(), get<uv_fs_t>(), path.data(), atime.count(), mtime.count(), &fsGenericCallback<Type::UTIME>);
    }

    /**
     * @brief Sync [utime](http://linux.die.net/man/2/utime).
     * @param path Path, as described in the official documentation.
     * @param atime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     * @param mtime `std::chrono::seconds`, having the same meaning as described
     * in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::UTIME> }`.
     */
    auto utimeSync(std::string path, Time atime, Time mtime) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_utime, parent(), req, path.data(), atime.count(), mtime.count());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::UTIME>{req->path});
    }

    /**
     * @brief Async [link](http://linux.die.net/man/2/link).
     *
     * Emit a `FsEvent<FileReq::Type::LINK>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     */
    void link(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_link, parent(), get<uv_fs_t>(), old.data(), path.data(), &fsGenericCallback<Type::LINK>);
    }

    /**
     * @brief Sync [link](http://linux.die.net/man/2/link).
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::LINK> }`.
     */
    auto linkSync(std::string old, std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_link, parent(), req, old.data(), path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::LINK>{req->path});
    }

    /**
     * @brief Async [symlink](http://linux.die.net/man/2/symlink).
     *
     * Emit a `FsEvent<FileReq::Type::SYMLINK>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @param flags Flags, as described in the official documentation.
     */
    void symlink(std::string old, std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_symlink, parent(), get<uv_fs_t>(), old.data(), path.data(), flags, &fsGenericCallback<Type::SYMLINK>);
    }

    /**
     * @brief Sync [symlink](http://linux.die.net/man/2/symlink).
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @param flags Flags, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::SYMLINK> }`.
     */
    auto symlinkSync(std::string old, std::string path, int flags) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_symlink, parent(), req, old.data(), path.data(), flags);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::SYMLINK>{req->path});
    }

    /**
     * @brief Async [readlink](http://linux.die.net/man/2/readlink).
     *
     * Emit a `FsEvent<FileReq::Type::READLINK>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     */
    void readlink(std::string path) {
        cleanupAndInvoke(&uv_fs_readlink, parent(), get<uv_fs_t>(), path.data(), &fsReadlinkCallback);
    }

    /**
     * @brief Sync [readlink](http://linux.die.net/man/2/readlink).
     * @param path Path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::READLINK> }`.
     */
    auto readlinkSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_readlink, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr), req->result});
    }

    /**
     * @brief Async [realpath](http://linux.die.net/man/3/realpath).
     *
     * Emit a `FsEvent<FileReq::Type::REALPATH>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     */
    void realpath(std::string path) {
        cleanupAndInvoke(&uv_fs_realpath, parent(), get<uv_fs_t>(), path.data(), &fsGenericCallback<Type::REALPATH>);
    }

    /**
     * @brief Sync [realpath](http://linux.die.net/man/3/realpath).
     * @param path Path, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::REALPATH> }`.
     */
    auto realpathSync(std::string path) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_realpath, parent(), req, path.data());
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::REALPATH>{req->path});
    }

    /**
     * @brief Async [chown](http://linux.die.net/man/2/chown).
     *
     * Emit a `FsEvent<FileReq::Type::CHOWN>` event when completed.<br/>
     * Emit an ErrorEvent event in case of errors.
     *
     * @param path Path, as described in the official documentation.
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     */
    void chown(std::string path, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_chown, parent(), get<uv_fs_t>(), path.data(), uid, gid, &fsGenericCallback<Type::CHOWN>);
    }

    /**
     * @brief Sync [chown](http://linux.die.net/man/2/chown).
     * @param path Path, as described in the official documentation.
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     * @return A pair `{ ErrorEvent, FsEvent<FileReq::Type::CHOWN> }`.
     */
    auto chownSync(std::string path, Uid uid, Gid gid) {
        auto req = get<uv_fs_t>();
        cleanupAndInvokeSync(&uv_fs_chown, parent(), req, path.data(), uid, gid);
        return std::make_pair(ErrorEvent{req->result}, FsEvent<Type::CHOWN>{req->path});
    }
};


}
