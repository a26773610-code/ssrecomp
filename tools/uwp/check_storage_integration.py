#!/usr/bin/env python3
"""Source-only checks; this does not build a UWP app."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
platform = (root / "frontend/platform.cpp").read_text(encoding="utf-8")
header = (root / "frontend/uwp/storage.h").read_text(encoding="utf-8")
source = (root / "frontend/uwp/storage.cpp").read_text(encoding="utf-8")
checks = {
    "UWP storage included in CMake": "frontend/uwp/storage.cpp" in cmake,
    "Platform references UWP storage": '"uwp/storage.h"' in platform,
    "Platform calls storage path helper": "ssrecomp::uwp::LocalDataDirectoryUtf8()" in platform,
    "Storage API declared": "std::string LocalDataDirectoryUtf8();" in header,
    "Storage API implemented": "std::string LocalDataDirectoryUtf8()" in source,
    "UWP sandbox storage selected": "ApplicationData::Current().LocalFolder()" in source,
    "Windows platform family header included": "#include <winapifamily.h>" in platform,
    "Local file existence uses sandbox": "FileExists(path)" in platform,
    "Local file open uses sandbox": "OpenFile(localPath, mode)" in platform,
    "Local write check uses sandbox": "CheckFileWritable(localPath)" in platform,
}
for label, passed in checks.items():
    print(f"{'PASS' if passed else 'FAIL'}: {label}")
if not all(checks.values()):
    raise SystemExit(1)
print("Source checks passed. No Xbox package was built.")
