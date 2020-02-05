#include "stream.h"

namespace uvw {

    void details::ShutdownReq::shutdown(uv_stream_t *handle) {
        invoke(&uv_shutdown, get(), handle, &defaultCallback<ShutdownEvent>);
    }

    void details::WriteReq::write(uv_stream_t *handle) {
        invoke(&uv_write, get(), handle, &buf, 1, &defaultCallback<WriteEvent>);
    }

    void details::WriteReq::write(uv_stream_t *handle, uv_stream_t *send) {
        invoke(&uv_write2, get(), handle, &buf, 1, send, &defaultCallback<WriteEvent>);
    }

}
