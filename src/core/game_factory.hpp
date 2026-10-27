#pragma once
#include "../games/blocks.hpp"
#include "../games/flappy.hpp"
#include "../games/invaders.hpp"
#include "../games/maze.hpp"
#include "../games/pong.hpp"
#include "../games/rpg.hpp"
#include "../games/runner.hpp"
#include "../games/snake.hpp"
namespace arcade {
struct GameDefinition {
    const char *name;
    const char *instructions;
    void (GameSession::*reset)();
    void (GameSession::*update)(const Input &, unsigned);
    void (GameSession::*draw)(Screen &);
    void (GameSession::*matrix)(Matrix &) const;
};
struct GameFactory {
    static constexpr unsigned count = 8;
    static const GameDefinition &get(unsigned index) {
        static const GameDefinition catalog[8] = {
            {"COBRINHA", "JOYSTICK: DIRECAO", &GameSession::reset_snake, &GameSession::update_snake,
             &GameSession::draw_snake, &GameSession::matrix_snake},
            {"PONG", "JOYSTICK: CIMA/BAIXO", &GameSession::reset_pong, &GameSession::update_pong,
             &GameSession::draw_pong, &GameSession::matrix_pong},
            {"BLOCOS", "JOYSTICK: RAQUETE", &GameSession::reset_blocks, &GameSession::update_blocks,
             &GameSession::draw_blocks, &GameSession::matrix_blocks},
            {"NAVE", "JOYSTICK + B: TIRO", &GameSession::reset_invaders,
             &GameSession::update_invaders, &GameSession::draw_invaders,
             &GameSession::matrix_invaders},
            {"PULINHO", "TOQUE B PARA VOAR", &GameSession::reset_flappy,
             &GameSession::update_flappy, &GameSession::draw_flappy, &GameSession::matrix_flappy},
            {"CORRIDA AZUL", "CORRA E SALTE COM B", &GameSession::reset_runner,
             &GameSession::update_runner, &GameSession::draw_runner, &GameSession::matrix_runner},
            {"MINI RPG", "B: ATACAR / FALAR", &GameSession::reset_rpg, &GameSession::update_rpg,
             &GameSession::draw_rpg, &GameSession::matrix_rpg},
            {"LABIRINTOS", "3 CHAVES. B: MAPA", &GameSession::reset_maze, &GameSession::update_maze,
             &GameSession::draw_maze, &GameSession::matrix_maze},
        };
        return catalog[index % count];
    }
};
} // namespace arcade
