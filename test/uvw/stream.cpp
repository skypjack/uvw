#include <gtest/gtest.h>
#include <uvw/stream.h>

struct fake_stream_t {
    void *data;
};

struct FakeStreamHandle: uvw::StreamHandle<FakeStreamHandle, fake_stream_t> {
    using StreamHandle::StreamHandle;

    template<typename... Args>
    bool init(Args &&...) {
        return true;
    }
};
