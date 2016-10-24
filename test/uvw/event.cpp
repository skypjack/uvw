#include <gtest/gtest.h>
#include <uvw/event.hpp>


struct EventA: uvw::Event<EventA> {};
struct EventB: uvw::Event<EventB> {};


TEST(Event, Uniqueness) {
    ASSERT_EQ(EventA::type(), EventA::type());
    ASSERT_EQ(EventA::type(), EventA{}.type());

    ASSERT_NE(EventA::type(), EventB::type());
    ASSERT_NE(EventA::type(), EventB{}.type());
}
