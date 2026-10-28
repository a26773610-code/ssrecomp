"""Fetch only public symbol metadata pinned by upstream; no ROM download."""
from pathlib import Path
import subprocess, sys
src = Path(sys.argv[1]).resolve()
out = Path(sys.argv[2]).resolve(); out.mkdir(parents=True, exist_ok=True)

def run(*args):
    subprocess.run([str(a) for a in args], check=True)

def symbols(name, repo, commit, pattern):
    dest = src.parent / name
    run('git', 'init', dest)
    run('git', '-C', dest, 'remote', 'add', 'origin', repo)
    run('git', '-C', dest, 'sparse-checkout', 'set', '--no-cone', pattern)
    run('git', '-C', dest, 'fetch', '--depth', '1', '--filter=blob:none', 'origin', commit)
    run('git', '-C', dest, 'checkout', '--detach', 'FETCH_HEAD')
    return dest

for name, sha, cfg, code in [
    ('bw2-symbols','753b3fd97683fc619bb98b52e7276afac3d5e32f','b2_us','IREO'),
    ('w2-symbols','be2c125a700c23702ba1c5a4ef63e0e970bcc87a','w2_us','IRDO')]:
    d = symbols(name, 'https://github.com/fuddlesworth/pokebw2.git', sha, f'/config/{cfg}/arm9/**/symbols.txt')
    # Also include the main ARM9 symbols file.
    run('git', '-C', d, 'sparse-checkout', 'add', '--no-cone', f'/config/{cfg}/arm9/symbols.txt')
    run(sys.executable, src/'tools/bw2/funcmap.py', d/f'config/{cfg}/arm9', '-', out/f'{code}.txt')
d = symbols('bw1-symbols', 'https://github.com/Project-Holon/aletheia.git',
            '0d05b6ccc61fcea090d8318a00af9f1634810e18', '/layout/function_addrs.tsv')
for code in ('IRBO','IRAO'):
    run(sys.executable, src/'tools/bw1/funcmap.py', d/'layout/function_addrs.tsv', '-', out/f'{code}.txt', *(['--white'] if code=='IRAO' else []))
(out/'CPUE.txt').write_bytes((src/'tools/platinum/CPUE.txt').read_bytes())
assert all((out/f'{c}.txt').stat().st_size > 1000 for c in ('IRBO','IRAO','IREO','IRDO','CPUE'))
