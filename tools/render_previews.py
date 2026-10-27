"""Gera previews PNG a partir dos framebuffers reais do motor C++, sem Pillow."""
from pathlib import Path
import argparse,binascii,re,struct,zlib
ROOT=Path(__file__).resolve().parents[1]
NAMES=['Cobrinha','Pong','Blocos','Nave','Pulinho','Corrida Azul','Mini RPG','Labirintos','Pokemon Mini']
SLUGS=['cobrinha','pong','blocos','nave','pulinho','corrida-azul','mini-rpg','labirintos','pokemon-mini']

def png(path,width,height,pixels):
    def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',binascii.crc32(kind+data)&0xffffffff)
    rows=b''.join(b'\0'+pixels[y*width*3:(y+1)*width*3] for y in range(height))
    path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>2I5B',width,height,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(rows,9))+chunk(b'IEND',b''))

def render(input_folder,output):
    output.mkdir(parents=True,exist_ok=True)
    font_text=(ROOT/'src/font6x8.h').read_text(encoding='utf-8-sig')
    font=[list(map(int,row.split(','))) for row in re.findall(r'\{([\d,]+)\}',font_text)]
    assert len(font)==95
    cards=[]
    for i,name in enumerate(NAMES):
        raw=(input_folder/f'game-{i}.oled').read_bytes();leds=(input_folder/f'game-{i}.rgb').read_bytes();assert len(raw)==1024 and len(leds)==75
        w,h=1000,550;canvas=bytearray(bytes([13,22,35])*w*h)
        def rect(x,y,ww,hh,color):
            for yy in range(max(0,y),min(h,y+hh)):
                lo=max(0,x);hi=min(w,x+ww)
                if hi>lo:canvas[(yy*w+lo)*3:(yy*w+hi)*3]=bytes(color)*(hi-lo)
        def text(x,y,label,scale=2,color=(226,235,245)):
            for c in label:
                glyph=font[ord(c)-32] if 32<=ord(c)<=126 else font[ord('?')-32]
                for xx,column in enumerate(glyph):
                    for yy in range(8):
                        if column&(1<<yy):rect(x+xx*scale,y+yy*scale,scale,scale,color)
                x+=6*scale
        text(28,22,name.upper(),4);text(28,65,'OLED 128 x 64 | PREVIA DO MOTOR',2,color=(105,180,220))
        rect(20,102,784,400,(40,60,80));rect(28,110,768,384,(0,0,0))
        for y in range(64):
            for x in range(128):
                if raw[(y//8)*128+x]&(1<<(y%8)):rect(28+x*6,110+y*6,6,6,(225,247,255))
        text(832,120,'MATRIZ',3)
        for y in range(5):
            for x in range(5):
                c=leds[(y*5+x)*3:(y*5+x+1)*3]
                rect(830+x*30,175+y*30,24,24,tuple(c) if any(c) else (24,38,50))
        text(28,520,'B/DIREITA: ACAO   A/ESQUERDA: PAUSA   SW: MENU',2)
        png(output/(SLUGS[i]+'.png'),w,h,canvas);cards.append((w,h,canvas))
    columns=3 if len(cards)%3==0 else 2;ow,oh=500*columns,275*((len(cards)+columns-1)//columns);overview=bytearray(bytes([13,22,35])*ow*oh)
    for i,(w,h,card) in enumerate(cards):
        x0=(i%columns)*500;y0=(i//columns)*275
        for y in range(275):
            for x in range(500):
                src=((y*2)*w+x*2)*3;dst=((y0+y)*ow+x0+x)*3;overview[dst:dst+3]=card[src:src+3]
    png(output/'todos-os-jogos.png',ow,oh,overview)
    (output/'README.md').write_text('# Origem das imagens\n\nCenarios demonstrativos deterministas renderizados pelo mesmo motor C++ dos jogos. O OLED mostra o framebuffer de 128x64 ampliado sem suavizacao; a matriz usa os 25 pixels calculados pelo motor. Sao previews de software, nao fotografias da placa nem prova de validacao fisica da versao atual.\n',encoding='utf-8')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('input',type=Path);p.add_argument('output',type=Path);a=p.parse_args();render(a.input,a.output)
