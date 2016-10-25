#include <type_traits>
#include <gtest/gtest.h>
#include <uvw.hpp>


struct Res: uvw::Resource<Res, int> { };


TEST(Resource, Basics) {
    ASSERT_FALSE(std::is_copy_constructible<Res>::value);
    ASSERT_FALSE(std::is_copy_assignable<Res>::value);

    ASSERT_FALSE(std::is_move_constructible<Res>::value);
    ASSERT_FALSE(std::is_move_assignable<Res>::value);

    ASSERT_NO_THROW(uvw::Loop::getDefault()->resource<uvw::AsyncHandle>()->loop());
}
