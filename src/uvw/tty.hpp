#pragma once


#include <utility>
#include <cstddef>
#include <memory>
#include <uv.h>
#include "event.hpp"
#include "stream.hpp"
#include "util.hpp"


namespace uvw {


class TTY final: public Stream<TTY> {
    explicit TTY(std::shared_ptr<Loop> ref,
                 FileDescriptor desc,
                 bool readable)
        : Stream{HandleType<uv_tty_t>{}, std::move(ref)},
          fd{static_cast<FileDescriptor::Type>(desc)},
          rw{readable ? 1 : 0}
    { }

public:
    enum class Mode: unsigned short int { NORMAL, RAW, IO };

    template<typename... Args>
    static std::shared_ptr<TTY> create(Args&&... args) {
        return std::shared_ptr<TTY>{new TTY{std::forward<Args>(args)...}};
    }

    bool init() { return initialize<uv_tty_t>(&uv_tty_init, fd, rw); }

    void mode(TTY::Mode m) {
        // uv_tty_set_mode is inline, cannot be used with invoke directly
        auto wrap = [](auto *handle, auto m) {
            return uv_tty_set_mode(handle, m);
        };

        switch(m) {
        case TTY::Mode::NORMAL:
            invoke(std::move(wrap), get<uv_tty_t>(), UV_TTY_MODE_NORMAL);
            break;
        case TTY::Mode::RAW:
            invoke(std::move(wrap), get<uv_tty_t>(), UV_TTY_MODE_RAW);
            break;
        case TTY::Mode::IO:
            invoke(std::move(wrap), get<uv_tty_t>(), UV_TTY_MODE_IO);
            break;
        }
    }

    void reset() { invoke(&uv_tty_reset_mode); }

    WinSize getWinSize() {
        std::pair<int, int> size{0, 0};
        int width;
        int height;

        if(0 == invoke(&uv_tty_get_winsize, get<uv_tty_t>(), &width, &height)) {
            size.first = width;
            size.second = height;
        }

        /**
         * See Boost/Mutant idiom:
         *     https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Boost_mutant
         */
        return reinterpret_cast<WinSize&>(size);
    }

private:
    FileDescriptor::Type fd;
    int rw;
};


}
