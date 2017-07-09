#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(FsEvent, StartAndStop) {
    const std::string dirname = std::string{TARGET_FS_EVENT_DIR};

    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::FsEventHandle>();
    auto req = loop->resource<uvw::FsReq>();

    bool checkFsEventEvent = false;

    req->mkdirSync(dirname, 0755);

    handle->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });
    req->on<uvw::ErrorEvent>([](const auto &, auto &) { FAIL(); });

    handle->on<uvw::FsEventEvent>([&checkFsEventEvent, &dirname](const auto &, auto &hndl) {
        ASSERT_FALSE(checkFsEventEvent);
        checkFsEventEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    req->rmdir(dirname);
    handle->start(dirname);

    ASSERT_EQ(handle->path(), dirname);
    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_TRUE(checkFsEventEvent);
}
