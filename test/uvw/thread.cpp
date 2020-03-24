#include <gtest/gtest.h>
#include <uvw.hpp>

TEST(Thread, Run) {
    auto loop = uvw::Loop::getDefault();
    auto has_run = std::make_shared<bool>();

    auto handle = loop->resource<uvw::Thread>([](std::shared_ptr<void> data) {
        if(auto has_run = std::static_pointer_cast<bool>(data); has_run) {
            *has_run = true;
        }
    }, has_run);

    ASSERT_TRUE(handle->run());
    ASSERT_TRUE(handle->join());
    ASSERT_TRUE(*has_run);

    loop->run();
}

TEST(ThreadLocalStorage, SetGet) {
    auto loop = uvw::Loop::getDefault();
    auto localStorage = loop->resource<uvw::ThreadLocalStorage>();
    auto flag{true};

    localStorage->set<bool>(&flag);
    ASSERT_TRUE(localStorage->get<bool>());
}
