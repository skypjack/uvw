#include <gtest/gtest.h>
#include <uvw.hpp>


struct S: uvw::Self<S> { };


TEST(Self, Basics) {
    std::shared_ptr<S> self = std::make_shared<S>();

    ASSERT_TRUE(self.unique());
    ASSERT_FALSE(self->self());

    self->leak();

    ASSERT_FALSE(self.unique());
    ASSERT_TRUE(self->self());

    self->reset();

    ASSERT_TRUE(self.unique());
    ASSERT_FALSE(self->self());
}
