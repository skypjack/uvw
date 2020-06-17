#ifdef UVW_AS_LIB
#include "stream.h"
#endif

#include "config.h"


namespace uvw {

    template<typename Deleter>
    UVW_INLINE void WriteReq<Deleter>::write(uv_stream_t *handle)  {
        this->invoke(&uv_write, this->get(), handle, &buf, 1, &this->template defaultCallback<WriteEvent>);
    }

    template<typename Deleter>
    UVW_INLINE void WriteReq<Deleter>::write(uv_stream_t *handle, uv_stream_t *send) {
        this->invoke(&uv_write2, this->get(), handle, &buf, 1, send, &this->template defaultCallback<WriteEvent>);
    }

    template<typename Deleter>
    UVW_INLINE WriteReq<Deleter>::WriteReq(WriteReq::ConstructorAccess ca, std::shared_ptr<Loop> loop,
                                std::unique_ptr<char[], Deleter> dt, unsigned int len)
            : Request<WriteReq<Deleter>, uv_write_t>{ca, std::move(loop)},
              data{std::move(dt)},
              buf{uv_buf_init(data.get(), len)}
    {}


    UVW_INLINE DataEvent::DataEvent(std::unique_ptr<char[]> buf, std::size_t len) noexcept
    : data{std::move(buf)}, length{len}
{}


UVW_INLINE void details::ShutdownReq::shutdown(uv_stream_t *handle) {
    invoke(&uv_shutdown, get(), handle, &defaultCallback<ShutdownEvent>);
}

template WriteReq<std::default_delete<char []> >::WriteReq(WriteReq::ConstructorAccess ca, std::shared_ptr<Loop> loop,
                                                            std::unique_ptr<char[], std::default_delete<char []>> dt, unsigned int len);
template WriteReq<void (*)(char*) >::WriteReq(WriteReq::ConstructorAccess ca, std::shared_ptr<Loop> loop,
                                                            std::unique_ptr<char[], void (*)(char*)> dt, unsigned int len);
template void WriteReq<std::default_delete<char []> >::write(uv_stream_s*);

template void WriteReq<void (*)(char*)>::write(uv_stream_s*);
template void WriteReq<std::default_delete<char []> >::write(uv_stream_t *handle, uv_stream_t *send);

}
