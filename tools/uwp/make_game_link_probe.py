#!/usr/bin/env python3
"""Generate ROM-free C++ symbols to test linking the *actual* game frontend.

This is NOT game data, cannot run SoulSilver, and must never be packaged or
advertised as a playable game. The user's ROM-derived sources are separately
produced by tools/recomp.py --uwp on a trusted local Windows machine.
"""
import pathlib
import sys


def main():
    if len(sys.argv) != 2:
        raise SystemExit("Usage: python tools/uwp/make_game_link_probe.py OUTPUT_DIRECTORY")
    directory = pathlib.Path(sys.argv[1]).resolve()
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "rc_image_000.cpp").write_text(
        '// Dummy image used only for link testing. NOT SoulSilver game data.\n'
        'extern "C" const unsigned char rc_image_0[] = {0, 0, 0, 0};\n',
        encoding="utf-8",
    )
    (directory / "rc_tables.cpp").write_text(
        '// Public nonfunctional link probe (NOT the recompilation).\n'
        '#include "recomp_rt.h"\n'
        'extern "C" const unsigned char rc_image_0[];\n'
        'void rc_link_probe(melonDS::ARMv5*, melonDS::u32) {}\n'
        'const RcChunk rc_chunks[] = {{0x02000000u, 0x02000004u, 0, 0, rc_link_probe}};\n'
        'const melonDS::u32 rc_num_chunks = 1;\n'
        'static const RcRange rc_ranges_0[] = {{0x02000000u, 0x02000004u}};\n'
        'RcSection rc_sections[] = {{"public-link-probe", 0x02000000u, 4u, '
        'rc_image_0, rc_ranges_0, 1, 0, 1}};\n'
        'const melonDS::u32 rc_num_sections = 1;\n',
        encoding="utf-8",
    )
    (directory / "sources.cmake").write_text(
        'set(RC_GENERATED_SOURCES\n'
        '  "${CMAKE_CURRENT_LIST_DIR}/rc_image_000.cpp"\n'
        '  "${CMAKE_CURRENT_LIST_DIR}/rc_tables.cpp"\n'
        ')\n',
        encoding="utf-8",
    )
    print("ROM-free full-game link test generated. Not runnable SoulSilver.")


if __name__ == "__main__":
    main()
