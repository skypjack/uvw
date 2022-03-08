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
    const std::string srcFilename = std::string{TARGET_FILE_REQ_SENDFILE_DIR} + std::string{"/src.file"};
    const std::string dstFilename = std::string{TARGET_FILE_REQ_SENDFILE_DIR} + std::string{"/dst.file"};

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
