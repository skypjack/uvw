#include <gtest/gtest.h>
#include <uvw.hpp>


TEST(Util, UnscopedFlags) {
    enum class UnscopedEnum { FOO = 1, BAR = 2, BAZ = 4, QUUX = 8 };

    uvw::Flags<UnscopedEnum> flags{};

    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>::from<UnscopedEnum::FOO, UnscopedEnum::BAR>()));
    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{UnscopedEnum::BAZ}));
    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{static_cast<uvw::Flags<UnscopedEnum>::Type>(UnscopedEnum::QUUX)}));

    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{std::move(flags)}));
    ASSERT_NO_THROW((flags = uvw::Flags<UnscopedEnum>{flags}));

    flags = uvw::Flags<UnscopedEnum>::from<UnscopedEnum::FOO, UnscopedEnum::QUUX>();

    ASSERT_TRUE(static_cast<bool>(flags));
    ASSERT_EQ(static_cast<uvw::Flags<UnscopedEnum>::Type>(flags), 9);

    ASSERT_TRUE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::FOO>());
    ASSERT_FALSE(flags & UnscopedEnum::BAR);
    ASSERT_FALSE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::BAZ>());
    ASSERT_TRUE(flags & UnscopedEnum::QUUX);

    ASSERT_NO_THROW(flags = flags | UnscopedEnum::BAR);
    ASSERT_NO_THROW(flags = flags | uvw::Flags<UnscopedEnum>::from<UnscopedEnum::BAZ>());

    ASSERT_TRUE(flags & UnscopedEnum::FOO);
    ASSERT_TRUE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::BAR>());
    ASSERT_TRUE(flags & UnscopedEnum::BAZ);
    ASSERT_TRUE(flags & uvw::Flags<UnscopedEnum>::from<UnscopedEnum::QUUX>());
}


TEST(Util, ScopedFlags) {
    enum class ScopedEnum { FOO = 1, BAR = 2, BAZ = 4, QUUX = 8 };

    uvw::Flags<ScopedEnum> flags{};

    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>::from<ScopedEnum::FOO, ScopedEnum::BAR>()));
    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{ScopedEnum::BAZ}));
    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{static_cast<uvw::Flags<ScopedEnum>::Type>(ScopedEnum::QUUX)}));

    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{std::move(flags)}));
    ASSERT_NO_THROW((flags = uvw::Flags<ScopedEnum>{flags}));

    flags = uvw::Flags<ScopedEnum>::from<ScopedEnum::FOO, ScopedEnum::QUUX>();

    ASSERT_TRUE(static_cast<bool>(flags));
    ASSERT_EQ(static_cast<uvw::Flags<ScopedEnum>::Type>(flags), 9);

    ASSERT_TRUE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::FOO>());
    ASSERT_FALSE(flags & ScopedEnum::BAR);
    ASSERT_FALSE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::BAZ>());
    ASSERT_TRUE(flags & ScopedEnum::QUUX);

    ASSERT_NO_THROW(flags = flags | ScopedEnum::BAR);
    ASSERT_NO_THROW(flags = flags | uvw::Flags<ScopedEnum>::from<ScopedEnum::BAZ>());

    ASSERT_TRUE(flags & ScopedEnum::FOO);
    ASSERT_TRUE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::BAR>());
    ASSERT_TRUE(flags & ScopedEnum::BAZ);
    ASSERT_TRUE(flags & uvw::Flags<ScopedEnum>::from<ScopedEnum::QUUX>());
}


TEST(Util, Utilities) {
    // TODO
}
