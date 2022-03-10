#include <gtest/gtest.h>
#include <uvw/thread.h>

TEST(Thread, Run) {
    auto loop = uvw::loop::get_default();
    auto has_run = std::make_shared<bool>();
    auto cb = [](std::shared_ptr<void> data) {
        if(auto has_run = std::static_pointer_cast<bool>(data); has_run) {
            *has_run = true;
        }
    };

    auto handle = loop->resource<uvw::thread>(cb, has_run);

    ASSERT_TRUE(handle->run());
    ASSERT_TRUE(handle->join());
    ASSERT_TRUE(*has_run);

    loop->run();
}

TEST(ThreadLocalStorage, SetGet) {
    auto loop = uvw::loop::get_default();
    auto localStorage = loop->resource<uvw::thread_local_storage>();
    auto flag{true};

    localStorage->set<bool>(&flag);
    ASSERT_TRUE(localStorage->get<bool>());

    loop->run();
}

TEST(Mutex, LockUnlock) {
    auto loop = uvw::loop::get_default();
    auto mtx = loop->resource<uvw::mutex>();

    mtx->lock();

#ifdef _MSC_VER
    // this is allowed by libuv on Windows
    ASSERT_TRUE(mtx->try_lock());
#else
    ASSERT_FALSE(mtx->try_lock());
#endif

    mtx->unlock();
    ASSERT_TRUE(mtx->try_lock());

#ifdef _MSC_VER
    // this is allowed by libuv on Windows
    ASSERT_TRUE(mtx->try_lock());
#else
    ASSERT_FALSE(mtx->try_lock());
#endif

    mtx->unlock();

    loop->run();
}

TEST(Mutex, RecursiveLockUnlock) {
    auto loop = uvw::loop::get_default();
    auto recursive_mtx = loop->resource<uvw::mutex>(true);

    recursive_mtx->lock();
    recursive_mtx->unlock();

    recursive_mtx->lock();
    ASSERT_TRUE(recursive_mtx->try_lock());
    recursive_mtx->unlock();
    recursive_mtx->unlock();

    loop->run();
}
