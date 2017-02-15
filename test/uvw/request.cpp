#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Request, Functionalities) {
    auto loop = uvw::Loop::getDefault();
    auto req = loop->resource<uvw::WorkReq>([](){});

    ASSERT_NE(req->size(), decltype(req->size()){0});
    ASSERT_FALSE(req->cancel());

    loop->run();
}
