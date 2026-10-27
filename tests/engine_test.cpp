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
  for(int g=0;g<int(arcade::kGameCount);g++){app.start(g);app.render(screen);app.matrix(colors);}
  app.start(5);arcade::Input rightInput;rightInput.x=1;
  for(int i=0;i<20;i++)app.tick(rightInput);float speed=game.vx;app.tick({});assert(game.vx>0&&game.vx<speed);
  app.start(5);arcade::Input charge;charge.y=1;charge.actionPressed=true;charge.actionHeld=true;app.tick(charge);assert(game.runner.charging&&game.vx==0);app.tick({});assert(!game.runner.charging&&game.vx>140);
  game.runner.checkpoint=402;game.py=100;int beforeLives=game.lives;app.tick({});assert(game.lives==beforeLives-1&&game.px>=402&&game.px<404);
  app.start(5);game.shieldMs=0;game.rings=12;arcade::runner_game::hurt(game);int scattered=0;for(auto ring:game.runner.scattered)scattered+=ring.ttl>0;assert(game.rings==0&&scattered==8);
  app.start(5);game.px=599;game.py=game.ground(599);game.vx=160;game.runner.rolling=true;app.tick(rightInput);assert(game.runner.inLoop);arcade::Input exitLoop;exitLoop.actionPressed=true;exitLoop.actionHeld=true;app.tick(exitLoop);assert(!game.runner.inLoop&&!game.grounded);
  app.start(8);assert(game.pokemon.count==1&&game.pokemon.team[0].species==0);
  assert(arcade::pokemon_game::damage(0,2,5)>arcade::pokemon_game::damage(0,1,5));
  arcade::pokemon_game::encounter(game,1);game.pokemon.wildHp=1;game.pokemon.cursor=1;game.rng=1;int balls=game.pokemon.balls;arcade::Input capture;capture.actionPressed=true;app.tick(capture);
  assert(game.pokemon.count==2&&game.pokemon.team[1].species==1&&game.pokemon.balls==balls-1);assert(game.pokemon.phase==arcade::PokemonPhase::Explore);
  // A quantidade de ajustes e independente da quantidade de jogos.
  app.mode=arcade::Mode::Settings;app.settingRow=arcade::kSettingCount-1;app.navClock=0;
  arcade::Input down;down.y=1;app.tick(down);assert(app.settingRow==0);
  app.mode=arcade::Mode::Menu;app.selection=8;app.navClock=0;app.tick(down);assert(app.selection==0);
  // Desligar a matriz apaga todos os pixels em todos os modos.
  app.settings.matrixEnabled=false;
  for(auto mode:{arcade::Mode::Menu,arcade::Mode::Settings,arcade::Mode::Help,arcade::Mode::Play,arcade::Mode::Pause,arcade::Mode::Over,arcade::Mode::Test}){app.mode=mode;colors.fill({255,255,255});app.matrix(colors);for(auto c:colors)assert(c.r==0&&c.g==0&&c.b==0);}
  app.settingRow=8;app.settingChange(1);assert(app.settings.matrixEnabled);
  app.settingRow=7;app.settingChange(1);assert(!app.settings.rgbEnabled);app.settingChange(1);assert(app.settings.rgbEnabled);
  app.settingRow=2;app.settings.brightness=0;app.settingChange(-1);assert(app.settings.brightness==5);app.settingChange(1);assert(app.settings.brightness==0);
  // Professor cura e reabastece; a rota ate o ginasio continua acessivel.
  app.start(8);game.pokemon.team[0].hp=1;game.pokemon.balls=0;app.tick(capture);
  assert(game.pokemon.team[0].hp==32&&game.pokemon.balls==5&&app.mode==arcade::Mode::Dialog);
  bool connected[640]{};std::queue<int> route;route.push(4*32+3);connected[4*32+3]=true;
  while(!route.empty()){int cell=route.front();route.pop();for(auto d:{arcade::Cell{1,0},arcade::Cell{-1,0},arcade::Cell{0,1},arcade::Cell{0,-1}}){int x=cell%32+d.x,y=cell/32+d.y;if(x<0||x>=32||y<0||y>=20)continue;int n=y*32+x;auto tile=game.world[n];if(!connected[n]&&tile!=1&&tile!=4&&tile!=6){connected[n]=true;route.push(n);}}}
  assert(connected[17*32+29]&&connected[3*32+8]&&connected[11*32+18]);
  app.start(8);arcade::pokemon_game::encounter(game,2);game.pokemon.wildHp=1;game.pokemon.cursor=0;app.tick(capture);assert(app.mode==arcade::Mode::Dialog&&game.pokemon.phase==arcade::PokemonPhase::Explore);
  app.start(8);arcade::pokemon_game::encounter(game,0,true);game.pokemon.wildHp=1;game.pokemon.cursor=0;app.tick(capture);assert(app.mode==arcade::Mode::Over&&game.won);
  assert(arcade::kColors[8].r>0);
  std::puts("ENGINE_TESTS_PASSED: right-button action, pause/menu, no cross-game high-score leak, 100 connected mazes, RPG quest, runner physics, Pokemon capture/heal/battles, menu bounds and 9 render strategies.");
}
