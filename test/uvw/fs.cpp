#include <gtest/gtest.h>
#include <uvw.hpp>

#ifdef _WIN32
#include <fcntl.h>
#endif


TEST(FileReq, OpenAndClose) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFsOpenEvent = false;
    bool checkFsCloseEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&checkFsCloseEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFsCloseEvent);
        checkFsCloseEvent = true;
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&checkFsOpenEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFsOpenEvent);
        checkFsOpenEvent = true;
        request.close();
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_WRONLY, 0644);
#else
    request->open(filename, O_CREAT | O_WRONLY, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFsOpenEvent);
    ASSERT_TRUE(checkFsCloseEvent);
}


TEST(FileReq, OpenAndCloseSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_WRONLY, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_WRONLY, 0644));
#endif

    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, RW) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFsWriteEvent = false;
    bool checkFsReadEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::READ>>([&checkFsReadEvent](const auto &event, auto &request) {
        ASSERT_FALSE(checkFsReadEvent);
        ASSERT_EQ(event.data[0], 42);
        checkFsReadEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::WRITE>>([&checkFsWriteEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFsWriteEvent);
        checkFsWriteEvent = true;
        request.read(0, 1);
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.write(std::unique_ptr<char[]>{new char[1]{ 42 }}, 1, 0);
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFsWriteEvent);
    ASSERT_TRUE(checkFsReadEvent);
}


TEST(FileReq, RWSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    auto writeR = request->writeSync(std::unique_ptr<char[]>{new char[1]{ 42 }}, 1, 0);

    ASSERT_TRUE(writeR.first);
    ASSERT_EQ(writeR.second, 1);

    auto readR = request->readSync(0, 1);

    ASSERT_TRUE(readR.first);
    ASSERT_EQ(readR.second.first[0], 42);
    ASSERT_EQ(readR.second.second, 1);

    ASSERT_TRUE(request->closeSync());

    loop->run();
}



TEST(FileReq, Stat) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFsStatEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSTAT>>([&checkFsStatEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFsStatEvent);
        checkFsStatEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.stat();
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFsStatEvent);
}



TEST(FileReq, StatSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    auto statR = request->statSync();

    ASSERT_TRUE(statR.first);

    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Sync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFsSyncEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSYNC>>([&checkFsSyncEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFsSyncEvent);
        checkFsSyncEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.sync();
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFsSyncEvent);
}


TEST(FileReq, SyncSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    ASSERT_TRUE(request->syncSync());
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Datasync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFsDatasyncEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FDATASYNC>>([&checkFsDatasyncEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFsDatasyncEvent);
        checkFsDatasyncEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.datasync();
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFsDatasyncEvent);
}


TEST(FileReq, DatasyncSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    ASSERT_TRUE(request->datasyncSync());
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


/*
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
