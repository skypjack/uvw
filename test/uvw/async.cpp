#include <gtest/gtest.h>
#include <uvw.hpp>

TEST(Async, Send) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::AsyncHandle>();

    bool checkErrorEvent = false;
    bool checkAsyncEvent = false;

    handle->on<uvw::ErrorEvent>([&checkErrorEvent](const uvw::ErrorEvent &, uvw::AsyncHandle &){
        ASSERT_FALSE(checkErrorEvent);
        checkErrorEvent = true;
    });

    handle->on<uvw::AsyncEvent>([&checkAsyncEvent](const uvw::AsyncEvent &, uvw::AsyncHandle &handle){
        ASSERT_FALSE(checkAsyncEvent);
        checkAsyncEvent = true;
        handle.close();
        ASSERT_TRUE(handle.closing());
    });

    handle->send();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run();

    ASSERT_FALSE(checkErrorEvent);
    ASSERT_TRUE(checkAsyncEvent);
}
