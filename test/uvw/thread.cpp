#include <memory>
#include <functional>
#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Thread, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::Thread>([](std::shared_ptr<void>){});

    resource->run();

    // TODO
}


TEST(ThreadLocalStorage, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::ThreadLocalStorage>();

    // TODO
}


TEST(Once, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::Once>();

    // TODO
}


TEST(Mutex, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::Mutex>();

    // TODO
}


TEST(RWLock, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::RWLock>();

    // TODO
}


TEST(Semaphore, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::Semaphore>(1);

    // TODO
}


TEST(Condition, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::Condition>();

    // TODO
}


TEST(Barrier, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto resource = loop->resource<uvw::Barrier>(1);

    // TODO
}
