#include "../include/Motor.hpp"
#include <iostream>

// Lista de inicialización obligatoria en SFML 3 para enlazar el sprite con su textura
Motor::Motor() : spriteOficina(texturaOficina) {
    ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
    ventana.setFramerateLimit(60);

    // Intentamos cargar la imagen de la oficina
    if (!texturaOficina.loadFromFile("assets/textures/oficina.png")) {
        std::cerr << "Error: No se encontro assets/textures/oficina.png" << std::endl;
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
    ventana.clear(sf::Color::Black);
    
    // Dibujamos el sprite directamente de forma segura
    ventana.draw(spriteOficina);
    
    ventana.display();
}

void Motor::ejecutarCiclo() {
    procesarEventos();
    actualizar();
    renderizar();
}