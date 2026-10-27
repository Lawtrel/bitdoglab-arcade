#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline void GameSession::reset_maze() {createMaze();}
inline void GameSession::update_maze(const Input& in,unsigned dt) {
  (void)in;(void)dt;const float t=dt/1000.0f;(void)t;
if(in.actionPressed)mapView=!mapView;if(!mapView)walk(in,dt,21,15);
}
inline void GameSession::draw_maze(Screen& s) {drawExplorer(s);}
inline void GameSession::matrix_maze(Matrix& leds)const {
  auto put=[&](int x,int y,Color c){if(x>=0&&x<5&&y>=0&&y<5)leds[y*5+x]=c;};
  const Color c=gameColor();(void)c;
int ww=game==6?32:21,hh=game==6?20:15;for(int y=0;y<5;y++)for(int x=0;x<5;x++){bool visited=false;for(int yy=y*hh/5;yy<(y+1)*hh/5;yy++)for(int xx=x*ww/5;xx<(x+1)*ww/5;xx++)visited|=seen[yy*ww+xx];if(visited)put(x,y,{0,30,30});}put(int(px)*5/ww,int(py)*5/hh,c);if(crystals>=3)put(4,4,{255,255,255});
}
}
