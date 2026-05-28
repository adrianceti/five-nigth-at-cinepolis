#include "../include/Motor.hpp"
#include <iostream>

Motor::Motor() {
    ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
    ventana.setFramerateLimit(60);
    std::cout << "Intentando cargar: assets/textures/oficina.png" << std::endl;
    if (!texturaOficina.loadFromFile("assets/textures/oficina.png")) {
        std::cerr << "Error: No se encontro assets/textures/oficina.png" << std::endl;
    } else {
        std::cout << "Textura cargada exitosamente" << std::endl;
        std::cout << "Dimensiones de la textura: " << texturaOficina.getSize().x << "x" << texturaOficina.getSize().y << std::endl;
        spriteOficina.emplace(texturaOficina);
        std::cout << "Sprite creado" << std::endl;
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
    if (spriteOficina.has_value()) {
        ventana.draw(spriteOficina.value());
    }
    ventana.display();
}

void Motor::ejecutarCiclo() {
    procesarEventos();
    actualizar();
    renderizar();
}