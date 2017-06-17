#include <gtest/gtest.h>
#include <uvw.hpp>


struct fake_stream_t { void *data; };


struct FakeStreamHandle: uvw::StreamHandle<FakeStreamHandle, fake_stream_t> {
    using StreamHandle::StreamHandle;

    template<typename... Args>
    bool init(Args&&...) { return true; }
};


TEST(Stream, TODO) {
    auto loop = uvw::Loop::getDefault();
    auto handle = FakeStreamHandle::create(loop);

    handle = nullptr;

    // TODO

}
