#include <gtest/gtest.h>
#include <uvw/lib.h>

TEST(SharedLib, Failure) {
    auto loop = uvw::loop::get_default();
    auto lib = loop->resource<uvw::shared_lib>("foobar.so");

    ASSERT_FALSE(static_cast<bool>(*lib));
    ASSERT_NE(lib->error(), nullptr);
    ASSERT_EQ(&lib->parent(), loop.get());

    // this forces a call to the destructor to invoke uv_dlclose
    lib.reset();
}

TEST(SharedLib, Success) {
    auto loop = uvw::loop::get_default();
    auto lib = loop->resource<uvw::shared_lib>(TARGET_LIB_SO);

    ASSERT_TRUE(static_cast<bool>(*lib));
    ASSERT_EQ(&lib->parent(), loop.get());
    ASSERT_EQ(lib->sym<int(double *)>("foobar"), nullptr);
    ASSERT_NE(lib->sym<int(double *)>("fake_func"), nullptr);
    double value{1.};
    ASSERT_EQ(-42, lib->sym<int(double *)>("fake_func")(&value));
    ASSERT_DOUBLE_EQ(-1., value);

    // this forces a call to the destructor to invoke uv_dlclose
    lib.reset();
}
