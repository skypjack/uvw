#include <chrono>
#include <gtest/gtest.h>
#include <uvw/fs.h>

#ifdef _WIN32
#    define _CRT_DECLARE_NONSTDC_NAMES 1
#    include <fcntl.h>
#endif

TEST(FileReq, OpenAndCloseErr) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/err.file"};

    auto loop = uvw::loop::get_default();
    auto openReq = loop->resource<uvw::file_req>();
    auto closeReq = loop->resource<uvw::file_req>();

    bool checkFileOpenErrorEvent = false;
    bool checkFileCloseErrorEvent = false;

    openReq->on<uvw::error_event>([&checkFileOpenErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFileOpenErrorEvent);
        checkFileOpenErrorEvent = true;
    });

    closeReq->on<uvw::error_event>([&checkFileCloseErrorEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFileCloseErrorEvent);
        checkFileCloseErrorEvent = true;
    });

    auto flags = uvw::file_req::file_open_flags::RDONLY;
    openReq->open(filename, flags, 0644);
    closeReq->close();

    loop->run();

    ASSERT_TRUE(checkFileOpenErrorEvent);
    ASSERT_TRUE(checkFileCloseErrorEvent);
}

TEST(FileReq, OpenAndCloseErrSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/err.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::RDONLY;

    ASSERT_FALSE(request->open_sync(filename, flags, 0644));
    ASSERT_FALSE(request->close_sync());

    loop->run();
}

TEST(FileReq, OpenAndClose) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileOpenEvent = false;
    bool checkFileCloseEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::CLOSE:
            ASSERT_FALSE(checkFileCloseEvent);
            checkFileCloseEvent = true;
            break;
        case uvw::fs_req::fs_type::OPEN:
            ASSERT_FALSE(checkFileOpenEvent);
            checkFileOpenEvent = true;
            req.close();
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::WRONLY;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileOpenEvent);
    ASSERT_TRUE(checkFileCloseEvent);
}

TEST(FileReq, OpenAndCloseSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::WRONLY;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, RWChecked) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileWriteEvent = false;
    bool checkFileReadEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::OPEN:
            req.write(std::unique_ptr<char[]>{new char[1]{42}}, 1, 0);
            break;
        case uvw::fs_req::fs_type::READ:
            ASSERT_FALSE(checkFileReadEvent);
            ASSERT_EQ(event.read.data[0], 42);
            checkFileReadEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::WRITE:
            ASSERT_FALSE(checkFileWriteEvent);
            checkFileWriteEvent = true;
            req.read(0, 1);
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileWriteEvent);
    ASSERT_TRUE(checkFileReadEvent);
}

TEST(FileReq, RWUnchecked) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};
    std::unique_ptr<char[]> data{new char[1]{42}};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileWriteEvent = false;
    bool checkFileReadEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::OPEN:
            req.write(data.get(), 1, 0);
            break;
        case uvw::fs_req::fs_type::READ:
            ASSERT_FALSE(checkFileReadEvent);
            ASSERT_EQ(event.read.data[0], 42);
            checkFileReadEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::WRITE:
            ASSERT_FALSE(checkFileWriteEvent);
            checkFileWriteEvent = true;
            req.read(0, 1);
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileWriteEvent);
    ASSERT_TRUE(checkFileReadEvent);
}

TEST(FileReq, RWSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));

    auto writeR = request->write_sync(std::unique_ptr<char[]>{new char[1]{42}}, 1, 0);

    ASSERT_TRUE(writeR.first);
    ASSERT_EQ(writeR.second, std::size_t{1});

    auto readR = request->read_sync(0, 1);

    ASSERT_TRUE(readR.first);
    ASSERT_EQ(readR.second.first[0], 42);
    ASSERT_EQ(readR.second.second, std::size_t{1});
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Stat) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileStatEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::OPEN:
            req.stat();
            break;
        case uvw::fs_req::fs_type::FSTAT:
            ASSERT_FALSE(checkFileStatEvent);
            checkFileStatEvent = true;
            req.close();
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileStatEvent);
}

TEST(FileReq, StatSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));

    auto statR = request->stat_sync();

    ASSERT_TRUE(statR.first);
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Sync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileSyncEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::FSYNC:
            ASSERT_FALSE(checkFileSyncEvent);
            checkFileSyncEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::OPEN:
            req.sync();
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileSyncEvent);
}

TEST(FileReq, SyncSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));
    ASSERT_TRUE(request->sync_sync());
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Datasync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileDatasyncEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::FDATASYNC:
            ASSERT_FALSE(checkFileDatasyncEvent);
            checkFileDatasyncEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::OPEN:
            req.datasync();
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileDatasyncEvent);
}

TEST(FileReq, DatasyncSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));
    ASSERT_TRUE(request->datasync_sync());
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Truncate) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileTruncateEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::FTRUNCATE:
            ASSERT_FALSE(checkFileTruncateEvent);
            checkFileTruncateEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::OPEN:
            req.truncate(0);
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileTruncateEvent);
}

TEST(FileReq, TruncateSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));
    ASSERT_TRUE(request->truncate_sync(0));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Chmod) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileChmodEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::FCHMOD:
            ASSERT_FALSE(checkFileChmodEvent);
            checkFileChmodEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::OPEN:
            req.chmod(0644);
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileChmodEvent);
}

TEST(FileReq, ChmodSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));
    ASSERT_TRUE(request->chmod_sync(0644));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Futime) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileUtimeEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        const auto value = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());

        switch(event.type) {
        case uvw::fs_req::fs_type::FUTIME:
            ASSERT_FALSE(checkFileUtimeEvent);
            checkFileUtimeEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::OPEN:
            req.futime(value, value);
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileUtimeEvent);
}

TEST(FileReq, FutimeSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    ASSERT_TRUE(request->futime_sync(value, value));
    ASSERT_TRUE(request->truncate_sync(0));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Chown) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileChownEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::FCHOWN:
            ASSERT_FALSE(checkFileChownEvent);
            checkFileChownEvent = true;
            req.close();
            break;
        case uvw::fs_req::fs_type::FSTAT:
            req.chown(static_cast<uvw::uid_type>(event.stat.st_uid), static_cast<uvw::uid_type>(event.stat.st_gid));
            break;
        case uvw::fs_req::fs_type::OPEN:
            req.stat();
            break;
        default:
            // nothing to do here
            break;
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileChownEvent);
}

TEST(FileReq, ChownSync) {
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, 0644));

    auto statR = request->stat_sync();

    ASSERT_TRUE(statR.first);
    auto uid = static_cast<uvw::uid_type>(statR.second.st_uid);
    auto gid = static_cast<uvw::uid_type>(statR.second.st_gid);
    ASSERT_TRUE(request->chown_sync(uid, gid));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}
