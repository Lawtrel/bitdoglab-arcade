#include "hardware.hpp"
#include "pico/bootrom.h"
#include "pico/stdio_usb.h"
#include <cstdio>
int main() {
    set_sys_clock_khz(125000, true);
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_usb, false);
    board::init();
    static arcade::Screen screen;
    screen.center(6, "BITDOG", 2);
    screen.center(28, "ARCADE", 2);
    screen.center(53, "9 JOGOS / OLED + LED");
    board::display(screen);
    bool passed = arcade::selfTest();
    if (!passed) {
        screen.clear();
        screen.center(12, "ERRO INTERNO");
        screen.center(30, "AUTOTESTE FALHOU");
        board::display(screen);
        for (;;) {
            board::tone(220, 120);
            sleep_ms(600);
        }
    }
    sleep_ms(700);
    static arcade::Engine engine;
    engine.session.rng ^= uint32_t(time_us_64());
    if (!engine.session.rng)
        engine.session.rng = 1;
    static std::array<arcade::Color, 25> colors{};
    uint32_t previous = board::now();
    unsigned frame = 0;
    for (;;) {
        uint32_t start = board::now();
        auto in = board::input(engine.settings);
        unsigned step = unsigned(std::min<uint32_t>(50, start - previous));
        engine.tick(in, step ? step : 20);
        board::sound(engine.event, engine.settings.sound);
        board::pump();
        if ((frame++ % 2) == 0) {
            engine.render(screen);
            board::display(screen);
            engine.matrix(colors);
            board::matrix(colors, engine.settings);
            board::rgb(engine.gameColor());
        }
        int ch = getchar_timeout_us(0);
        if (ch == '?') {
            std::printf("{\"app\":\"bitdoglab_arcade_v3\",\"games\":9,\"selftest\":true,\"oled\":%"
                        "s,\"oled_sda\":%d,\"game\":%d,\"score\":%d}\n",
                        board::oled ? "true" : "false", board::oledSda, engine.session.game,
                        engine.session.score);
            stdio_flush();
        }
        // Passo de 20 ms; display em ate 25 Hz. Sem espera por USB para jogar.
        uint32_t spent = board::now() - start;
        if (spent < 20)
            sleep_ms(20 - spent);
        previous = start;
    }
}
