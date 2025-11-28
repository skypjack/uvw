#include <chrono>
#include <gtest/gtest.h>
#include <uvw/fs.h>

#ifdef _WIN32
// NOLINTNEXTLINE(bugprone-reserved-identifier,cppcoreguidelines-macro-usage)
#    define _CRT_DECLARE_NONSTDC_NAMES 1
#    include <fcntl.h>
#endif

TEST(FileReq, OpenAndCloseErr) {
    static constexpr auto mode_0644 = 0644;
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
    openReq->open(filename, flags, mode_0644);
    closeReq->close();

    loop->run();

    ASSERT_TRUE(checkFileOpenErrorEvent);
    ASSERT_TRUE(checkFileCloseErrorEvent);
}

TEST(FileReq, OpenAndCloseErrSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/err.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::RDONLY;

    ASSERT_FALSE(request->open_sync(filename, flags, mode_0644));
    ASSERT_FALSE(request->close_sync());

    loop->run();
}

TEST(FileReq, OpenAndClose) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileOpenEvent = false;
    bool checkFileCloseEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            ASSERT_FALSE(checkFileCloseEvent);
            checkFileCloseEvent = true;
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            ASSERT_FALSE(checkFileOpenEvent);
            checkFileOpenEvent = true;
            req.close();
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::WRONLY;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileOpenEvent);
    ASSERT_TRUE(checkFileCloseEvent);
}

TEST(FileReq, OpenAndCloseSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::WRONLY;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, RWChecked) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileWriteEvent = false;
    bool checkFileReadEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.write(std::unique_ptr<char[]>{new char[1]{3}}, 1, 0);
        } else if(event.type == uvw::fs_req::fs_type::READ) {
            ASSERT_FALSE(checkFileReadEvent);
            ASSERT_EQ(event.read.data[0], 3);
            checkFileReadEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::WRITE) {
            ASSERT_FALSE(checkFileWriteEvent);
            checkFileWriteEvent = true;
            req.read(0, 1);
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileWriteEvent);
    ASSERT_TRUE(checkFileReadEvent);
}

TEST(FileReq, RWUnchecked) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};
    std::unique_ptr<char[]> data{new char[1]{3}};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileWriteEvent = false;
    bool checkFileReadEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.write(data.get(), 1, 0);
        } else if(event.type == uvw::fs_req::fs_type::READ) {
            ASSERT_FALSE(checkFileReadEvent);
            ASSERT_EQ(event.read.data[0], 3);
            checkFileReadEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::WRITE) {
            ASSERT_FALSE(checkFileWriteEvent);
            checkFileWriteEvent = true;
            req.read(0, 1);
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileWriteEvent);
    ASSERT_TRUE(checkFileReadEvent);
}

TEST(FileReq, RWSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));

    auto writeR = request->write_sync(std::unique_ptr<char[]>{new char[1]{3}}, 1, 0);

    ASSERT_TRUE(writeR.first);
    ASSERT_EQ(writeR.second, std::size_t{1});

    auto readR = request->read_sync(0, 1);

    ASSERT_TRUE(readR.first);
    ASSERT_EQ(readR.second.first[0], 3);
    ASSERT_EQ(readR.second.second, std::size_t{1});
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Stat) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileStatEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.stat();
        } else if(event.type == uvw::fs_req::fs_type::FSTAT) {
            ASSERT_FALSE(checkFileStatEvent);
            checkFileStatEvent = true;
            req.close();
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileStatEvent);
}

TEST(FileReq, StatSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));

    auto statR = request->stat_sync();

    ASSERT_TRUE(statR.first);
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Sync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileSyncEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::FSYNC) {
            ASSERT_FALSE(checkFileSyncEvent);
            checkFileSyncEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.sync();
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileSyncEvent);
}

TEST(FileReq, SyncSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));
    ASSERT_TRUE(request->sync_sync());
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Datasync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileDatasyncEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::FDATASYNC) {
            ASSERT_FALSE(checkFileDatasyncEvent);
            checkFileDatasyncEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.datasync();
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileDatasyncEvent);
}

TEST(FileReq, DatasyncSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));
    ASSERT_TRUE(request->datasync_sync());
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Truncate) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileTruncateEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::FTRUNCATE) {
            ASSERT_FALSE(checkFileTruncateEvent);
            checkFileTruncateEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.truncate(0);
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileTruncateEvent);
}

TEST(FileReq, TruncateSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));
    ASSERT_TRUE(request->truncate_sync(0));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Chmod) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileChmodEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::FCHMOD) {
            ASSERT_FALSE(checkFileChmodEvent);
            checkFileChmodEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.chmod(mode_0644);
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileChmodEvent);
}

TEST(FileReq, ChmodSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));
    ASSERT_TRUE(request->chmod_sync(mode_0644));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Futime) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileUtimeEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        const auto value = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());

        if(event.type == uvw::fs_req::fs_type::FUTIME) {
            ASSERT_FALSE(checkFileUtimeEvent);
            checkFileUtimeEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.futime(value, value);
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileUtimeEvent);
}

TEST(FileReq, FutimeSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    ASSERT_TRUE(request->futime_sync(value, value));
    ASSERT_TRUE(request->truncate_sync(0));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}

TEST(FileReq, Chown) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();

    bool checkFileChownEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::FCHOWN) {
            ASSERT_FALSE(checkFileChownEvent);
            checkFileChownEvent = true;
            req.close();
        } else if(event.type == uvw::fs_req::fs_type::FSTAT) {
            req.chown(static_cast<uvw::uid_type>(event.stat.st_uid), static_cast<uvw::uid_type>(event.stat.st_gid));
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.stat();
        };
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, mode_0644);

    loop->run();

    ASSERT_TRUE(checkFileChownEvent);
}

TEST(FileReq, ChownSync) {
    static constexpr auto mode_0644 = 0644;
    const std::string filename = std::string{TARGET_FILE_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::file_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(request->open_sync(filename, flags, mode_0644));

    auto statR = request->stat_sync();

    ASSERT_TRUE(statR.first);
    auto uid = static_cast<uvw::uid_type>(statR.second.st_uid);
    auto gid = static_cast<uvw::uid_type>(statR.second.st_gid);
    ASSERT_TRUE(request->chown_sync(uid, gid));
    ASSERT_TRUE(request->close_sync());

    loop->run();
}
