#include "../include/Juego.hpp"
#include "../include/Motor.hpp"

Juego::Juego() {
    
    motor = std::make_unique<Motor>();
}

Juego::~Juego() {
   
}

void Juego::run() {
    
    while (motor->estaAbierto()) {
        motor->ejecutarCiclo();
    }
}