#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline void GameSession::reset_blocks() {
    resetBlocks();
}
inline void GameSession::update_blocks(const Input &in, unsigned dt) {
    (void)in;
    (void)dt;
    const float t = dt / 1000.0f;
    (void)t;

    px = clamp(px + in.x * 105 * t, 13, 115);
    if (stuck) {
        bx = px;
        by = 55;
        if (in.actionPressed) {
            stuck = false;
            vy = -55 - level * 3;
            vx = (random() & 1) ? 43 : -43;
            event = Event::Shot;
        }
        return;
    }
    bx += vx * t;
    by += vy * t;
    if (bx < 3) {
        bx = 3;
        vx = std::fabs(vx);
    }
    if (bx > 124) {
        bx = 124;
        vx = -std::fabs(vx);
    }
    if (by < 13) {
        by = 13;
        vy = std::fabs(vy);
    }
    if (vy > 0 && by >= 55 && by <= 61 && std::fabs(bx - px) <= 13) {
        by = 54;
        vy = -std::fabs(vy);
        vx = (bx - px) * 6;
        event = Event::Shot;
    }
    for (int i = 0; i < 24; i++)
        if (blocks[i]) {
            int x = 4 + (i % 8) * 15, y = 16 + (i / 8) * 6;
            if (bx >= x - 1 && bx <= x + 13 && by >= y - 1 && by <= y + 4) {
                blocks[i] = false;
                vy = -vy;
                point(10);
                break;
            }
        }
    if (by > 65) {
        if (--lives <= 0)
            over();
        else {
            stuck = true;
            event = Event::Miss;
        }
    }
    if (std::none_of(blocks.begin(), blocks.end(), [](bool b) { return b; })) {
        point(50);
        level++;
        resetBlocks();
        event = Event::Win;
    }
}
inline void GameSession::draw_blocks(Screen &s) {
    for (int i = 0; i < 24; i++)
        if (blocks[i])
            s.rect(4 + (i % 8) * 15, 16 + (i / 8) * 6, 13, 4, (i / 8) != 1);
    s.rect(int(px) - 11, 59, 23, 3);
    s.rect(int(bx) - 1, int(by) - 1, 3, 3);
    if (stuck)
        s.center(43, "B: LANCAR");
}
inline void GameSession::matrix_blocks(Matrix &leds) const {
    auto put = [&](int x, int y, Color c) {
        if (x >= 0 && x < 5 && y >= 0 && y < 5)
            leds[y * 5 + x] = c;
    };
    const Color c = gameColor();
    (void)c;
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 5; x++) {
            int col = x * 8 / 5;
            bool on = blocks[y * 8 + col];
            if (on)
                put(x, y, c);
        }
    put(int(px * 5 / 128), 4, {255, 255, 255});
    put(int(bx * 5 / 128), int((by - 12) * 5 / 52), {0, 255, 70});
}
} // namespace arcade
