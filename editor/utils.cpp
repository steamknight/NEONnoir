#pragma warning(push)
#pragma warning(disable:26812)

#include <nfd.h>
#include "utils.h"

namespace NEONnoir
{
    std::optional<std::string_view> open_file_dialog(std::string_view const& filter)
    {
        nfdchar_t* path = nullptr;
        if (NFD_OpenDialog(filter.data(), nullptr, &path) == NFD_OKAY)
        {
            return std::string_view{ path };
        }

        return {};
    }

    std::optional<std::string_view> save_file_dialog(std::string_view const& filter)
    {
        nfdchar_t* path = nullptr;
        if (NFD_SaveDialog(filter.data(), nullptr, &path) == NFD_OKAY)
        {
            return std::string_view{ path };
        }

        return {};
    }

}

#pragma warning(pop)