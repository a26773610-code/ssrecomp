"""Apply the Xbox frontend adapter to the pinned Switch sources; never reads a ROM."""
from pathlib import Path
import json, subprocess, sys
src = Path(sys.argv[1]).resolve()
here = Path(__file__).resolve().parent

def replace(path, old, new):
    p = src / path
    s = p.read_text()
    if old not in s:
        raise RuntimeError(f'Pinned source mismatch: {path}: {old[:60]}')
    p.write_text(s.replace(old, new), newline='\n')

# The application is compiled with ClangCL to retain the original SIMD vectors,
# compiler intrinsics and emulator CPU implementation without rewriting them.
replace('melonDS/src/FATStorage.cpp', '#include <dirent.h>\n', '')
replace('melonDS/src/CMakeLists.txt', 'target_include_directories(core PUBLIC ${DIRENT_INCLUDE_DIRS})',
        'if(DIRENT_INCLUDE_DIRS)\n            target_include_directories(core PUBLIC ${DIRENT_INCLUDE_DIRS})\n        endif()')
replace('melonDS/src/CMakeLists.txt', 'target_link_libraries(core PRIVATE ole32 comctl32 wsock32 ws2_32)',
        'target_link_libraries(core PRIVATE WindowsApp)')
replace('melonDS/CMakeLists.txt', 'if (NOT APPLE)\n    set(CMAKE_EXE_LINKER_FLAGS_RELEASE',
        'if (NOT APPLE AND NOT MSVC)\n    set(CMAKE_EXE_LINKER_FLAGS_RELEASE')
replace('melonDS/src/CMakeLists.txt', 'set(SSR_WIDE_FLAGS -O3 -ffast-math)',
        'set(SSR_WIDE_FLAGS /O2 /clang:-ffast-math)')
replace('melonDS/src/GPU3D_Wide.cpp', 'namespace melonDS\n{',
        '#ifdef _WIN32\n#undef near\n#undef far\n#endif\nnamespace melonDS\n{')
for p in (src / 'frontend').glob('*'):
    if p.suffix in ('.h', '.cpp'):
        s = p.read_text()
        # SDL's installed CMake target provides the include/SDL2 directory.
        p.write_text(s.replace('<SDL2/', '<'), newline='\n')
replace('frontend/main.cpp', '#include "host.h"',
        '#include "host.h"\n#include "xbox_start.h"\n#include "gamepad_reader.h"')
replace('frontend/main.cpp', '    Options opt;\n#ifdef __ANDROID__',
        '    if (!XboxStart(argc, argv)) return 1;\n    Options opt;\n#ifdef __ANDROID__')
# Reuse the same native controller mapping already developed on xbox-uwp-port.
previous_main = (here.parent.parent.parent / 'frontend/main.cpp').read_text()
a = previous_main.index('    ssrecomp::uwp::GamepadState xbox;')
b = previous_main.index('\n#endif', a)
start = (src / 'frontend/main.cpp').read_text().index('static PadState ReadPad(')
p = src / 'frontend/main.cpp'
s = p.read_text(); end = s.index('    return st;', start)
s = s[:end] + previous_main[a:b] + '\n' + s[end:]
p.write_text(s, newline='\n')
replace('frontend/main.cpp', '        {\n            // In every log:',
        '        ssrecomp::uwp::InstallXboxBackHandler();\n        {\n            // In every log:')

# The layout already contains instruction-range addresses and overlay identities.
# Emit watch ranges only, never ARM instructions or static recompiled game code.
layout = json.loads((src / 'layouts/soulsilver-us.json').read_text())
base = src / 'build-watch-input'; base.mkdir(exist_ok=True)
sections = layout['sections']
chunks = []; ranges = []; section_rows = []
for i, sec in enumerate(sections):
    rows = sec['ranges']; first = len(chunks)
    last = sec['base']
    for a, b, thumb in rows:
        assert sec['base'] <= a < b <= sec['base'] + sec['size']
        assert a >= last and thumb in (0, 1)
        last = b
        chunks.append(f'    {{0x{a:08X}u, 0x{b:08X}u, {thumb}, {i}, nullptr}},')
    ranges.append(f'const RcRange ranges{i}[] = {{' + ','.join(f'{{{a}u,{b}u}}' for a,b,_ in rows) + '};')
    section_rows.append(f'    {{"{sec["name"]}", 0x{sec["base"]:08X}u, 0x{sec["size"]:08X}u, nullptr, ranges{i}, {len(rows)}, {first}, {len(rows)}}},')
(base / 'rc_tables.cpp').write_text('#include "recomp_rt.h"\n' + '\n'.join(ranges) + '\nconst RcChunk rc_chunks[] = {\n' + '\n'.join(chunks) + '\n};\nconst melonDS::u32 rc_num_chunks = '+str(len(chunks))+';\nRcSection rc_sections[] = {\n'+'\n'.join(section_rows)+'\n};\nconst melonDS::u32 rc_num_sections = '+str(len(sections))+';\n')
subprocess.run([sys.executable, str(src/'tools/apps.py'), '--layout', str(src/'layouts/soulsilver-us.json'), str(base)], check=True)
subprocess.run([sys.executable, str(src/'tools/soulsilver/tables_only.py'), str(base), str(src/'build-watch')], check=True)
# Every watched UI function must be inside a matching instruction range.
by_name = {s['name']: s for s in sections}
for row in layout['apps'] + layout['focus']:
    name, sec, address = row[:3]
    assert any(a <= (address & ~1) < b for a,b,_ in by_name[sec]['ranges']), (name, sec, address)
print(f'Validated {len(chunks)} address-only ranges, {len(sections)} sections; CPU/runtime sources unchanged.')
