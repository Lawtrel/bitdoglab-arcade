#pragma once
#include "game_factory.hpp"
namespace arcade {
// Facade do console; a navegacao usa FSM e os jogos sao estrategias estaticas.
struct ArcadeApp {
    Settings settings;
    Mode mode = Mode::Menu;
    Event event = Event::None;
    GameSession session;
    Input last{};
    int selection = 0, settingRow = 0;
    std::array<int, 8> best{};
    uint32_t elapsed = 0, navClock = 0;
    bool longHandled = false;
    static float clamp(float x, float low, float high) {
        return GameSession::clamp(x, low, high);
    }
    void remember() {
        best[session.game] = std::max(best[session.game], session.score);
    }
    void start(int index) {
        session.resetCommon(index % 8);
        (session.*GameFactory::get(session.game).reset)();
        mode = Mode::Play;
        event = Event::Start;
    }
    void menu() {
        remember();
        bool fromHelp = mode == Mode::Help;
        mode = Mode::Menu;
        if (!fromHelp)
            selection = session.game;
        navClock = elapsed + 200;
        event = Event::Select;
    }
    void settingChange(int direction) {
        switch (settingRow) {
        case 0:
            settings.sound = !settings.sound;
            break;
        case 1:
            settings.secondScreen = !settings.secondScreen;
            break;
        case 2:
            settings.brightness = (settings.brightness + direction + 3) % 3;
            break;
        case 3:
            settings.rotation = (settings.rotation + direction + 4) % 4;
            break;
        case 4:
            settings.swapXY = !settings.swapXY;
            break;
        case 5:
            settings.invertX = !settings.invertX;
            break;
        case 6:
            settings.invertY = !settings.invertY;
            break;
        case 7:
            mode = Mode::Test;
            break;
        }
        event = Event::Select;
    }
    void tick(const Input &in, unsigned dt = 20) {
        dt = std::min(dt, 50u);
        elapsed += dt;
        last = in;
        event = Event::None;
        bool longBack = in.pauseHeldMs >= 750 && !longHandled;
        if (in.pauseHeldMs == 0)
            longHandled = false;
        if (longBack)
            longHandled = true;
        if (in.menuPressed || longBack) {
            if (mode == Mode::Menu) {
                mode = Mode::Settings;
                navClock = elapsed + 250;
            } else
                menu();
            return;
        }
        bool navReady = elapsed >= navClock;
        if (mode == Mode::Menu) {
            if (navReady && std::fabs(in.y) > .5f) {
                selection = (selection + (in.y > 0 ? 1 : 7)) % 8;
                navClock = elapsed + 190;
                event = Event::Select;
            }
            if (in.actionPressed) {
                mode = Mode::Help;
                event = Event::Select;
            } else if (in.pauseReleased) {
                mode = Mode::Settings;
                navClock = elapsed + 220;
            }
            return;
        }
        if (mode == Mode::Settings) {
            if (navReady && std::fabs(in.y) > .5f) {
                settingRow = (settingRow + (in.y > 0 ? 1 : 7)) % 8;
                navClock = elapsed + 180;
                event = Event::Select;
            } else if (navReady && std::fabs(in.x) > .5f) {
                settingChange(in.x > 0 ? 1 : -1);
                navClock = elapsed + 200;
            } else if (in.actionPressed)
                settingChange(1);
            if (in.pauseReleased)
                menu();
            return;
        }
        if (mode == Mode::Test) {
            if (in.pauseReleased)
                mode = Mode::Settings;
            if (in.actionPressed)
                event = Event::Point;
            return;
        }
        if (mode == Mode::Help) {
            if (in.actionPressed)
                start(selection);
            else if (in.pauseReleased)
                menu();
            return;
        }
        if (mode == Mode::Dialog) {
            if (in.actionPressed || in.pauseReleased) {
                mode = Mode::Play;
                session.signal = GameSignal::Running;
                navClock = elapsed + 150;
            }
            return;
        }
        if (mode == Mode::Over) {
            if (in.actionPressed)
                start(session.game);
            else if (in.pauseReleased)
                menu();
            return;
        }
        if (in.pauseReleased) {
            mode = mode == Mode::Pause ? Mode::Play : Mode::Pause;
            event = Event::Select;
            return;
        }
        if (mode == Mode::Pause)
            return;

        session.elapsed = elapsed;
        session.event = Event::None;
        (session.*GameFactory::get(session.game).update)(in, dt);
        event = session.event;
        remember();
        if (session.signal == GameSignal::Finished)
            mode = Mode::Over;
        else if (session.signal == GameSignal::Dialog)
            mode = Mode::Dialog;
    }
    Color gameColor() const {
        return kColors[(mode == Mode::Menu || mode == Mode::Help) ? selection : session.game];
    }
    void render(Screen &s) {
        s.clear();
        char b[40];
        if (mode == Mode::Menu) {
            s.center(0, "BITDOG ARCADE");
            s.line(0, 10, 127, 10);
            for (int row = 0; row < 3; row++) {
                int n = (selection + row + 7) % 8;
                int y = 14 + row * 11;
                if (row == 1) {
                    s.rect(0, y - 1, 128, 10);
                    s.text(8, y, GameFactory::get(n).name, 1, false);
                } else
                    s.text(8, y, GameFactory::get(n).name);
            }
            std::snprintf(b, sizeof(b), "RECORDE %05d", best[selection]);
            s.center(48, b);
            s.center(57, "B:JOGAR  A:AJUSTES");
            return;
        }
        if (mode == Mode::Settings) {
            s.center(0, "AJUSTES");
            const char *labels[] = {"SOM",        "MATRIZ",     "BRILHO",     "GIRO MATRIZ",
                                    "TROCAR X/Y", "INVERTER X", "INVERTER Y", "TESTAR PLACA"};
            int top = std::max(0, std::min(4, settingRow - 1));
            for (int r = 0; r < 4; r++) {
                int n = top + r;
                char value[16] = "";
                if (n == 0)
                    std::snprintf(value, sizeof(value), "%s", settings.sound ? "ON" : "OFF");
                if (n == 1)
                    std::snprintf(value, sizeof(value), "%s",
                                  settings.secondScreen ? "EXTRA" : "PONTOS");
                if (n == 2)
                    std::snprintf(value, sizeof(value), "%d", settings.brightness + 1);
                if (n == 3)
                    std::snprintf(value, sizeof(value), "%d", settings.rotation * 90);
                if (n == 4)
                    std::snprintf(value, sizeof(value), "%s", settings.swapXY ? "SIM" : "NAO");
                if (n == 5)
                    std::snprintf(value, sizeof(value), "%s", settings.invertX ? "SIM" : "NAO");
                if (n == 6)
                    std::snprintf(value, sizeof(value), "%s", settings.invertY ? "SIM" : "NAO");
                std::snprintf(b, sizeof(b), "%c%-12s%s", n == settingRow ? '>' : ' ', labels[n],
                              value);
                s.text(0, 13 + r * 10, b);
            }
            s.center(56, "B:MUDA  A:VOLTAR");
            return;
        }
        if (mode == Mode::Test) {
            s.center(0, "TESTE DA PLACA");
            std::snprintf(b, sizeof(b), "X:%4u Y:%4u", unsigned(last.rawX), unsigned(last.rawY));
            s.text(2, 14, b);
            s.line(64, 28, 64, 49);
            s.line(53, 39, 75, 39);
            s.rect(62 + int(last.x * 24), 37 + int(last.y * 10), 4, 4);
            s.center(54, "B:SOM A:VOLTAR");
            return;
        }
        if (mode == Mode::Help) {
            s.center(3, GameFactory::get(selection).name,
                     std::strlen(GameFactory::get(selection).name) > 10 ? 1 : 2);
            s.center(25, GameFactory::get(selection).instructions);
            s.center(37, "A:PAUSA SW:MENU");
            s.center(51, "B: INICIAR");
            return;
        }
        session.hud(s);
        (session.*GameFactory::get(session.game).draw)(s);
        if (session.game == 6 || session.game == 7) {
            char b[24];
            std::snprintf(b, sizeof(b), "%c%d/3", session.game == 6 ? 'C' : 'K', session.crystals);
            s.text(52, 0, b);
        }
        if (mode == Mode::Dialog) {
            s.rect(2, 16, 124, 44, true, false);
            s.rect(2, 16, 124, 44, false);
            s.center(21, session.dialog1);
            s.center(33, session.dialog2);
            s.center(47, session.dialog3);
        }
        if (mode == Mode::Pause || mode == Mode::Over) {
            s.rect(10, 19, 108, 35, true, false);
            s.rect(10, 19, 108, 35, false);
            s.center(23,
                     mode == Mode::Pause ? "PAUSA" : (session.won ? "VOCE VENCEU!" : "FIM DE JOGO"),
                     mode == Mode::Pause ? 2 : 1);
            s.center(42, mode == Mode::Pause ? "A:CONTINUA SW:MENU" : "B:DE NOVO A:MENU");
        }
    }
    void matrix(Matrix &leds) const {
        leds.fill({});
        auto put = [&](int x, int y, Color c) {
            if (x >= 0 && x < 5 && y >= 0 && y < 5)
                leds[y * 5 + x] = c;
        };
        Color c = gameColor();
        if (mode == Mode::Settings || mode == Mode::Help || mode == Mode::Menu) {
            constexpr uint8_t symbols[8][5] = {{30, 16, 30, 2, 30},  {17, 10, 4, 10, 17},
                                               {31, 21, 31, 21, 31}, {14, 31, 21, 14, 17},
                                               {6, 15, 30, 12, 4},   {7, 14, 31, 12, 22},
                                               {4, 14, 4, 14, 17},   {31, 17, 23, 20, 31}};
            int n = (mode == Mode::Menu || mode == Mode::Help) ? selection : session.game;
            for (int y = 0; y < 5; y++)
                for (int x = 0; x < 5; x++)
                    if (symbols[n][y] & (1 << (4 - x)))
                        put(x, y, c);
            return;
        }
        if (mode == Mode::Over) {
            if (session.won) {
                put(0, 2, {0, 255, 0});
                put(1, 3, {0, 255, 0});
                put(2, 4, {0, 255, 0});
                put(3, 3, {0, 255, 0});
                put(4, 2, {0, 255, 0});
            } else
                for (int i = 0; i < 5; i++) {
                    put(i, i, {255, 0, 0});
                    put(4 - i, i, {255, 0, 0});
                }
            return;
        }
        if (mode == Mode::Pause) {
            for (int y = 0; y < 5; y++) {
                put(1, y, c);
                put(3, y, c);
            }
            return;
        }
        if (mode == Mode::Test) {
            for (int i = 0; i < 5; i++) {
                put(i, 0, {0, 30, 90});
                put(i, 4, {0, 30, 90});
            }
            put(int((last.x + 1) * 2), int((last.y + 1) * 2), {0, 255, 0});
            return;
        }
        if (!settings.secondScreen) {
            constexpr uint8_t digits[10][5] = {{7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7},
                                               {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1}, {7, 4, 7, 1, 7},
                                               {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1}, {7, 5, 7, 5, 7},
                                               {7, 5, 7, 1, 7}};
            char number[16];
            std::snprintf(number, sizeof(number), "%d", session.score);
            int count = int(std::strlen(number)), phase = int(elapsed / 850) % (count + 1);
            if (phase < count) {
                for (int y = 0; y < 5; y++)
                    for (int x = 0; x < 3; x++)
                        if (digits[number[phase] - '0'][y] & (1 << (2 - x)))
                            put(x + 1, y, c);
                put(0, phase % 5, {30, 30, 50});
            }
            return;
        }
        (session.*GameFactory::get(session.game).matrix)(leds);
    }
};
inline bool selfTest() {
    static Screen frame;
    frame.clear();
    frame.pixel(-1, 0);
    frame.rect(125, 62, 10, 10);
    if (frame.data[1023] != 0xc0)
        return false;
    static ArcadeApp app;
    app.start(0);
    app.session.food = {8, 4};
    for (int i = 0; i < 10; i++)
        app.tick({});
    if (app.session.score != 1 || app.session.length != 5)
        return false;
    Input reverse;
    reverse.x = -1;
    app.tick(reverse);
    if (app.session.nextX == -1)
        return false;
    Input pause;
    pause.pauseReleased = true;
    app.tick(pause);
    if (app.mode != Mode::Pause)
        return false;
    auto head = app.session.snake[0];
    for (int i = 0; i < 100; i++)
        app.tick({});
    if (app.session.snake[0].x != head.x)
        return false;
    app.tick(pause);
    if (app.mode != Mode::Play)
        return false;
    for (int game = 0; game < 8; game++) {
        app.start(game);
        for (int i = 0; i < 300; i++) {
            Input in;
            in.x = (i / 23) % 3 - 1;
            in.y = (i / 31) % 3 - 1;
            in.actionHeld = i % 3 != 0;
            in.actionPressed = i % 13 == 0;
            app.tick(in);
            app.render(frame);
            Matrix pixels;
            app.matrix(pixels);
            if (app.session.length < 1 || app.session.length > 160 ||
                !std::isfinite(app.session.bx) || !std::isfinite(app.session.py) ||
                app.session.score < 0)
                return false;
        }
    }
    return true;
}
} // namespace arcade
