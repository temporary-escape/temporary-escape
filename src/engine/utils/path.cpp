#include "path.hpp"
#include "exceptions.hpp"
#include <fstream>

#ifdef _WIN32
#include <Shlobj.h>
#include <Windows.h>
#include <direct.h>
#else
#include <climits>
#include <pwd.h>
#include <unistd.h>
#endif

using namespace Engine;

Path Engine::getAppDataPath() {
#ifdef _WIN32
    char appDataPath[MAX_PATH];
    SHGetSpecialFolderPathA(nullptr, appDataPath, CSIDL_APPDATA, false);
    return std::filesystem::path(std::string(appDataPath)).append("Temporary Escape");
#else
    struct passwd* pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    return std::filesystem::path(std::string(homedir)).append(".temporary-escape");
#endif
}

Path Engine::getExecutablePath() {
#ifdef _WIN32
    std::string exePath;
    exePath.resize(MAX_PATH);
    GetModuleFileNameA(nullptr, &exePath[0], static_cast<DWORD>(exePath.size()));
    exePath.resize(std::strlen(exePath.c_str()));
    return std::filesystem::path(exePath).parent_path();
#else
    std::string result;
    result.resize(PATH_MAX);
    std::string szTmp;
    szTmp.resize(32);
    sprintf(&szTmp[0], "/proc/%d/exe", getpid());
    int bytes = std::min(readlink(&szTmp[0], &result[0], result.size()), ssize_t(result.size() - 1));
    if (bytes >= 0)
        result[bytes] = '\0';
    result.resize(std::strlen(result.c_str()));
    return std::filesystem::path(result).parent_path();
#endif
}

std::string Engine::readFileStr(const Path& path) {
    std::ifstream file(path, std::ios::in);
    if (!file) {
        EXCEPTION("Failed to open file: '{}'", path);
    }

    return {(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
}

std::vector<char> Engine::readFileBinary(const Path& path) {
    std::vector<char> data;

    std::fstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        EXCEPTION("Failed to open file: '{}'", path);
    }

    file.seekg(0, std::ios::end);
    data.resize(file.tellg());
    file.seekg(0, std::ios::beg);

    file.read(data.data(), data.size());

    return data;
}

void Engine::writeFileBinary(const Path& path, const void* data, const size_t size) {
    std::fstream file(path, std::ios::out | std::ios::binary);
    if (!file) {
        EXCEPTION("Failed to open file: '{}'", path);
    }

    file.write(reinterpret_cast<const char*>(data), size);
}

void Engine::writeFileBinary(const Path& path, const std::vector<char>& data) {
    writeFileBinary(path, data.data(), data.size());
}
