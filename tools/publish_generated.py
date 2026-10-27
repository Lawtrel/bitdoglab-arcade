"""Publica apenas imagens e firmware gerados pelo workflow deste repositorio."""
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def git(*args,check=True):return subprocess.run(['git',*args],cwd=ROOT,check=check)
git('config','user.name','github-actions[bot]')
git('config','user.email','41898282+github-actions[bot]@users.noreply.github.com')
git('add','src','docs/images','firmware/bitdoglab_arcade.uf2','firmware/bitdoglab_arcade.bin','firmware/audit.json','firmware/build-info.json')
if git('diff','--cached','--quiet',check=False).returncode:
    git('commit','-m','Generate tested game previews and RP2040 firmware')
    git('push','origin','HEAD:main')
