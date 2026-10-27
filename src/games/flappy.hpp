#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline void GameSession::reset_flappy() {
    py = 34;
    vy = 0;
    pipes[0] = {135, 24, false};
    pipes[1] = {207, 18, false};
}
inline void GameSession::update_flappy(const Input &in, unsigned dt) {
    (void)in;
    (void)dt;
    const float t = dt / 1000.0f;
    (void)t;

    if (in.actionPressed) {
        vy = -43;
        event = Event::Jump;
    }
    vy += 106 * t;
    py += vy * t;
    if (py < 14 || py > 60) {
        over();
        return;
    }
    for (auto &p : pipes) {
        p.x -= (31 + std::min(20, score)) * t;
        if (!p.counted && p.x + 12 < 22) {
            p.counted = true;
            point();
        }
        if (p.x < -13) {
            float furthest = std::max(pipes[0].x, pipes[1].x);
            p.x = furthest + 72;
            p.top = 17 + int(random() % 20);
            p.counted = false;
        }
        if (p.x < 28 && p.x + 12 > 19 && (py - 2 < p.top || py + 3 > p.top + 23)) {
            over();
            return;
        }
    }
}
inline void GameSession::draw_flappy(Screen &s) {
    for (auto p : pipes) {
        int x = int(p.x);
        s.rect(x, 12, 12, p.top - 12);
        s.rect(x, p.top + 23, 12, 64 - p.top - 23);
        s.rect(x - 1, p.top - 3, 14, 3);
        s.rect(x - 1, p.top + 23, 14, 3);
    }
    s.rect(19, int(py) - 2, 7, 5);
    s.pixel(24, int(py) - 1, false);
    s.pixel(26, int(py));
    s.line(0, 63, 127, 63);
}
inline void GameSession::matrix_flappy(Matrix &leds) const {
    auto put = [&](int x, int y, Color c) {
        if (x >= 0 && x < 5 && y >= 0 && y < 5)
            leds[y * 5 + x] = c;
    };
    const Color c = gameColor();
    (void)c;
    for (auto p : pipes)
        if (p.x >= 0 && p.x < 128) {
            int x = int(p.x * 5 / 128);
            for (int y = 0; y < 5; y++)
                if (y < int((p.top - 12) * 5 / 52) || y > int((p.top + 23 - 12) * 5 / 52))
                    put(x, y, {0, 80, 0});
        }
    put(1, int((py - 12) * 5 / 52), c);
}
} // namespace arcade
