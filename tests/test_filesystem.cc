#include "doctest.h"
#include "filesystem.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

using namespace pixellib::core::filesystem;

static std::string make_temp_dir() {
    std::string base = FileSystem::temp_directory_path();
    if (base.empty()) return std::string();

    
    if (!base.empty() && (base.back() == '/' || base.back() == '\\')) {
        base.pop_back();
    }

    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    for (int i = 0; i < 100; ++i) {
        std::string dir = base + "/pixellib_test_" + std::to_string(std::rand());
        if (!FileSystem::exists(dir)) {
            if (FileSystem::create_directories(dir)) {
                return dir;
            }
        }
    }

    return std::string();
}

static void remove_dir_tree(const std::string &path) {
    auto entries = FileSystem::directory_iterator(path);
    for (const auto &e : entries) {
        std::string p = path + "/" + e;
        if (FileSystem::is_directory(p)) {
            remove_dir_tree(p);
            FileSystem::remove(p);
        } else {
            FileSystem::remove(p);
        }
    }
    FileSystem::remove(path);
}

TEST_SUITE("filesystem_module") {

TEST_CASE("read_write_and_exists") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string file = dir + "/test.txt";
    CHECK(FileSystem::write_file(file, "hello world"));
    CHECK(FileSystem::exists(file));
    CHECK(FileSystem::is_regular_file(file));

    std::string contents = FileSystem::read_file(file);
    CHECK(contents == "hello world");

    CHECK(FileSystem::file_size(file) == static_cast<long long>(contents.size()));

    CHECK(FileSystem::remove(file));
    CHECK(FileSystem::remove(dir));
}

TEST_CASE("create_directories_and_directory_iterator") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string nested = dir + "/a/b/c";
    CHECK(FileSystem::create_directories(nested));
    CHECK(FileSystem::is_directory(nested));

    std::string f1 = dir + "/a/f1.txt";
    std::string f2 = dir + "/a/b/f2.txt";
    CHECK(FileSystem::write_file(f1, "x"));
    CHECK(FileSystem::write_file(f2, "y"));

    auto entries = FileSystem::directory_iterator(dir + "/a");
    CHECK(std::find(entries.begin(), entries.end(), "f1.txt") != entries.end());
    CHECK(std::find(entries.begin(), entries.end(), "b") != entries.end());

    remove_dir_tree(dir);
}

TEST_CASE("copy_and_rename") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string src = dir + "/src.txt";
    std::string dst = dir + "/dst.txt";
    CHECK(FileSystem::write_file(src, "content"));
    CHECK(FileSystem::copy_file(src, dst));
    CHECK(FileSystem::exists(dst));

    std::string renamed = dir + "/renamed.txt";
    CHECK(FileSystem::rename(dst, renamed));
    CHECK(FileSystem::exists(renamed));

    remove_dir_tree(dir);
}

TEST_CASE("temp_and_current_path") {
    std::string tmp = FileSystem::temp_directory_path();
    CHECK(!tmp.empty());

    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());
    std::string cwd = FileSystem::current_path();

    CHECK(FileSystem::current_path(dir));
    std::string new_cwd = FileSystem::current_path();
    
    CHECK(FileSystem::exists(new_cwd));
    CHECK(FileSystem::is_directory(new_cwd));

    CHECK(FileSystem::current_path(cwd));
    remove_dir_tree(dir);
}

TEST_CASE("last_write_time") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string file = dir + "/test.txt";
    CHECK(FileSystem::write_file(file, "test content"));
    
    std::time_t mtime = FileSystem::last_write_time(file);
    CHECK(mtime != -1);
    CHECK(mtime > 0);

    CHECK(FileSystem::last_write_time(dir + "/nonexistent.txt") == -1);

    remove_dir_tree(dir);
}

TEST_CASE("create_directory_single_level") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string single = dir + "/single";
    CHECK(FileSystem::create_directory(single));
    CHECK(FileSystem::is_directory(single));

    remove_dir_tree(dir);
}

TEST_CASE("error_handling") {
    std::string content = FileSystem::read_file("nonexistent_file_xyz123.txt");
    CHECK(content.empty());

    CHECK(FileSystem::file_size("nonexistent_file_xyz123.txt") == -1);

    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());
    CHECK_FALSE(FileSystem::copy_file("nonexistent_source_xyz123.txt", dir + "/dest.txt"));

    CHECK_FALSE(FileSystem::remove(dir + "/nonexistent.txt"));

    CHECK_FALSE(FileSystem::exists("nonexistent_path_xyz123"));

    CHECK_FALSE(FileSystem::is_directory("nonexistent_path_xyz123"));

    CHECK_FALSE(FileSystem::is_regular_file("nonexistent_path_xyz123"));

    remove_dir_tree(dir);
}

}
