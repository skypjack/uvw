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

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::MKDIR) {
            ASSERT_FALSE(checkFsMkdirEvent);
            checkFsMkdirEvent = true;
            req.rmdir(dirname);
        } else if(event.type == uvw::fs_req::fs_type::RMDIR) {
            ASSERT_FALSE(checkFsRmdirEvent);
            checkFsRmdirEvent = true;
        };
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

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::MKDTEMP) {
            ASSERT_FALSE(checkFsMkdtempEvent);
            ASSERT_NE(event.path, nullptr);
            checkFsMkdtempEvent = true;
            req.rmdir(event.path);
        } else if(event.type == uvw::fs_req::fs_type::RMDIR) {
            ASSERT_FALSE(checkFsRmdirEvent);
            checkFsRmdirEvent = true;
        };
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &) {
        if(event.type == uvw::fs_req::fs_type::STAT) {
            ASSERT_FALSE(checkFsStatEvent);
            checkFsStatEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->stat(filename);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &) {
        if(event.type == uvw::fs_req::fs_type::LSTAT) {
            ASSERT_FALSE(checkFsLstatEvent);
            checkFsLstatEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->lstat(filename);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &) {
        if(event.type == uvw::fs_req::fs_type::RENAME) {
            ASSERT_FALSE(checkFsRenameEvent);
            checkFsRenameEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->rename(filename, rename);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &) {
        if(event.type == uvw::fs_req::fs_type::ACCESS) {
            ASSERT_FALSE(checkFsAccessEvent);
            checkFsAccessEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->access(filename, R_OK);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &) {
        if(event.type == uvw::fs_req::fs_type::CHMOD) {
            ASSERT_FALSE(checkFsChmodEvent);
            checkFsChmodEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->chmod(filename, 0644);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &) {
        if(event.type == uvw::fs_req::fs_type::UTIME) {
            ASSERT_FALSE(checkFsUtimeEvent);
            checkFsUtimeEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        const auto value = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());

        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->utime(filename, value, value);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::LINK) {
            ASSERT_FALSE(checkFsLinkEvent);
            checkFsLinkEvent = true;
            req.unlink(linkname);
        } else if(event.type == uvw::fs_req::fs_type::UNLINK) {
            ASSERT_FALSE(checkFsUnlinkEvent);
            checkFsUnlinkEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->link(filename, linkname);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::SYMLINK) {
            ASSERT_FALSE(checkFsLinkEvent);
            checkFsLinkEvent = true;
            req.unlink(linkname);
        } else if(event.type == uvw::fs_req::fs_type::UNLINK) {
            ASSERT_FALSE(checkFsUnlinkEvent);
            checkFsUnlinkEvent = true;
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->symlink(filename, linkname);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::READLINK) {
            ASSERT_FALSE(checkFsReadlinkEvent);
            checkFsReadlinkEvent = true;
            req.unlink(linkname);
        } else if(event.type == uvw::fs_req::fs_type::SYMLINK) {
            req.readlink(linkname);
        }
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->symlink(filename, linkname);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &) {
        if(event.type == uvw::fs_req::fs_type::REALPATH) {
            ASSERT_FALSE(checkFsRealpathEvent);
            ASSERT_NE(event.path, nullptr);
            checkFsRealpathEvent = true;
        };
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->realpath(filename);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CHOWN) {
            ASSERT_FALSE(checkFsChownEvent);
            checkFsChownEvent = true;
        } else if(event.type == uvw::fs_req::fs_type::STAT) {
            req.chown(filename, static_cast<uvw::uid_type>(event.stat.st_uid), static_cast<uvw::uid_type>(event.stat.st_gid));
        };
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->stat(filename);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::LCHOWN) {
            ASSERT_FALSE(checkFsLChownEvent);
            checkFsLChownEvent = true;
        } else if(event.type == uvw::fs_req::fs_type::STAT) {
            req.lchown(filename, static_cast<uvw::uid_type>(event.stat.st_uid), static_cast<uvw::uid_type>(event.stat.st_gid));
        };
    });

    fileReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSE) {
            fsReq->stat(filename);
        } else if(event.type == uvw::fs_req::fs_type::OPEN) {
            req.close();
        }
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

    fsReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::CLOSEDIR) {
            ASSERT_FALSE(checkFsCloseDirEvent);
            checkFsCloseDirEvent = true;
        } else if(event.type == uvw::fs_req::fs_type::OPENDIR) {
            ASSERT_FALSE(checkFsOpenDirEvent);
            checkFsOpenDirEvent = true;
            req.readdir();
        } else if(event.type == uvw::fs_req::fs_type::READDIR) {
            ASSERT_FALSE(checkFsReadDirEvent);
            if(!event.dirent.eos) {
                req.readdir();
            } else {
                checkFsReadDirEvent = true;
                req.closedir();
            }
        }
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
