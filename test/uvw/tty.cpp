#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(TTY, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::TTYHandle>(uvw::StdOUT, false);
    auto timer = loop->resource<uvw::TimerHandle>();

    bool checkWriteEvent = false;

    handle->on<uvw::WriteEvent>([&checkWriteEvent](const auto &, auto &hndl){
        ASSERT_FALSE(checkWriteEvent);
        checkWriteEvent = true;
        hndl.close();
    });

    timer->on<uvw::TimerEvent>([handle](const auto &, auto &hndl){
        auto data = std::make_unique<char[]>('*');
        handle->write(std::move(data), 1);
        hndl.close();
    });

    ASSERT_TRUE(handle->reset());
    ASSERT_TRUE(handle->mode(uvw::TTYHandle::Mode::NORMAL));
    ASSERT_NO_THROW(handle->getWinSize());

    timer->start(uvw::TimerHandle::Time{0}, uvw::TimerHandle::Time{0});
    loop->run();

    ASSERT_TRUE(checkWriteEvent);
}
