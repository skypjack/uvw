#include "poll.h"

namespace uvw {

    void PollHandle::startCallback(uv_poll_t *handle, int status, int events) {
        PollHandle &poll = *(static_cast<PollHandle*>(handle->data));
        if(status) { poll.publish(ErrorEvent{status}); }
        else { poll.publish(PollEvent{static_cast<std::underlying_type_t<Event>>(events)}); }
    }

    bool PollHandle::init() {
        return (tag == SOCKET)
               ? initialize(&uv_poll_init_socket, socket)
               : initialize(&uv_poll_init, fd);
    }

    void PollHandle::start(Flags<PollHandle::Event> flags) {
        invoke(&uv_poll_start, get(), flags, &startCallback);
    }

    void PollHandle::start(PollHandle::Event event) {
        start(Flags<Event>{event});
    }

    void PollHandle::stop() {
        invoke(&uv_poll_stop, get());
    }
}
