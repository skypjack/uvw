#include <gtest/gtest.h>
#include <uvw.hpp>
#include <chrono>

#ifdef _WIN32
#include <fcntl.h>
#endif


TEST(FileReq, OpenAndClose) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
*/


TEST(FileReq, Chmod) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileChmodEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FCHMOD>>([&checkFileChmodEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileChmodEvent);
        checkFileChmodEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.chmod(0644);
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFileChmodEvent);
}


TEST(FileReq, ChmodSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    ASSERT_TRUE(request->chmodSync(0644));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Utime) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileUtimeEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FUTIME>>([&checkFileUtimeEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileUtimeEvent);
        checkFileUtimeEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        request.utime(value, value);
    });

#ifdef _WIN32
    request->open(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644);
#else
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#endif

    loop->run();

    ASSERT_TRUE(checkFileUtimeEvent);
}


TEST(FileReq, UtimeSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

#ifdef _WIN32
    ASSERT_TRUE(request->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    ASSERT_TRUE(request->utimeSync(value, value));
    ASSERT_TRUE(request->truncateSync(0));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


/*
TEST(FileReq, Chown) {
    // TODO
}


TEST(FileReq, ChownSync) {
    // TODO
}
*/


TEST(FsReq, MkdirAndRmdir) {
    const std::string dirname = std::string{TARGET_FS_DIR} + std::string{"/test.dir"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    bool checkFsMkdirEvent = false;
    bool checkFsRmdirEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::RMDIR>>([&checkFsRmdirEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsRmdirEvent);
        checkFsRmdirEvent = true;
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::MKDIR>>([&checkFsMkdirEvent, &dirname](const auto &, auto &request) {
        ASSERT_FALSE(checkFsMkdirEvent);
        checkFsMkdirEvent = true;
        request.rmdir(dirname);
    });

    request->mkdir(dirname, 0755);

    loop->run();

    ASSERT_TRUE(checkFsMkdirEvent);
    ASSERT_TRUE(checkFsRmdirEvent);
}


TEST(FsReq, MkdirAndRmdirSync) {
    const std::string dirname = std::string{TARGET_FS_DIR} + std::string{"/test.dir"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(request->mkdirSync(dirname, 0755));
    ASSERT_TRUE(request->rmdirSync(dirname));

    loop->run();
}


TEST(FsReq, Mkdtemp) {
    const std::string dirname = std::string{TARGET_FS_DIR} + std::string{"/test.dir.XXXXXX"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    bool checkFsMkdtempEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::MKDTEMP>>([&checkFsMkdtempEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsMkdtempEvent);
        checkFsMkdtempEvent = true;
    });

    request->mkdtemp(dirname);

    loop->run();

    ASSERT_TRUE(checkFsMkdtempEvent);
}


TEST(FsReq, MkdtempSync) {
    const std::string dirname = std::string{TARGET_FS_DIR} + std::string{"/test.dir.XXXXXX"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(request->mkdtempSync(dirname));

    loop->run();
}


/*
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
*/


TEST(FsReq, Rename) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};
    const std::string rename = std::string{TARGET_FS_DIR} + std::string{"/test.rename"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsRenameEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fsReq->on<uvw::FsEvent<uvw::FileReq::Type::RENAME>>([&checkFsRenameEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsRenameEvent);
        checkFsRenameEvent = true;
    });

    fileReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename, &rename](const auto &, auto &) {
        fsReq->rename(filename, rename);
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

    ASSERT_TRUE(checkFsRenameEvent);
}


TEST(FsReq, RenameSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};
    const std::string rename = std::string{TARGET_FS_DIR} + std::string{"/test.rename"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

#ifdef _WIN32
    ASSERT_TRUE(fileReq->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->renameSync(filename, rename));

    loop->run();
}


/*
TEST(FsReq, Access) {
    // TODO
}


TEST(FsReq, AccessSync) {
    // TODO
}
*/


TEST(FsReq, Chmod) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

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


TEST(FsReq, Utime) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsUtimeEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fsReq->on<uvw::FsEvent<uvw::FileReq::Type::UTIME>>([&checkFsUtimeEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsUtimeEvent);
        checkFsUtimeEvent = true;
    });

    fileReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        fsReq->utime(filename, value, value);
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

    ASSERT_TRUE(checkFsUtimeEvent);
}


TEST(FsReq, UtimeSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

#ifdef _WIN32
    ASSERT_TRUE(fileReq->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->utimeSync(filename, value, value));

    loop->run();
}


TEST(FsReq, LinkAndUnlink) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsLinkEvent = false;
    bool checkFsUnlinkEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fsReq->on<uvw::FsEvent<uvw::FileReq::Type::UNLINK>>([&checkFsUnlinkEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsUnlinkEvent);
        checkFsUnlinkEvent = true;
    });

    fsReq->on<uvw::FsEvent<uvw::FileReq::Type::LINK>>([&checkFsLinkEvent, &linkname](const auto &, auto &request) {
        ASSERT_FALSE(checkFsLinkEvent);
        checkFsLinkEvent = true;
        request.unlink(linkname);
    });

    fileReq->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename, &linkname](const auto &, auto &) {
        fsReq->link(filename, linkname);
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

    ASSERT_TRUE(checkFsLinkEvent);
    ASSERT_TRUE(checkFsUnlinkEvent);
}


TEST(FsReq, LinkAndUnlinkSync) {
    const std::string filename = std::string{TARGET_FS_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

#ifdef _WIN32
    ASSERT_TRUE(fileReq->openSync(filename, _O_CREAT | _O_RDWR | _O_TRUNC, 0644));
#else
    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
#endif

    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->linkSync(filename, linkname));
    ASSERT_TRUE(fsReq->unlinkSync(linkname));

    loop->run();
}


/*
TEST(FsReq, SymlinkAndUnlink) {
    // TODO
}


TEST(FsReq, SymlinkAndUnlinkSync) {
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
