#ifndef UVW_FS_INCLUDE_H
#define UVW_FS_INCLUDE_H

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <uv.h>
#include "config.h"
#include "enum.hpp"
#include "loop.h"
#include "request.hpp"
#include "util.h"

namespace uvw {

namespace details {

enum class uvw_fs_type : std::underlying_type_t<uv_fs_type> {
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
    REALPATH = UV_FS_REALPATH,
    COPYFILE = UV_FS_COPYFILE,
    LCHOWN = UV_FS_LCHOWN,
    OPENDIR = UV_FS_OPENDIR,
    READDIR = UV_FS_READDIR,
    CLOSEDIR = UV_FS_CLOSEDIR,
    STATFS = UV_FS_STATFS,
    MKSTEMP = UV_FS_MKSTEMP,
    LUTIME = UV_FS_LUTIME
};

enum class uvw_dirent_type_t : std::underlying_type_t<uv_dirent_type_t> {
    UNKNOWN = UV_DIRENT_UNKNOWN,
    FILE = UV_DIRENT_FILE,
    DIR = UV_DIRENT_DIR,
    LINK = UV_DIRENT_LINK,
    FIFO = UV_DIRENT_FIFO,
    SOCKET = UV_DIRENT_SOCKET,
    CHAR = UV_DIRENT_CHAR,
    BLOCK = UV_DIRENT_BLOCK
};

enum class uvw_file_open_flags : int {
    APPEND = UV_FS_O_APPEND,
    CREAT = UV_FS_O_CREAT,
    DIRECT = UV_FS_O_DIRECT,
    DIRECTORY = UV_FS_O_DIRECTORY,
    DSYNC = UV_FS_O_DSYNC,
    EXCL = UV_FS_O_EXCL,
    EXLOCK = UV_FS_O_EXLOCK,
    FILEMAP = UV_FS_O_FILEMAP,
    NOATIME = UV_FS_O_NOATIME,
    NOCTTY = UV_FS_O_NOCTTY,
    NOFOLLOW = UV_FS_O_NOFOLLOW,
    NONBLOCK = UV_FS_O_NONBLOCK,
    RANDOM = UV_FS_O_RANDOM,
    RDONLY = UV_FS_O_RDONLY,
    RDWR = UV_FS_O_RDWR,
    SEQUENTIAL = UV_FS_O_SEQUENTIAL,
    SHORT_LIVED = UV_FS_O_SHORT_LIVED,
    SYMLINK = UV_FS_O_SYMLINK,
    SYNC = UV_FS_O_SYNC,
    TEMPORARY = UV_FS_O_TEMPORARY,
    TRUNC = UV_FS_O_TRUNC,
    WRONLY = UV_FS_O_WRONLY,
    _UVW_ENUM = 0
};

enum class uvw_copy_file_flags : int {
    EXCL = UV_FS_COPYFILE_EXCL,
    FICLONE = UV_FS_COPYFILE_FICLONE,
    FICLONE_FORCE = UV_FS_COPYFILE_FICLONE_FORCE,
    _UVW_ENUM = 0
};

enum class uvw_symlink_flags : int {
    DIR = UV_FS_SYMLINK_DIR,
    JUNCTION = UV_FS_SYMLINK_JUNCTION,
    _UVW_ENUM = 0
};

} // namespace details

/**
 * @brief Common fs event.
 *
 * Available types are:
 *
 * * `fs_request::fs_type::UNKNOWN`
 * * `fs_request::fs_type::CUSTOM`
 * * `fs_request::fs_type::OPEN`
 * * `fs_request::fs_type::CLOSE`
 * * `fs_request::fs_type::READ`
 * * `fs_request::fs_type::WRITE`
 * * `fs_request::fs_type::SENDFILE`
 * * `fs_request::fs_type::STAT`
 * * `fs_request::fs_type::LSTAT`
 * * `fs_request::fs_type::FSTAT`
 * * `fs_request::fs_type::FTRUNCATE`
 * * `fs_request::fs_type::UTIME`
 * * `fs_request::fs_type::FUTIME`
 * * `fs_request::fs_type::ACCESS`
 * * `fs_request::fs_type::CHMOD`
 * * `fs_request::fs_type::FCHMOD`
 * * `fs_request::fs_type::FSYNC`
 * * `fs_request::fs_type::FDATASYNC`
 * * `fs_request::fs_type::UNLINK`
 * * `fs_request::fs_type::RMDIR`
 * * `fs_request::fs_type::MKDIR`
 * * `fs_request::fs_type::MKDTEMP`
 * * `fs_request::fs_type::RENAME`
 * * `fs_request::fs_type::SCANDIR`
 * * `fs_request::fs_type::LINK`
 * * `fs_request::fs_type::SYMLINK`
 * * `fs_request::fs_type::READLINK`
 * * `fs_request::fs_type::CHOWN`
 * * `fs_request::fs_type::FCHOWN`
 * * `fs_request::fs_type::REALPATH`
 * * `fs_request::fs_type::COPYFILE`
 * * `fs_request::fs_type::LCHOWN`
 * * `fs_request::fs_type::OPENDIR`
 * * `fs_request::fs_type::READDIR`
 * * `fs_request::fs_type::CLOSEDIR`
 * * `fs_request::fs_type::STATFS`
 * * `fs_request::fs_type::MKSTEMP`
 * * `fs_request::fs_type::LUTIME`
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs.html#c.uv_fs_type)
 * for further details.
 */
struct fs_event {
    using fs_type = details::uvw_fs_type;
    using entry_type = details::uvw_dirent_type_t;

    fs_event(const uv_fs_t &req, std::unique_ptr<const char[]> data)
        : fs_event{req} {
        read.data = std::move(data);
    }

    fs_event(const uv_fs_t &req)
        : type{req.fs_type},
          path{req.path},
          result{static_cast<std::size_t>(req.result)} {
        switch(type) {
        case fs_type::STAT:
        case fs_type::LSTAT:
        case fs_type::FSTAT:
            stat = *static_cast<file_info *>(req.ptr);
            break;
        case fs_type::READLINK:
            readlink.data = static_cast<char *>(req.ptr);
            break;
        case fs_type::READDIR:
            dirent.name = static_cast<uv_dir_t *>(req.ptr)->dirents[0].name;
            dirent.type = static_cast<entry_type>(static_cast<uv_dir_t *>(req.ptr)->dirents[0].type);
            dirent.eos = !req.result;
            break;
        case fs_type::STATFS:
            statfs = *static_cast<fs_info *>(req.ptr);
            break;
        default:
            // nothing to do here
            break;
        }
    }

    fs_type type;       /*!< Actual event type. */
    const char *path;   /*!< The path affecting the request. */
    std::size_t result; /*!< Result value for the specific type. */

    struct {
        std::unique_ptr<const char[]> data; /*!< A bunch of data read from the given path. */
    } read;

    struct {
        const char *data; /*!< The content of a symbolic link. */
    } readlink;

    file_info stat; /*!< An initialized instance of file_info. */
    fs_info statfs; /*!< An initialized instance of fs_info. */

    struct {
        const char *name; /*!< The name of the last entry. */
        entry_type type;  /*!< The entry type. */
        bool eos;         /*!< True if there a no more entries to read. */
    } dirent;
};

/**
 * @brief Base class for fs/file request.
 *
 * Not directly instantiable, should not be used by the users of the library.
 */
template<typename T>
class fs_request: public request<T, uv_fs_t, fs_event> {
protected:
    static void fs_request_callback(uv_fs_t *req) {
        if(auto ptr = request<T, uv_fs_t, fs_event>::reserve(req); req->result < 0) {
            ptr->publish(error_event{req->result});
        } else {
            ptr->publish(fs_event{*req});
        }
    }

public:
    using time = std::chrono::duration<double>;
    using fs_type = details::uvw_fs_type;
    using entry_type = details::uvw_dirent_type_t;

    using request<T, uv_fs_t, fs_event>::request;
};

/**
 * @brief The file request.
 *
 * Cross-platform sync and async filesystem operations.<br/>
 * All file operations are run on the threadpool.
 *
 * To create a `file_req` through a `loop`, no arguments are required.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs.html)
 * for further details.
 */
class file_req final: public fs_request<file_req> {
    static constexpr uv_file BAD_FD = -1;

    static void fs_open_callback(uv_fs_t *req);
    static void fs_close_callback(uv_fs_t *req);
    static void fs_read_callback(uv_fs_t *req);

public:
    using file_open_flags = details::uvw_file_open_flags;

    using fs_request::fs_request;

    ~file_req() noexcept;

    /**
     * @brief Async [close](http://linux.die.net/man/2/close).
     *
     * Emit a `fs_event` event when completed.
     */
    void close();

    /**
     * @brief Sync [close](http://linux.die.net/man/2/close).
     * @return True in case of success, false otherwise.
     */
    bool close_sync();

    /**
     * @brief Async [open](http://linux.die.net/man/2/open).
     *
     * Emit a `fs_event` event when completed.
     *
     * Available flags are:
     *
     * * `file_req::file_open_flags::APPEND`
     * * `file_req::file_open_flags::CREAT`
     * * `file_req::file_open_flags::DIRECT`
     * * `file_req::file_open_flags::DIRECTORY`
     * * `file_req::file_open_flags::DSYNC`
     * * `file_req::file_open_flags::EXCL`
     * * `file_req::file_open_flags::EXLOCK`
     * * `file_req::file_open_flags::FILEMAP`
     * * `file_req::file_open_flags::NOATIME`
     * * `file_req::file_open_flags::NOCTTY`
     * * `file_req::file_open_flags::NOFOLLOW`
     * * `file_req::file_open_flags::NONBLOCK`
     * * `file_req::file_open_flags::RANDOM`
     * * `file_req::file_open_flags::RDONLY`
     * * `file_req::file_open_flags::RDWR`
     * * `file_req::file_open_flags::SEQUENTIAL`
     * * `file_req::file_open_flags::SHORT_LIVED`
     * * `file_req::file_open_flags::SYMLINK`
     * * `file_req::file_open_flags::SYNC`
     * * `file_req::file_open_flags::TEMPORARY`
     * * `file_req::file_open_flags::TRUNC`
     * * `file_req::file_open_flags::WRONLY`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/fs.html#file-open-constants)
     * for further details.
     *
     * @param path A valid path name for a file.
     * @param flags Flags made out of underlying constants.
     * @param mode Mode, as described in the official documentation.
     */
    void open(const std::string &path, file_open_flags flags, int mode);

    /**
     * @brief Sync [open](http://linux.die.net/man/2/open).
     *
     * Available flags are:
     *
     * * `file_req::file_open_flags::APPEND`
     * * `file_req::file_open_flags::CREAT`
     * * `file_req::file_open_flags::DIRECT`
     * * `file_req::file_open_flags::DIRECTORY`
     * * `file_req::file_open_flags::DSYNC`
     * * `file_req::file_open_flags::EXCL`
     * * `file_req::file_open_flags::EXLOCK`
     * * `file_req::file_open_flags::FILEMAP`
     * * `file_req::file_open_flags::NOATIME`
     * * `file_req::file_open_flags::NOCTTY`
     * * `file_req::file_open_flags::NOFOLLOW`
     * * `file_req::file_open_flags::NONBLOCK`
     * * `file_req::file_open_flags::RANDOM`
     * * `file_req::file_open_flags::RDONLY`
     * * `file_req::file_open_flags::RDWR`
     * * `file_req::file_open_flags::SEQUENTIAL`
     * * `file_req::file_open_flags::SHORT_LIVED`
     * * `file_req::file_open_flags::SYMLINK`
     * * `file_req::file_open_flags::SYNC`
     * * `file_req::file_open_flags::TEMPORARY`
     * * `file_req::file_open_flags::TRUNC`
     * * `file_req::file_open_flags::WRONLY`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/fs.html#file-open-constants)
     * for further details.
     *
     * @param path A valid path name for a file.
     * @param flags Flags made out of underlying constants.
     * @param mode Mode, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool open_sync(const std::string &path, file_open_flags flags, int mode);

    /**
     * @brief Async [read](http://linux.die.net/man/2/preadv).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param offset Offset, as described in the official documentation.
     * @param len Length, as described in the official documentation.
     */
    void read(int64_t offset, unsigned int len);

    /**
     * @brief Sync [read](http://linux.die.net/man/2/preadv).
     *
     * @param offset Offset, as described in the official documentation.
     * @param len Length, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::pair` composed as it follows:
     *   * A bunch of data read from the given path.
     *   * The amount of data read from the given path.
     */
    std::pair<bool, std::pair<std::unique_ptr<const char[]>, std::size_t>> read_sync(int64_t offset, unsigned int len);

    /**
     * @brief Async [write](http://linux.die.net/man/2/pwritev).
     *
     * The request takes the ownership of the data and it is in charge of delete
     * them.
     *
     * Emit a `fs_event` event when completed.
     *
     * @param buf The data to be written.
     * @param len The lenght of the submitted data.
     * @param offset Offset, as described in the official documentation.
     */
    void write(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset);

    /**
     * @brief Async [write](http://linux.die.net/man/2/pwritev).
     *
     * The request doesn't take the ownership of the data. Be sure that their
     * lifetime overcome the one of the request.
     *
     * Emit a `fs_event` event when completed.
     *
     * @param buf The data to be written.
     * @param len The lenght of the submitted data.
     * @param offset Offset, as described in the official documentation.
     */
    void write(char *buf, unsigned int len, int64_t offset);

    /**
     * @brief Sync [write](http://linux.die.net/man/2/pwritev).
     *
     * @param buf The data to be written.
     * @param len The lenght of the submitted data.
     * @param offset Offset, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * The amount of data written to the given path.
     */
    std::pair<bool, std::size_t> write_sync(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset);

    /**
     * @brief Async [fstat](http://linux.die.net/man/2/fstat).
     *
     * Emit a `fs_event` event when completed.
     */
    void stat();

    /**
     * @brief Sync [fstat](http://linux.die.net/man/2/fstat).
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * An initialized instance of file_info.
     */
    std::pair<bool, file_info> stat_sync();

    /**
     * @brief Async [fsync](http://linux.die.net/man/2/fsync).
     *
     * Emit a `fs_event` event when completed.
     */
    void sync();

    /**
     * @brief Sync [fsync](http://linux.die.net/man/2/fsync).
     * @return True in case of success, false otherwise.
     */
    bool sync_sync();

    /**
     * @brief Async [fdatasync](http://linux.die.net/man/2/fdatasync).
     *
     * Emit a `fs_event` event when completed.
     */
    void datasync();

    /**
     * @brief Sync [fdatasync](http://linux.die.net/man/2/fdatasync).
     * @return True in case of success, false otherwise.
     */
    bool datasync_sync();

    /**
     * @brief Async [ftruncate](http://linux.die.net/man/2/ftruncate).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param offset Offset, as described in the official documentation.
     */
    void truncate(int64_t offset);

    /**
     * @brief Sync [ftruncate](http://linux.die.net/man/2/ftruncate).
     * @param offset Offset, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool truncate_sync(int64_t offset);

    /**
     * @brief Async [sendfile](http://linux.die.net/man/2/sendfile).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param out A valid instance of file_handle.
     * @param offset Offset, as described in the official documentation.
     * @param length Length, as described in the official documentation.
     */
    void sendfile(file_handle out, int64_t offset, std::size_t length);

    /**
     * @brief Sync [sendfile](http://linux.die.net/man/2/sendfile).
     *
     * @param out A valid instance of file_handle.
     * @param offset Offset, as described in the official documentation.
     * @param length Length, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * The amount of data transferred.
     */
    std::pair<bool, std::size_t> sendfile_sync(file_handle out, int64_t offset, std::size_t length);

    /**
     * @brief Async [fchmod](http://linux.die.net/man/2/fchmod).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param mode Mode, as described in the official documentation.
     */
    void chmod(int mode);

    /**
     * @brief Sync [fchmod](http://linux.die.net/man/2/fchmod).
     * @param mode Mode, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool chmod_sync(int mode);

    /**
     * @brief Async [futime](http://linux.die.net/man/3/futimes).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param atime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @param mtime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     */
    void futime(time atime, time mtime);

    /**
     * @brief Sync [futime](http://linux.die.net/man/3/futimes).
     * @param atime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @param mtime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool futime_sync(time atime, time mtime);

    /**
     * @brief Async [fchown](http://linux.die.net/man/2/fchown).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     */
    void chown(uid_type uid, gid_type gid);

    /**
     * @brief Sync [fchown](http://linux.die.net/man/2/fchown).
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool chown_sync(uid_type uid, gid_type gid);

    /**
     * @brief Cast operator to file_handle.
     *
     * Cast operator to an internal representation of the underlying file
     * handle.
     *
     * @return A valid instance of file_handle (the descriptor can be invalid).
     */
    operator file_handle() const noexcept;

private:
    std::unique_ptr<char[]> current{nullptr};
    uv_buf_t buffer{};
    uv_file file{BAD_FD};
};

/**
 * @brief The fs request.
 *
 * Cross-platform sync and async filesystem operations.<br/>
 * All file operations are run on the threadpool.
 *
 * To create a `fs_req` through a `loop`, no arguments are required.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/fs.html)
 * for further details.
 */
class fs_req final: public fs_request<fs_req> {
public:
    using copy_file_flags = details::uvw_copy_file_flags;
    using symlink_flags = details::uvw_symlink_flags;

    using fs_request::fs_request;

    ~fs_req() noexcept;

    /**
     * @brief Async [unlink](http://linux.die.net/man/2/unlink).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     */
    void unlink(const std::string &path);

    /**
     * @brief Sync [unlink](http://linux.die.net/man/2/unlink).
     * @param path Path, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool unlink_sync(const std::string &path);

    /**
     * @brief Async [mkdir](http://linux.die.net/man/2/mkdir).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     */
    void mkdir(const std::string &path, int mode);

    /**
     * @brief Sync [mkdir](http://linux.die.net/man/2/mkdir).
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool mkdir_sync(const std::string &path, int mode);

    /**
     * @brief Async [mktemp](http://linux.die.net/man/3/mkdtemp).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param tpl Template, as described in the official documentation.
     */
    void mkdtemp(const std::string &tpl);

    /**
     * @brief Sync [mktemp](http://linux.die.net/man/3/mkdtemp).
     *
     * @param tpl Template, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * The actual path of the newly created directory.
     */
    std::pair<bool, const char *> mkdtemp_sync(const std::string &tpl);

    /**
     * @brief Async [mkstemp](https://linux.die.net/man/3/mkstemp).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param tpl Template, as described in the official documentation.
     */
    void mkstemp(const std::string &tpl);

    /**
     * @brief Sync [mkstemp](https://linux.die.net/man/3/mkstemp).
     *
     * Returns a composed value where:
     *
     * * The first parameter indicates the created file path.
     * * The second parameter is the file descriptor as an integer.
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/fs.html#c.uv_fs_mkstemp)
     * for further details.
     *
     * @param tpl Template, as described in the official documentation.
     *
     * @return A pair where:

     * * The first parameter is a boolean value that is true in case of success,
     * false otherwise.
     * * The second parameter is a composed value (see above).
     */
    std::pair<bool, std::pair<std::string, std::size_t>> mkstemp_sync(const std::string &tpl);

    /**
     * @brief Async [lutime](http://linux.die.net/man/3/lutimes).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param atime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @param mtime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     */
    void lutime(const std::string &path, time atime, time mtime);

    /**
     * @brief Sync [lutime](http://linux.die.net/man/3/lutimes).
     * @param path Path, as described in the official documentation.
     * @param atime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @param mtime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool lutime_sync(const std::string &path, time atime, time mtime);

    /**
     * @brief Async [rmdir](http://linux.die.net/man/2/rmdir).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     */
    void rmdir(const std::string &path);

    /**
     * @brief Sync [rmdir](http://linux.die.net/man/2/rmdir).
     * @param path Path, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool rmdir_sync(const std::string &path);

    /**
     * @brief Async [scandir](http://linux.die.net/man/3/scandir).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param flags Flags, as described in the official documentation.
     */
    void scandir(const std::string &path, int flags);

    /**
     * @brief Sync [scandir](http://linux.die.net/man/3/scandir).
     *
     * @param path Path, as described in the official documentation.
     * @param flags Flags, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * The number of directory entries selected.
     */
    std::pair<bool, std::size_t> scandir_sync(const std::string &path, int flags);

    /**
     * @brief Gets entries populated with the next directory entry data.
     *
     * Returns a composed value where:
     *
     * * The first parameter indicates the entry type (see below).
     * * The second parameter is a string that contains the actual value.
     *
     * Available entry types are:
     *
     * * `fs_req::entry_type::UNKNOWN`
     * * `fs_req::entry_type::FILE`
     * * `fs_req::entry_type::DIR`
     * * `fs_req::entry_type::LINK`
     * * `fs_req::entry_type::FIFO`
     * * `fs_req::entry_type::SOCKET`
     * * `fs_req::entry_type::CHAR`
     * * `fs_req::entry_type::BLOCK`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/fs.html#c.uv_dirent_t)
     * for further details.
     *
     * @return A pair where:
     *
     * * The first parameter is a boolean value that indicates if the current
     * entry is still valid.
     * * The second parameter is a composed value (see above).
     */
    std::pair<bool, std::pair<entry_type, const char *>> scandir_next();

    /**
     * @brief Async [stat](http://linux.die.net/man/2/stat).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     */
    void stat(const std::string &path);

    /**
     * @brief Sync [stat](http://linux.die.net/man/2/stat).
     *
     * @param path Path, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * An initialized instance of file_info.
     */
    std::pair<bool, file_info> stat_sync(const std::string &path);

    /**
     * @brief Async [lstat](http://linux.die.net/man/2/lstat).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     */
    void lstat(const std::string &path);

    /**
     * @brief Sync [lstat](http://linux.die.net/man/2/lstat).
     *
     * @param path Path, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * An initialized instance of file_info.
     */
    std::pair<bool, file_info> lstat_sync(const std::string &path);

    /**
     * @brief Async [statfs](http://linux.die.net/man/2/statfs).
     *
     * Emit a `fs_event` event when completed.
     *
     * Any fields in the resulting object that are not supported by the
     * underlying operating system are set to zero.
     *
     * @param path Path, as described in the official documentation.
     */
    void statfs(const std::string &path);

    /**
     * @brief Sync [statfs](http://linux.die.net/man/2/statfs).
     *
     * Any fields in the resulting object that are not supported by the
     * underlying operating system are set to zero.
     *
     * @param path Path, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * An initialized instance of fs_info.
     */
    std::pair<bool, fs_info> statfs_sync(const std::string &path);

    /**
     * @brief Async [rename](http://linux.die.net/man/2/rename).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     */
    void rename(const std::string &old, const std::string &path);

    /**
     * @brief Sync [rename](http://linux.die.net/man/2/rename).
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool rename_sync(const std::string &old, const std::string &path);

    /**
     * @brief Copies a file asynchronously from a path to a new one.
     *
     * Emit a `fs_event` event when completed.
     *
     * Available flags are:
     *
     * * `fs_req::copy_file_flags::EXCL`: it fails if the destination path
     * already exists (the default behavior is to overwrite the destination if
     * it exists).
     * * `fs_req::copy_file_flags::FICLONE`: If present, it will attempt to
     * create a copy-on-write reflink. If the underlying platform does not
     * support copy-on-write, then a fallback copy mechanism is used.
     * * `fs_req::copy_file_flags::FICLONE_FORCE`: If present, it will attempt
     * to create a copy-on-write reflink. If the underlying platform does not
     * support copy-on-write, then an error is returned.
     *
     * @warning
     * If the destination path is created, but an error occurs while copying the
     * data, then the destination path is removed. There is a brief window of
     * time between closing and removing the file where another process could
     * access the file.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @param flags Optional additional flags.
     */
    void copyfile(const std::string &old, const std::string &path, copy_file_flags flags = copy_file_flags::_UVW_ENUM);

    /**
     * @brief Copies a file synchronously from a path to a new one.
     *
     * Available flags are:
     *
     * * `fs_req::copy_file_flags::EXCL`: it fails if the destination path
     * already exists (the default behavior is to overwrite the destination if
     * it exists).
     *
     * If the destination path is created, but an error occurs while copying the
     * data, then the destination path is removed. There is a brief window of
     * time between closing and removing the file where another process could
     * access the file.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @param flags Optional additional flags.
     * @return True in case of success, false otherwise.
     */
    bool copyfile_sync(const std::string &old, const std::string &path, copy_file_flags flags = copy_file_flags::_UVW_ENUM);

    /**
     * @brief Async [access](http://linux.die.net/man/2/access).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     */
    void access(const std::string &path, int mode);

    /**
     * @brief Sync [access](http://linux.die.net/man/2/access).
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool access_sync(const std::string &path, int mode);

    /**
     * @brief Async [chmod](http://linux.die.net/man/2/chmod).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     */
    void chmod(const std::string &path, int mode);

    /**
     * @brief Sync [chmod](http://linux.die.net/man/2/chmod).
     * @param path Path, as described in the official documentation.
     * @param mode Mode, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool chmod_sync(const std::string &path, int mode);

    /**
     * @brief Async [utime](http://linux.die.net/man/2/utime).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param atime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @param mtime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     */
    void utime(const std::string &path, time atime, time mtime);

    /**
     * @brief Sync [utime](http://linux.die.net/man/2/utime).
     * @param path Path, as described in the official documentation.
     * @param atime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @param mtime `std::chrono::duration<double>`, having the same meaning as
     * described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool utime_sync(const std::string &path, time atime, time mtime);

    /**
     * @brief Async [link](http://linux.die.net/man/2/link).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     */
    void link(const std::string &old, const std::string &path);

    /**
     * @brief Sync [link](http://linux.die.net/man/2/link).
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool link_sync(const std::string &old, const std::string &path);

    /**
     * @brief Async [symlink](http://linux.die.net/man/2/symlink).
     *
     * Emit a `fs_event` event when completed.
     *
     * Available flags are:
     *
     * * `fs_req::symlink_flags::DIR`: it indicates that the old path points to
     * a directory.
     * * `fs_req::symlink_flags::JUNCTION`: it requests that the symlink is
     * created using junction points.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @param flags Optional additional flags.
     */
    void symlink(const std::string &old, const std::string &path, symlink_flags flags = symlink_flags::_UVW_ENUM);

    /**
     * @brief Sync [symlink](http://linux.die.net/man/2/symlink).
     *
     * Available flags are:
     *
     * * `fs_req::symlink_flags::DIR`: it indicates that the old path points to
     * a directory.
     * * `fs_req::symlink_flags::JUNCTION`: it requests that the symlink is
     * created using junction points.
     *
     * @param old Old path, as described in the official documentation.
     * @param path New path, as described in the official documentation.
     * @param flags Flags, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool symlink_sync(const std::string &old, const std::string &path, symlink_flags flags = symlink_flags::_UVW_ENUM);

    /**
     * @brief Async [readlink](http://linux.die.net/man/2/readlink).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     */
    void readlink(const std::string &path);

    /**
     * @brief Sync [readlink](http://linux.die.net/man/2/readlink).
     *
     * @param path Path, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * A `std::pair` composed as it follows:
     *   * A bunch of data read from the given path.
     *   * The amount of data read from the given path.
     */
    std::pair<bool, std::pair<const char *, std::size_t>> readlink_sync(const std::string &path);

    /**
     * @brief Async [realpath](http://linux.die.net/man/3/realpath).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     */
    void realpath(const std::string &path);

    /**
     * @brief Sync [realpath](http://linux.die.net/man/3/realpath).
     *
     * @param path Path, as described in the official documentation.
     *
     * @return A `std::pair` composed as it follows:
     * * A boolean value that is true in case of success, false otherwise.
     * * The canonicalized absolute pathname.
     */
    std::pair<bool, const char *> realpath_sync(const std::string &path);

    /**
     * @brief Async [chown](http://linux.die.net/man/2/chown).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     */
    void chown(const std::string &path, uid_type uid, gid_type gid);

    /**
     * @brief Sync [chown](http://linux.die.net/man/2/chown).
     * @param path Path, as described in the official documentation.
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool chown_sync(const std::string &path, uid_type uid, gid_type gid);

    /**
     * @brief Async [lchown](https://linux.die.net/man/2/lchown).
     *
     * Emit a `fs_event` event when completed.
     *
     * @param path Path, as described in the official documentation.
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     */
    void lchown(const std::string &path, uid_type uid, gid_type gid);

    /**
     * @brief Sync [lchown](https://linux.die.net/man/2/lchown).
     * @param path Path, as described in the official documentation.
     * @param uid UID, as described in the official documentation.
     * @param gid GID, as described in the official documentation.
     * @return True in case of success, false otherwise.
     */
    bool lchown_sync(const std::string &path, uid_type uid, gid_type gid);

    /**
     * @brief Opens a path asynchronously as a directory stream.
     *
     * Emit a `fs_event` event when completed.
     *
     * The contents of the directory can be iterated over by means of the
     * `readdir` od `readdir_sync` member functions. The memory allocated by
     * this function must be freed by calling `closedir` or `closedir_sync`.
     *
     * @param path The path to open as a directory stream.
     */
    void opendir(const std::string &path);

    /**
     * @brief Opens a path synchronously as a directory stream.
     *
     * The contents of the directory can be iterated over by means of the
     * `readdir` od `readdir_sync` member functions. The memory allocated by
     * this function must be freed by calling `closedir` or `closedir_sync`.
     *
     * @param path The path to open as a directory stream.
     * @return True in case of success, false otherwise.
     */
    bool opendir_sync(const std::string &path);

    /**
     * @brief Closes asynchronously a directory stream.
     *
     * Emit a `fs_event` event when completed.
     *
     * It frees also the memory allocated internally when a path has been opened
     * as a directory stream.
     */
    void closedir();

    /**
     * @brief Closes synchronously a directory stream.
     *
     * It frees also the memory allocated internally when a path has been opened
     * as a directory stream.
     *
     * @return True in case of success, false otherwise.
     */
    bool closedir_sync();

    /**
     * @brief Iterates asynchronously over a directory stream one entry at a
     * time.
     *
     * Emit a `fs_event` event when completed.
     *
     * This function isn't thread safe. Moreover, it doesn't return the `.` and
     * `..` entries.
     */
    void readdir();

    /**
     * @brief Iterates synchronously over a directory stream one entry at a
     * time.
     *
     * Returns a composed value where:
     *
     * * The first parameter indicates the entry type (see below).
     * * The second parameter is a string that contains the actual value.
     *
     * Available entry types are:
     *
     * * `fs_req::entry_type::UNKNOWN`
     * * `fs_req::entry_type::FILE`
     * * `fs_req::entry_type::DIR`
     * * `fs_req::entry_type::LINK`
     * * `fs_req::entry_type::FIFO`
     * * `fs_req::entry_type::SOCKET`
     * * `fs_req::entry_type::CHAR`
     * * `fs_req::entry_type::BLOCK`
     *
     * See the official
     * [documentation](http://docs.libuv.org/en/v1.x/fs.html#c.uv_dirent_t)
     * for further details.
     *
     * This function isn't thread safe. Moreover, it doesn't return the `.` and
     * `..` entries.
     *
     * @return A pair where:
     *
     * * The first parameter is a boolean value that indicates if the current
     * entry is still valid.
     * * The second parameter is a composed value (see above).
     */
    std::pair<bool, std::pair<entry_type, const char *>> readdir_sync();

private:
    uv_dirent_t dirents[1];
};

/*! @brief Helper functions. */
struct fs_helper {
    /**
     * @brief Gets the OS dependent handle.
     *
     * For a file descriptor in the C runtime, get the OS-dependent handle. On
     * UNIX, returns the file descriptor as-is. On Windows, this calls a system
     * function.<br/>
     * Note that the return value is still owned by the C runtime, any attempts
     * to close it or to use it after closing the file descriptor may lead to
     * malfunction.
     */
    static os_file_descriptor handle(file_handle file) noexcept;

    /**
     * @brief Gets the file descriptor.
     *
     * For a OS-dependent handle, get the file descriptor in the C runtime. On
     * UNIX, returns the file descriptor as-is. On Windows, this calls a system
     * function.<br/>
     * Note that the return value is still owned by the C runtime, any attempts
     * to close it or to use it after closing the handle may lead to
     * malfunction.
     */
    static file_handle open(os_file_descriptor descriptor) noexcept;
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "fs.cpp"
#endif

#endif // UVW_FS_INCLUDE_H
