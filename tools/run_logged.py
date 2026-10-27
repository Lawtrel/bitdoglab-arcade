"""Executa ferramenta oficial sem transformar stderr informativo em erro PowerShell."""
from pathlib import Path
import subprocess,sys
log=Path(sys.argv[1]);args=sys.argv[2:]
with log.open('w',encoding='utf-8') as target:
    process=subprocess.Popen(args,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace')
    for line in process.stdout:
        target.write(line);target.flush();print(line,end='',flush=True)
    code=process.wait()
sys.exit(code)
