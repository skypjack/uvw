#include <memory>
#include <type_traits>
#include <gtest/gtest.h>
#include <uvw/async.h>
#include <uvw/request.hpp>

TEST(Resource, Functionalities) {
    ASSERT_FALSE(std::is_copy_constructible_v<uvw::async_handle>);
    ASSERT_FALSE(std::is_copy_assignable_v<uvw::async_handle>);

    ASSERT_FALSE(std::is_move_constructible_v<uvw::async_handle>);
    ASSERT_FALSE(std::is_move_assignable_v<uvw::async_handle>);

    auto loop = uvw::loop::get_default();
    auto resource = loop->resource<uvw::async_handle>();

    ASSERT_EQ(&resource->parent(), loop.get());

    resource->data(std::make_shared<int>(2));

    ASSERT_EQ(*std::static_pointer_cast<int>(resource->data()), 2);
    ASSERT_EQ(*resource->data<int>(), 2);

    resource->close();
    loop->run();
}
