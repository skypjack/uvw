#include <gtest/gtest.h>
#include <uvw.hpp>
#include <chrono>

#ifdef _WIN32
#define _CRT_DECLARE_NONSTDC_NAMES 1
#include <fcntl.h>
#endif


TEST(FsReq, MkdirAndRmdir) {
    const std::string dirname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.dir"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    bool checkFsMkdirEvent = false;
    bool checkFsRmdirEvent = false;

    request->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FsReq::Type::RMDIR>>([&checkFsRmdirEvent](const uvw::FsEvent<uvw::FsReq::Type::RMDIR> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsRmdirEvent);
        checkFsRmdirEvent = true;
    });

    request->on<uvw::FsEvent<uvw::FsReq::Type::MKDIR>>([&checkFsMkdirEvent, &dirname](const uvw::FsEvent<uvw::FsReq::Type::MKDIR> &, uvw::FsReq &req) {
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

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(request->mkdirSync(dirname, 0755));
    ASSERT_TRUE(request->rmdirSync(dirname));

    loop->run();
}


TEST(FsReq, MkdtempAndRmdir) {
    const std::string dirname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.dir.XXXXXX"};

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    bool checkFsMkdtempEvent = false;
    bool checkFsRmdirEvent = false;

    request->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });

    request->on<uvw::FsEvent<uvw::FsReq::Type::RMDIR>>([&checkFsRmdirEvent](const uvw::FsEvent<uvw::FsReq::Type::RMDIR> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsRmdirEvent);
        checkFsRmdirEvent = true;
    });

    request->on<uvw::FsEvent<uvw::FsReq::Type::MKDTEMP>>([&checkFsMkdtempEvent](const uvw::FsEvent<uvw::FsReq::Type::MKDTEMP> &event, uvw::FsReq &req) {
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

    auto loop = uvw::Loop::getDefault();
    auto request = loop->resource<uvw::FsReq>();

    auto mkdtempR = request->mkdtempSync(dirname);

    ASSERT_TRUE(mkdtempR.first);
    ASSERT_NE(mkdtempR.second, nullptr);
    ASSERT_TRUE(request->rmdirSync(mkdtempR.second));

    loop->run();
}


/*
TEST(FsReq, Scandir) {
    // TODO
}


TEST(FsReq, ScandirSync) {
    // TODO
}
*/


TEST(FsReq, Stat) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsStatEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::STAT>>([&checkFsStatEvent](const uvw::FsEvent<uvw::FsReq::Type::STAT> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsStatEvent);
        checkFsStatEvent = true;
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->stat(filename);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsStatEvent);
}


TEST(FsReq, StatSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());

    auto statR = fsReq->statSync(filename);

    ASSERT_TRUE(statR.first);

    loop->run();
}


TEST(FsReq, Lstat) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsLstatEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::LSTAT>>([&checkFsLstatEvent](const uvw::FsEvent<uvw::FsReq::Type::LSTAT> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsLstatEvent);
        checkFsLstatEvent = true;
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->lstat(filename);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLstatEvent);
}


TEST(FsReq, LstatSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());

    auto statR = fsReq->lstatSync(filename);

    ASSERT_TRUE(statR.first);

    loop->run();
}


TEST(FsReq, Rename) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string rename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.rename"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsRenameEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::RENAME>>([&checkFsRenameEvent](const uvw::FsEvent<uvw::FsReq::Type::RENAME> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsRenameEvent);
        checkFsRenameEvent = true;
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename, &rename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->rename(filename, rename);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsRenameEvent);
}


TEST(FsReq, RenameSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string rename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.rename"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->renameSync(filename, rename));

    loop->run();
}


TEST(FsReq, CopyFile) {
    // TODO
}


TEST(FsReq, CopyFileSync) {
    // TODO
}


TEST(FsReq, Access) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsAccessEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::ACCESS>>([&checkFsAccessEvent](const uvw::FsEvent<uvw::FsReq::Type::ACCESS> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsAccessEvent);
        checkFsAccessEvent = true;
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->access(filename, R_OK);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsAccessEvent);
}


TEST(FsReq, AccessSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->accessSync(filename, R_OK));

    loop->run();
}


TEST(FsReq, Chmod) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsChmodEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::CHMOD>>([&checkFsChmodEvent](const uvw::FsEvent<uvw::FsReq::Type::CHMOD> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsChmodEvent);
        checkFsChmodEvent = true;
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->chmod(filename, 0644);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsChmodEvent);
}


TEST(FsReq, ChmodSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->chmodSync(filename, 0644));

    loop->run();
}


TEST(FsReq, Utime) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsUtimeEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::UTIME>>([&checkFsUtimeEvent](const uvw::FsEvent<uvw::FsReq::Type::UTIME> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsUtimeEvent);
        checkFsUtimeEvent = true;
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        fsReq->utime(filename, value, value);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsUtimeEvent);
}


TEST(FsReq, UtimeSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->utimeSync(filename, value, value));

    loop->run();
}


TEST(FsReq, LinkAndUnlink) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsLinkEvent = false;
    bool checkFsUnlinkEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::UNLINK>>([&checkFsUnlinkEvent](const uvw::FsEvent<uvw::FsReq::Type::UNLINK> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsUnlinkEvent);
        checkFsUnlinkEvent = true;
    });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::LINK>>([&checkFsLinkEvent, &linkname](const uvw::FsEvent<uvw::FsReq::Type::LINK> &, uvw::FsReq &request) {
        ASSERT_FALSE(checkFsLinkEvent);
        checkFsLinkEvent = true;
        request.unlink(linkname);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename, &linkname](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->link(filename, linkname);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLinkEvent);
    ASSERT_TRUE(checkFsUnlinkEvent);
}


TEST(FsReq, LinkAndUnlinkSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->linkSync(filename, linkname));
    ASSERT_TRUE(fsReq->unlinkSync(linkname));

    loop->run();
}


TEST(FsReq, SymlinkAndUnlink) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsLinkEvent = false;
    bool checkFsUnlinkEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::UNLINK>>([&checkFsUnlinkEvent](const uvw::FsEvent<uvw::FsReq::Type::UNLINK> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsUnlinkEvent);
        checkFsUnlinkEvent = true;
    });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::SYMLINK>>([&checkFsLinkEvent, &linkname](const uvw::FsEvent<uvw::FsReq::Type::SYMLINK> &, uvw::FsReq &request) {
        ASSERT_FALSE(checkFsLinkEvent);
        checkFsLinkEvent = true;
        request.unlink(linkname);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename, &linkname](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->symlink(filename, linkname);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLinkEvent);
    ASSERT_TRUE(checkFsUnlinkEvent);
}


TEST(FsReq, SymlinkAndUnlinkSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->symlinkSync(filename, linkname));
    ASSERT_TRUE(fsReq->unlinkSync(linkname));

    loop->run();
}


TEST(FsReq, Readlink) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsReadlinkEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::READLINK>>([&checkFsReadlinkEvent, &linkname](const uvw::FsEvent<uvw::FsReq::Type::READLINK> &, uvw::FsReq &request) {
        ASSERT_FALSE(checkFsReadlinkEvent);
        checkFsReadlinkEvent = true;
        request.unlink(linkname);
    });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::SYMLINK>>([&linkname](const uvw::FsEvent<uvw::FsReq::Type::SYMLINK> &, uvw::FsReq &request) {
        request.readlink(linkname);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename, &linkname](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->symlink(filename, linkname, 0);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsReadlinkEvent);
}


TEST(FsReq, ReadlinkSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};
    const std::string linkname = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.link"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());
    ASSERT_TRUE(fsReq->symlinkSync(filename, linkname, 0));

    auto readlinkR = fsReq->readlinkSync(linkname);

    ASSERT_TRUE(readlinkR.first);
    ASSERT_TRUE(fsReq->unlinkSync(linkname));

    loop->run();
}


TEST(FsReq, Realpath) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsRealpathEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::REALPATH>>([&checkFsRealpathEvent](const uvw::FsEvent<uvw::FsReq::Type::REALPATH> &event, uvw::FsReq &) {
        ASSERT_FALSE(checkFsRealpathEvent);
        ASSERT_NE(event.path, nullptr);
        checkFsRealpathEvent = true;
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->realpath(filename);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsRealpathEvent);
}


TEST(FsReq, RealpathSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());

    auto realpathR = fsReq->realpathSync(filename);

    ASSERT_TRUE(realpathR.first);
    ASSERT_NE(realpathR.second, nullptr);

    loop->run();
}


TEST(FsReq, Chown) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsChownEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::CHOWN>>([&checkFsChownEvent](const uvw::FsEvent<uvw::FsReq::Type::CHOWN> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsChownEvent);
        checkFsChownEvent = true;
    });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::STAT>>([&filename](const uvw::FsEvent<uvw::FsReq::Type::STAT> &event, uvw::FsReq &request) {
        auto uid = static_cast<uvw::Uid>(event.stat.st_uid);
        auto gid = static_cast<uvw::Uid>(event.stat.st_gid);
        request.chown(filename, uid, gid);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->stat(filename);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsChownEvent);
}


TEST(FsReq, ChownSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());

    auto statR = fsReq->statSync(filename);

    ASSERT_TRUE(statR.first);
    auto uid = static_cast<uvw::Uid>(statR.second.st_uid);
    auto gid = static_cast<uvw::Uid>(statR.second.st_gid);
    ASSERT_TRUE(fsReq->chownSync(filename, uid, gid));

    loop->run();
}


TEST(FsReq, Lchown) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    bool checkFsLChownEvent = false;

    fsReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FsReq &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::FileReq &) { FAIL(); });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::LCHOWN>>([&checkFsLChownEvent](const uvw::FsEvent<uvw::FsReq::Type::LCHOWN> &, uvw::FsReq &) {
        ASSERT_FALSE(checkFsLChownEvent);
        checkFsLChownEvent = true;
    });

    fsReq->on<uvw::FsEvent<uvw::FsReq::Type::STAT>>([&filename](const uvw::FsEvent<uvw::FsReq::Type::STAT> &event, uvw::FsReq &request) {
        auto uid = static_cast<uvw::Uid>(event.stat.st_uid);
        auto gid = static_cast<uvw::Uid>(event.stat.st_gid);
        request.lchown(filename, uid, gid);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::CLOSE>>([&fsReq, &filename](const uvw::FsEvent<uvw::FsReq::Type::CLOSE> &, uvw::FileReq &) {
        fsReq->stat(filename);
    });

    fileReq->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const uvw::FsEvent<uvw::FsReq::Type::OPEN> &, uvw::FileReq &request) {
        request.close();
    });

    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    fileReq->open(filename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFsLChownEvent);
}


TEST(FsReq, LchownSync) {
    const std::string filename = std::string{TARGET_FS_REQ_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto fileReq = loop->resource<uvw::FileReq>();
    auto fsReq = loop->resource<uvw::FsReq>();

    ASSERT_TRUE(fileReq->openSync(filename, O_CREAT | O_RDWR | O_TRUNC, 0644));
    ASSERT_TRUE(fileReq->closeSync());

    auto statR = fsReq->statSync(filename);

    ASSERT_TRUE(statR.first);
    auto uid = static_cast<uvw::Uid>(statR.second.st_uid);
    auto gid = static_cast<uvw::Uid>(statR.second.st_gid);
    ASSERT_TRUE(fsReq->lchownSync(filename, uid, gid));

    loop->run();
}
