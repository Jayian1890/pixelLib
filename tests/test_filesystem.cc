#include "doctest.h"
#include "interlaced_core/filesystem.hpp"

TEST_SUITE("interlaced_core_filesystem") {

TEST_CASE("test") {
    CHECK(interlaced::core::filesystem::FileSystem::exists("/nonexistent") == false);
    CHECK(interlaced::core::filesystem::FileSystem::exists("/tmp") == true);
}

} // TEST_SUITE
