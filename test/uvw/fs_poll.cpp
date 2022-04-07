#include <gtest/gtest.h>
#include <uvw/fs.h>
#include <uvw/fs_event.h>
#include <uvw/fs_poll.h>

TEST(FsPoll, Functionalities) {
    const std::string filename = std::string{TARGET_FS_POLL_DIR} + std::string{"/test.file"};

    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::fs_poll_handle>();
    auto request = loop->resource<uvw::file_req>();

    bool checkFsPollEvent = false;

    handle->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });
    request->on<uvw::error_event>([](const auto &, auto &) { FAIL(); });

    handle->on<uvw::fs_poll_event>([&checkFsPollEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkFsPollEvent);
        checkFsPollEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    request->on<uvw::fs_event>([&](const auto &event, auto &req) {
        if(event.type == uvw::fs_req::fs_type::WRITE) {
            req.close();
        };
    });

    request->open_sync(filename, uvw::file_req::file_open_flags::CREAT | uvw::file_req::file_open_flags::RDWR | uvw::file_req::file_open_flags::TRUNC, 0755);
    handle->start(filename, uvw::fs_poll_handle::time{1000});
    request->write(std::unique_ptr<char[]>{new char[1]{42}}, 1, 0);

    ASSERT_EQ(handle->path(), filename);
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkFsPollEvent);
}
