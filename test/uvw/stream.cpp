#include <gtest/gtest.h>
#include <uvw/stream.h>

struct fake_stream_t {
    void *data;
};

struct fake_stream_handle: uvw::stream_handle<fake_stream_handle, fake_stream_t> {
    using stream_handle::stream_handle;

    template<typename... Args>
    int init(Args &&...) {
        return 0;
    }
};
