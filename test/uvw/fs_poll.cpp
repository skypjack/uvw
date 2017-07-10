#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(FsPoll, Functionalities) {
    const std::string filename = std::string{TARGET_FS_POLL_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::FsPollHandle>();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFsPollEvent = false;

    handle->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    handle->on<uvw::FsPollEvent>([&checkFsPollEvent](const auto &event, auto &hndl) {
        ASSERT_FALSE(checkFsPollEvent);
        checkFsPollEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::WRITE>>([](const auto &, auto &req) {
        req.close();
    });

    request->on<uvw::FsEvent<uvw::FileReq::Type::OPEN>>([](const auto &, auto &req) {
        req.write(std::unique_ptr<char[]>{new char[1]{ 42 }}, 1, 0);
    });

    handle->start(filename, uvw::FsPollHandle::Time{5});
    request->open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);

    ASSERT_EQ(handle->path(), filename);
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkFsPollEvent);
}
