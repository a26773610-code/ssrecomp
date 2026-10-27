#include "storage.h"

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#include <winrt/Windows.Storage.h>
#include <winrt/base.h>

namespace ssrecomp::uwp {
std::string LocalDataDirectoryUtf8()
{
    const auto path = winrt::Windows::Storage::ApplicationData::Current().LocalFolder().Path();
    return winrt::to_string(path);
}
}
#else
namespace ssrecomp::uwp {
std::string LocalDataDirectoryUtf8() { return {}; }
}
#endif
