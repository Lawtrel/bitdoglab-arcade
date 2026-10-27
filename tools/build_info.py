"""Manifesto publicavel da compilacao CI; sem caminhos da maquina."""
import hashlib,json,os,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
files={p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (root/'src').rglob('*') if p.is_file()}
result={'source_commit':os.environ.get('GITHUB_SHA'),'source_files_sha256':files,'compiler':subprocess.check_output(['arm-none-eabi-g++','--version'],text=True).splitlines()[0],'sdk_commit':'95ea6acad131124694cda1c162c52cd30e0aece0','controls':{'action':'B/right/GPIO6','pause':'A/left/GPIO5','menu':'joystick/GPIO22'},'host_tests':'passed_before_build','preview_origin':'deterministic C++ framebuffer scenes, not device photos','physical_validation':'pending_for_this_version'}
(root/'firmware/build-info.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
