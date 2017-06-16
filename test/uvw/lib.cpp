#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(SharedLib, Failure) {
    auto loop = uvw::Loop::getDefault();
    auto lib = loop->resource<uvw::SharedLib>("foobar.so");

    ASSERT_FALSE(static_cast<bool>(*lib));
    ASSERT_NE(lib->error(), nullptr);
    ASSERT_EQ(&lib->loop(), loop.get());

    // this forces a call to the destructor to invoke uv_dlclose
    lib.reset();
}


TEST(SharedLib, Success) {
    auto loop = uvw::Loop::getDefault();
    auto lib = loop->resource<uvw::SharedLib>(TARGET_LIB_SO);

    ASSERT_TRUE(static_cast<bool>(*lib));
    ASSERT_EQ(&lib->loop(), loop.get());
    ASSERT_EQ(lib->sym<int(double *)>("foobar"), nullptr);
    ASSERT_NE(lib->sym<int(double *)>("uv_uptime"), nullptr);

    // this forces a call to the destructor to invoke uv_dlclose
    lib.reset();
}
