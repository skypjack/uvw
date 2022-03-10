#include <chrono>
#include <gtest/gtest.h>
#include <uvw/fs.h>

#ifdef _WIN32
#    define _CRT_DECLARE_NONSTDC_NAMES 1
#    include <fcntl.h>
#endif

TEST(FsReq, MkdirAndRmdir) {
    const std::string dirname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.dir"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::fs_req>();

    bool checkFsMkdirEvent = false;
    bool checkFsRmdirEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event<uvw::fs_req::fs_type::RMDIR>>([&checkFsRmdirEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsRmdirEvent);
        checkFsRmdirEvent = true;
    });

    request->on<uvw::fs_event<uvw::fs_req::fs_type::MKDIR>>([&checkFsMkdirEvent, &dirname](const auto &, auto &req) {
        ASSERT_FALSE(checkFsMkdirEvent);
        checkFsMkdirEvent = true;
        req.rmdir(dirname);
    });

    request->mkdir(dirname, 0755);

    loop->run();

    ASSERT_TRUE(checkFsMkdirEvent);
    ASSERT_TRUE(checkFsRmdirEvent);
}

TEST(FsReq, MkdirAndRmdirSync) {
    const std::string dirname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.dir"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::fs_req>();

    ASSERT_TRUE(request->mkdir_sync(dirname, 0755));
    ASSERT_TRUE(request->rmdir_sync(dirname));

    loop->run();
}

TEST(FsReq, MkdtempAndRmdir) {
    const std::string dirname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.dir.XXXXXX"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::fs_req>();

    bool checkFsMkdtempEvent = false;
    bool checkFsRmdirEvent = false;

    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    request->on<uvw::fs_event<uvw::fs_req::fs_type::RMDIR>>([&checkFsRmdirEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsRmdirEvent);
        checkFsRmdirEvent = true;
    });

    request->on<uvw::fs_event<uvw::fs_req::fs_type::MKDTEMP>>([&checkFsMkdtempEvent](const auto &event, auto &req) {
        ASSERT_FALSE(checkFsMkdtempEvent);
        ASSERT_NE(event.path, nullptr);
        checkFsMkdtempEvent = true;
        req.rmdir(event.path);
    });

    request->mkdtemp(dirname);

    loop->run();

    ASSERT_TRUE(checkFsMkdtempEvent);
    ASSERT_TRUE(checkFsRmdirEvent);
}

TEST(FsReq, MkdtempAndRmdirSync) {
    const std::string dirname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.dir.XXXXXX"};

    auto loop = uvw::loop::get_default();
    auto request = loop->resource<uvw::fs_req>();

    auto mkdtempR = request->mkdtemp_sync(dirname);

    ASSERT_TRUE(mkdtempR.first);
    ASSERT_NE(mkdtempR.second, nullptr);
    ASSERT_TRUE(request->rmdir_sync(mkdtempR.second));

    loop->run();
}

TEST(FsReq, Stat) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsStatEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::STAT>>([&checkFsStatEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsStatEvent);
        checkFsStatEvent = true;
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->stat(filename);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsStatEvent);
}

TEST(FsReq, StatSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());

    auto statR = fsReq->stat_sync(filename);

    ASSERT_TRUE(statR.first);

    loop->run();
}

TEST(FsReq, Lstat) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsLstatEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::LSTAT>>([&checkFsLstatEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsLstatEvent);
        checkFsLstatEvent = true;
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->lstat(filename);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLstatEvent);
}

TEST(FsReq, LstatSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());

    auto statR = fsReq->lstat_sync(filename);

    ASSERT_TRUE(statR.first);

    loop->run();
}

TEST(FsReq, Rename) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string rename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.rename"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsRenameEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::RENAME>>([&checkFsRenameEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsRenameEvent);
        checkFsRenameEvent = true;
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename, &rename](const auto &, auto &) {
        fsReq->rename(filename, rename);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsRenameEvent);
}

TEST(FsReq, RenameSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string rename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.rename"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());
    ASSERT_TRUE(fsReq->rename_sync(filename, rename));

    loop->run();
}

TEST(FsReq, Access) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsAccessEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::ACCESS>>([&checkFsAccessEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsAccessEvent);
        checkFsAccessEvent = true;
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->access(filename, R_OK);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsAccessEvent);
}

TEST(FsReq, AccessSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());
    ASSERT_TRUE(fsReq->access_sync(filename, R_OK));

    loop->run();
}

TEST(FsReq, Chmod) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsChmodEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::CHMOD>>([&checkFsChmodEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsChmodEvent);
        checkFsChmodEvent = true;
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->chmod(filename, 0644);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsChmodEvent);
}

TEST(FsReq, ChmodSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());
    ASSERT_TRUE(fsReq->chmod_sync(filename, 0644));

    loop->run();
}

TEST(FsReq, Utime) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsUtimeEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::UTIME>>([&checkFsUtimeEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsUtimeEvent);
        checkFsUtimeEvent = true;
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        fsReq->utime(filename, value, value);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsUtimeEvent);
}

TEST(FsReq, UtimeSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    ASSERT_TRUE(fileReq->close_sync());
    ASSERT_TRUE(fsReq->utime_sync(filename, value, value));

    loop->run();
}

TEST(FsReq, LinkAndUnlink) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsLinkEvent = false;
    bool checkFsUnlinkEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::UNLINK>>([&checkFsUnlinkEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsUnlinkEvent);
        checkFsUnlinkEvent = true;
    });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::LINK>>([&checkFsLinkEvent, &linkname](const auto &, auto &request) {
        ASSERT_FALSE(checkFsLinkEvent);
        checkFsLinkEvent = true;
        request.unlink(linkname);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename, &linkname](const auto &, auto &) {
        fsReq->link(filename, linkname);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLinkEvent);
    ASSERT_TRUE(checkFsUnlinkEvent);
}

TEST(FsReq, LinkAndUnlinkSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());
    ASSERT_TRUE(fsReq->link_sync(filename, linkname));
    ASSERT_TRUE(fsReq->unlink_sync(linkname));

    loop->run();
}

TEST(FsReq, SymlinkAndUnlink) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsLinkEvent = false;
    bool checkFsUnlinkEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::UNLINK>>([&checkFsUnlinkEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsUnlinkEvent);
        checkFsUnlinkEvent = true;
    });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::SYMLINK>>([&checkFsLinkEvent, &linkname](const auto &, auto &request) {
        ASSERT_FALSE(checkFsLinkEvent);
        checkFsLinkEvent = true;
        request.unlink(linkname);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename, &linkname](const auto &, auto &) {
        fsReq->symlink(filename, linkname);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLinkEvent);
    ASSERT_TRUE(checkFsUnlinkEvent);
}

TEST(FsReq, SymlinkAndUnlinkSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());
    ASSERT_TRUE(fsReq->symlink_sync(filename, linkname));
    ASSERT_TRUE(fsReq->unlink_sync(linkname));

    loop->run();
}

TEST(FsReq, Readlink) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsReadlinkEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::READLINK>>([&checkFsReadlinkEvent, &linkname](const auto &, auto &request) {
        ASSERT_FALSE(checkFsReadlinkEvent);
        checkFsReadlinkEvent = true;
        request.unlink(linkname);
    });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::SYMLINK>>([&linkname](const auto &, auto &request) {
        request.readlink(linkname);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename, &linkname](const auto &, auto &) {
        fsReq->symlink(filename, linkname);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsReadlinkEvent);
}

TEST(FsReq, ReadlinkSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());
    ASSERT_TRUE(fsReq->symlink_sync(filename, linkname));

    auto readlinkR = fsReq->readlink_sync(linkname);

    ASSERT_TRUE(readlinkR.first);
    ASSERT_TRUE(fsReq->unlink_sync(linkname));

    loop->run();
}

TEST(FsReq, Realpath) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsRealpathEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::REALPATH>>([&checkFsRealpathEvent](const auto &event, auto &) {
        ASSERT_FALSE(checkFsRealpathEvent);
        ASSERT_NE(event.path, nullptr);
        checkFsRealpathEvent = true;
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->realpath(filename);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsRealpathEvent);
}

TEST(FsReq, RealpathSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());

    auto realpathR = fsReq->realpath_sync(filename);

    ASSERT_TRUE(realpathR.first);
    ASSERT_NE(realpathR.second, nullptr);

    loop->run();
}

TEST(FsReq, Chown) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsChownEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::CHOWN>>([&checkFsChownEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsChownEvent);
        checkFsChownEvent = true;
    });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::STAT>>([&filename](const auto &event, auto &request) {
        auto uid = static_cast<uvw::uid_type>(event.stat.st_uid);
        auto gid = static_cast<uvw::uid_type>(event.stat.st_gid);
        request.chown(filename, uid, gid);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->stat(filename);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsChownEvent);
}

TEST(FsReq, ChownSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());

    auto statR = fsReq->stat_sync(filename);

    ASSERT_TRUE(statR.first);
    auto uid = static_cast<uvw::uid_type>(statR.second.st_uid);
    auto gid = static_cast<uvw::uid_type>(statR.second.st_gid);
    ASSERT_TRUE(fsReq->chown_sync(filename, uid, gid));

    loop->run();
}

TEST(FsReq, Lchown) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsLChownEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::LCHOWN>>([&checkFsLChownEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsLChownEvent);
        checkFsLChownEvent = true;
    });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::STAT>>([&filename](const auto &event, auto &request) {
        auto uid = static_cast<uvw::uid_type>(event.stat.st_uid);
        auto gid = static_cast<uvw::uid_type>(event.stat.st_gid);
        request.lchown(filename, uid, gid);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::CLOSE>>([&fsReq, &filename](const auto &, auto &) {
        fsReq->stat(filename);
    });

    fileReq->on<uvw::fs_event<uvw::file_req::fs_type::OPEN>>([](const auto &, auto &request) {
        request.close();
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLChownEvent);
}

TEST(FsReq, LchownSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto fileReq = loop->resource<uvw::file_req>();
    auto fsReq = loop->resource<uvw::fs_req>();
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;

    ASSERT_TRUE(fileReq->open_sync(filename, flags, 0644));
    ASSERT_TRUE(fileReq->close_sync());

    auto statR = fsReq->stat_sync(filename);

    ASSERT_TRUE(statR.first);
    auto uid = static_cast<uvw::uid_type>(statR.second.st_uid);
    auto gid = static_cast<uvw::uid_type>(statR.second.st_gid);
    ASSERT_TRUE(fsReq->lchown_sync(filename, uid, gid));

    loop->run();
}

TEST(FsReq, ReadDir) {
    const std::string dir_name = std::string{TARGET_FS_REQ_DIR};

    auto loop = uvw::loop::get_default();
    auto fsReq = loop->resource<uvw::fs_req>();

    bool checkFsReadDirEvent = false;
    bool checkFsOpenDirEvent = false;
    bool checkFsCloseDirEvent = false;

    fsReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::CLOSEDIR>>([&checkFsCloseDirEvent](const auto &, auto &) {
        ASSERT_FALSE(checkFsCloseDirEvent);
        checkFsCloseDirEvent = true;
    });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::READDIR>>([&checkFsReadDirEvent](const auto &event, auto &hndl) {
        ASSERT_FALSE(checkFsReadDirEvent);
        if(!event.eos) {
            hndl.readdir();
        } else {
            checkFsReadDirEvent = true;
            hndl.closedir();
        }
    });

    fsReq->on<uvw::fs_event<uvw::fs_req::fs_type::OPENDIR>>([&checkFsOpenDirEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkFsOpenDirEvent);
        checkFsOpenDirEvent = true;
        hndl.readdir();
    });

    fsReq->opendir(dir_name);
    loop->run();

    ASSERT_TRUE(checkFsCloseDirEvent);
    ASSERT_TRUE(checkFsReadDirEvent);
    ASSERT_TRUE(checkFsOpenDirEvent);
}

TEST(FsReq, ReadDirSync) {
    const std::string dir_name = std::string{TARGET_FS_REQ_DIR};

    auto loop = uvw::loop::get_default();
    auto fsReq = loop->resource<uvw::fs_req>();

    ASSERT_TRUE(fsReq->opendir_sync(dir_name));
    auto res = fsReq->readdir_sync();
    while(res.first) {
        res = fsReq->readdir_sync();
    }
    ASSERT_TRUE(fsReq->closedir_sync());

    loop->run();
}
