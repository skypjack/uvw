#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(FileReq, OpenAndClose) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkErrorEvent = false;
    bool checkFsEvent = false;

    request->on<uvw::ErrorEvent>([&checkErrorEvent](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&checkFsEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFsEvent);
        checkFsEvent = true;
        request.close();
    });

#ifdef _WIN32
    auto flags = _O_RDWR | _O_CREAT;
#else
    auto flags = O_RDWR | O_CREAT;
#endif

    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_FALSE(checkErrorEvent);
    ASSERT_TRUE(checkFsEvent);
}


TEST(FileReq, OpenAndCloseSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    auto flags = _O_RDWR | _O_CREAT;
#else
    auto flags = O_RDWR | O_CREAT;
#endif

    ASSERT_TRUE(request->openSync(filename, flags, 0644));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


/*
TEST(FileReq, RW) {
    // TODO
}


TEST(FileReq, RWSync) {
    // TODO
}


TEST(FileReq, Stat) {
    // TODO
}


TEST(FileReq, StatSync) {
    // TODO
}


TEST(FileReq, Sync) {
    // TODO
}


TEST(FileReq, SyncSync) {
    // TODO
}


TEST(FileReq, Datasync) {
    // TODO
}


TEST(FileReq, DatasyncSync) {
    // TODO
}


TEST(FileReq, Truncate) {
    // TODO
}


TEST(FileReq, TruncateSync) {
    // TODO
}


TEST(FileReq, SendFile) {
    // TODO
}


TEST(FileReq, SendFileSync) {
    // TODO
}


TEST(FileReq, Chmod) {
    // TODO
}


TEST(FileReq, ChmodSync) {
    // TODO
}


TEST(FileReq, Utime) {
    // TODO
}


TEST(FileReq, UtimeSync) {
    // TODO
}


TEST(FileReq, Chown) {
    // TODO
}


TEST(FileReq, ChownSync) {
    // TODO
}


TEST(FsReq, Unlink) {
    // TODO
}


TEST(FsReq, UnlinkSync) {
    // TODO
}


TEST(FsReq, Mkdir) {
    // TODO
}


TEST(FsReq, MkdirSync) {
    // TODO
}


TEST(FsReq, Mkdtemp) {
    // TODO
}


TEST(FsReq, MkdtempSync) {
    // TODO
}


TEST(FsReq, Rmdir) {
    // TODO
}


TEST(FsReq, RmdirSync) {
    // TODO
}


TEST(FsReq, Scandir) {
    // TODO
}


TEST(FsReq, ScandirSync) {
    // TODO
}


TEST(FsReq, Stat) {
    // TODO
}


TEST(FsReq, StatSync) {
    // TODO
}


TEST(FsReq, Lstat) {
    // TODO
}


TEST(FsReq, LstatSync) {
    // TODO
}


TEST(FsReq, Rename) {
    // TODO
}


TEST(FsReq, RenameSync) {
    // TODO
}


TEST(FsReq, Access) {
    // TODO
}


TEST(FsReq, AccessSync) {
    // TODO
}


TEST(FsReq, Chmod) {
    // TODO
}


TEST(FsReq, ChmodSync) {
    // TODO
}


TEST(FsReq, Utime) {
    // TODO
}


TEST(FsReq, UtimeSync) {
    // TODO
}


TEST(FsReq, Link) {
    // TODO
}


TEST(FsReq, LinkSync) {
    // TODO
}


TEST(FsReq, Symlink) {
    // TODO
}


TEST(FsReq, SymlinkSync) {
    // TODO
}


TEST(FsReq, Readlink) {
    // TODO
}


TEST(FsReq, ReadlinkSync) {
    // TODO
}


TEST(FsReq, Realpath) {
    // TODO
}


TEST(FsReq, RealpathSync) {
    // TODO
}


TEST(FsReq, Chown) {
    // TODO
}


TEST(FsReq, ChownSync) {
    // TODO
}
*/
