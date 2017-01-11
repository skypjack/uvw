#include <memory>
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
    auto resource = loop->resource<uvw::AsyncHandle>();

    ASSERT_NO_THROW(resource->loop());

    auto data = std::make_shared<int>(42);

    ASSERT_NO_THROW(resource->data(data));
    ASSERT_EQ(*std::static_pointer_cast<int>(resource->data()), 42);
    ASSERT_EQ(*resource->data<int>(), 42);

    resource->close();
    loop->run();
}
