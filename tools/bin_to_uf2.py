"""Empacota um BIN XIP RP2040 em UF2, sem executar picotool."""
import argparse,struct
from pathlib import Path

def convert(source,target,replace=False):
    data=source.read_bytes()
    if len(data)<256 or len(data)>2*1024*1024:raise ValueError('BIN fora do tamanho esperado para Pico W')
    count=(len(data)+255)//256
    with target.open('wb' if replace else 'xb') as output:
        for index in range(count):
            payload=data[index*256:(index+1)*256].ljust(256,b'\x00')
            output.write(struct.pack('<8I',0x0A324655,0x9E5D5157,0x2000,0x10000000+index*256,256,index,count,0xE48BFF56)+payload+bytes(220)+struct.pack('<I',0x0AB16F30))
    return count

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('source',type=Path);parser.add_argument('target',type=Path);parser.add_argument('--replace',action='store_true');args=parser.parse_args();convert(args.source,args.target,args.replace)
