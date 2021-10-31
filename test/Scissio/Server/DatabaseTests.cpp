#include "../Common.hpp"
#include <Server/Database.hpp>

TEST("Put Get Delete") {
    auto tmpDir = std::filesystem::temp_directory_path();
    Database db(tmpDir / "scissio");
}
