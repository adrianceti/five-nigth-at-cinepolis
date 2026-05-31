#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include "Guardia.hpp"

class Motor {
private:
    sf::RenderWindow ventana;

    sf::Texture texturaOficina;
    std::optional<sf::Sprite> spriteOficina;

    // --- NUEVO: Control de Cámara (Vista) ---
    sf::View vistaOficina;
    float posicionCamaraX;
    float velocidadCamara;

    // Instancia del jugador y los relojes de control
    Guardia jugador;
    sf::Clock relojEnergia;
    sf::Clock relojTerminal;

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            // --- CONTROL POR TECLADO ---
            if (const auto* botonPresionado = evento->getIf<sf::Event::KeyPressed>()) {
                if (botonPresionado->code == sf::Keyboard::Key::A) {
                    jugador.alternarPuertaIzquierda();
                }
                if (botonPresionado->code == sf::Keyboard::Key::D) {
                    jugador.alternarPuertaDerecha();
                }
            }
        }
    }

    void actualizar() {
        float dt = relojEnergia.restart().asSeconds();
        jugador.bajarEnergia(dt);

        // --- LÓGICA DE MOVIMIENTO DE CÁMARA CON EL MOUSE ---
        sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);
        
        // Solo movemos la cámara si el mouse está dentro de los límites de la ventana
        if (posicionMouse.x >= 0 && posicionMouse.x <= 1280 && posicionMouse.y >= 0 && posicionMouse.y <= 720) {
            // Si el mouse está en el extremo derecho, la cámara va a la derecha
            if (posicionMouse.x > 1000) {
                posicionCamaraX += velocidadCamara * dt;
            }
            // Si el mouse está en el extremo izquierdo, la cámara va a la izquierda
            else if (posicionMouse.x < 280) {
                posicionCamaraX -= velocidadCamara * dt;
            }

            // Límites para que la cámara no muestre el vacío
            if (posicionCamaraX < 640.0f) posicionCamaraX = 640.0f;
            if (posicionCamaraX > 960.0f) posicionCamaraX = 960.0f;

            // Aplicamos la nueva posición al centro de la cámara
            vistaOficina.setCenter({posicionCamaraX, 360.0f});
        }

        // Efecto visual de apagón
        if (spriteOficina.has_value()) {
            if (jugador.getEnergia() <= 0.0f) {
                spriteOficina.value().setColor(sf::Color(10, 10, 30)); 
            } else {
                spriteOficina.value().setColor(sf::Color::White);
            }
        }
    }

    void renderizar() {
        ventana.clear(sf::Color(40, 40, 40));
        
        // Asignamos la vista de la oficina antes de dibujar para que se mueva
        ventana.setView(vistaOficina);
        
        if (spriteOficina.has_value()) {
            ventana.draw(spriteOficina.value());
        }
        
        ventana.display();
    }

public:
    Motor() {
        ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
        ventana.setFramerateLimit(60);
        
        // Inicializamos la vista en el centro estándar (1280 / 2 = 640, 720 / 2 = 360)
        posicionCamaraX = 640.0f;
        velocidadCamara = 400.0f; // Píxeles por segundo que se desplazará la vista
        vistaOficina.setSize({1280.0f, 720.0f});
        vistaOficina.setCenter({posicionCamaraX, 360.0f});

        if (!texturaOficina.loadFromFile("assets/textures/oficina.png")) {
            std::cerr << "Error: No se encontro assets/textures/oficina.png" << std::endl;
        } else {
            spriteOficina.emplace(texturaOficina);
        }

        relojEnergia.restart();
        relojTerminal.restart();
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