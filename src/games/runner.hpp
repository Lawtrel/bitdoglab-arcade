#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline float GameSession::ground(float x) const {
    int n = int(x), shift = (level - 1) * 18;
    for (int hole : {196, 436, 696, 936})
        if (n >= hole + shift && n < hole + shift + 24 + (level - 1) * 4)
            return 200;
    int period = 280 - level * 20, u = n % period;
    float slope = .25f + level * .05f;
    if (u >= 50 && u < 90)
        return 56 - (u - 50) * slope;
    if (u >= 90 && u < 130)
        return 56 - (130 - u) * slope;
    return 56;
}
inline void GameSession::resetRunner() {
    runner = RunnerState{};
    px = 18;
    py = ground(px);
    vx = 0;
    vy = 0;
    grounded = true;
    rings = 0;
    shieldMs = 900;
    coins.fill(true);
    crawlers.fill(true);
}
inline void GameSession::reset_runner() {
    resetRunner();
}
namespace runner_game {
inline void camera(GameSession &s, float t) {
    float target = GameSession::clamp(s.px - 38, 0, 1112);
    s.runner.camera += (target - s.runner.camera) * std::min(1.0f, t * 10);
}
inline void respawn(GameSession &s) {
    s.px = s.runner.checkpoint;
    s.py = s.ground(s.px);
    s.vx = s.vy = 0;
    s.grounded = true;
    s.rings = 0;
    s.shieldMs = 1100;
    s.runner.charging = false;
    s.runner.rolling = false;
    s.runner.inLoop = false;
    s.runner.loopUsed = s.runner.checkpoint > 600;
    s.runner.scattered.fill({});
    s.runner.camera = GameSession::clamp(s.px - 38, 0, 1112);
}
inline void hurt(GameSession &s) {
    if (s.shieldMs)
        return;
    if (s.rings) {
        int n = std::min(8, s.rings);
        for (int i = 0; i < n; i++) {
            float angle = 3.14159265f * float(i + 1) / float(n + 1);
            s.runner.scattered[i] = {s.px, s.py - 6, std::cos(angle) * 65, -std::sin(angle) * 90,
                                     2800};
        }
        s.rings = 0;
        s.shieldMs = 1600;
        s.vx = -s.runner.facing * 75;
        s.vy = -65;
        s.grounded = false;
        s.event = Event::Miss;
    } else {
        if (--s.lives <= 0)
            s.over();
        else {
            respawn(s);
            s.event = Event::Miss;
        }
    }
}
} // namespace runner_game
inline void GameSession::update_runner(const Input &in, unsigned dt) {
    const float t = dt / 1000.0f;
    runner.stageMs += dt;
    shieldMs = std::max(0, shieldMs - int(dt));
    for (auto &ring : runner.scattered)
        if (ring.ttl) {
            ring.ttl = ring.ttl > dt ? ring.ttl - dt : 0;
            ring.vy += 175 * t;
            ring.x += ring.vx * t;
            ring.y += ring.vy * t;
            float floor = ground(ring.x);
            if (floor < 70 && ring.y > floor - 2 && ring.vy > 0) {
                ring.y = floor - 2;
                ring.vy = -ring.vy * .6f;
            }
            if (ring.ttl < 2200 && std::fabs(px - ring.x) < 6 && std::fabs(py - 5 - ring.y) < 8) {
                ring.ttl = 0;
                rings++;
                point(10);
            }
        }
    if (runner.inLoop) {
        if (in.actionPressed) {
            float a = runner.loopAngle;
            vx = std::cos(a) * runner.loopSpeed;
            vy = -std::sin(a) * runner.loopSpeed - 30;
            runner.inLoop = false;
            grounded = false;
            event = Event::Jump;
        } else {
            runner.loopAngle += runner.loopSpeed * t / 16;
            float a = runner.loopAngle;
            px = 600 + 16 * std::sin(a);
            py = ground(600) - 16 + 16 * std::cos(a);
            runner.rolling = true;
            grounded = false;
            if (a >= 6.2831853f) {
                runner.inLoop = false;
                px = 619;
                py = ground(px);
                vx = runner.loopSpeed;
                vy = 0;
                grounded = true;
            }
            runner_game::camera(*this, t);
            return;
        }
    }
    bool down = in.y > .55f;
    if (std::fabs(in.x) > .25f)
        runner.facing = in.x > 0 ? 1 : -1;
    if (grounded && down && std::fabs(vx) < 25 && in.actionPressed) {
        runner.charging = true;
        runner.rolling = true;
        runner.charge = std::min(3, runner.charge + 1);
        event = Event::Shot;
    }
    if (runner.charging) {
        vx = 0;
        vy = 0;
        if (down) {
            runner_game::camera(*this, t);
            return;
        }
        vx = runner.facing * (140 + runner.charge * 30);
        runner.charging = false;
        runner.charge = 0;
        runner.rolling = true;
        event = Event::Shot;
    }
    if (grounded && down && std::fabs(vx) > 35)
        runner.rolling = true;
    if (grounded && std::fabs(vx) < 25 && !down)
        runner.rolling = false;
    float previousX = px, previousY = py;
    bool wasGrounded = grounded;
    if (std::fabs(in.x) > .25f) {
        float acceleration = vx * in.x < 0 ? 420 : 210;
        vx += in.x * acceleration * t;
    } else {
        float friction = runner.rolling ? 38 : 110;
        vx = std::fabs(vx) < friction * t ? 0 : vx - (vx > 0 ? 1 : -1) * friction * t;
    }
    if (grounded) {
        float slope = (ground(px + 2) - ground(px - 2)) / 4;
        if (std::fabs(slope) < 1)
            vx += slope * (runner.rolling ? 340 : 180) * t;
    }
    float limit = runner.rolling || !grounded ? 245.0f : 180.0f;
    vx = clamp(vx, -limit, limit);
    if (in.actionPressed && grounded && !down) {
        vy = -105;
        grounded = false;
        wasGrounded = false;
        runner.rolling = true;
        event = Event::Jump;
    }
    if (!in.actionHeld && vy < -60)
        vy = -60;
    px = clamp(px + vx * t, 4, 1240);
    vy += 230 * t;
    py += vy * t;
    float floor = ground(px);
    if (vy >= 0 && floor < 70 &&
        ((wasGrounded && std::fabs(py - floor) < 7) || (previousY <= floor + 3 && py >= floor))) {
        py = floor;
        vy = 0;
        grounded = true;
    } else
        grounded = false;
    if (!runner.loopUsed && grounded && vx > 110 && previousX < 600 && px >= 600) {
        runner.inLoop = true;
        runner.loopUsed = true;
        runner.loopAngle = 0;
        runner.loopSpeed = std::max(140.0f, vx);
        event = Event::Jump;
    }
    if (grounded)
        for (int spring : {176, 416, 676, 916, 1156})
            if (std::fabs(px - (spring + (level - 1) * 18)) < 4) {
                vy = -132;
                grounded = false;
                runner.rolling = true;
                event = Event::Jump;
            }
    for (int i = 0; i < 86; i++)
        if (coins[i]) {
            float x = 40 + i * 14, y = ground(x) - 13;
            if (y < 70 && std::fabs(px - x) < 6 && std::fabs(py - 5 - y) < 10) {
                coins[i] = false;
                rings++;
                point(10);
            }
        }
    for (int i = 0; i < 8; i++)
        if (crawlers[i]) {
            float x = 130 + i * 139 + 4 * std::sin(elapsed * .003f), y = ground(x);
            if (y < 70 && std::fabs(px - x) < 8 && std::fabs(py - y) < 10) {
                if (runner.rolling || (vy > 12 && previousY < y - 3)) {
                    crawlers[i] = false;
                    if (!grounded)
                        vy = -65;
                    point(50);
                } else {
                    runner_game::hurt(*this);
                    if (signal == GameSignal::Finished)
                        return;
                }
                break;
            }
        }
    if (py > 83) {
        if (--lives <= 0) {
            over();
            return;
        }
        runner_game::respawn(*this);
        event = Event::Miss;
    }
    if (px > 400 && runner.checkpoint < 400) {
        runner.checkpoint = 402;
        event = Event::Point;
    }
    if (px > 800 && runner.checkpoint < 800) {
        runner.checkpoint = 804;
        event = Event::Point;
    }
    if (px >= 1238) {
        point(100 + rings * 10);
        if (level >= 3) {
            won = true;
            signal = GameSignal::Finished;
            event = Event::Win;
        } else {
            level++;
            resetRunner();
            event = Event::Win;
        }
    }
    runner_game::camera(*this, t);
}
inline void GameSession::drawRunner(Screen &s) {
    int camera = int(runner.camera);
    // Fundo com paralaxe; terrenos e objetos permanecem no mesmo espaco de mundo.
    for (int i = 0; i < 5; i++) {
        int x = (i * 67 - int(camera * .3f)) % 180;
        if (x < -20)
            x += 180;
        int y = 20 + (i % 2) * 9;
        s.line(x, y, x + 12, y);
        s.line(x + 3, y - 2, x + 8, y - 2);
    }
    for (int x = 0; x < 128; x++) {
        int y = int(ground(float(camera + x)));
        if (y < 64) {
            s.line(x, y, x, 63);
            for (int yy = y + 3; yy < 64; yy++)
                if (((camera + x) / 4 + (yy - y) / 4) % 2)
                    s.pixel(x, yy, false);
        }
    }
    float centerY = ground(600) - 16;
    int loopX = 600 - camera;
    for (int i = 0; i < 32; i++) {
        float a = i * 6.2831853f / 32, b = (i + 1) * 6.2831853f / 32;
        s.line(loopX + int(16 * std::sin(a)), int(centerY + 16 * std::cos(a)),
               loopX + int(16 * std::sin(b)), int(centerY + 16 * std::cos(b)));
    }
    for (int i = 0; i < 86; i++)
        if (coins[i]) {
            int wx = 40 + i * 14, x = wx - camera, y = int(ground(float(wx))) - 13;
            if (y < 64 && x >= 0 && x < 128)
                s.rect(x - 1, y - 1, 3, 4, false);
        }
    for (auto ring : runner.scattered)
        if (ring.ttl && ((ring.ttl / 90) % 2)) {
            s.rect(int(ring.x) - camera - 1, int(ring.y) - 1, 3, 3, false);
        }
    for (int spring : {176, 416, 676, 916, 1156}) {
        int wx = spring + (level - 1) * 18, x = wx - camera, y = int(ground(float(wx)));
        if (x >= 0 && x < 128) {
            s.rect(x - 3, y - 5, 7, 2);
            s.line(x - 2, y - 2, x + 2, y);
        }
    }
    for (int checkpoint : {402, 804}) {
        int x = checkpoint - camera, y = int(ground(float(checkpoint)));
        if (x >= 0 && x < 128) {
            s.line(x, y - 18, x, y);
            s.rect(x, y - 18, 7, 5, runner.checkpoint >= checkpoint);
        }
    }
    for (int i = 0; i < 8; i++)
        if (crawlers[i]) {
            int wx = int(130 + i * 139 + 4 * std::sin(elapsed * .003f)), x = wx - camera,
                y = int(ground(float(wx)));
            if (x >= 0 && x < 128 && y < 64) {
                s.rect(x - 3, y - 5, 7, 4);
                s.pixel(x - 2, y - 4, false);
                s.pixel(x + 2, y - 4, false);
                s.line(x - 5, y, x - 2, y - 2);
                s.line(x + 2, y - 2, x + 5, y);
            }
        }
    if (!shieldMs || (elapsed / 90) % 2) {
        int x = int(px) - camera, y = int(py);
        if (runner.rolling || !grounded || runner.charging) {
            s.rect(x - 3, y - 6, 6, 6, false);
            s.line(x - 2, y - 5, x + 2, y - 1);
            s.pixel(x + 3, y - 4);
        } else {
            constexpr uint8_t body[9] = {0x38, 0x7c, 0xf8, 0xee, 0xfc, 0x7c, 0x38, 0x3c, 0x66};
            for (int yy = 0; yy < 9; yy++)
                for (int xx = 0; xx < 8; xx++)
                    if (body[yy] & (1 << (7 - xx)))
                        s.pixel(x + (runner.facing > 0 ? xx - 4 : 3 - xx), y - 8 + yy);
            if (std::fabs(vx) > 40) {
                int foot = (elapsed / 65) % 2;
                s.line(x - 4 + foot, y, x + 1 + foot, y);
                s.line(x - 7, y - 4, x - 11, y - 4);
            }
        }
    }
    int flag = 1225 - camera;
    if (flag > 0 && flag < 128) {
        s.line(flag, 24, flag, 55);
        s.rect(flag, 24, 7, 5);
    }
    char text[24];
    std::snprintf(text, sizeof(text), "R%03d %03u", std::min(999, rings),
                  std::min(999u, runner.stageMs / 1000));
    s.text(39, 0, text);
}
inline void GameSession::draw_runner(Screen &s) {
    drawRunner(s);
}
inline void GameSession::matrix_runner(Matrix &leds) const {
    auto put = [&](int x, int y, Color c) {
        if (x >= 0 && x < 5 && y >= 0 && y < 5)
            leds[y * 5 + x] = c;
    };
    int speed = std::min(5, int(std::fabs(vx) * 5 / 245));
    for (int x = 0; x < speed; x++)
        for (int y = 4; y >= 4 - x; y--)
            put(x, y, {0, 60, 255});
    put(int(px * 5 / 1241), 0, {255, 255, 255});
    if (runner.charging)
        for (int x = 0; x < runner.charge; x++)
            put(x, 1, {255, 200, 0});
}
} // namespace arcade
