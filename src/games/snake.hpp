#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline void GameSession::reset_snake() {
    length = 4;
    dx = nextX = 1;
    dy = nextY = 0;
    for (int i = 0; i < length; i++)
        snake[i] = {7 - i, 4};
    food = {15, 4};
    gameClock = 0;
}
inline void GameSession::update_snake(const Input &in, unsigned dt) {
    (void)in;
    (void)dt;
    const float t = dt / 1000.0f;
    (void)t;

    if (std::fabs(in.x) > .55f && std::fabs(in.x) >= std::fabs(in.y)) {
        int d = in.x > 0 ? 1 : -1;
        if (d != -dx) {
            nextX = d;
            nextY = 0;
        }
    } else if (std::fabs(in.y) > .55f) {
        int d = in.y > 0 ? 1 : -1;
        if (d != -dy) {
            nextX = 0;
            nextY = d;
        }
    }
    gameClock += dt;
    unsigned interval = unsigned(std::max(65, 190 - score * 4));
    if (gameClock < interval)
        return;
    gameClock -= interval;
    dx = nextX;
    dy = nextY;
    Cell head{snake[0].x + dx, snake[0].y + dy};
    bool eat = head.x == food.x && head.y == food.y;
    if (head.x < 0 || head.x >= 20 || head.y < 0 || head.y >= 8) {
        over();
        return;
    }
    for (int i = 0; i < length - (eat ? 0 : 1); i++)
        if (head.x == snake[i].x && head.y == snake[i].y) {
            over();
            return;
        }
    if (eat && length < 160)
        length++;
    for (int i = length - 1; i > 0; i--)
        snake[i] = snake[i - 1];
    snake[0] = head;
    if (eat) {
        point();
        spawnFood();
    }
}
inline void GameSession::draw_snake(Screen &s) {
    s.rect(3, 13, 122, 50, false);
    for (int i = length - 1; i >= 0; i--) {
        s.rect(4 + snake[i].x * 6, 14 + snake[i].y * 6, 5, 5);
        if (i == 0)
            s.pixel(6 + snake[i].x * 6, 16 + snake[i].y * 6, false);
    }
    int xx = 6 + food.x * 6, yy = 16 + food.y * 6;
    s.line(xx - 2, yy, xx + 2, yy);
    s.line(xx, yy - 2, xx, yy + 2);
}
inline void GameSession::matrix_snake(Matrix &leds) const {
    auto put = [&](int x, int y, Color c) {
        if (x >= 0 && x < 5 && y >= 0 && y < 5)
            leds[y * 5 + x] = c;
    };
    const Color c = gameColor();
    (void)c;
    for (int i = length - 1; i >= 0; i--)
        put(snake[i].x / 4, snake[i].y * 5 / 8, i ? Color{0, 70, 0} : Color{0, 255, 40});
    put(food.x / 4, food.y * 5 / 8, {255, 0, 0});
}
} // namespace arcade
