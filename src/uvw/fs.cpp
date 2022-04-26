#ifdef UVW_AS_LIB
#    include "fs.h"
#endif

#include "config.h"

namespace uvw {

UVW_INLINE void file_req::fs_open_callback(uv_fs_t *req) {
    if(auto ptr = reserve(req); req->result < 0) {
        ptr->publish(error_event{req->result});
    } else {
        ptr->file = static_cast<uv_file>(req->result);
        ptr->publish(fs_event{*req});
    }
}

UVW_INLINE void file_req::fs_close_callback(uv_fs_t *req) {
    if(auto ptr = reserve(req); req->result < 0) {
        ptr->publish(error_event{req->result});
    } else {
        ptr->file = BAD_FD;
        ptr->publish(fs_event{*req});
    }
}

UVW_INLINE void file_req::fs_read_callback(uv_fs_t *req) {
    if(auto ptr = reserve(req); req->result < 0) {
        ptr->publish(error_event{req->result});
    } else {
        ptr->publish(fs_event{*req, std::move(ptr->current)});
    }
}

UVW_INLINE file_req::~file_req() noexcept {
    uv_fs_req_cleanup(raw());
}

UVW_INLINE void file_req::close() {
    uv_fs_req_cleanup(this->raw());
    uv_fs_close(parent().raw(), raw(), file, &fs_close_callback);
}

UVW_INLINE bool file_req::close_sync() {
    auto req = raw();

    uv_fs_req_cleanup(this->raw());
    uv_fs_close(parent().raw(), req, file, nullptr);

    if(req->result >= 0) {
        file = BAD_FD;
    }

    return !(req->result < 0);
}

UVW_INLINE void file_req::open(const std::string &path, file_open_flags flags, int mode) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_open(parent().raw(), raw(), path.data(), static_cast<int>(flags), mode, &fs_open_callback);
}

UVW_INLINE bool file_req::open_sync(const std::string &path, file_open_flags flags, int mode) {
    auto req = raw();

    uv_fs_req_cleanup(this->raw());
    uv_fs_open(parent().raw(), req, path.data(), static_cast<int>(flags), mode, nullptr);

    if(req->result >= 0) {
        file = static_cast<uv_file>(req->result);
    }

    return !(req->result < 0);
}

UVW_INLINE void file_req::read(int64_t offset, unsigned int len) {
    current = std::unique_ptr<char[]>{new char[len]};
    buffer = uv_buf_init(current.get(), len);
    uv_buf_t bufs[] = {buffer};
    uv_fs_req_cleanup(this->raw());
    uv_fs_read(parent().raw(), raw(), file, bufs, 1, offset, &fs_read_callback);
}

UVW_INLINE std::pair<bool, std::pair<std::unique_ptr<const char[]>, std::size_t>> file_req::read_sync(int64_t offset, unsigned int len) {
    current = std::unique_ptr<char[]>{new char[len]};
    buffer = uv_buf_init(current.get(), len);
    uv_buf_t bufs[] = {buffer};
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_read(parent().raw(), req, file, bufs, 1, offset, nullptr);
    bool err = req->result < 0;
    return std::make_pair(!err, std::make_pair(std::move(current), err ? 0 : std::size_t(req->result)));
}

UVW_INLINE void file_req::write(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset) {
    current = std::move(buf);
    uv_buf_t bufs[] = {uv_buf_init(current.get(), len)};
    uv_fs_req_cleanup(this->raw());
    uv_fs_write(parent().raw(), raw(), file, bufs, 1, offset, &fs_request_callback);
}

UVW_INLINE void file_req::write(char *buf, unsigned int len, int64_t offset) {
    uv_buf_t bufs[] = {uv_buf_init(buf, len)};
    uv_fs_req_cleanup(this->raw());
    uv_fs_write(parent().raw(), raw(), file, bufs, 1, offset, &fs_request_callback);
}

UVW_INLINE std::pair<bool, std::size_t> file_req::write_sync(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset) {
    current = std::move(buf);
    uv_buf_t bufs[] = {uv_buf_init(current.get(), len)};
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_write(parent().raw(), req, file, bufs, 1, offset, nullptr);
    bool err = req->result < 0;
    return std::make_pair(!err, err ? 0 : std::size_t(req->result));
}

UVW_INLINE void file_req::stat() {
    uv_fs_req_cleanup(this->raw());
    uv_fs_fstat(parent().raw(), raw(), file, &fs_request_callback);
}

UVW_INLINE std::pair<bool, file_info> file_req::stat_sync() {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_fstat(parent().raw(), req, file, nullptr);
    return std::make_pair(!(req->result < 0), req->statbuf);
}

UVW_INLINE void file_req::sync() {
    uv_fs_req_cleanup(this->raw());
    uv_fs_fsync(parent().raw(), raw(), file, &fs_request_callback);
}

UVW_INLINE bool file_req::sync_sync() {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_fsync(parent().raw(), req, file, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void file_req::datasync() {
    uv_fs_req_cleanup(this->raw());
    uv_fs_fdatasync(parent().raw(), raw(), file, &fs_request_callback);
}

UVW_INLINE bool file_req::datasync_sync() {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_fdatasync(parent().raw(), req, file, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void file_req::truncate(int64_t offset) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_ftruncate(parent().raw(), raw(), file, offset, &fs_request_callback);
}

UVW_INLINE bool file_req::truncate_sync(int64_t offset) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_ftruncate(parent().raw(), req, file, offset, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void file_req::sendfile(file_handle out, int64_t offset, std::size_t length) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_sendfile(parent().raw(), raw(), out, file, offset, length, &fs_request_callback);
}

UVW_INLINE std::pair<bool, std::size_t> file_req::sendfile_sync(file_handle out, int64_t offset, std::size_t length) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_sendfile(parent().raw(), req, out, file, offset, length, nullptr);
    bool err = req->result < 0;
    return std::make_pair(!err, err ? 0 : std::size_t(req->result));
}

UVW_INLINE void file_req::chmod(int mode) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_fchmod(parent().raw(), raw(), file, mode, &fs_request_callback);
}

UVW_INLINE bool file_req::chmod_sync(int mode) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_fchmod(parent().raw(), req, file, mode, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void file_req::futime(fs_request::time atime, fs_request::time mtime) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_futime(parent().raw(), raw(), file, atime.count(), mtime.count(), &fs_request_callback);
}

UVW_INLINE bool file_req::futime_sync(fs_request::time atime, fs_request::time mtime) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_futime(parent().raw(), req, file, atime.count(), mtime.count(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void file_req::chown(uid_type uid, gid_type gid) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_fchown(parent().raw(), raw(), file, uid, gid, &fs_request_callback);
}

UVW_INLINE bool file_req::chown_sync(uid_type uid, gid_type gid) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_fchown(parent().raw(), req, file, uid, gid, nullptr);
    return !(req->result < 0);
}

UVW_INLINE file_req::operator file_handle() const noexcept {
    return file;
}

UVW_INLINE fs_req::~fs_req() noexcept {
    uv_fs_req_cleanup(raw());
}

UVW_INLINE void fs_req::unlink(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_unlink(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE bool fs_req::unlink_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_unlink(parent().raw(), req, path.data(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::mkdir(const std::string &path, int mode) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_mkdir(parent().raw(), raw(), path.data(), mode, &fs_request_callback);
}

UVW_INLINE bool fs_req::mkdir_sync(const std::string &path, int mode) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_mkdir(parent().raw(), req, path.data(), mode, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::mkdtemp(const std::string &tpl) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_mkdtemp(parent().raw(), raw(), tpl.data(), &fs_request_callback);
}

UVW_INLINE std::pair<bool, const char *> fs_req::mkdtemp_sync(const std::string &tpl) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_mkdtemp(parent().raw(), req, tpl.data(), nullptr);
    return std::make_pair(!(req->result < 0), req->path);
}

UVW_INLINE void fs_req::mkstemp(const std::string &tpl) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_mkstemp(parent().raw(), raw(), tpl.data(), &fs_request_callback);
}

UVW_INLINE std::pair<bool, std::pair<std::string, std::size_t>> fs_req::mkstemp_sync(const std::string &tpl) {
    std::pair<bool, std::pair<std::string, std::size_t>> ret{false, {}};
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_mkstemp(parent().raw(), req, tpl.data(), nullptr);
    ret.first = !(req->result < 0);

    if(ret.first) {
        ret.second.first = req->path;
        ret.second.second = static_cast<std::size_t>(req->result);
    }

    return ret;
}

UVW_INLINE void fs_req::lutime(const std::string &path, time atime, time mtime) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_lutime(parent().raw(), raw(), path.data(), atime.count(), mtime.count(), &fs_request_callback);
}

UVW_INLINE bool fs_req::lutime_sync(const std::string &path, time atime, time mtime) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_lutime(parent().raw(), req, path.data(), atime.count(), mtime.count(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::rmdir(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_rmdir(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE bool fs_req::rmdir_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_rmdir(parent().raw(), req, path.data(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::scandir(const std::string &path, int flags) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_scandir(parent().raw(), raw(), path.data(), flags, &fs_request_callback);
}

UVW_INLINE std::pair<bool, std::size_t> fs_req::scandir_sync(const std::string &path, int flags) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_scandir(parent().raw(), req, path.data(), flags, nullptr);
    bool err = req->result < 0;
    return std::make_pair(!err, err ? 0 : std::size_t(req->result));
}

UVW_INLINE std::pair<bool, std::pair<fs_req::entry_type, const char *>> fs_req::scandir_next() {
    std::pair<bool, std::pair<entry_type, const char *>> ret{false, {entry_type::UNKNOWN, nullptr}};

    uv_fs_req_cleanup(raw());
    auto res = uv_fs_scandir_next(raw(), dirents);

    if(UV_EOF != res) {
        ret.second.first = static_cast<entry_type>(dirents[0].type);
        ret.second.second = dirents[0].name;
        ret.first = true;
    }

    return ret;
}

UVW_INLINE void fs_req::stat(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_stat(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE std::pair<bool, file_info> fs_req::stat_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_stat(parent().raw(), req, path.data(), nullptr);
    return std::make_pair(!(req->result < 0), req->statbuf);
}

UVW_INLINE void fs_req::lstat(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_lstat(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE std::pair<bool, file_info> fs_req::lstat_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_lstat(parent().raw(), req, path.data(), nullptr);
    return std::make_pair(!(req->result < 0), req->statbuf);
}

UVW_INLINE void fs_req::statfs(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_statfs(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE std::pair<bool, fs_info> fs_req::statfs_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_statfs(parent().raw(), req, path.data(), nullptr);
    return std::make_pair(!(req->result < 0), *static_cast<uv_statfs_t *>(req->ptr));
}

UVW_INLINE void fs_req::rename(const std::string &old, const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_rename(parent().raw(), raw(), old.data(), path.data(), &fs_request_callback);
}

UVW_INLINE bool fs_req::rename_sync(const std::string &old, const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_rename(parent().raw(), req, old.data(), path.data(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::copyfile(const std::string &old, const std::string &path, copy_file_flags flags) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_copyfile(parent().raw(), raw(), old.data(), path.data(), static_cast<int>(flags), &fs_request_callback);
}

UVW_INLINE bool fs_req::copyfile_sync(const std::string &old, const std::string &path, copy_file_flags flags) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_copyfile(parent().raw(), raw(), old.data(), path.data(), static_cast<int>(flags), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::access(const std::string &path, int mode) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_access(parent().raw(), raw(), path.data(), mode, &fs_request_callback);
}

UVW_INLINE bool fs_req::access_sync(const std::string &path, int mode) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_access(parent().raw(), req, path.data(), mode, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::chmod(const std::string &path, int mode) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_chmod(parent().raw(), raw(), path.data(), mode, &fs_request_callback);
}

UVW_INLINE bool fs_req::chmod_sync(const std::string &path, int mode) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_chmod(parent().raw(), req, path.data(), mode, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::utime(const std::string &path, fs_request::time atime, fs_request::time mtime) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_utime(parent().raw(), raw(), path.data(), atime.count(), mtime.count(), &fs_request_callback);
}

UVW_INLINE bool fs_req::utime_sync(const std::string &path, fs_request::time atime, fs_request::time mtime) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_utime(parent().raw(), req, path.data(), atime.count(), mtime.count(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::link(const std::string &old, const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_link(parent().raw(), raw(), old.data(), path.data(), &fs_request_callback);
}

UVW_INLINE bool fs_req::link_sync(const std::string &old, const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_link(parent().raw(), req, old.data(), path.data(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::symlink(const std::string &old, const std::string &path, symlink_flags flags) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_symlink(parent().raw(), raw(), old.data(), path.data(), static_cast<int>(flags), &fs_request_callback);
}

UVW_INLINE bool fs_req::symlink_sync(const std::string &old, const std::string &path, symlink_flags flags) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_symlink(parent().raw(), req, old.data(), path.data(), static_cast<int>(flags), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::readlink(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_readlink(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE std::pair<bool, std::pair<const char *, std::size_t>> fs_req::readlink_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_readlink(parent().raw(), req, path.data(), nullptr);
    bool err = req->result < 0;
    return std::make_pair(!err, std::make_pair(static_cast<char *>(req->ptr), err ? 0 : std::size_t(req->result)));
}

UVW_INLINE void fs_req::realpath(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_realpath(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE std::pair<bool, const char *> fs_req::realpath_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_realpath(parent().raw(), req, path.data(), nullptr);
    return std::make_pair(!(req->result < 0), req->path);
}

UVW_INLINE void fs_req::chown(const std::string &path, uid_type uid, gid_type gid) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_chown(parent().raw(), raw(), path.data(), uid, gid, &fs_request_callback);
}

UVW_INLINE bool fs_req::chown_sync(const std::string &path, uid_type uid, gid_type gid) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_chown(parent().raw(), req, path.data(), uid, gid, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::lchown(const std::string &path, uid_type uid, gid_type gid) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_lchown(parent().raw(), raw(), path.data(), uid, gid, &fs_request_callback);
}

UVW_INLINE bool fs_req::lchown_sync(const std::string &path, uid_type uid, gid_type gid) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_lchown(parent().raw(), req, path.data(), uid, gid, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::opendir(const std::string &path) {
    uv_fs_req_cleanup(this->raw());
    uv_fs_opendir(parent().raw(), raw(), path.data(), &fs_request_callback);
}

UVW_INLINE bool fs_req::opendir_sync(const std::string &path) {
    auto req = raw();
    uv_fs_req_cleanup(this->raw());
    uv_fs_opendir(parent().raw(), req, path.data(), nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::closedir() {
    auto req = raw();
    auto *dir = static_cast<uv_dir_t *>(req->ptr);
    uv_fs_req_cleanup(this->raw());
    uv_fs_closedir(parent().raw(), req, dir, &fs_request_callback);
}

UVW_INLINE bool fs_req::closedir_sync() {
    auto req = raw();
    auto *dir = static_cast<uv_dir_t *>(req->ptr);
    uv_fs_req_cleanup(this->raw());
    uv_fs_closedir(parent().raw(), req, dir, nullptr);
    return !(req->result < 0);
}

UVW_INLINE void fs_req::readdir() {
    auto req = raw();
    auto *dir = static_cast<uv_dir_t *>(req->ptr);
    dir->dirents = dirents;
    dir->nentries = 1;
    uv_fs_req_cleanup(this->raw());
    uv_fs_readdir(parent().raw(), req, dir, &fs_request_callback);
}

UVW_INLINE std::pair<bool, std::pair<fs_req::entry_type, const char *>> fs_req::readdir_sync() {
    auto req = raw();
    auto *dir = static_cast<uv_dir_t *>(req->ptr);
    dir->dirents = dirents;
    dir->nentries = 1;
    uv_fs_req_cleanup(this->raw());
    uv_fs_readdir(parent().raw(), req, dir, nullptr);
    return {req->result != 0, {static_cast<entry_type>(dirents[0].type), dirents[0].name}};
}

UVW_INLINE os_file_descriptor fs_helper::handle(file_handle file) noexcept {
    return uv_get_osfhandle(file);
}

UVW_INLINE file_handle fs_helper::open(os_file_descriptor descriptor) noexcept {
    return uv_open_osfhandle(descriptor);
}

} // namespace uvw
