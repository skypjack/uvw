#include <chrono>
#include <gtest/gtest.h>
#include <uvw/fs.h>

#ifdef _WIN32
#    define _CRT_DECLARE_NONSTDC_NAMES 1
#    include <fcntl.h>
#endif

TEST(FileReq, SendFile) {
    const std::string srcFilename = std::string{TARGET_FILE_REQ_SENDFILE_DIR} + std::string{"/src.file"};
    const std::string dstFilename = std::string{TARGET_FILE_REQ_SENDFILE_DIR} + std::string{"/dst.file"};

    auto loop = uvw::loop::get_default();
    auto srcReq = loop->resource<uvw::file_req>();
    auto dstReq = loop->resource<uvw::file_req>();

    bool checkFileSendFileEvent = false;

    dstReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    srcReq->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    dstReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::OPEN:
            srcReq->sendfile(static_cast<uvw::file_handle>(req), 0, 0);
            break;
        default:
            // nothing to do here
            break;
        }
    });

    srcReq->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::SENDFILE:
            ASSERT_FALSE(checkFileSendFileEvent);
            checkFileSendFileEvent = true;
            dstReq->close();
            req.close();
            break;
        case uvw::fs_req::fs_type::OPEN:
            auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::WRONLY | uvw::file_req::file_open_flags::TRUNC;
            dstReq->open(dstFilename, flags, 0644);
            break;
        default:
            // nothing to do here
            break;
        }
    });

    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDONLY | uvw::file_req::file_open_flags::TRUNC;
    srcReq->open(srcFilename, flags, 0644);

    loop->run();

    ASSERT_TRUE(checkFileSendFileEvent);
}

TEST(FileReq, SendFileSync) {
    const std::string srcFilename = std::string{TARGET_FILE_REQ_SENDFILE_DIR} + std::string{"/src.file"};
    const std::string dstFilename = std::string{TARGET_FILE_REQ_SENDFILE_DIR} + std::string{"/dst.file"};

    auto loop = uvw::loop::get_default();
    auto srcReq = loop->resource<uvw::file_req>();
    auto dstReq = loop->resource<uvw::file_req>();

    ASSERT_TRUE(srcReq->open_sync(srcFilename, uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDONLY | uvw::file_req::file_open_flags::TRUNC, 0644));
    ASSERT_TRUE(dstReq->open_sync(dstFilename, uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::WRONLY | uvw::file_req::file_open_flags::TRUNC, 0644));

    auto sendfileR = srcReq->sendfile_sync(static_cast<uvw::file_handle>(*dstReq), 0, 0);

    ASSERT_TRUE(sendfileR.first);
    ASSERT_TRUE(srcReq->close_sync());
    ASSERT_TRUE(dstReq->close_sync());

    loop->run();
}
