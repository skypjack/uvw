#include "fs.h"

namespace uvw {

    void FileReq::fsOpenCallback(uv_fs_t *req) {
        auto ptr = reserve(req);

        if(req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        } else {
            ptr->file = static_cast<uv_file>(req->result);
            ptr->publish(FsEvent<Type::OPEN>{req->path});
        }
    }

    void FileReq::fsCloseCallback(uv_fs_t *req) {
        auto ptr = reserve(req);

        if(req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        } else {
            ptr->file = BAD_FD;
            ptr->publish(FsEvent<Type::CLOSE>{req->path});
        }
    }

    void FileReq::fsReadCallback(uv_fs_t *req) {
        auto ptr = reserve(req);
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<Type::READ>{req->path, std::move(ptr->current), static_cast<std::size_t>(req->result)}); }
    }

    FileReq::~FileReq() noexcept {
        uv_fs_req_cleanup(get());
    }

    void FileReq::close() {
        cleanupAndInvoke(&uv_fs_close, parent(), get(), file, &fsCloseCallback);
    }

    bool FileReq::closeSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_close, parent(), req, file);
        if(req->result >= 0) { file = BAD_FD; }
        return !(req->result < 0);
    }

    void FileReq::open(std::string path, Flags<FileOpen> flags, int mode) {
        cleanupAndInvoke(&uv_fs_open, parent(), get(), path.data(), flags, mode, &fsOpenCallback);
    }

    bool FileReq::openSync(std::string path, Flags<FileOpen> flags, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_open, parent(), req, path.data(), flags, mode);
        if(req->result >= 0) { file = static_cast<uv_file>(req->result); }
        return !(req->result < 0);
    }

    void FileReq::read(int64_t offset, unsigned int len) {
        current = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(current.get(), len);
        uv_buf_t bufs[] = { buffer };
        cleanupAndInvoke(&uv_fs_read, parent(), get(), file, bufs, 1, offset, &fsReadCallback);
    }

    std::pair<bool, std::pair<std::unique_ptr<const char[]>, std::size_t>>
    FileReq::readSync(int64_t offset, unsigned int len) {
        current = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(current.get(), len);
        uv_buf_t bufs[] = { buffer };
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_read, parent(), req, file, bufs, 1, offset);
        bool err = req->result < 0;
        return std::make_pair(!err, std::make_pair(std::move(current), err ? 0 : std::size_t(req->result)));
    }

    void FileReq::write(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset) {
        current = std::move(buf);
        uv_buf_t bufs[] = { uv_buf_init(current.get(), len) };
        cleanupAndInvoke(&uv_fs_write, parent(), get(), file, bufs, 1, offset, &fsResultCallback<Type::WRITE>);
    }

    void FileReq::write(char *buf, unsigned int len, int64_t offset) {
        uv_buf_t bufs[] = { uv_buf_init(buf, len) };
        cleanupAndInvoke(&uv_fs_write, parent(), get(), file, bufs, 1, offset, &fsResultCallback<Type::WRITE>);
    }

    std::pair<bool, std::size_t> FileReq::writeSync(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset) {
        current = std::move(buf);
        uv_buf_t bufs[] = { uv_buf_init(current.get(), len) };
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_write, parent(), req, file, bufs, 1, offset);
        bool err = req->result < 0;
        return std::make_pair(!err, err ? 0 : std::size_t(req->result));
    }

    void FileReq::stat() {
        cleanupAndInvoke(&uv_fs_fstat, parent(), get(), file, &fsStatCallback<Type::FSTAT>);
    }

    std::pair<bool, Stat> FileReq::statSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fstat, parent(), req, file);
        return std::make_pair(!(req->result < 0), req->statbuf);
    }

    void FileReq::sync() {
        cleanupAndInvoke(&uv_fs_fsync, parent(), get(), file, &fsGenericCallback<Type::FSYNC>);
    }

    bool FileReq::syncSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fsync, parent(), req, file);
        return !(req->result < 0);
    }

    void FileReq::datasync() {
        cleanupAndInvoke(&uv_fs_fdatasync, parent(), get(), file, &fsGenericCallback<Type::FDATASYNC>);
    }

    bool FileReq::datasyncSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fdatasync, parent(), req, file);
        return !(req->result < 0);
    }

    void FileReq::truncate(int64_t offset) {
        cleanupAndInvoke(&uv_fs_ftruncate, parent(), get(), file, offset, &fsGenericCallback<Type::FTRUNCATE>);
    }

    bool FileReq::truncateSync(int64_t offset) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_ftruncate, parent(), req, file, offset);
        return !(req->result < 0);
    }

    void FileReq::sendfile(FileHandle out, int64_t offset, std::size_t length) {
        cleanupAndInvoke(&uv_fs_sendfile, parent(), get(), out, file, offset, length, &fsResultCallback<Type::SENDFILE>);
    }

    std::pair<bool, std::size_t> FileReq::sendfileSync(FileHandle out, int64_t offset, std::size_t length) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_sendfile, parent(), req, out, file, offset, length);
        bool err = req->result < 0;
        return std::make_pair(!err, err ? 0 : std::size_t(req->result));
    }

    void FileReq::chmod(int mode) {
        cleanupAndInvoke(&uv_fs_fchmod, parent(), get(), file, mode, &fsGenericCallback<Type::FCHMOD>);
    }

    bool FileReq::chmodSync(int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fchmod, parent(), req, file, mode);
        return !(req->result < 0);
    }

    void FileReq::utime(FsRequest::Time atime, FsRequest::Time mtime) {
        cleanupAndInvoke(&uv_fs_futime, parent(), get(), file, atime.count(), mtime.count(), &fsGenericCallback<Type::FUTIME>);
    }

    bool FileReq::utimeSync(FsRequest::Time atime, FsRequest::Time mtime) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_futime, parent(), req, file, atime.count(), mtime.count());
        return !(req->result < 0);
    }

    void FileReq::chown(Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_fchown, parent(), get(), file, uid, gid, &fsGenericCallback<Type::FCHOWN>);
    }

    bool FileReq::chownSync(Uid uid, Gid gid) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fchown, parent(), req, file, uid, gid);
        return !(req->result < 0);
    }

    FileReq::operator FileHandle() const noexcept { return file; }

    void FsReq::fsReadlinkCallback(uv_fs_t *req) {
        auto ptr = reserve(req);
        if(req->result < 0) { ptr->publish(ErrorEvent{req->result}); }
        else { ptr->publish(FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr), static_cast<std::size_t>(req->result)}); }
    }

    void FsReq::fsReaddirCallback(uv_fs_t *req) {
        auto ptr = reserve(req);

        if(req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        } else {
            auto *dir = static_cast<uv_dir_t *>(req->ptr);
            ptr->publish(FsEvent<Type::READDIR>{dir->dirents[0].name, static_cast<EntryType>(dir->dirents[0].type), !req->result});
        }
    }

    FsReq::~FsReq() noexcept {
        uv_fs_req_cleanup(get());
    }

    void FsReq::unlink(std::string path) {
        cleanupAndInvoke(&uv_fs_unlink, parent(), get(), path.data(), &fsGenericCallback<Type::UNLINK>);
    }

    bool FsReq::unlinkSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_unlink, parent(), req, path.data());
        return !(req->result < 0);
    }

    void FsReq::mkdir(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_mkdir, parent(), get(), path.data(), mode, &fsGenericCallback<Type::MKDIR>);
    }

    bool FsReq::mkdirSync(std::string path, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_mkdir, parent(), req, path.data(), mode);
        return !(req->result < 0);
    }

    void FsReq::mkdtemp(std::string tpl) {
        cleanupAndInvoke(&uv_fs_mkdtemp, parent(), get(), tpl.data(), &fsGenericCallback<Type::MKDTEMP>);
    }

    std::pair<bool, const char *> FsReq::mkdtempSync(std::string tpl) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_mkdtemp, parent(), req, tpl.data());
        return std::make_pair(!(req->result < 0), req->path);
    }

    void FsReq::mkstemp(std::string tpl) {
        cleanupAndInvoke(&uv_fs_mkstemp, parent(), get(), tpl.data(), &fsResultCallback<Type::MKSTEMP>);
    }

    std::pair<bool, std::pair<std::string, std::size_t>> FsReq::mkstempSync(std::string tpl) {
        std::pair<bool, std::pair<std::string, std::size_t>> ret{false, {}};
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_mkdtemp, parent(), req, tpl.data());
        ret.first = !(req->result < 0);

        if(ret.first) {
            ret.second.first = req->path;
            ret.second.second = static_cast<std::size_t>(req->result);
        }

        return ret;
    }

    void FsReq::rmdir(std::string path) {
        cleanupAndInvoke(&uv_fs_rmdir, parent(), get(), path.data(), &fsGenericCallback<Type::RMDIR>);
    }

    bool FsReq::rmdirSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_rmdir, parent(), req, path.data());
        return !(req->result < 0);
    }

    void FsReq::scandir(std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_scandir, parent(), get(), path.data(), flags, &fsResultCallback<Type::SCANDIR>);
    }

    std::pair<bool, std::size_t> FsReq::scandirSync(std::string path, int flags) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_scandir, parent(), req, path.data(), flags);
        bool err = req->result < 0;
        return std::make_pair(!err, err ? 0 : std::size_t(req->result));
    }

    void FsReq::stat(std::string path) {
        cleanupAndInvoke(&uv_fs_stat, parent(), get(), path.data(), &fsStatCallback<Type::STAT>);
    }

    std::pair<bool, Stat> FsReq::statSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_stat, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), req->statbuf);
    }

    void FsReq::lstat(std::string path) {
        cleanupAndInvoke(&uv_fs_lstat, parent(), get(), path.data(), &fsStatCallback<Type::LSTAT>);
    }

    std::pair<bool, Stat> FsReq::lstatSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_lstat, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), req->statbuf);
    }

    void FsReq::stasfs(std::string path) {
        cleanupAndInvoke(&uv_fs_statfs, parent(), get(), path.data(), &fsStatfsCallback);
    }

    std::pair<bool, Statfs> FsReq::statfsSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_statfs, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), *static_cast<uv_statfs_t *>(req->ptr));
    }

    void FsReq::rename(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_rename, parent(), get(), old.data(), path.data(), &fsGenericCallback<Type::RENAME>);
    }

    bool FsReq::renameSync(std::string old, std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_rename, parent(), req, old.data(), path.data());
        return !(req->result < 0);
    }

    void FsReq::copyfile(std::string old, std::string path, Flags<CopyFile> flags) {
        cleanupAndInvoke(&uv_fs_copyfile, parent(), get(), old.data(), path.data(), flags, &fsGenericCallback<Type::COPYFILE>);
    }

    bool FsReq::copyfileSync(std::string old, std::string path, Flags<CopyFile> flags) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_copyfile, parent(), get(), old.data(), path.data(), flags);
        return !(req->result < 0);
    }

    void FsReq::access(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_access, parent(), get(), path.data(), mode, &fsGenericCallback<Type::ACCESS>);
    }

    bool FsReq::accessSync(std::string path, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_access, parent(), req, path.data(), mode);
        return !(req->result < 0);
    }

    void FsReq::chmod(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_chmod, parent(), get(), path.data(), mode, &fsGenericCallback<Type::CHMOD>);
    }

    bool FsReq::chmodSync(std::string path, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_chmod, parent(), req, path.data(), mode);
        return !(req->result < 0);
    }

    void FsReq::utime(std::string path, FsRequest::Time atime, FsRequest::Time mtime) {
        cleanupAndInvoke(&uv_fs_utime, parent(), get(), path.data(), atime.count(), mtime.count(), &fsGenericCallback<Type::UTIME>);
    }

    bool FsReq::utimeSync(std::string path, FsRequest::Time atime, FsRequest::Time mtime) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_utime, parent(), req, path.data(), atime.count(), mtime.count());
        return !(req->result < 0);
    }

    void FsReq::link(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_link, parent(), get(), old.data(), path.data(), &fsGenericCallback<Type::LINK>);
    }

    bool FsReq::linkSync(std::string old, std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_link, parent(), req, old.data(), path.data());
        return !(req->result < 0);
    }

    void FsReq::symlink(std::string old, std::string path, Flags<SymLink> flags) {
        cleanupAndInvoke(&uv_fs_symlink, parent(), get(), old.data(), path.data(), flags, &fsGenericCallback<Type::SYMLINK>);
    }

    bool FsReq::symlinkSync(std::string old, std::string path, Flags<SymLink> flags) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_symlink, parent(), req, old.data(), path.data(), flags);
        return !(req->result < 0);
    }

    void FsReq::readlink(std::string path) {
        cleanupAndInvoke(&uv_fs_readlink, parent(), get(), path.data(), &fsReadlinkCallback);
    }

    std::pair<bool, std::pair<const char *, std::size_t>> FsReq::readlinkSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_readlink, parent(), req, path.data());
        bool err = req->result < 0;
        return std::make_pair(!err, std::make_pair(static_cast<char *>(req->ptr), err ? 0 : std::size_t(req->result)));
    }

    void FsReq::realpath(std::string path) {
        cleanupAndInvoke(&uv_fs_realpath, parent(), get(), path.data(), &fsGenericCallback<Type::REALPATH>);
    }

    std::pair<bool, const char *> FsReq::realpathSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_realpath, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), req->path);
    }

    void FsReq::chown(std::string path, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_chown, parent(), get(), path.data(), uid, gid, &fsGenericCallback<Type::CHOWN>);
    }

    bool FsReq::chownSync(std::string path, Uid uid, Gid gid) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_chown, parent(), req, path.data(), uid, gid);
        return !(req->result < 0);
    }

    void FsReq::lchown(std::string path, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_lchown, parent(), get(), path.data(), uid, gid, &fsGenericCallback<Type::LCHOWN>);
    }

    bool FsReq::lchownSync(std::string path, Uid uid, Gid gid) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_lchown, parent(), req, path.data(), uid, gid);
        return !(req->result < 0);
    }

    void FsReq::opendir(std::string path) {
        cleanupAndInvoke(&uv_fs_opendir, parent(), get(), path.data(), &fsGenericCallback<Type::OPENDIR>);
    }

    bool FsReq::opendirSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_opendir, parent(), req, path.data());
        return !(req->result < 0);
    }

    void FsReq::closedir() {
        auto req = get();
        auto *dir = static_cast<uv_dir_t *>(req->ptr);
        cleanupAndInvoke(&uv_fs_closedir, parent(), req, dir, &fsGenericCallback<Type::CLOSEDIR>);
    }

    bool FsReq::closedirSync() {
        auto req = get();
        auto *dir = static_cast<uv_dir_t *>(req->ptr);
        cleanupAndInvokeSync(&uv_fs_closedir, parent(), req, dir);
        return !(req->result < 0);
    }

    void FsReq::readdir() {
        auto req = get();
        auto *dir = static_cast<uv_dir_t *>(req->ptr);
        dir->dirents = dirents;
        dir->nentries = 1;
        cleanupAndInvoke(&uv_fs_readdir, parent(), req, dir, &fsReaddirCallback);
    }

    OSFileDescriptor FsHelper::handle(FileHandle file) noexcept {
        return uv_get_osfhandle(file);
    }

    FileHandle FsHelper::open(OSFileDescriptor descriptor) noexcept {
        return uv_open_osfhandle(descriptor);
    }
}
