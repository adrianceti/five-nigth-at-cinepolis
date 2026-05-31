#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>

class Motor {
private:
    sf::RenderWindow ventana;

    sf::Texture texturaOficina;
    std::optional<sf::Sprite> spriteOficina;

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }
        }
    }

    void actualizar() {}

    void renderizar() {
        ventana.clear(sf::Color(40, 40, 40));
        if (spriteOficina.has_value()) {
            ventana.draw(spriteOficina.value());
        }
        ventana.display();
    }

public:
    Motor() {
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

    ~Motor() {}

    void ejecutarCiclo() {
        procesarEventos();
        actualizar();
        renderizar();
    }

    bool estaAbierto() const {
        return ventana.isOpen();
    }
};