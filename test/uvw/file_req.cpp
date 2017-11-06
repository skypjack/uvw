#include <gtest/gtest.h>
#include <uvw.hpp>
#include <chrono>

#ifdef _WIN32
#define _CRT_DECLARE_NONSTDC_NAMES 1
#include <fcntl.h>
#endif


TEST(FileReq, OpenAndCloseErr) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/err.file"};

    auto loop = uvw::Loop::getDefault();
    auto openReq = loop->resource<uvw::FileReq>();
    auto closeReq = loop->resource<uvw::FileReq>();

    bool checkFileOpenErrorEvent = false;
    bool checkFileCloseErrorEvent = false;

    openReq->on<uvw::ErrorEvent>([&checkFileOpenErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFileOpenErrorEvent);
        checkFileOpenErrorEvent = true;
    });

    closeReq->on<uvw::ErrorEvent>([&checkFileCloseErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFileCloseErrorEvent);
        checkFileCloseErrorEvent = true;
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::RDONLY>();
    openReq->open(filename, flags, 0644);
    closeReq->close();

    loop->run();

    ASSERT_TRUE(checkFileOpenErrorEvent);
    ASSERT_TRUE(checkFileCloseErrorEvent);
}


TEST(FileReq, OpenAndCloseErrSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/err.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_FALSE(request->openSync(filename, O_RDONLY, 0644));
    ASSERT_FALSE(request->closeSync());

    loop->run();
}


TEST(FileReq, OpenAndClose) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileOpenEvent = false;
    bool checkFileCloseEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&checkFileCloseEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFileCloseEvent);
        checkFileCloseEvent = true;
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&checkFileOpenEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileOpenEvent);
        checkFileOpenEvent = true;
        req.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::WRONLY>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileOpenEvent);
    ASSERT_TRUE(checkFileCloseEvent);
}


TEST(FileReq, OpenAndCloseSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_WRONLY, 0644));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, RWChecked) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileWriteEvent = false;
    bool checkFileReadEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::READ>>([&checkFileReadEvent](const auto &event, auto &req) {
        ASSERT_FALSE(checkFileReadEvent);
        ASSERT_EQ(event.data[0], 42);
        checkFileReadEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::WRITE>>([&checkFileWriteEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileWriteEvent);
        checkFileWriteEvent = true;
        req.read(0, 1);
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.write(std::unique_ptr<char[]>{new char[1]{ 42 }}, 1, 0);
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileWriteEvent);
    ASSERT_TRUE(checkFileReadEvent);
}


TEST(FileReq, RWUnchecked) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};
    std::unique_ptr<char[]> data{new char[1]{ 42 }};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileWriteEvent = false;
    bool checkFileReadEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::READ>>([&checkFileReadEvent](const auto &event, auto &req) {
        ASSERT_FALSE(checkFileReadEvent);
        ASSERT_EQ(event.data[0], 42);
        checkFileReadEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::WRITE>>([&checkFileWriteEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileWriteEvent);
        checkFileWriteEvent = true;
        req.read(0, 1);
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&data](const auto &, auto &req) {
        req.write(data.get(), 1, 0);
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileWriteEvent);
    ASSERT_TRUE(checkFileReadEvent);
}


TEST(FileReq, RWSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));

    auto writeR = request->writeSync(std::unique_ptr<char[]>{new char[1]{ 42 }}, 1, 0);

    ASSERT_TRUE(writeR.first);
    ASSERT_EQ(writeR.second, std::size_t{1});

    auto readR = request->readSync(0, 1);

    ASSERT_TRUE(readR.first);
    ASSERT_EQ(readR.second.first[0], 42);
    ASSERT_EQ(readR.second.second, std::size_t{1});
    ASSERT_TRUE(request->closeSync());

    loop->run();
}



TEST(FileReq, Stat) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileStatEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSTAT>>([&checkFileStatEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileStatEvent);
        checkFileStatEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.stat();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileStatEvent);
}



TEST(FileReq, StatSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));

    auto statR = request->statSync();

    ASSERT_TRUE(statR.first);
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Sync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileSyncEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSYNC>>([&checkFileSyncEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileSyncEvent);
        checkFileSyncEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.sync();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileSyncEvent);
}


TEST(FileReq, SyncSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(request->syncSync());
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Datasync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileDatasyncEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FDATASYNC>>([&checkFileDatasyncEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileDatasyncEvent);
        checkFileDatasyncEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.datasync();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileDatasyncEvent);
}


TEST(FileReq, DatasyncSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(request->datasyncSync());
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Truncate) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileTruncateEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FTRUNCATE>>([&checkFileTruncateEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileTruncateEvent);
        checkFileTruncateEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.truncate(0);
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileTruncateEvent);
}


TEST(FileReq, TruncateSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(request->truncateSync(0));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, SendFile) {
    const std::string srcFilename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/src.file"};
    const std::string dstFilename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/dst.file"};

    auto loop = uvw::Loop::getDefault();
    auto srcReq = loop->resource<uvw::FileReq>();
    auto dstReq = loop->resource<uvw::FileReq>();

    bool checkFileSendFileEvent = false;

    dstReq->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    srcReq->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    dstReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&srcReq](const auto &, auto &req) {
        srcReq->sendfile(static_cast<uvw::FileHandle>(req), 0, 0);
    });

    srcReq->on<uvw::FsEvent<uvw::FileReq::Type::SENDFILE>>([&checkFileSendFileEvent, &dstReq](const auto &, auto &req) {
        ASSERT_FALSE(checkFileSendFileEvent);
        checkFileSendFileEvent = true;
        dstReq->close();
        req.close();
    });

    srcReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&dstFilename, &dstReq](const auto &, auto &) {
        auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::WRONLY, uvw::FileReq::FileOpen::TRUNC>();
        dstReq->open(dstFilename, flags, 0644);
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDONLY, uvw::FileReq::FileOpen::TRUNC>();
    srcReq->open(srcFilename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileSendFileEvent);
}


TEST(FileReq, SendFileSync) {
    const std::string srcFilename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/src.file"};
    const std::string dstFilename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/dst.file"};

    auto loop = uvw::Loop::getDefault();
    auto srcReq = loop->resource<uvw::FileReq>();
    auto dstReq = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(srcReq->openSync(srcFilename, O_CREAT | O_RDONLY | O_TRUNC, 0644));
    ASSERT_TRUE(dstReq->openSync(dstFilename, O_CREAT | O_WRONLY | O_TRUNC, 0644));

    auto sendfileR = srcReq->sendfileSync(static_cast<uvw::FileHandle>(*dstReq), 0, 0);

    ASSERT_TRUE(sendfileR.first);
    ASSERT_TRUE(srcReq->closeSync());
    ASSERT_TRUE(dstReq->closeSync());

    loop->run();
}


TEST(FileReq, Chmod) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileChmodEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FCHMOD>>([&checkFileChmodEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileChmodEvent);
        checkFileChmodEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.chmod(0644);
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileChmodEvent);
}


TEST(FileReq, ChmodSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(request->chmodSync(0644));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Utime) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileUtimeEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FUTIME>>([&checkFileUtimeEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileUtimeEvent);
        checkFileUtimeEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        req.utime(value, value);
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileUtimeEvent);
}


TEST(FileReq, UtimeSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    ASSERT_TRUE(request->utimeSync(value, value));
    ASSERT_TRUE(request->truncateSync(0));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}


TEST(FileReq, Chown) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFileChownEvent = false;

    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FCHOWN>>([&checkFileChownEvent](const auto &, auto &req) {
        ASSERT_FALSE(checkFileChownEvent);
        checkFileChownEvent = true;
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSTAT>>([](const auto &event, auto &req) {
        auto uid = static_cast<uvw::Uid>(event.stat.st_uid);
        auto gid = static_cast<uvw::Uid>(event.stat.st_gid);
        req.chown(uid, gid);
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.stat();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileChownEvent);
}


TEST(FileReq, ChownSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FileReq>();

    ASSERT_TRUE(request->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));

    auto statR = request->statSync();

    ASSERT_TRUE(statR.first);
    auto uid = static_cast<uvw::Uid>(statR.second.st_uid);
    auto gid = static_cast<uvw::Uid>(statR.second.st_gid);
    ASSERT_TRUE(request->chownSync(uid, gid));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}
