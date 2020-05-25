#ifndef UVW_CHECK_INCLUDE_H
#define UVW_CHECK_INCLUDE_H


#include <uv.h>
#include "handle.hpp"
#include "loop.h"
#include "config.h"

UVW_MSVC_WARNING_PUSH_DISABLE_DLLINTERFACE();

namespace uvw {


/**
 * @brief CheckEvent event.
 *
 * It will be emitted by CheckHandle according with its functionalities.
 */
struct UVW_EXTERN CheckEvent {};


/**
 * @brief The CheckHandle handle.
 *
 * Check handles will emit a CheckEvent event once per loop iteration, right
 * after polling for I/O.
 *
 * To create a `CheckHandle` through a `Loop`, no arguments are required.
 */
class UVW_EXTERN CheckHandle final : public Handle<CheckHandle, uv_check_t> {
    static void startCallback(uv_check_t *handle);

public:
    using Handle::Handle;

    /**
     * @brief Initializes the handle.
     * @return True in case of success, false otherwise.
     */
    bool init();

    /**
     * @brief Starts the handle.
     *
     * A CheckEvent event will be emitted once per loop iteration, right after
     * polling for I/O.
     */
    void start();

    /**
     * @brief Stops the handle.
     */
    void stop();
};


}


#ifndef UVW_AS_LIB
#include "check.cpp"
#endif

UVW_MSVC_WARNING_POP();

#endif // UVW_CHECK_INCLUDE_H
