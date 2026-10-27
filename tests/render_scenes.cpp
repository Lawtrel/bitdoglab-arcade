#include "engine.hpp"
#include <fstream>
#include <string>
int main(int argc,char** argv){
  if(argc!=2)return 1;
  arcade::Engine app;arcade::Screen screen;arcade::Matrix matrix;
  for(int game=0;game<8;game++){
    app.start(game);app.elapsed=1000;app.session.elapsed=1000;
    // Cenarios demonstrativos deterministas, desenhados pelo motor real.
    if(game==4){app.session.pipes[0].x=75;app.session.pipes[1].x=147;}
    if(game==5){app.session.px=350;app.session.py=app.session.ground(350);app.session.vx=110;}
    if(game==7)app.session.mapView=true;
    app.render(screen);app.matrix(matrix);
    std::string base=std::string(argv[1])+"/game-"+std::to_string(game);
    std::ofstream frame(base+".oled",std::ios::binary);frame.write(reinterpret_cast<const char*>(screen.data.data()),1024);
    std::ofstream leds(base+".rgb",std::ios::binary);for(auto c:matrix){leds.put(char(c.r));leds.put(char(c.g));leds.put(char(c.b));}
  }
}
