#include <gtest/gtest.h>
#include <uvw/fs.h>
#include <uvw/fs_event.h>

TEST(FsEvent, Functionalities) {
    const std::string filename = std::string{TARGET_FS_EVENT_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::fs_event_handle>();
    auto request = loop->resource<uvw::file_req>();

    bool checkFsEventEvent = false;

    handle->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    handle->on<uvw::fs_event_event>([&checkFsEventEvent](const auto &event, auto &hndl) {
        ASSERT_FALSE(checkFsEventEvent);
        ASSERT_EQ(std::string{event.filename}, std::string{"test.file"});
        checkFsEventEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        switch(event.type) {
        case uvw::fs_req::fs_type::WRITE:
            req.close();
            break;
        case uvw::fs_req::fs_type::OPEN:
            req.write(std::unique_ptr<char[]>{new char[1]{42}}, 1, 0);
            break;
        }
    });

    handle->start(std::string{TARGET_FS_EVENT_DIR}, uvw::fs_event_handle::event_flags::RECURSIVE);
    auto flags = uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC;
    request->open(filename, flags, 0644);

    ASSERT_EQ(handle->path(), std::string{TARGET_FS_EVENT_DIR});
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkFsEventEvent);
}
