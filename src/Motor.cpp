#include "../include/Motor.hpp"
#include <iostream>

Motor::Motor() {
    ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
    ventana.setFramerateLimit(60);
    if (!texturaOficina.loadFromFile("../assets/textures/oficina.png")) {
        std::cerr << "Error: No se encontro ../assets/textures/oficina.png" << std::endl;
    } else {
        spriteOficina.setTexture(texturaOficina);
    }
}

Motor::~Motor() {}

bool Motor::estaAbierto() const {
    return ventana.isOpen();
}

void Motor::procesarEventos() {
    while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
        if (evento->is<sf::Event::Closed>()) {
            ventana.close();
        }
    }
}

void Motor::actualizar() {}

void Motor::renderizar() {
    ventana.clear(sf::Color(40, 40, 40));
    ventana.draw(spriteOficina);
    
    ventana.display();
}

void Motor::ejecutarCiclo() {
    procesarEventos();
    actualizar();
    renderizar();
}