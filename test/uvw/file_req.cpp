#include <gtest/gtest.h>
#include <uvw.hpp>
#include <chrono>

#ifdef _WIN32
#define _CRT_DECLARE_NONSTDC_NAMES 1
#include <fcntl.h>
#endif


TEST(FileReq, OpenAndClose) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

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

    request->open(filename, O_CREAT | O_WRONLY, 0644);

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

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([&data](const auto &, auto &request) {
        request.write(data.get(), 1, 0);
    });

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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


/*
TEST(FileReq, SendFile) {
    // TODO
}


TEST(FileReq, SendFileSync) {
    // TODO
}
*/


TEST(FileReq, Chmod) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

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

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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

    request->on<uvw::ErrorEvent>([](const auto &, auto &) {
        FAIL();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FCHOWN>>([&checkFileChownEvent](const auto &, auto &request) {
        ASSERT_FALSE(checkFileChownEvent);
        checkFileChownEvent = true;
        request.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::FSTAT>>([](const auto &event, auto &request) {
        request.chown(event.stat.st_uid, event.stat.st_gid);
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &request) {
        request.stat();
    });

    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

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
    ASSERT_TRUE(request->chownSync(statR.second.st_uid, statR.second.st_gid));
    ASSERT_TRUE(request->closeSync());

    loop->run();
}
