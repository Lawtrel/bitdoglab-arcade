#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline void GameSession::reset_pong() {enemyY=30;pongServe();}
inline void GameSession::update_pong(const Input& in,unsigned dt) {
  (void)in;(void)dt;const float t=dt/1000.0f;(void)t;

      py=clamp(py+in.y*90*t,13,49);float target=by-6;enemyY+=clamp(target-enemyY,-(39+level*3)*t,(39+level*3)*t);enemyY=clamp(enemyY,13,49);
      if(gameClock){gameClock=gameClock>dt?gameClock-dt:0;return;}
      float oldX=bx;bx+=vx*t;by+=vy*t;
      if(by<14){by=14;vy=std::fabs(vy);}if(by>60){by=60;vy=-std::fabs(vy);}
      if(vx<0&&oldX>=9&&bx<=9&&by>=py-2&&by<=py+14){bx=9;vx=std::min(145.0f,std::fabs(vx)*1.05f);vy=(by-(py+6))*7;point();}
      if(vx>0&&oldX<=119&&bx>=119&&by>=enemyY-2&&by<=enemyY+14){bx=119;vx=-std::fabs(vx);vy+=(int(random()%11)-5);event=Event::Shot;}
      if(bx<0){if(--lives<=0)over();else{event=Event::Miss;pongServe();}}
      if(bx>128){point(5);level++;pongServe();}

}
inline void GameSession::draw_pong(Screen& s) {for(int y=14;y<62;y+=5)s.rect(63,y,1,2);s.rect(4,int(py),3,13);s.rect(121,int(enemyY),3,13);s.rect(int(bx)-1,int(by)-1,3,3);}
inline void GameSession::matrix_pong(Matrix& leds)const {
  auto put=[&](int x,int y,Color c){if(x>=0&&x<5&&y>=0&&y<5)leds[y*5+x]=c;};
  const Color c=gameColor();(void)c;
put(0,int((py+6-12)*5/52),c);put(4,int((enemyY+6-12)*5/52),c);put(int(bx*5/128),int((by-12)*5/52),{255,255,255});
}
}
