#pragma once
#include "../ui/framebuffer.hpp"
#include "types.hpp"
namespace arcade {
struct GameSession {
    RunnerState runner;PokemonState pokemon;
    GameSignal signal = GameSignal::Running;
    Event event = Event::None;
    int game = 0, score = 0, lives = 3, level = 1;
    uint32_t rng = 0x526f636b, elapsed = 0, gameClock = 0, shotClock = 0, enemyClock = 0;
    float px = 64, py = 36, bx = 64, by = 36, vx = 0, vy = 0, enemyY = 30;
    std::array<Cell, 160> snake{};
    int length = 4, dx = 1, dy = 0, nextX = 1, nextY = 0;
    Cell food{15, 4};
    std::array<bool, 24> blocks{};
    bool stuck = false;
    std::array<bool, 15> aliens{};
    float fleetX = 12, fleetY = 16, fleetDir = 1, bulletX = 0, bulletY = -1, enemyX = 0,
          enemyBulletY = -1;
    int shieldMs = 0;
    struct Pipe {
        float x = 0;
        int top = 20;
        bool counted = false;
    };
    std::array<Pipe, 2> pipes{};
    bool won = false, grounded = false, mapView = false;
    int rings = 0, crystals = 0, steps = 0, facingX = 1, facingY = 0;
    std::array<bool, 96> coins{};
    std::array<bool, 8> crawlers{};
    std::array<uint8_t, 640> world{};
    std::array<bool, 640> seen{};
    struct Enemy {
        int x = 0, y = 0;
        bool alive = false;
    };
    std::array<Enemy, 7> monsters{};
    const char *dialog1 = "", *dialog2 = "", *dialog3 = "";

    void resetCommon(int index) {
        game = index;
        score = 0;
        lives = (index == 0 || index == 4 || index == 7) ? 1 : 3;
        level = 1;
        won = false;
        signal = GameSignal::Running;
        gameClock = 0;
        shotClock = 0;
        enemyClock = 0;
        px = 64;
        py = 36;
        event = Event::Start;
    }
    uint32_t random() {
        rng ^= rng << 13;
        rng ^= rng >> 17;
        rng ^= rng << 5;
        return rng;
    }
    static float clamp(float x, float lo, float hi) {
        return std::max(lo, std::min(hi, x));
    }
    void point(int n = 1) {
        score += n;
        event = Event::Point;
    }
    void over() {
        signal = GameSignal::Finished;
        event = Event::Miss;
    }
    Color gameColor() const {
        return kColors[game];
    }
    void talk(const char *a, const char *b, const char *c) {
        dialog1 = a;
        dialog2 = b;
        dialog3 = c;
        signal = GameSignal::Dialog;
        event = Event::Select;
    }
    uint8_t tile(int x, int y, int width = 32, int height = 20) const {
        return x < 0 || y < 0 || x >= width || y >= height ? 1 : world[y * width + x];
    }
    void reveal(int width, int height) {
        for (int yy = int(py) - 3; yy <= int(py) + 3; yy++)
            for (int xx = int(px) - 3; xx <= int(px) + 3; xx++)
                if (xx >= 0 && yy >= 0 && xx < width && yy < height)
                    seen[yy * width + xx] = true;
    }
    float ground(float x) const;
    void resetRunner();
    void createRpg() {
        world.fill(0);
        seen.fill(false);
        for (int y = 0; y < 20; y++)
            for (int x = 0; x < 32; x++) {
                if (x == 0 || y == 0 || x == 31 || y == 19)
                    world[y * 32 + x] = 1;
                else if (x == 15 && y != 5 && y != 14)
                    world[y * 32 + x] = 1;
                else if (x > 2 && x < 7 && y > 7 && y < 12)
                    world[y * 32 + x] = 1;
                else if (x > 21 && x < 27 && y > 3 && y < 8)
                    world[y * 32 + x] = 1;
            }
        world[4 * 32 + 4] = 4;
        world[5 * 32 + 11] = 2;
        world[15 * 32 + 8] = 2;
        world[12 * 32 + 27] = 2;
        world[17 * 32 + 29] = 5;
        world[13 * 32 + 13] = 3;
        world[9 * 32 + 22] = 3;
        monsters = {Enemy{10, 5, true}, Enemy{7, 14, true},  Enemy{26, 12, true},
                    Enemy{20, 5, true}, Enemy{21, 16, true}, Enemy{11, 12, true},
                    Enemy{28, 16, true}};
        px = 3;
        py = 4;
        crystals = 0;
        lives = 5;
        gameClock = 0;
        shotClock = 400;
        shieldMs = 500;
        reveal(32, 20);
    }
    void createMaze() {
        world.fill(1);
        seen.fill(false);
        std::array<Cell, 315> stack{};
        int count = 1;
        stack[0] = {1, 1};
        world[22] = 0;
        while (count) {
            Cell here = stack[count - 1];
            std::array<Cell, 4> options{};
            int n = 0;
            for (Cell d : {Cell{2, 0}, Cell{-2, 0}, Cell{0, 2}, Cell{0, -2}}) {
                int x = here.x + d.x, y = here.y + d.y;
                if (x > 0 && y > 0 && x < 20 && y < 14 && world[y * 21 + x] == 1)
                    options[n++] = {x, y};
            }
            if (!n) {
                count--;
                continue;
            }
            Cell next = options[random() % unsigned(n)];
            world[((here.y + next.y) / 2) * 21 + (here.x + next.x) / 2] = 0;
            world[next.y * 21 + next.x] = 0;
            stack[count++] = next;
        }
        world[13 * 21 + 19] = 5;
        for (int k = 0; k < 3; k++) {
            for (int tries = 0; tries < 1000; tries++) {
                int x = 1 + int(random() % 19), y = 1 + int(random() % 13);
                if (world[y * 21 + x] == 0 && x + y > 8) {
                    world[y * 21 + x] = 2;
                    break;
                }
            }
        }
        px = 1;
        py = 1;
        crystals = 0;
        steps = 0;
        lives = 1;
        mapView = false;
        gameClock = 0;
        reveal(21, 15);
    }
    void walk(const Input &in, unsigned dt, int width, int height) {
        gameClock += dt;
        if (gameClock < 135)
            return;
        int xx = 0, yy = 0;
        if (std::fabs(in.x) > .5f && std::fabs(in.x) >= std::fabs(in.y))
            xx = in.x > 0 ? 1 : -1;
        else if (std::fabs(in.y) > .5f)
            yy = in.y > 0 ? 1 : -1;
        else
            return;
        gameClock = 0;
        facingX = xx;
        facingY = yy;
        int x = int(px) + xx, y = int(py) + yy;
        if (tile(x, y, width, height) == 1 || tile(x, y, width, height) == 4)
            return;
        for (auto m : monsters)
            if (game == 6 && m.alive && m.x == x && m.y == y)
                return;
        px = float(x);
        py = float(y);
        steps++;
        reveal(width, height);
        uint8_t &value = world[y * width + x];
        if (value == 2) {
            crystals++;
            point(game == 6 ? 25 : 50);
            value = 0;
        }
        if (value == 3) {
            lives = std::min(5, lives + 2);
            point(5);
            value = 0;
        }
        if (value == 5) {
            if (crystals >= 3) {
                point(200);
                won = true;
                event = Event::Win;
                signal = GameSignal::Finished;
            } else
                talk("PORTAL FECHADO", "ENCONTRE 3 CRISTAIS", "B: CONTINUAR");
        }
        if (game == 6) {
            for (auto &m : monsters)
                if (m.alive) {
                    int distance = std::abs(m.x - int(px)) + std::abs(m.y - int(py));
                    if (distance <= 1) {
                        if (!shieldMs) {
                            shieldMs = 650;
                            if (--lives <= 0) {
                                over();
                                return;
                            }
                            event = Event::Miss;
                        }
                        continue;
                    }
                    if (distance > 7)
                        continue;
                    int mx = m.x, my = m.y;
                    if (std::abs(m.x - int(px)) >= std::abs(m.y - int(py)))
                        mx += px > m.x ? 1 : -1;
                    else
                        my += py > m.y ? 1 : -1;
                    bool occupied = false;
                    for (auto other : monsters)
                        occupied |= other.alive && other.x == mx && other.y == my;
                    if (tile(mx, my) == 0 && !occupied && !(mx == int(px) && my == int(py))) {
                        m.x = mx;
                        m.y = my;
                    }
                }
        }
    }
    void spawnFood() {
        int free = 160 - length;
        if (free <= 0) {
            won = true;
            signal = GameSignal::Finished;
            event = Event::Win;
            return;
        }
        int chosen = int(random() % unsigned(free));
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 20; x++) {
                bool used = false;
                for (int i = 0; i < length; i++)
                    used |= snake[i].x == x && snake[i].y == y;
                if (!used && chosen-- == 0) {
                    food = {x, y};
                    return;
                }
            }
    }
    void pongServe() {
        bx = 64;
        by = 36;
        vx = -55 - level * 3;
        vy = (random() & 1) ? 27 : -27;
        gameClock = 650;
    }
    void resetBlocks() {
        blocks.fill(true);
        stuck = true;
        bx = px;
        by = 55;
        vx = 43;
        vy = -53;
    }
    void resetFleet() {
        aliens.fill(true);
        fleetX = 12;
        fleetY = 16;
        fleetDir = 1;
        bulletY = -1;
        enemyBulletY = -1;
        enemyClock = 0;
        shieldMs = 700;
    }
    void hud(Screen &s) {
        char b[24];
        std::snprintf(b, sizeof(b), "%05d", score);
        s.text(1, 0, b);
        std::snprintf(b, sizeof(b), "V:%d", lives);
        s.text(104, 0, b);
        s.line(0, 10, 127, 10);
    }
    void drawExplorer(Screen &s) {
        bool maze = game == 7;
        int width = maze ? 21 : 32, height = maze ? 15 : 20, sz = maze ? 7 : 8;
        if (maze && mapView) {
            s.rect(0, 12, 128, 52, true, false);
            for (int y = 0; y < 15; y++)
                for (int x = 0; x < 21; x++) {
                    auto v = tile(x, y, 21, 15);
                    if (v == 1)
                        s.rect(22 + x * 4, 15 + y * 3, 4, 3);
                    else if (v == 2)
                        s.pixel(23 + x * 4, 16 + y * 3);
                    else if (v == 5)
                        s.rect(22 + x * 4, 15 + y * 3, 4, 3, false);
                }
            if ((elapsed / 160) % 2)
                s.rect(22 + int(px) * 4, 15 + int(py) * 3, 3, 3);
            return;
        }
        int cols = 128 / sz, rows = 48 / sz,
            cx = std::max(0, std::min(width - cols, int(px) - cols / 2)),
            cy = std::max(0, std::min(height - rows, int(py) - rows / 2));
        for (int y = 0; y < rows; y++)
            for (int x = 0; x < cols; x++) {
                int wx = cx + x, wy = cy + y, xx = x * sz, yy = 14 + y * sz;
                if (!seen[wy * width + wx]) {
                    if ((x + y) % 2 == 0)
                        s.pixel(xx + sz / 2, yy + sz / 2);
                    continue;
                }
                uint8_t v = tile(wx, wy, width, height);
                if (v == 1) {
                    s.rect(xx, yy, sz - 1, sz - 1);
                    s.pixel(xx + 2, yy + 2, false);
                }
                if (v == 2) {
                    s.line(xx + sz / 2, yy, xx + sz - 2, yy + sz / 2);
                    s.line(xx + sz - 2, yy + sz / 2, xx + sz / 2, yy + sz - 2);
                    s.line(xx + sz / 2, yy + sz - 2, xx, yy + sz / 2);
                    s.line(xx, yy + sz / 2, xx + sz / 2, yy);
                }
                if (v == 3) {
                    s.line(xx + 1, yy + 3, xx + 5, yy + 3);
                    s.line(xx + 3, yy + 1, xx + 3, yy + 5);
                }
                if (v == 4) {
                    s.rect(xx + 2, yy, 3, 3);
                    s.rect(xx + 1, yy + 4, 5, 3);
                }
                if (v == 5) {
                    s.rect(xx, yy, sz - 1, sz - 1, false);
                    if ((elapsed / 200) % 2)
                        s.rect(xx + 2, yy + 2, sz - 5, sz - 5);
                }
            }
        if (!maze)
            for (auto m : monsters)
                if (m.alive && m.x >= cx && m.x < cx + cols && m.y >= cy && m.y < cy + rows &&
                    seen[m.y * width + m.x]) {
                    int x = (m.x - cx) * sz, y = 14 + (m.y - cy) * sz;
                    s.rect(x + 1, y + 1, 6, 5);
                    s.pixel(x + 2, y + 2, false);
                    s.pixel(x + 5, y + 2, false);
                }
        int hx = (int(px) - cx) * sz, hy = 14 + (int(py) - cy) * sz;
        if (!shieldMs || maze || (elapsed / 100) % 2) {
            s.rect(hx + 2, hy, 3, 3);
            s.rect(hx + 1, hy + 4, 5, 3);
        }
        if (!maze && shotClock < 110) {
            int x = hx + 3 + facingX * 6, y = hy + 3 + facingY * 6;
            s.line(x - 2, y, x + 2, y);
            s.line(x, y - 2, x, y + 2);
        }
    }
    void drawRunner(Screen &s);
    void reset_snake();
    void update_snake(const Input &in, unsigned dt);
    void draw_snake(Screen &s);
    void matrix_snake(Matrix &leds) const;
    void reset_pong();
    void update_pong(const Input &in, unsigned dt);
    void draw_pong(Screen &s);
    void matrix_pong(Matrix &leds) const;
    void reset_blocks();
    void update_blocks(const Input &in, unsigned dt);
    void draw_blocks(Screen &s);
    void matrix_blocks(Matrix &leds) const;
    void reset_invaders();
    void update_invaders(const Input &in, unsigned dt);
    void draw_invaders(Screen &s);
    void matrix_invaders(Matrix &leds) const;
    void reset_flappy();
    void update_flappy(const Input &in, unsigned dt);
    void draw_flappy(Screen &s);
    void matrix_flappy(Matrix &leds) const;
    void reset_runner();
    void update_runner(const Input &in, unsigned dt);
    void draw_runner(Screen &s);
    void matrix_runner(Matrix &leds) const;
    void reset_rpg();
    void update_rpg(const Input &in, unsigned dt);
    void draw_rpg(Screen &s);
    void matrix_rpg(Matrix &leds) const;
    void reset_maze();
    void update_maze(const Input &in, unsigned dt);
    void draw_maze(Screen &s);
    void matrix_maze(Matrix &leds) const;
    void reset_pokemon();
    void update_pokemon(const Input&,unsigned);
    void draw_pokemon(Screen&);
    void matrix_pokemon(Matrix&)const;
};
} // namespace arcade
