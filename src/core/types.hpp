#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
namespace arcade {
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
    bool sound = true, swapXY = false, invertX = false, invertY = true, secondScreen = true;
    int brightness = 1, rotation = 0;
};
struct Cell {
    int x = 0, y = 0;
};
inline constexpr Color kColors[8] = {{0, 255, 50},  {30, 100, 255}, {255, 90, 0},  {190, 0, 255},
                                     {255, 190, 0}, {0, 60, 255},   {255, 0, 100}, {0, 255, 190}};
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
