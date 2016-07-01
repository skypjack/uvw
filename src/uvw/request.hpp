#pragma once


#include <uv.h>


namespace uvw {


template<uv_req_type>
struct RequestType;

template<> struct RequestType<UV_CONNECT> { };
template<> struct RequestType<UV_WRITE> { };
template<> struct RequestType<UV_SHUTDOWN> { };
template<> struct RequestType<UV_UDP_SEND> { };
template<> struct RequestType<UV_FS> { };
template<> struct RequestType<UV_WORK> { };
template<> struct RequestType<UV_GETADDRINFO> { };
template<> struct RequestType<UV_GETNAMEINFO> { };


template<typename T>
struct Request: T {
    // TODO room for a pointer to a memory pool and a better memory management
};


}
