#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(FsEvent, Functionalities) {
    const std::string relative = std::string{"test.file"};
    const std::string absolute = std::string{TARGET_FS_EVENT_DIR} + "/" + relative;
    const std::string rename = std::string{TARGET_FS_EVENT_DIR} + std::string{"/test.rename"};

    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::FsEventHandle>();
    auto timer = loop->resource<uvw::TimerHandle>();
    auto fileReq = loop->resource<uvw::FileReq>();

    bool checkFsEventEvent = false;

    handle->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    timer->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    fileReq->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    handle->on<uvw::FsEventEvent>([&checkFsEventEvent, &relative](const auto &event, auto &hndl) {
        ASSERT_FALSE(checkFsEventEvent);
        ASSERT_EQ(std::string{event.filename}, relative);
        checkFsEventEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    timer->on<uvw::TimerEvent>([&absolute, &rename](const auto &, auto &hndl) {
        auto fsReq = hndl.loop().template resource<uvw::FsReq>();
        fsReq->renameSync(absolute, rename);
        hndl.close();
    });

    fileReq->openSync(absolute, O_CREAT | O_RDWR | O_TRUNC, 0644);
    timer->start(uvw::TimerHandle::Time{500}, uvw::TimerHandle::Time{0});
    handle->start(absolute);

    ASSERT_EQ(handle->path(), absolute);
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkFsEventEvent);
}
