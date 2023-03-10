#include <memory>
#include <type_traits>
#include <gtest/gtest.h>
#include <uvw/async.h>
#include <uvw/request.hpp>

TEST(Resource, Functionalities) {
    ASSERT_FALSE(std::is_copy_constructible<uvw::async_handle>::value);
    ASSERT_FALSE(std::is_copy_assignable<uvw::async_handle>::value);

    ASSERT_FALSE(std::is_move_constructible<uvw::async_handle>::value);
    ASSERT_FALSE(std::is_move_assignable<uvw::async_handle>::value);

    auto loop = uvw::loop::get_default();
    auto resource = loop->resource<uvw::async_handle>();

    ASSERT_EQ(&resource->parent(), loop.get());

    resource->data(std::make_shared<int>(42));

    ASSERT_EQ(*std::static_pointer_cast<int>(resource->data()), 42);
    ASSERT_EQ(*resource->data<int>(), 42);

    resource->close();
    loop->run();
}
