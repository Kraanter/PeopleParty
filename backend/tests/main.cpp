#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("Example test") {
    int x = 1;
    int y = 2;
    CHECK(x + y == 3);
}
