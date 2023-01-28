#include "pch.h"
#include "GetPathToProgramDirectory.h"

std::optional<std::string> GetPathToProgramDirectory()
{
    // Get user AppData folder
    PWSTR path_tmp;
    const auto app_data_path_res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path_tmp);

    // Convert to C++ path
    std::filesystem::path app_data_path{path_tmp};

    // Free resources for path_tmp
    CoTaskMemFree(path_tmp);

    if (app_data_path_res == S_OK)
    {
        static const auto folder_name = "\\GuildWarsSM";

        const auto full_path = app_data_path.string() + folder_name;

        return full_path;
    }

    return std::nullopt;
}
