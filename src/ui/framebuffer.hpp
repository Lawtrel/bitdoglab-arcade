#pragma once
#include "../font6x8.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
namespace arcade {
constexpr int W = 128, H = 64;
struct Screen {
    std::array<uint8_t, 1024> data{};
    void clear() {
        data.fill(0);
    }
    void pixel(int x, int y, bool on = true) {
        if (x < 0 || x >= W || y < 0 || y >= H)
            return;
        auto &b = data[(y / 8) * W + x];
        if (on)
            b |= 1u << (y % 8);
        else
            b &= ~(1u << (y % 8));
    }
    void rect(int x, int y, int w, int h, bool fill = true, bool on = true) {
        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++)
                if (fill || i == 0 || j == 0 || i == w - 1 || j == h - 1)
                    pixel(x + i, y + j, on);
    }
    void line(int x, int y, int xx, int yy) {
        int dx = std::abs(xx - x), sx = x < xx ? 1 : -1, dy = -std::abs(yy - y),
            sy = y < yy ? 1 : -1, err = dx + dy;
        for (;;) {
            pixel(x, y);
            if (x == xx && y == yy)
                break;
            int e = 2 * err;
            if (e >= dy) {
                err += dy;
                x += sx;
            }
            if (e <= dx) {
                err += dx;
                y += sy;
            }
        }
    }
    void text(int x, int y, const char *s, int scale = 1, bool on = true) {
        for (; *s; s++, x += 6 * scale) {
            unsigned c = static_cast<unsigned char>(*s);
            if (c < 32 || c > 126)
                c = '?';
            for (int i = 0; i < 6; i++)
                for (int j = 0; j < 8; j++)
                    if (kFont6x8[c - 32][i] & (1u << j))
                        rect(x + i * scale, y + j * scale, scale, scale, true, on);
        }
    }
    void center(int y, const char *s, int scale = 1) {
        text((W - int(std::strlen(s)) * 6 * scale) / 2, y, s, scale);
    }
};
} // namespace arcade
