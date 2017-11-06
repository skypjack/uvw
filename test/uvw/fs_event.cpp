#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(FsEvent, Functionalities) {
    const std::string filename = std::string{TARGET_FS_EVENT_DIR} + std::string{"/test.file"};

    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::FsEventHandle>();
    auto request = loop->resource<uvw::FileReq>();

    bool checkFsEventEvent = false;

    handle->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    request->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    handle->on<uvw::FsEventEvent>([&checkFsEventEvent](const auto &event, auto &hndl) {
        ASSERT_FALSE(checkFsEventEvent);
        ASSERT_EQ(std::string{event.filename}, std::string{"test.file"});
        checkFsEventEvent = true;
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

    handle->start(std::string{TARGET_FS_EVENT_DIR}, uvw::FsEventHandle::Event::RECURSIVE);
    auto flags = uvw::Flags<uvw::FileReq::FileOpen>::from<uvw::FileReq::FileOpen::CREAT, uvw::FileReq::FileOpen::RDWR, uvw::FileReq::FileOpen::TRUNC>();
    request->open(filename, flags, 0644);

    ASSERT_EQ(handle->path(), std::string{TARGET_FS_EVENT_DIR});
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkFsEventEvent);
}
