#pragma once
#include "../engine.hpp"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/pio_instructions.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
namespace pico_adapter {
constexpr uint PIN_A = 5, PIN_B = 6, PIN_SW = 22, PIN_MATRIX = 7, PIN_BUZZ = 21;
inline bool oled = false;
inline uint8_t oledAddress = 0x3c;
inline int oledSda = 14;
inline PIO matrixPio = pio0;
inline uint matrixSm = 0;
inline float centerX = 2048, centerY = 2048;
inline uint32_t toneEnd = 0;
inline uint32_t now() {
    return to_ms_since_boot(get_absolute_time());
}
struct Button {
    uint pin;
    bool stable = false, lastRaw = false, pressed = false, released = false;
    uint32_t changed = 0, downAt = 0, duration = 0;
    void update(uint32_t t) {
        pressed = released = false;
        bool raw = !gpio_get(pin);
        if (raw != lastRaw) {
            lastRaw = raw;
            changed = t;
        }
        if (raw != stable && t - changed >= 25) {
            stable = raw;
            if (stable) {
                pressed = true;
                downAt = t;
            } else {
                released = true;
                duration = t - downAt;
            }
        }
    }
};
inline Button left{PIN_A}, right{PIN_B}, sw{PIN_SW};
inline bool send(const uint8_t *p, size_t n) {
    return i2c_write_timeout_us(i2c1, oledAddress, p, n, false, 5000) == int(n);
}
inline void display(const arcade::Screen &screen) {
    if (!oled)
        return;
    const uint8_t cmd[] = {0, 0x21, 0, 127, 0x22, 0, 7};
    if (!send(cmd, sizeof(cmd))) {
        oled = false;
        return;
    }
    for (int i = 0; i < 1024; i += 32) {
        uint8_t p[33];
        p[0] = 0x40;
        std::memcpy(p + 1, screen.data.data() + i, 32);
        if (!send(p, 33)) {
            oled = false;
            return;
        }
    }
}
inline uint16_t sample(unsigned channel) {
    adc_select_input(channel);
    adc_read();
    unsigned sum = 0;
    for (int i = 0; i < 4; i++)
        sum += adc_read();
    return uint16_t(sum / 4);
}
inline void stopTone() {
    pwm_set_gpio_level(PIN_BUZZ, 0);
    toneEnd = 0;
}
inline void tone(unsigned hz, unsigned duration) {
    auto slice = pwm_gpio_to_slice_num(PIN_BUZZ);
    float divider = 125;
    uint16_t top = uint16_t(clock_get_hz(clk_sys) / (divider * hz) - 1);
    pwm_set_clkdiv(slice, divider);
    pwm_set_wrap(slice, top);
    pwm_set_gpio_level(PIN_BUZZ, (top + 1) / 8);
    pwm_set_enabled(slice, true);
    toneEnd = now() + duration;
}
inline void sound(arcade::Event e, bool enabled) {
    if (!enabled) {
        stopTone();
        return;
    }
    switch (e) {
    case arcade::Event::Select:
        tone(850, 25);
        break;
    case arcade::Event::Start:
        tone(1300, 90);
        break;
    case arcade::Event::Point:
        tone(1800, 45);
        break;
    case arcade::Event::Miss:
        tone(220, 180);
        break;
    case arcade::Event::Shot:
        tone(2300, 15);
        break;
    case arcade::Event::Jump:
        tone(1100, 35);
        break;
    case arcade::Event::Win:
        tone(2100, 220);
        break;
    default:
        break;
    }
}
inline int matrixIndex(int x, int y, int rotation) {
    for (int r = 0; r < rotation; r++) {
        int z = x;
        x = 4 - y;
        y = z;
    }
    return 24 - (y * 5 + ((y % 2) ? (4 - x) : x));
}
inline void matrix(const std::array<arcade::Color, 25> &colors, const arcade::Settings &settings) {
    uint32_t values[25]{};
    unsigned scale = settings.matrixEnabled ? arcade::kLedBrightness[settings.brightness] : 0;
    for (int y = 0; y < 5; y++)
        for (int x = 0; x < 5; x++) {
            auto c = colors[y * 5 + x];
            unsigned r = unsigned(c.r) * scale / 255, g = unsigned(c.g) * scale / 255,
                     bl = unsigned(c.b) * scale / 255;
            values[matrixIndex(x, y, settings.rotation)] = (g << 16) | (r << 8) | bl;
        }
    for (auto value : values)
        pio_sm_put_blocking(matrixPio, matrixSm, value << 8);
}
inline void init() {
    for (uint pin : {PIN_A, PIN_B, PIN_SW}) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
    for (uint pin : {13u, 11u, 12u}) {
        gpio_set_function(pin, GPIO_FUNC_PWM);
        auto rgbSlice = pwm_gpio_to_slice_num(pin);
        pwm_set_wrap(rgbSlice, 255);
        pwm_set_clkdiv(rgbSlice, 16.0f);
        pwm_set_gpio_level(pin, 0);
        pwm_set_enabled(rgbSlice, true);
    }
    gpio_set_function(PIN_BUZZ, GPIO_FUNC_PWM);
    auto slice = pwm_gpio_to_slice_num(PIN_BUZZ);
    pwm_set_enabled(slice, true);
    stopTone();
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    for (int i = 0; i < 32; i++) {
        centerX += sample(1);
        centerY += sample(0);
        sleep_ms(2);
    }
    centerX /= 33;
    centerY /= 33;
    // A placa EmbarcaTech costuma usar 14/15; tambem reconhece OLED I2C1 em 2/3.
    for (auto pair : {std::array<int, 2>{14, 15}, std::array<int, 2>{2, 3}}) {
        i2c_init(i2c1, 400000);
        gpio_set_function(pair[0], GPIO_FUNC_I2C);
        gpio_set_function(pair[1], GPIO_FUNC_I2C);
        gpio_pull_up(pair[0]);
        gpio_pull_up(pair[1]);
        for (uint8_t addr : {uint8_t(0x3c), uint8_t(0x3d)}) {
            oledAddress = addr;
            const uint8_t test[] = {0, 0xae};
            if (send(test, 2)) {
                oled = true;
                oledSda = pair[0];
                break;
            }
        }
        if (oled)
            break;
        i2c_deinit(i2c1);
        gpio_init(pair[0]);
        gpio_init(pair[1]);
    }
    if (oled) {
        const uint8_t init[] = {0,    0xae, 0xd5, 0x80, 0xa8, 0x3f, 0xd3, 0,    0x40,
                                0x8d, 0x14, 0x20, 0,    0xa1, 0xc8, 0xda, 0x12, 0x81,
                                0x7f, 0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0x2e, 0xaf};
        oled = send(init, sizeof(init));
    }
    // As mesmas quatro instrucoes de ws2812.pio, codificadas pelo SDK. Sem pioasm host.
    const uint16_t instructions[] = {
        uint16_t(pio_encode_out(pio_x, 1) | pio_encode_sideset(1, 0) | pio_encode_delay(2)),
        uint16_t(pio_encode_jmp_not_x(3) | pio_encode_sideset(1, 1) | pio_encode_delay(1)),
        uint16_t(pio_encode_jmp(0) | pio_encode_sideset(1, 1) | pio_encode_delay(4)),
        uint16_t(pio_encode_nop() | pio_encode_sideset(1, 0) | pio_encode_delay(4))};
    pio_program program{};
    program.instructions = instructions;
    program.length = 4;
    program.origin = -1;
    uint offset = pio_add_program(matrixPio, &program);
    pio_gpio_init(matrixPio, PIN_MATRIX);
    pio_sm_set_consecutive_pindirs(matrixPio, matrixSm, PIN_MATRIX, 1, true);
    auto conf = pio_get_default_sm_config();
    sm_config_set_wrap(&conf, offset, offset + 3);
    sm_config_set_sideset(&conf, 1, false, false);
    sm_config_set_sideset_pins(&conf, PIN_MATRIX);
    sm_config_set_out_shift(&conf, false, true, 24);
    sm_config_set_fifo_join(&conf, PIO_FIFO_JOIN_TX);
    sm_config_set_clkdiv(&conf, float(clock_get_hz(clk_sys)) / (800000 * 10));
    pio_sm_init(matrixPio, matrixSm, offset, &conf);
    pio_sm_set_enabled(matrixPio, matrixSm, true);
    std::array<arcade::Color, 25> blank{};
    matrix(blank, {});
    sleep_ms(1);
}
inline float axis(float raw, float center) {
    float v = (raw - center) / std::max(900.0f, std::min(center, 4095 - center));
    if (std::fabs(v) < .16f)
        return 0;
    return arcade::Engine::clamp(v, -1, 1);
}
inline arcade::Input input(const arcade::Settings &settings) {
    uint32_t t = now();
    left.update(t);
    right.update(t);
    sw.update(t);
    uint16_t rawX = sample(1), rawY = sample(0);
    float x = axis(rawX, centerX), y = axis(rawY, centerY);
    if (settings.swapXY)
        std::swap(x, y);
    if (settings.invertX)
        x = -x;
    if (settings.invertY)
        y = -y;
    arcade::ButtonSample l{left.stable, left.pressed, left.released,
                           left.stable ? unsigned(t - left.downAt) : 0, left.duration};
    arcade::ButtonSample r{right.stable, right.pressed, right.released,
                           right.stable ? unsigned(t - right.downAt) : 0, right.duration};
    arcade::ButtonSample stick{sw.stable, sw.pressed, sw.released,
                               sw.stable ? unsigned(t - sw.downAt) : 0, sw.duration};
    auto in = arcade::InputMapper::map(x, y, l, r, stick);
    in.rawX = rawX;
    in.rawY = rawY;
    return in;
}
inline void rgb(arcade::Color c, const arcade::Settings &settings) {
    unsigned scale = settings.rgbEnabled ? arcade::kLedBrightness[settings.brightness] : 0;
    pwm_set_gpio_level(13, unsigned(c.r) * scale / 255);
    pwm_set_gpio_level(11, unsigned(c.g) * scale / 255);
    pwm_set_gpio_level(12, unsigned(c.b) * scale / 255);
}
inline void pump() {
    if (toneEnd && int32_t(now() - toneEnd) >= 0)
        stopTone();
}
} // namespace pico_adapter
