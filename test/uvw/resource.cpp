#include <type_traits>
#include <gtest/gtest.h>
#include <uvw.hpp>


struct Res: uvw::Resource<Res, int> { };


TEST(Resource, Basics) {
    ASSERT_FALSE(std::is_copy_constructible<uvw::AsyncHandle>::value);
    ASSERT_FALSE(std::is_copy_assignable<uvw::AsyncHandle>::value);

    ASSERT_FALSE(std::is_move_constructible<uvw::AsyncHandle>::value);
    ASSERT_FALSE(std::is_move_assignable<uvw::AsyncHandle>::value);

    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::AsyncHandle>();

    ASSERT_NO_THROW(handle->loop());
}
