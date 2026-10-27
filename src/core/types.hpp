#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
namespace arcade {
inline constexpr unsigned kGameCount=9;
inline constexpr int kSettingCount=10;
// Teto dos canais de 8 bits: dois niveis abaixo do minimo da V2 (3/255).
inline constexpr std::array<unsigned,6> kLedBrightness={1,2,3,5,8,18};
struct Color {
    uint8_t r = 0, g = 0, b = 0;
};
using Matrix = std::array<Color, 25>;
struct Input {
    float x = 0, y = 0;
    bool actionHeld = false, actionPressed = false, pauseReleased = false, menuPressed = false;
    unsigned pauseHeldMs = 0;
    uint16_t rawX = 2048, rawY = 2048;
};
enum class Mode { Menu, Help, Play, Pause, Over, Settings, Test, Dialog };
enum class GameSignal { Running, Dialog, Finished };
enum class Event { None, Select, Start, Point, Miss, Shot, Jump, Win };
struct Settings {
    bool rgbEnabled=true,matrixEnabled=true;
    bool sound = true, swapXY = false, invertX = false, invertY = true, secondScreen = true;
    int brightness = 1, rotation = 0;
};
struct Cell {
    int x = 0, y = 0;
};
inline constexpr Color kColors[kGameCount] = {{0, 255, 50},  {30, 100, 255}, {255, 90, 0},  {190, 0, 255},
                                     {255, 190, 0}, {0, 60, 255},   {255, 0, 100}, {0, 255, 190}, {255,200,0}};
struct ButtonSample {
    bool down = false, pressed = false, released = false;
    unsigned heldMs = 0, durationMs = 0;
};
// Adapter de comandos: GPIO6/B (direita) e acao; GPIO5/A (esquerda) e pausa.
struct InputMapper {
    static Input map(float x, float y, const ButtonSample &left, const ButtonSample &right,
                     const ButtonSample &stick) {
        Input in;
        in.x = x;
        in.y = y;
        in.actionHeld = right.down;
        in.actionPressed = right.pressed;
        in.pauseReleased = left.released && left.durationMs < 750;
        in.pauseHeldMs = left.down ? left.heldMs : 0;
        in.menuPressed = stick.pressed;
        return in;
    }
};
} // namespace arcade

namespace arcade {
struct RingParticle {float x=0,y=0,vx=0,vy=0;unsigned ttl=0;};
struct RunnerState {bool rolling=false,charging=false,inLoop=false,loopUsed=false;int charge=0,facing=1;float checkpoint=18,camera=0,loopAngle=0,loopSpeed=150;unsigned stageMs=0;std::array<RingParticle,8> scattered{};};
enum class PokemonPhase {Explore,Battle,Party};
struct PartyMonster {int species=-1,level=1,hp=0,maxHp=1,xp=0;};
struct PokemonState {
  PokemonPhase phase=PokemonPhase::Explore;std::array<PartyMonster,4> team{};
  int count=1,active=0,wild=1,wildLevel=3,wildHp=24,wildMaxHp=24,balls=5,potions=3,cursor=0;
  bool trainer=false;uint32_t menuClock=0;char feedback[32]="B: ESCOLHER";
};
}
