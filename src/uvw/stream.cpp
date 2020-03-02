#include "stream.h"
#include "config.h"


namespace uvw {


UVW_INLINE DataEvent::DataEvent(std::unique_ptr<char[]> buf, std::size_t len) noexcept
    : data{std::move(buf)}, length{len}
{}


UVW_INLINE void details::ShutdownReq::shutdown(uv_stream_t *handle) {
    invoke(&uv_shutdown, get(), handle, &defaultCallback<ShutdownEvent>);
}


UVW_INLINE details::WriteReq::WriteReq(ConstructorAccess ca, std::shared_ptr<Loop> loop, std::unique_ptr<char[], Deleter> dt, unsigned int len)
    : Request<WriteReq, uv_write_t>{ca, std::move(loop)},
      data{std::move(dt)},
      buf{uv_buf_init(data.get(), len)}
{}


UVW_INLINE void details::WriteReq::write(uv_stream_t *handle) {
    invoke(&uv_write, get(), handle, &buf, 1, &defaultCallback<WriteEvent>);
}


UVW_INLINE void details::WriteReq::write(uv_stream_t *handle, uv_stream_t *send) {
    invoke(&uv_write2, get(), handle, &buf, 1, send, &defaultCallback<WriteEvent>);
}


}
