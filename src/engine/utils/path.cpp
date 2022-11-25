#include "path.hpp"

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
