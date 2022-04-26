#include <gtest/gtest.h>
#include <uvw/timer.h>
#include <uvw/tty.h>

TEST(TTY, Functionalities) {
    auto loop = uvw::loop::get_default();
    auto handle = loop->resource<uvw::tty_handle>(uvw::std_out, false);
    auto timer = loop->resource<uvw::timer_handle>();

    bool checkWriteEvent = false;

    handle->on<uvw::write_event>([&checkWriteEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkWriteEvent);
        checkWriteEvent = true;
        hndl.close();
    });

    timer->on<uvw::timer_event>([handle](const auto &, auto &hndl) {
        auto data = std::make_unique<char[]>('*');

        ASSERT_EQ(0, (handle->write(std::move(data), 1)));

        hndl.close();
    });

    ASSERT_TRUE(handle->reset_mode());
    ASSERT_TRUE(!handle->readable() || handle->mode(uvw::tty_handle::tty_mode::NORMAL));
    ASSERT_NO_THROW(handle->get_win_size());

    timer->start(uvw::timer_handle::time{0}, uvw::timer_handle::time{0});
    loop->run();

    ASSERT_TRUE(checkWriteEvent);
}
