#ifdef UVW_BUILD_STATIC_LIB
#include "fs.h"
#endif //UVW_BUILD_STATIC_LIB
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void FileReq::fsOpenCallback(uv_fs_t *req) {
        auto ptr = reserve(req);

        if (req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        }
        else {
            ptr->file = static_cast<uv_file>(req->result);
            ptr->publish(FsEvent<Type::OPEN>{req->path});
        }
    }

    UVW_INLINE_SPECIFIER void FileReq::fsCloseCallback(uv_fs_t *req) {
        auto ptr = reserve(req);

        if (req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        }
        else {
            ptr->file = BAD_FD;
            ptr->publish(FsEvent<Type::CLOSE>{req->path});
        }
    }

    UVW_INLINE_SPECIFIER void FileReq::fsReadCallback(uv_fs_t *req) {
        auto ptr = reserve(req);
        if (req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        }
        else {
            ptr->publish(
                    FsEvent<Type::READ>{req->path, std::move(ptr->current), static_cast<std::size_t>(req->result)});
        }
    }

    UVW_INLINE_SPECIFIER FileReq::~FileReq() noexcept {
        uv_fs_req_cleanup(get());
    }

    UVW_INLINE_SPECIFIER void FileReq::close() {
        cleanupAndInvoke(&uv_fs_close, parent(), get(), file, &fsCloseCallback);
    }

    UVW_INLINE_SPECIFIER bool FileReq::closeSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_close, parent(), req, file);
        if (req->result >= 0) {
            file = BAD_FD;
        }
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FileReq::open(std::string path, Flags<FileOpen> flags, int mode) {
        cleanupAndInvoke(&uv_fs_open, parent(), get(), path.data(), flags, mode, &fsOpenCallback);
    }

    UVW_INLINE_SPECIFIER bool FileReq::openSync(std::string path, Flags<FileOpen> flags, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_open, parent(), req, path.data(), flags, mode);
        if (req->result >= 0) {
            file = static_cast<uv_file>(req->result);
        }
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FileReq::read(int64_t offset, unsigned int len) {
        current = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(current.get(), len);
        uv_buf_t bufs[] = {buffer};
        cleanupAndInvoke(&uv_fs_read, parent(), get(), file, bufs, 1, offset, &fsReadCallback);
    }

    UVW_INLINE_SPECIFIER std::pair<bool, std::pair<std::unique_ptr<const char[]>, std::size_t>>
    FileReq::readSync(int64_t offset, unsigned int len) {
        current = std::unique_ptr<char[]>{new char[len]};
        buffer = uv_buf_init(current.get(), len);
        uv_buf_t bufs[] = {buffer};
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_read, parent(), req, file, bufs, 1, offset);
        bool err = req->result < 0;
        return std::make_pair(!err, std::make_pair(std::move(current), err ? 0 : std::size_t(req->result)));
    }

    UVW_INLINE_SPECIFIER void FileReq::write(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset) {
        current = std::move(buf);
        uv_buf_t bufs[] = {uv_buf_init(current.get(), len)};
        cleanupAndInvoke(&uv_fs_write, parent(), get(), file, bufs, 1, offset, &fsResultCallback < Type::WRITE > );
    }

    UVW_INLINE_SPECIFIER void FileReq::write(char *buf, unsigned int len, int64_t offset) {
        uv_buf_t bufs[] = {uv_buf_init(buf, len)};
        cleanupAndInvoke(&uv_fs_write, parent(), get(), file, bufs, 1, offset, &fsResultCallback < Type::WRITE > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, std::size_t>
    FileReq::writeSync(std::unique_ptr<char[]> buf, unsigned int len, int64_t offset) {
        current = std::move(buf);
        uv_buf_t bufs[] = {uv_buf_init(current.get(), len)};
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_write, parent(), req, file, bufs, 1, offset);
        bool err = req->result < 0;
        return std::make_pair(!err, err ? 0 : std::size_t(req->result));
    }

    UVW_INLINE_SPECIFIER void FileReq::stat() {
        cleanupAndInvoke(&uv_fs_fstat, parent(), get(), file, &fsStatCallback < Type::FSTAT > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, Stat> FileReq::statSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fstat, parent(), req, file);
        return std::make_pair(!(req->result < 0), req->statbuf);
    }

    UVW_INLINE_SPECIFIER void FileReq::sync() {
        cleanupAndInvoke(&uv_fs_fsync, parent(), get(), file, &fsGenericCallback < Type::FSYNC > );
    }

    UVW_INLINE_SPECIFIER bool FileReq::syncSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fsync, parent(), req, file);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FileReq::datasync() {
        cleanupAndInvoke(&uv_fs_fdatasync, parent(), get(), file, &fsGenericCallback < Type::FDATASYNC > );
    }

    UVW_INLINE_SPECIFIER bool FileReq::datasyncSync() {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fdatasync, parent(), req, file);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FileReq::truncate(int64_t offset) {
        cleanupAndInvoke(&uv_fs_ftruncate, parent(), get(), file, offset, &fsGenericCallback < Type::FTRUNCATE > );
    }

    UVW_INLINE_SPECIFIER bool FileReq::truncateSync(int64_t offset) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_ftruncate, parent(), req, file, offset);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FileReq::sendfile(FileHandle out, int64_t offset, std::size_t length) {
        cleanupAndInvoke(&uv_fs_sendfile, parent(), get(), out, file, offset, length,
                         &fsResultCallback < Type::SENDFILE > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, std::size_t>
    FileReq::sendfileSync(FileHandle out, int64_t offset, std::size_t length) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_sendfile, parent(), req, out, file, offset, length);
        bool err = req->result < 0;
        return std::make_pair(!err, err ? 0 : std::size_t(req->result));
    }

    UVW_INLINE_SPECIFIER void FileReq::chmod(int mode) {
        cleanupAndInvoke(&uv_fs_fchmod, parent(), get(), file, mode, &fsGenericCallback < Type::FCHMOD > );
    }

    UVW_INLINE_SPECIFIER bool FileReq::chmodSync(int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fchmod, parent(), req, file, mode);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FileReq::utime(FsRequest::Time atime, FsRequest::Time mtime) {
        cleanupAndInvoke(&uv_fs_futime, parent(), get(), file, atime.count(), mtime.count(),
                         &fsGenericCallback < Type::FUTIME > );
    }

    UVW_INLINE_SPECIFIER bool FileReq::utimeSync(FsRequest::Time atime, FsRequest::Time mtime) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_futime, parent(), req, file, atime.count(), mtime.count());
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FileReq::chown(Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_fchown, parent(), get(), file, uid, gid, &fsGenericCallback < Type::FCHOWN > );
    }

    UVW_INLINE_SPECIFIER bool FileReq::chownSync(Uid uid, Gid gid) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_fchown, parent(), req, file, uid, gid);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER FileReq::operator FileHandle() const noexcept {
        return file;
    }

    UVW_INLINE_SPECIFIER void FsReq::fsReadlinkCallback(uv_fs_t *req) {
        auto ptr = reserve(req);
        if (req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        }
        else {
            ptr->publish(FsEvent<Type::READLINK>{req->path, static_cast<char *>(req->ptr),
                                                 static_cast<std::size_t>(req->result)});
        }
    }

    UVW_INLINE_SPECIFIER void FsReq::fsReaddirCallback(uv_fs_t *req) {
        auto ptr = reserve(req);

        if (req->result < 0) {
            ptr->publish(ErrorEvent{req->result});
        }
        else {
            auto *dir = static_cast<uv_dir_t *>(req->ptr);
            ptr->publish(FsEvent<Type::READDIR>{dir->dirents[0].name, static_cast<EntryType>(dir->dirents[0].type),
                                                !req->result});
        }
    }

    UVW_INLINE_SPECIFIER FsReq::~FsReq() noexcept {
        uv_fs_req_cleanup(get());
    }

    UVW_INLINE_SPECIFIER void FsReq::unlink(std::string path) {
        cleanupAndInvoke(&uv_fs_unlink, parent(), get(), path.data(), &fsGenericCallback < Type::UNLINK > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::unlinkSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_unlink, parent(), req, path.data());
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::mkdir(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_mkdir, parent(), get(), path.data(), mode, &fsGenericCallback < Type::MKDIR > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::mkdirSync(std::string path, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_mkdir, parent(), req, path.data(), mode);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::mkdtemp(std::string tpl) {
        cleanupAndInvoke(&uv_fs_mkdtemp, parent(), get(), tpl.data(), &fsGenericCallback < Type::MKDTEMP > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, const char *> FsReq::mkdtempSync(std::string tpl) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_mkdtemp, parent(), req, tpl.data());
        return std::make_pair(!(req->result < 0), req->path);
    }

    UVW_INLINE_SPECIFIER void FsReq::mkstemp(std::string tpl) {
        cleanupAndInvoke(&uv_fs_mkstemp, parent(), get(), tpl.data(), &fsResultCallback < Type::MKSTEMP > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, std::pair<std::string, std::size_t>> FsReq::mkstempSync(std::string tpl) {
        std::pair<bool, std::pair<std::string, std::size_t>> ret{false, {}};
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_mkdtemp, parent(), req, tpl.data());
        ret.first = !(req->result < 0);

        if (ret.first) {
            ret.second.first = req->path;
            ret.second.second = static_cast<std::size_t>(req->result);
        }

        return ret;
    }

    UVW_INLINE_SPECIFIER void FsReq::rmdir(std::string path) {
        cleanupAndInvoke(&uv_fs_rmdir, parent(), get(), path.data(), &fsGenericCallback < Type::RMDIR > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::rmdirSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_rmdir, parent(), req, path.data());
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::scandir(std::string path, int flags) {
        cleanupAndInvoke(&uv_fs_scandir, parent(), get(), path.data(), flags, &fsResultCallback < Type::SCANDIR > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, std::size_t> FsReq::scandirSync(std::string path, int flags) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_scandir, parent(), req, path.data(), flags);
        bool err = req->result < 0;
        return std::make_pair(!err, err ? 0 : std::size_t(req->result));
    }

    UVW_INLINE_SPECIFIER void FsReq::stat(std::string path) {
        cleanupAndInvoke(&uv_fs_stat, parent(), get(), path.data(), &fsStatCallback < Type::STAT > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, Stat> FsReq::statSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_stat, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), req->statbuf);
    }

    UVW_INLINE_SPECIFIER void FsReq::lstat(std::string path) {
        cleanupAndInvoke(&uv_fs_lstat, parent(), get(), path.data(), &fsStatCallback < Type::LSTAT > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, Stat> FsReq::lstatSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_lstat, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), req->statbuf);
    }

    UVW_INLINE_SPECIFIER void FsReq::stasfs(std::string path) {
        cleanupAndInvoke(&uv_fs_statfs, parent(), get(), path.data(), &fsStatfsCallback);
    }

    UVW_INLINE_SPECIFIER std::pair<bool, Statfs> FsReq::statfsSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_statfs, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), *static_cast<uv_statfs_t *>(req->ptr));
    }

    UVW_INLINE_SPECIFIER void FsReq::rename(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_rename, parent(), get(), old.data(), path.data(), &fsGenericCallback < Type::RENAME > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::renameSync(std::string old, std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_rename, parent(), req, old.data(), path.data());
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::copyfile(std::string old, std::string path, Flags<CopyFile> flags) {
        cleanupAndInvoke(&uv_fs_copyfile, parent(), get(), old.data(), path.data(), flags,
                         &fsGenericCallback < Type::COPYFILE > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::copyfileSync(std::string old, std::string path, Flags<CopyFile> flags) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_copyfile, parent(), get(), old.data(), path.data(), flags);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::access(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_access, parent(), get(), path.data(), mode, &fsGenericCallback < Type::ACCESS > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::accessSync(std::string path, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_access, parent(), req, path.data(), mode);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::chmod(std::string path, int mode) {
        cleanupAndInvoke(&uv_fs_chmod, parent(), get(), path.data(), mode, &fsGenericCallback < Type::CHMOD > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::chmodSync(std::string path, int mode) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_chmod, parent(), req, path.data(), mode);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::utime(std::string path, FsRequest::Time atime, FsRequest::Time mtime) {
        cleanupAndInvoke(&uv_fs_utime, parent(), get(), path.data(), atime.count(), mtime.count(),
                         &fsGenericCallback < Type::UTIME > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::utimeSync(std::string path, FsRequest::Time atime, FsRequest::Time mtime) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_utime, parent(), req, path.data(), atime.count(), mtime.count());
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::link(std::string old, std::string path) {
        cleanupAndInvoke(&uv_fs_link, parent(), get(), old.data(), path.data(), &fsGenericCallback < Type::LINK > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::linkSync(std::string old, std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_link, parent(), req, old.data(), path.data());
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::symlink(std::string old, std::string path, Flags<SymLink> flags) {
        cleanupAndInvoke(&uv_fs_symlink, parent(), get(), old.data(), path.data(), flags,
                         &fsGenericCallback < Type::SYMLINK > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::symlinkSync(std::string old, std::string path, Flags<SymLink> flags) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_symlink, parent(), req, old.data(), path.data(), flags);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::readlink(std::string path) {
        cleanupAndInvoke(&uv_fs_readlink, parent(), get(), path.data(), &fsReadlinkCallback);
    }

    UVW_INLINE_SPECIFIER std::pair<bool, std::pair<const char *, std::size_t>> FsReq::readlinkSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_readlink, parent(), req, path.data());
        bool err = req->result < 0;
        return std::make_pair(!err, std::make_pair(static_cast<char *>(req->ptr), err ? 0 : std::size_t(req->result)));
    }

    UVW_INLINE_SPECIFIER void FsReq::realpath(std::string path) {
        cleanupAndInvoke(&uv_fs_realpath, parent(), get(), path.data(), &fsGenericCallback < Type::REALPATH > );
    }

    UVW_INLINE_SPECIFIER std::pair<bool, const char *> FsReq::realpathSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_realpath, parent(), req, path.data());
        return std::make_pair(!(req->result < 0), req->path);
    }

    UVW_INLINE_SPECIFIER void FsReq::chown(std::string path, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_chown, parent(), get(), path.data(), uid, gid, &fsGenericCallback < Type::CHOWN > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::chownSync(std::string path, Uid uid, Gid gid) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_chown, parent(), req, path.data(), uid, gid);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::lchown(std::string path, Uid uid, Gid gid) {
        cleanupAndInvoke(&uv_fs_lchown, parent(), get(), path.data(), uid, gid, &fsGenericCallback < Type::LCHOWN > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::lchownSync(std::string path, Uid uid, Gid gid) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_lchown, parent(), req, path.data(), uid, gid);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::opendir(std::string path) {
        cleanupAndInvoke(&uv_fs_opendir, parent(), get(), path.data(), &fsGenericCallback < Type::OPENDIR > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::opendirSync(std::string path) {
        auto req = get();
        cleanupAndInvokeSync(&uv_fs_opendir, parent(), req, path.data());
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::closedir() {
        auto req = get();
        auto *dir = static_cast<uv_dir_t *>(req->ptr);
        cleanupAndInvoke(&uv_fs_closedir, parent(), req, dir, &fsGenericCallback < Type::CLOSEDIR > );
    }

    UVW_INLINE_SPECIFIER bool FsReq::closedirSync() {
        auto req = get();
        auto *dir = static_cast<uv_dir_t *>(req->ptr);
        cleanupAndInvokeSync(&uv_fs_closedir, parent(), req, dir);
        return !(req->result < 0);
    }

    UVW_INLINE_SPECIFIER void FsReq::readdir() {
        auto req = get();
        auto *dir = static_cast<uv_dir_t *>(req->ptr);
        dir->dirents = dirents;
        dir->nentries = 1;
        cleanupAndInvoke(&uv_fs_readdir, parent(), req, dir, &fsReaddirCallback);
    }

    UVW_INLINE_SPECIFIER OSFileDescriptor FsHelper::handle(FileHandle file) noexcept {
        return uv_get_osfhandle(file);
    }

    UVW_INLINE_SPECIFIER FileHandle FsHelper::open(OSFileDescriptor descriptor) noexcept {
        return uv_open_osfhandle(descriptor);
    }
}
