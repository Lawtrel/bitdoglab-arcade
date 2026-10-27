from pathlib import Path
import argparse,json,time
import serial
from serial.tools.list_ports import comports

parser=argparse.ArgumentParser(description='Consulta o firmware arcade, sem gravar a placa.')
parser.add_argument('--out',type=Path,required=True)
args=parser.parse_args()
found=None;deadline=time.monotonic()+30
while time.monotonic()<deadline:
    ports=[p for p in comports() if p.vid==0x2e8a]
    if len(ports)>1:raise RuntimeError('Mais de uma porta Pico; conecte apenas a BitDogLab.')
    if len(ports)==1:found=ports[0].device;break
    time.sleep(1)
if found is None:raise RuntimeError('Porta USB nao apareceu em 30 segundos; confira o OLED e reconecte sem BOOTSEL.')
with serial.Serial(found,115200,timeout=.3,write_timeout=3) as connection:
    connection.dtr=True;connection.reset_input_buffer();connection.write(b'?')
    deadline=time.monotonic()+12
    while time.monotonic()<deadline:
        try:result=json.loads(connection.readline(4096))
        except (ValueError,UnicodeDecodeError):continue
        if result.get('app')=='bitdoglab_arcade_v3':break
    else:raise RuntimeError('O firmware Arcade nao respondeu. Feche outros monitores e confira a gravacao.')
if result.get('games')!=9 or result.get('selftest') is not True:raise RuntimeError('Resposta de autoteste/jogos invalida.')
result['port']=found;result['verified_at_unix']=time.time()
args.out.write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result,indent=2))
print('ARCADE_EXECUTANDO: autoteste aprovado; resposta USB recebida.')
if not result.get('oled'):print('ATENCAO: OLED nao respondeu ao firmware. Verifique a tela antes de considerar a instalacao concluida.')
print('A resposta USB nao confirma visualmente matriz, som e direcao do joystick.')
