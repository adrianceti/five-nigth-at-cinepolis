#pragma once

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