"""Audita UF2 RP2040 e equivalencia com o BIN, sem gravar a placa."""
from pathlib import Path
import argparse,hashlib,json,struct

def audit(folder):
    uf2=folder/'bitdoglab_arcade.uf2';binary=folder/'bitdoglab_arcade.bin'
    blob=uf2.read_bytes();raw=binary.read_bytes()
    if len(blob)%512 or not blob:raise ValueError('Tamanho UF2 invalido')
    total=len(blob)//512;seen=set();covered=bytearray(len(raw))
    for offset in range(0,len(blob),512):
        block=blob[offset:offset+512]
        m0,m1,flags,address,length,index,count,family=struct.unpack_from('<8I',block)
        if (m0,m1,struct.unpack_from('<I',block,508)[0])!=(0x0A324655,0x9E5D5157,0x0AB16F30):raise ValueError('Magic UF2 invalido')
        if count!=total or index in seen or index>=total:raise ValueError('Indices UF2 inconsistentes')
        if not flags&0x2000 or family!=0xE48BFF56 or flags&1:raise ValueError('UF2 nao e imagem RP2040 esperada')
        if not 0<length<=476 or address<0x10000000 or address+length>0x10200000:raise ValueError('Destino UF2 fora da flash Pico W')
        seen.add(index);start=address-0x10000000;end=min(len(raw),start+length)
        if start>=len(raw):raise ValueError('UF2 contem bloco alem do BIN')
        if any(covered[start:end]):raise ValueError('Sobreposicao de blocos')
        if raw[start:end]!=block[32:32+end-start]:raise ValueError('UF2 e BIN nao coincidem')
        if end-start<length and any(block[32+end-start:32+length]):raise ValueError('Padding UF2 nao e zero')
        covered[start:end]=b'\x01'*(end-start)
    if not all(covered):raise ValueError('BIN nao foi coberto integralmente')
    report={'status':'compiled_and_audited_not_physically_verified','app':'bitdoglab_arcade_v3','games':9,
        'uf2_sha256':hashlib.sha256(blob).hexdigest(),'uf2_bytes':len(blob),'blocks':total,
        'flash_image_bytes':len(raw),'bin_sha256':hashlib.sha256(raw).hexdigest(),
        'selftest':'Embedded self-test runs after boot; not executed by this audit.',
        'games_list':['Cobrinha','Pong','Blocos','Nave','Pulinho','Corrida Azul','Mini RPG','Labirintos','Pokemon Mini']}
    (folder/'audit.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    return report

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('folder',type=Path);args=parser.parse_args()
    print(json.dumps(audit(args.folder),indent=2))
