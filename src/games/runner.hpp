#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline void GameSession::reset_runner() {resetRunner();}
inline void GameSession::update_runner(const Input& in,unsigned dt) {
  (void)in;(void)dt;const float t=dt/1000.0f;(void)t;

      shieldMs=std::max(0,shieldMs-int(dt));float previousY=py;bool onGround=grounded;
      if(std::fabs(in.x)>.25f)vx=clamp(vx+in.x*280*t,-145,175);else vx=std::fabs(vx)<210*t?0:vx-(vx>0?1:-1)*210*t;
      if(in.actionPressed&&grounded){vy=-94;grounded=false;onGround=false;event=Event::Jump;}
      px=clamp(px+vx*t,4,1240);vy+=(in.actionHeld?200:280)*t;py+=vy*t;float floor=ground(px);
      if(vy>=0&&floor<70&&((onGround&&std::fabs(py-floor)<6)||(previousY<=floor+3&&py>=floor))){py=floor;vy=0;grounded=true;}else grounded=false;
      if(grounded)for(int spring:{176,416,676,916,1156})if(std::fabs(px-spring)<4){vy=-128;grounded=false;event=Event::Jump;}
      for(int i=0;i<86;i++)if(coins[i]){float x=40+i*14,y=ground(x)-13;if(y<70&&std::fabs(px-x)<6&&std::fabs(py-5-y)<10){coins[i]=false;rings++;point(10);}}
      for(int i=0;i<8;i++)if(crawlers[i]){float x=130+i*139+4*std::sin(elapsed*.003f),y=ground(x);if(y<70&&std::fabs(px-x)<7&&std::fabs(py-y)<9){if(vy>12&&previousY<y-4){crawlers[i]=false;vy=-65;point(50);}else if(!shieldMs){if(rings){rings=std::max(0,rings-10);shieldMs=1300;vx=-vx*.5f;vy=-45;event=Event::Miss;}else{if(--lives<=0){over();return;}resetRunner();}}}}
      if(py>83){if(--lives<=0){over();return;}resetRunner();event=Event::Miss;}
      if(px>=1238){point(100);if(level>=3){won=true;signal=GameSignal::Finished;event=Event::Win;}else{level++;resetRunner();event=Event::Win;}}

}
inline void GameSession::draw_runner(Screen& s) {drawRunner(s);}
inline void GameSession::matrix_runner(Matrix& leds)const {
  auto put=[&](int x,int y,Color c){if(x>=0&&x<5&&y>=0&&y<5)leds[y*5+x]=c;};
  const Color c=gameColor();(void)c;
int speed=int(std::fabs(vx)*5/176);for(int x=0;x<speed;x++)for(int y=4;y>=4-x;y--)put(x,y,c);put(int(px*5/1241),0,{255,255,255});
}
}
