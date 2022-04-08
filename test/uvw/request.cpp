#include <gtest/gtest.h>
#include <uvw/request.hpp>
#include <uvw/work.h>

TEST(Request, Functionalities) {
    auto loop = uvw::loop::get_default();
    auto req = loop->resource<uvw::work_req>([]() {});

    ASSERT_NE(req->size(), decltype(req->size()){0});
    ASSERT_EQ(0, req->cancel());

    loop->run();
}
