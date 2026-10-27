#include "engine.hpp"
#include <cassert>
#include <cstdio>
#include <queue>
int main(){
  assert(arcade::selfTest());
  arcade::Engine app;auto& game=app.session;
  arcade::ButtonSample left,right,stick;
  right.down=true;right.pressed=true;
  auto input=arcade::InputMapper::map(0,0,left,right,stick);
  assert(input.actionHeld&&input.actionPressed&&!input.pauseReleased);
  right={};left.released=true;left.durationMs=120;
  input=arcade::InputMapper::map(0,0,left,right,stick);
  assert(input.pauseReleased&&!input.actionPressed);
  app.start(4);arcade::Input jump;jump.actionPressed=true;app.tick(jump);assert(game.vy<0);
  app.start(2);app.tick(jump);assert(!game.stuck);
  arcade::Input held;held.pauseHeldMs=800;
  app.start(1);app.tick(held);assert(app.mode==arcade::Mode::Menu);app.tick(held);assert(app.mode==arcade::Mode::Menu);
  app.start(0);game.score=17;app.menu();app.selection=1;app.tick(jump);assert(app.mode==arcade::Mode::Help);
  arcade::Input back;back.pauseReleased=true;app.tick(back);assert(app.best[1]==0&&app.best[0]==17);
  for(unsigned seed=1;seed<=100;seed++){
    game.rng=seed;app.start(7);int keys=0,walkable=0;for(int i=0;i<315;i++){keys+=game.world[i]==2;walkable+=game.world[i]!=1;}assert(keys==3);
    bool visited[315]{};std::queue<int> queue;queue.push(22);visited[22]=true;int reached=0;
    while(!queue.empty()){int cell=queue.front();queue.pop();reached++;int x=cell%21,y=cell/21;for(auto d:{arcade::Cell{1,0},arcade::Cell{-1,0},arcade::Cell{0,1},arcade::Cell{0,-1}}){int xx=x+d.x,yy=y+d.y;if(xx<0||xx>=21||yy<0||yy>=15)continue;int n=yy*21+xx;if(!visited[n]&&game.world[n]!=1){visited[n]=true;queue.push(n);}}}
    assert(reached==walkable);assert(visited[13*21+19]);
  }
  app.start(6);assert(game.crystals==0&&game.lives==5);int crystals=0;for(auto tile:game.world)crystals+=tile==2;assert(crystals==3);
  arcade::Screen screen;arcade::Matrix colors;
  for(int g=0;g<8;g++){app.start(g);app.render(screen);app.matrix(colors);}
  std::puts("ENGINE_TESTS_PASSED: right-button action, pause/menu, no cross-game high-score leak, 100 connected mazes, RPG quest and 8 render strategies.");
}
