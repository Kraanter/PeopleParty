#include <doctest/doctest.h>
#include <string>

#include "../src/party.h"

TEST_CASE("Party test") {
    int i1 = generate_party_id();
    int i2 = generate_party_id();

    CHECK(i1 != i2);
    CHECK_EQ(std::to_string(i1).length(), 4);
    CHECK_EQ(std::to_string(i2).length(), 4);
}