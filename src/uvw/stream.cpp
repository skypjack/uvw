#include "stream.h"
#include "defines.h"

namespace uvw {

    UVW_INLINE_SPECIFIER void details::ShutdownReq::shutdown(uv_stream_t *handle) {
        invoke(&uv_shutdown, get(), handle, &defaultCallback < ShutdownEvent > );
    }

    UVW_INLINE_SPECIFIER void details::WriteReq::write(uv_stream_t *handle) {
        invoke(&uv_write, get(), handle, &buf, 1, &defaultCallback < WriteEvent > );
    }

    UVW_INLINE_SPECIFIER void details::WriteReq::write(uv_stream_t *handle, uv_stream_t *send) {
        invoke(&uv_write2, get(), handle, &buf, 1, send, &defaultCallback < WriteEvent > );
    }

}
