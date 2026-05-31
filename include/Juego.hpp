#pragma once

#include <memory>
#include "Motor.hpp"

class Juego {
private:
    std::unique_ptr<Motor> motor;

public:
    Juego() {
        motor = std::make_unique<Motor>();
    }

    ~Juego() {}

    void run() {
        while (motor->estaAbierto()) {
            motor->ejecutarCiclo();
        }
    }
};