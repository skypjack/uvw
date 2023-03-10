#ifndef UVW_WORK_INCLUDE_H
#define UVW_WORK_INCLUDE_H

#include <functional>
#include <memory>
#include <uv.h>
#include "loop.h"
#include "request.hpp"

namespace uvw {

/*! @brief Work event. */
struct work_event {};

/**
 * @brief The work request.
 *
 * It runs user code using a thread from the threadpool and gets notified in the
 * loop thread by means of an event.
 *
 * To create a `work_req` through a `loop`, arguments follow:
 *
 * * A valid instance of a `Task`, that is of type `std::function<void(void)>`.
 *
 * See the official
 * [documentation](http://docs.libuv.org/en/v1.x/threadpool.html)
 * for further details.
 */
class work_req final: public request<work_req, uv_work_t, work_event> {
    static void work_callback(uv_work_t *req);
    static void after_work_callback(uv_work_t *req, int status);

public:
    using task = std::function<void(void)>;

    explicit work_req(loop::token token, std::shared_ptr<loop> ref, task t);

    /**
     * @brief Runs the given task in a separate thread.
     *
     * A work event will be emitted on the loop thread when the task is
     * finished.<br/>
     * This request can be cancelled with `cancel()`.
     *
     * @return Underlying return value.
     */
    int queue();

private:
    task func{};
};

} // namespace uvw

#ifndef UVW_AS_LIB
#    include "work.cpp"
#endif

#endif // UVW_WORK_INCLUDE_H
