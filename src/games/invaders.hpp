#pragma once
#include "../core/game_session.hpp"
namespace arcade {
inline void GameSession::reset_invaders() {
    resetFleet();
}
inline void GameSession::update_invaders(const Input &in, unsigned dt) {
    (void)in;
    (void)dt;
    const float t = dt / 1000.0f;
    (void)t;

    px = clamp(px + in.x * 93 * t, 6, 121);
    shotClock += dt;
    enemyClock += dt;
    shieldMs = std::max(0, shieldMs - int(dt));
    fleetX += fleetDir * (10 + level * 3) * t;
    if (fleetX < 3 || fleetX + 84 > 125) {
        fleetX = clamp(fleetX, 3, 41);
        fleetDir = -fleetDir;
        fleetY += 3;
    }
    if (in.actionHeld && bulletY < 0 && shotClock >= 220) {
        bulletX = px;
        bulletY = 54;
        shotClock = 0;
        event = Event::Shot;
    }
    if (bulletY >= 0) {
        bulletY -= 145 * t;
        for (int i = 0; i < 15; i++)
            if (aliens[i]) {
                float x = fleetX + (i % 5) * 19, y = fleetY + (i / 5) * 9;
                if (bulletX >= x - 2 && bulletX <= x + 9 && bulletY >= y - 2 && bulletY <= y + 6) {
                    aliens[i] = false;
                    bulletY = -1;
                    point(10 + (2 - i / 5) * 5);
                    break;
                }
            }
    }
    if (enemyBulletY < 0 && enemyClock >= unsigned(std::max(300, 1000 - level * 60))) {
        int count = 0;
        for (bool b : aliens)
            count += b;
        if (count) {
            int n = int(random() % unsigned(count));
            for (int i = 0; i < 15; i++)
                if (aliens[i] && n-- == 0) {
                    enemyX = fleetX + (i % 5) * 19 + 4;
                    enemyBulletY = fleetY + (i / 5) * 9 + 6;
                    break;
                }
        }
        enemyClock = 0;
    }
    if (enemyBulletY >= 0) {
        enemyBulletY += (43 + level * 3) * t;
        if (enemyBulletY > 64)
            enemyBulletY = -1;
        else if (enemyBulletY >= 53 && std::fabs(enemyX - px) < 6 && !shieldMs) {
            enemyBulletY = -1;
            shieldMs = 900;
            if (--lives <= 0)
                over();
            else
                event = Event::Miss;
        }
    }
    for (int i = 0; i < 15; i++)
        if (aliens[i] && fleetY + (i / 5) * 9 + 5 >= 54) {
            over();
            break;
        }
    if (std::none_of(aliens.begin(), aliens.end(), [](bool b) { return b; })) {
        point(50);
        level++;
        resetFleet();
        event = Event::Win;
    }
}
inline void GameSession::draw_invaders(Screen &s) {
    for (int i = 0; i < 18; i++)
        s.pixel((i * 37 + 11) % 128, 13 + (i * 17) % 49);
    for (int i = 0; i < 15; i++)
        if (aliens[i]) {
            int x = int(fleetX) + (i % 5) * 19, y = int(fleetY) + (i / 5) * 9;
            s.rect(x + 1, y, 6, 4);
            s.pixel(x + 2, y + 1, false);
            s.pixel(x + 5, y + 1, false);
            s.pixel(x, y + 2);
            s.pixel(x + 7, y + 2);
            s.pixel(x + 1, y + 5);
            s.pixel(x + 6, y + 5);
        }
    if (!shieldMs || (elapsed / 100) % 2) {
        s.line(int(px) - 5, 60, int(px), 54);
        s.line(int(px), 54, int(px) + 5, 60);
        s.line(int(px) - 5, 60, int(px) + 5, 60);
    }
    if (bulletY >= 0)
        s.rect(int(bulletX), int(bulletY), 1, 4);
    if (enemyBulletY >= 0)
        s.rect(int(enemyX), int(enemyBulletY), 2, 3);
}
inline void GameSession::matrix_invaders(Matrix &leds) const {
    auto put = [&](int x, int y, Color c) {
        if (x >= 0 && x < 5 && y >= 0 && y < 5)
            leds[y * 5 + x] = c;
    };
    const Color c = gameColor();
    (void)c;
    for (int i = 0; i < 15; i++)
        if (aliens[i])
            put(i % 5, i / 5, c);
    put(int(px * 5 / 128), 4, {0, 255, 80});
    if (bulletY >= 0)
        put(int(bulletX * 5 / 128), 3, {255, 255, 255});
}
} // namespace arcade
