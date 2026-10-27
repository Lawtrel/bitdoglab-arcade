#pragma once
#include "../games/blocks.hpp"
#include "../games/flappy.hpp"
#include "../games/invaders.hpp"
#include "../games/maze.hpp"
#include "../games/pong.hpp"
#include "../games/pokemon.hpp"
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
    static constexpr unsigned count = kGameCount;
    static const GameDefinition &get(unsigned index) {
        static const GameDefinition catalog[kGameCount] = {
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
            {"CORRIDA AZUL", "BAIXO+B:GIRO B:SALTO", &GameSession::reset_runner,
             &GameSession::update_runner, &GameSession::draw_runner, &GameSession::matrix_runner},
            {"MINI RPG", "B: ATACAR / FALAR", &GameSession::reset_rpg, &GameSession::update_rpg,
             &GameSession::draw_rpg, &GameSession::matrix_rpg},
            {"LABIRINTOS", "3 CHAVES. B: MAPA", &GameSession::reset_maze, &GameSession::update_maze,
             &GameSession::draw_maze, &GameSession::matrix_maze},
            {"POKEMON MINI","B: EQUIPE / BATALHA",&GameSession::reset_pokemon,&GameSession::update_pokemon,&GameSession::draw_pokemon,&GameSession::matrix_pokemon},
        };
        return catalog[index % count];
    }
};
} // namespace arcade
