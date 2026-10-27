#pragma once
#include "../core/game_session.hpp"
namespace arcade::pokemon_game {
inline constexpr const char* names[4]={"PIKACHU","BULBASAUR","SQUIRTLE","PIDGEY"};
inline constexpr Color colors[4]={{255,220,0},{20,220,50},{10,100,255},{230,120,20}};
// Desenhos proprios de 16x16 pixels; nao utiliza sprites de ROM.
inline constexpr uint16_t sprites[4][16]={
 {0x2400,0x6600,0x6600,0x7c00,0x3e00,0x7f00,0x6d20,0x7f60,0x3fe0,0x1fc0,0x3fc0,0x3f80,0x3f00,0x3f00,0x3300,0x6300},
 {0x0180,0x03c0,0x07e0,0x0ff0,0x17e8,0x3ff0,0x7ff8,0x6ed8,0x7ff8,0x3ff0,0x3ff0,0x3ff0,0x3660,0x3660,0x6660,0x6000},
 {0x0f00,0x1f80,0x3680,0x3f80,0x1f00,0x0f00,0x3fc0,0x7fe0,0x6db0,0x6df0,0x7fe0,0x3fc0,0x1f80,0x3fc0,0x3180,0x6180},
 {0x0000,0x0380,0x07c0,0x0de0,0x1fc0,0x3fe0,0x7ff0,0xeef8,0x7ff0,0x3fe0,0x1fc0,0x0fc0,0x0f80,0x1980,0x3300,0x0000}
};
inline void sprite(Screen& screen,int x,int y,int species,int scale=1,bool mirror=false){
 for(int yy=0;yy<16;yy++)for(int xx=0;xx<16;xx++)if(sprites[species%4][yy]&(1u<<(15-xx)))screen.rect(x+(mirror?15-xx:xx)*scale,y+yy*scale,scale,scale);
}
inline void encounter(GameSession& s,int species,bool trainer=false){
 auto& p=s.pokemon;p.phase=PokemonPhase::Battle;p.wild=species%4;p.trainer=trainer;p.wildLevel=trainer?9:2+int(s.random()%5);p.wildMaxHp=trainer?48:15+p.wildLevel*3;p.wildHp=p.wildMaxHp;p.cursor=0;p.menuClock=s.elapsed+200;std::snprintf(p.feedback,sizeof(p.feedback),"%s",trainer?"DESAFIO DO GINASIO":"CRIATURA SELVAGEM");s.event=Event::Start;
}
inline int damage(int attacker,int defender,int level){
 int value=5+level;
 if((attacker==0&&(defender==2||defender==3))||(attacker==1&&defender==2))value*=2;
 if((attacker==0&&defender==1)||(attacker==2&&defender==1))value=(value+1)/2;
 return std::max(1,value);
}
inline void experience(GameSession& s,int amount){auto& m=s.pokemon.team[s.pokemon.active];m.xp+=amount;if(m.xp>=m.level*5){m.xp-=m.level*5;m.level++;m.maxHp+=4;m.hp=m.maxHp;}}
inline void counter(GameSession& s){
 auto& p=s.pokemon;auto& m=p.team[p.active];int hit=std::max(2,damage(p.wild,m.species,p.wildLevel)/2);m.hp=std::max(0,m.hp-hit);
 if(m.hp==0){int replacement=-1;for(int i=0;i<p.count;i++)if(p.team[i].hp>0){replacement=i;break;}
  if(replacement>=0){p.active=replacement;s.talk("TROCA DE CRIATURA",names[p.team[p.active].species],"B: CONTINUAR");}
  else {for(int i=0;i<p.count;i++)p.team[i].hp=p.team[i].maxHp;p.phase=PokemonPhase::Explore;s.px=3;s.py=4;s.talk("EQUIPE RECUPERADA","VOLTOU PARA A VILA","B: CONTINUAR");}
 }
}
inline void victory(GameSession& s){auto& p=s.pokemon;experience(s,8);s.point(p.trainer?150:15);p.phase=PokemonPhase::Explore;if(p.trainer){s.won=true;s.signal=GameSignal::Finished;s.event=Event::Win;}else s.talk("BATALHA VENCIDA","SUA EQUIPE GANHOU XP","B: CONTINUAR");}
inline void command(GameSession& s){
 auto& p=s.pokemon;auto& m=p.team[p.active];
 if(p.cursor==0){int hit=damage(m.species,p.wild,m.level);p.wildHp=std::max(0,p.wildHp-hit);std::snprintf(p.feedback,sizeof(p.feedback),"GOLPE! -%d HP",hit);s.event=Event::Shot;if(p.wildHp==0){victory(s);return;}counter(s);return;}
 if(p.cursor==1){if(p.trainer){s.talk("BATALHA DE TREINADOR","USE GOLPE OU POCAO","B: CONTINUAR");return;}if(!p.balls){s.talk("SEM BOLAS","PROFESSOR REABASTECE","B: CONTINUAR");return;}p.balls--;int chance=35+60*(p.wildMaxHp-p.wildHp)/p.wildMaxHp;
  if(int(s.random()%100)<chance){bool known=false;for(int i=0;i<p.count;i++)known|=p.team[i].species==p.wild;if(!known&&p.count<4){p.team[p.count++]={p.wild,p.wildLevel,p.wildMaxHp,p.wildMaxHp,0};s.point(30);}p.phase=PokemonPhase::Explore;experience(s,3);s.talk(known?"JA ESTA NA EQUIPE":"CAPTUROU!",names[p.wild],"B: CONTINUAR");s.event=Event::Win;return;}
  std::snprintf(p.feedback,sizeof(p.feedback),"ESCAPOU DA BOLA!");counter(s);return;
 }
 if(p.cursor==2){if(!p.potions){s.talk("SEM POCOES","PROFESSOR REABASTECE","B: CONTINUAR");return;}p.potions--;m.hp=std::min(m.maxHp,m.hp+18);std::snprintf(p.feedback,sizeof(p.feedback),"POCAO: +18 HP");s.event=Event::Point;counter(s);return;}
 if(p.trainer){s.talk("DESAFIO DO GINASIO","VENCA PARA CONCLUIR","B: CONTINUAR");return;}
 if(s.random()%100<80){p.phase=PokemonPhase::Explore;s.talk("FUGIU DA BATALHA","EXPLORE A ROTA","B: CONTINUAR");}else{std::snprintf(p.feedback,sizeof(p.feedback),"NAO CONSEGUIU FUGIR");counter(s);}
}
} // namespace arcade::pokemon_game
namespace arcade {
inline void GameSession::reset_pokemon(){
 pokemon=PokemonState{};pokemon.team[0]={0,5,32,32,0};pokemon.count=1;pokemon.active=0;lives=1;world.fill(0);seen.fill(false);monsters.fill({});px=3;py=4;gameClock=0;
 for(int y=0;y<20;y++)for(int x=0;x<32;x++){
  if(x==0||y==0||x==31||y==19)world[y*32+x]=1;
  else if((x>=8&&x<=13&&y>=3&&y<=8)||(x>=18&&x<=28&&y>=11&&y<=17))world[y*32+x]=7;
  else if(x>=19&&x<=24&&y>=3&&y<=7)world[y*32+x]=6;
  else if(x>=3&&x<=6&&y>=9&&y<=11)world[y*32+x]=1;
 }
 world[4*32+4]=4;world[15*32+10]=4;world[17*32+29]=5;reveal(32,20);
}
inline void GameSession::update_pokemon(const Input& in,unsigned dt){
 auto& p=pokemon;
 if(p.phase==PokemonPhase::Battle||p.phase==PokemonPhase::Party){
  if(elapsed>=p.menuClock){int previous=p.cursor;if(p.phase==PokemonPhase::Battle){if(std::fabs(in.x)>.5f)p.cursor^=1;else if(std::fabs(in.y)>.5f)p.cursor^=2;}else if(std::fabs(in.y)>.5f)p.cursor=(p.cursor+(in.y>0?1:4))%5;
   if(previous!=p.cursor){p.menuClock=elapsed+190;event=Event::Select;}
  }
  if(in.actionPressed){if(p.phase==PokemonPhase::Battle)pokemon_game::command(*this);else{if(p.cursor<p.count&&p.team[p.cursor].hp>0)p.active=p.cursor;p.phase=PokemonPhase::Explore;}}
  return;
 }
 if(in.actionPressed){bool npc=false;for(Cell d:{Cell{1,0},Cell{-1,0},Cell{0,1},Cell{0,-1}})npc|=tile(int(px)+d.x,int(py)+d.y)==4;
  if(npc){for(int i=0;i<p.count;i++)p.team[i].hp=p.team[i].maxHp;p.balls=std::max(p.balls,5);p.potions=std::max(p.potions,3);talk("EQUIPE CURADA","CAPTURE 3 E VA A LESTE","B: CONTINUAR");event=Event::Win;}
  else{p.phase=PokemonPhase::Party;p.cursor=p.active;p.menuClock=elapsed+200;event=Event::Select;}return;
 }
 gameClock+=dt;if(gameClock<140)return;int dx=0,dy=0;if(std::fabs(in.x)>.5f&&std::fabs(in.x)>=std::fabs(in.y))dx=in.x>0?1:-1;else if(std::fabs(in.y)>.5f)dy=in.y>0?1:-1;else return;gameClock=0;
 int xx=int(px)+dx,yy=int(py)+dy;auto t=tile(xx,yy);if(t==1||t==4||t==6)return;px=float(xx);py=float(yy);reveal(32,20);
 if(t==7&&random()%4==0){pokemon_game::encounter(*this,1+int(random()%3));return;}
 if(t==5){if(p.count>=3)pokemon_game::encounter(*this,0,true);else talk("GINASIO FECHADO","CAPTURE 3 ESPECIES","B: CONTINUAR");}
}
inline void GameSession::draw_pokemon(Screen& s){
 auto& p=pokemon;s.clear();char text[32];
 if(p.phase==PokemonPhase::Battle){auto& active=p.team[p.active];std::snprintf(text,sizeof(text),"%s L%d",pokemon_game::names[p.wild],p.wildLevel);s.text(43,0,text);
  s.rect(52,10,48,5,false);s.rect(53,11,46*p.wildHp/p.wildMaxHp,3);pokemon_game::sprite(s,94,18,p.wild);
  std::snprintf(text,sizeof(text),"%s L%d",pokemon_game::names[active.species],active.level);s.text(0,23,text);pokemon_game::sprite(s,13,5,active.species,1,true);
  s.rect(0,33,43,5,false);s.rect(1,34,41*active.hp/active.maxHp,3);std::snprintf(text,sizeof(text),"%d/%d",active.hp,active.maxHp);s.text(48,32,text);s.text(0,40,p.feedback);
  const char* options[4]={"GOLPE","BOLA","POCAO","FUGIR"};for(int i=0;i<4;i++){if(i==1||i==2)std::snprintf(text,sizeof(text),"%c%s%d",p.cursor==i?'>':' ',options[i],i==1?p.balls:p.potions);else std::snprintf(text,sizeof(text),"%c%s",p.cursor==i?'>':' ',options[i]);s.text((i%2)*64,48+(i/2)*8,text);}return;
 }
 if(p.phase==PokemonPhase::Party){s.center(0,"EQUIPE / B: ESCOLHER");for(int i=0;i<4;i++){if(i<p.count)std::snprintf(text,sizeof(text),"%c%s %d/%d",p.cursor==i?'>':' ',pokemon_game::names[p.team[i].species],p.team[i].hp,p.team[i].maxHp);else std::snprintf(text,sizeof(text),"%c-- VAZIO --",p.cursor==i?'>':' ');s.text(0,13+i*10,text);}s.text(0,55,p.cursor==4?">VOLTAR":" VOLTAR");return;}
 auto& active=p.team[p.active];std::snprintf(text,sizeof(text),"%s L%d",pokemon_game::names[active.species],active.level);s.text(0,0,text);std::snprintf(text,sizeof(text),"E%d/4",p.count);s.text(99,0,text);std::snprintf(text,sizeof(text),"HP%d/%d B:EQUIPE",active.hp,active.maxHp);s.text(0,8,text);
 int cx=std::max(0,std::min(16,int(px)-8)),cy=std::max(0,std::min(14,int(py)-3));
 for(int y=0;y<6;y++)for(int x=0;x<16;x++){int wx=cx+x,wy=cy+y,xx=x*8,yy=16+y*8;auto v=tile(wx,wy);if(!seen[wy*32+wx]){if((x+y)%2==0)s.pixel(xx+4,yy+4);continue;}
  if(v==1){s.line(xx+3,yy,xx,yy+5);s.line(xx+3,yy,xx+7,yy+5);s.line(xx,yy+5,xx+7,yy+5);s.rect(xx+3,yy+5,2,3);}
  if(v==6){s.line(xx,yy+2,xx+5,yy+2);s.line(xx+2,yy+5,xx+7,yy+5);}
  if(v==7){s.line(xx+1,yy+6,xx+1,yy+4);s.line(xx+4,yy+5,xx+4,yy+3);s.line(xx+6,yy+7,xx+6,yy+5);}
  if(v==4){s.rect(xx+2,yy,3,3);s.rect(xx+1,yy+4,5,4);s.pixel(xx+2,yy+5,false);}
  if(v==5){s.rect(xx,yy,8,8,false);s.text(xx+1,yy,"G");}
 }
 int x=(int(px)-cx)*8,y=16+(int(py)-cy)*8;s.rect(x+2,y,4,3);s.rect(x+1,y+4,6,3);s.pixel(x+2,y+6,false);s.pixel(x+5,y+6,false);
}
inline void GameSession::matrix_pokemon(Matrix& leds)const{
 const auto& p=pokemon;auto put=[&](int x,int y,Color c){if(x>=0&&x<5&&y>=0&&y<5)leds[y*5+x]=c;};
 for(int i=0;i<p.count;i++){int bars=(p.team[i].hp*4+p.team[i].maxHp-1)/p.team[i].maxHp;for(int y=0;y<bars;y++)put(i,3-y,pokemon_game::colors[p.team[i].species]);}
 put(p.active,4,{255,255,255});if(p.phase==PokemonPhase::Battle){put(4,0,pokemon_game::colors[p.wild]);int bars=(p.wildHp*3+p.wildMaxHp-1)/p.wildMaxHp;for(int y=1;y<=bars;y++)put(4,y,{200,0,0});}
}
} // namespace arcade
