#include <gtest/gtest.h>
#include <uvw.hpp>

#ifdef _WIN32
#include <fcntl.h>
#endif


TEST(FileReq, OpenAndClose) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileOpenEvent = false;
    bool checkFileCloseEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&checkFileCloseEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileCloseEvent);
        checkFileCloseEvent = true;
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&checkFileOpenEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileOpenEvent);
        checkFileOpenEvent = true;
        request.close();
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_WRONLY, 0644);
#else
    request->open(filename, O_CREAT | O_WRONLY, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFileOpenEvent);
    ASSERT_TRUE(checkFileCloseEvent);
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

    bool checkFileWriteEvent = false;
    bool checkFileReadEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::READ>>([&checkFileReadEvent](const auto &event, auto &request) {
        ASSERT_FALSE(checkFileReadEvent);
        ASSERT_EQ(event.data[0], 42);
        checkFileReadEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::WRITE>>([&checkFileWriteEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileWriteEvent);
        checkFileWriteEvent = true;
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

    ASSERT_TRUE(checkFileWriteEvent);
    ASSERT_TRUE(checkFileReadEvent);
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

    bool checkFileStatEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSTAT>>([&checkFileStatEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileStatEvent);
        checkFileStatEvent = true;
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

    ASSERT_TRUE(checkFileStatEvent);
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

    bool checkFileSyncEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSYNC>>([&checkFileSyncEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileSyncEvent);
        checkFileSyncEvent = true;
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

    ASSERT_TRUE(checkFileSyncEvent);
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

    bool checkFileDatasyncEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FDATASYNC>>([&checkFileDatasyncEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileDatasyncEvent);
        checkFileDatasyncEvent = true;
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

    ASSERT_TRUE(checkFileDatasyncEvent);
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


TEST(FileReq, Truncate) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileTruncateEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FTRUNCATE>>([&checkFileTruncateEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileTruncateEvent);
        checkFileTruncateEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.truncate(0);
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFileTruncateEvent);
}


TEST(FileReq, TruncateSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    ASSERT_TRUE(request->truncateSync(0));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


/*
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
*/


TEST(FsReq, Chmod) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsChmodEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fsReq->on<uvw::FsEvent<uvw::FileReq::Type::CHMOD>>([&checkFsChmodEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsChmodEvent);
        checkFsChmodEvent = true;
    });

    fileReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->chmod(filename, 0644);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

#ifdef _WIN32
    fileReq->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    fileReq->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFsChmodEvent);
}


TEST(FsReq, ChmodSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.fs"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

#ifdef _WIN32
    ASSERT_TRUE(fileReq->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->chmodSync(filename, 0644));

    loop->run();
}


/*
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
