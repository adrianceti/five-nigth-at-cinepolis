#ifndef JUEGO_HPP
#define JUEGO_HPP

#include <SFML/Graphics.hpp>
#include <memory>

class Motor;

class Juego {
private:
    std::unique_ptr<Motor> motor;

public:
    Juego();
    ~Juego();
    void run();
};

#endif