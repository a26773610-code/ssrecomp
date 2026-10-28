#pragma once
#include <string>

namespace ssrecomp::uwp {
// Writable application-local data directory on UWP.
// Returns an empty string on unsupported platforms.
std::string LocalDataDirectoryUtf8();
}
