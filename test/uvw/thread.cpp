#include <gtest/gtest.h>
#include <uvw/thread.h>

TEST(Thread, Run) {
    auto loop = uvw::Loop::getDefault();
    auto has_run = std::make_shared<bool>();
    auto cb = [](std::shared_ptr<void> data) {
        if(auto has_run = std::static_pointer_cast<bool>(data); has_run) {
            *has_run = true;
        }
    };

    auto handle = loop->resource<uvw::Thread>(cb, has_run);

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

    loop->run();
}

TEST(Mutex, LockUnlock) {
    auto loop = uvw::Loop::getDefault();
    auto mtx = loop->resource<uvw::Mutex>();

    mtx->lock();

#ifdef _MSC_VER
    // this is allowed by libuv on Windows
    ASSERT_TRUE(mtx->tryLock());
#else
    ASSERT_FALSE(mtx->tryLock());
#endif

    mtx->unlock();
    ASSERT_TRUE(mtx->tryLock());

#ifdef _MSC_VER
    // this is allowed by libuv on Windows
    ASSERT_TRUE(mtx->tryLock());
#else
    ASSERT_FALSE(mtx->tryLock());
#endif

    mtx->unlock();

    loop->run();
}

TEST(Mutex, RecursiveLockUnlock) {
    auto loop = uvw::Loop::getDefault();
    auto recursive_mtx = loop->resource<uvw::Mutex>(true);

    recursive_mtx->lock();
    recursive_mtx->unlock();

    recursive_mtx->lock();
    ASSERT_TRUE(recursive_mtx->tryLock());
    recursive_mtx->unlock();
    recursive_mtx->unlock();

    loop->run();
}
