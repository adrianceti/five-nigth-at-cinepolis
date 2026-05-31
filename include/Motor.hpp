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

    // Instancia del jugador y los relojes de control
    Guardia jugador;
    sf::Clock relojEnergia;
    sf::Clock relojTerminal; // Controla la velocidad de refresco de la consola

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

            // --- CONTROL POR MOUSE ---
            if (const auto* clickMouse = evento->getIf<sf::Event::MouseButtonPressed>()) {
                if (clickMouse->button == sf::Mouse::Button::Left) {
                    sf::Vector2i posicionMouse = clickMouse->position;

                    // Zona izquierda (Mapeo de clicks en borde de pantalla)
                    if (posicionMouse.x >= 0 && posicionMouse.x <= 150) {
                        jugador.alternarPuertaIzquierda();
                    }
                    // Zona derecha
                    else if (posicionMouse.x >= 1130 && posicionMouse.x <= 1280) {
                        jugador.alternarPuertaDerecha();
                    }
                }
            }
        }
    }

    void actualizar() {
        float dt = relojEnergia.restart().asSeconds();
        jugador.bajarEnergia(dt);

        // REFRESO DE TERMINAL CONTROLADO: Solo imprime cada 100 milisegundos
        if (relojTerminal.getElapsedTime().asSeconds() >= 0.1f) {
            // Limpia la terminal de forma estándar en Windows/Linux
#ifdef _WIN32
            std::system("cls");
#else
            std::system("clear");
#endif

            std::cout << "=========================================\n";
            std::cout << "     SISTEMA DE SEGURIDAD - CINEPOLIS    \n";
            std::cout << "=========================================\n";
            std::cout << " ENERGIA RESTANTE : " << static_cast<int>(jugador.getEnergia()) << "%\n";
            std::cout << " NIVEL DE CONSUMO : [ " << jugador.getNivelConsumo() << " ] ";
            
            for (int i = 0; i < jugador.getNivelConsumo(); i++) std::cout << "Z"; // Carácter simple para telemetría
            std::cout << "\n-----------------------------------------\n";
            std::cout << " PUERTA IZQUIERDA : " << (jugador.esPuertaIzquierdaCerrada() ? "[X] CERRADA" : "[ ] ABIERTA") << "\n";
            std::cout << " PUERTA DERECHA   : " << (jugador.esPuertaDerechaCerrada() ? "[X] CERRADA" : "[ ] ABIERTA") << "\n";
            std::cout << "=========================================\n";
            std::cout << " Instrucciones: Presiona 'A' o 'D' con la ventana activa.\n";

            relojTerminal.restart();
        }

        // Efecto visual de apagón en la ventana gráfica
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
        if (spriteOficina.has_value()) {
            ventana.draw(spriteOficina.value());
        }
        ventana.display();
    }

public:
    Motor() {
        ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
        ventana.setFramerateLimit(60);
        
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