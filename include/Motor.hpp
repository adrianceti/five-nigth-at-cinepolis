#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <ctime>      
#include "Guardia.hpp"
#include "MonitorCamaras.hpp"
#include "Personaje.hpp" 

class Motor {
private:
    sf::RenderWindow ventana;

    sf::Texture texturaOficina;
    std::optional<sf::Sprite> spriteOficina;

    // Control de Cámara (Vista)
    sf::View vistaOficina;
    sf::View vistaInterfaz; 
    float posicionCamaraX;
    float velocidadCamara;

    // Componentes Visuales de las Puertas
    sf::RectangleShape visualPuertaIzquierda;
    sf::RectangleShape visualPuertaDerecha;

    // Entidades del juego
    Guardia jugador;
    MonitorCamaras monitor; 
    Personaje gobo; 

    // Relojes de control
    sf::Clock relojEnergia;
    sf::Clock relojTerminal;

    // --- NUEVO: Control del Estado de la Partida ---
    bool juegoTerminado;
    float tiempoMuerteAcumulado;

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            // Si el juego terminó, se ignoran por completo los inputs del jugador
            if (juegoTerminado) return;

            // --- CONTROL POR TECLADO ---
            if (const auto* botonPresionado = evento->getIf<sf::Event::KeyPressed>()) {
                if (botonPresionado->code == sf::Keyboard::Key::A) {
                    jugador.alternarPuertaIzquierda();
                }
                if (botonPresionado->code == sf::Keyboard::Key::D) {
                    jugador.alternarPuertaDerecha();
                }
                
                if (botonPresionado->code == sf::Keyboard::Key::Space) {
                    jugador.alternarMonitor();
                }

                if (jugador.esMonitorAbierto()) {
                    if (botonPresionado->code == sf::Keyboard::Key::Num1) monitor.cambiarCamara(TipoCamara::CAM_01_DULCERIA);
                    if (botonPresionado->code == sf::Keyboard::Key::Num2) monitor.cambiarCamara(TipoCamara::CAM_02_PASILLO_A);
                    if (botonPresionado->code == sf::Keyboard::Key::Num3) monitor.cambiarCamara(TipoCamara::CAM_03_PASILLO_B); 
                    if (botonPresionado->code == sf::Keyboard::Key::Num4) monitor.cambiarCamara(TipoCamara::CAM_04_SALAS);
                    if (botonPresionado->code == sf::Keyboard::Key::Num5) monitor.cambiarCamara(TipoCamara::CAM_05_BANOS);
                }
            }
        }
    }

    void actualizar() {
        float dt = relojEnergia.restart().asSeconds();

        // --- NUEVA LÓGICA DE JUEGO TERMINADO ---
        if (juegoTerminado) {
            tiempoMuerteAcumulado += dt;
            // Después de 4 segundos en pantalla de Game Over, cerramos automáticamente el juego
            if (tiempoMuerteAcumulado >= 4.0f) {
                ventana.close();
            }
            return; 
        }

        jugador.bajarEnergia(dt);

        // Pasamos si la puerta izquierda está realmente cerrada a la IA de Gobo
        gobo.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada());

        // CONDICIÓN DE DERROTA: Si Gobo logró entrar a la oficina, el Jumpscare es inminente
        if (gobo.esEstaAdentro()) {
            juegoTerminado = true;
            tiempoMuerteAcumulado = 0.0f;
            return;
        }

        // El mouse solo mueve la cámara si el monitor está CERRADO
        if (!jugador.esMonitorAbierto()) {
            sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);
            
            if (posicionMouse.x >= 0 && posicionMouse.x <= 1280 && posicionMouse.y >= 0 && posicionMouse.y <= 720) {
                if (posicionMouse.x > 1000) posicionCamaraX += velocidadCamara * dt;
                else if (posicionMouse.x < 280) posicionCamaraX -= velocidadCamara * dt;

                if (posicionCamaraX < 640.0f) posicionCamaraX = 640.0f;
                if (posicionCamaraX > 960.0f) posicionCamaraX = 960.0f;

                vistaOficina.setCenter({posicionCamaraX, 360.0f});
            }
        }

        // --- MONITOR DE TELEMETRÍA EN TERMINAL ---
        if (relojTerminal.getElapsedTime().asSeconds() >= 0.1f) {
            if (jugador.esMonitorAbierto()) {
                monitor.mostrarEnTerminal();
            } else {
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
                for (int i = 0; i < jugador.getNivelConsumo(); i++) std::cout << "Z";
                std::cout << "\n-----------------------------------------\n";
                std::cout << " PUERTA IZQUIERDA : " << (jugador.esPuertaIzquierdaCerrada() ? "[X] CERRADA" : "[ ] ABIERTA") << "\n";
                std::cout << " PUERTA DERECHA   : " << (jugador.esPuertaDerechaCerrada() ? "[X] CERRADA" : "[ ] ABIERTA") << "\n";
                std::cout << "-----------------------------------------\n";
                
                if (gobo.esEnLaPuerta()) {
                    std::cout << " ALERTA CRITICA   : [!] ALGUIEN EN LA PUERTA IZQUIERDA\n";
                    std::cout << "                    ¡CIERRA LA PUERTA YA!\n";
                } else {
                    std::cout << " STATUS MONITOR   : [PANTALLA APAGADA]\n";
                    std::cout << " VISTA ACTUAL     : Oficina Principal\n";
                }
                std::cout << "=========================================\n";
            }
            relojTerminal.restart();
        }

        if (spriteOficina.has_value()) {
            if (jugador.getEnergia() <= 0.0f) {
                spriteOficina.value().setColor(sf::Color(10, 10, 30)); 
            } else if (gobo.esEnLaPuerta()) {
                spriteOficina.value().setColor(sf::Color(255, 150, 150)); 
            } else {
                spriteOficina.value().setColor(sf::Color::White);
            }
        }
    }

    void renderizar() {
        // Si perdimos, la ventana gráfica se va por completo a negro (Simulando el ataque súbito)
        if (juegoTerminado) {
            ventana.clear(sf::Color::Black);
            
            // Mensaje de muerte directo en la terminal
            #ifdef _WIN32
                std::system("cls");
            #else
                std::system("clear");
            #endif
            std::cout << "#########################################\n";
            std::cout << "           G A M E   O V E R             \n";
            std::cout << "#########################################\n\n";
            std::cout << "   " << gobo.getNombre() << " TE ATACÓ EN LA OFICINA...\n";
            std::cout << "   No cerraste la puerta izquierda a tiempo.\n\n";
            std::cout << "#########################################\n";
            
            ventana.display();
            return;
        }

        if (jugador.esMonitorAbierto()) {
            ventana.clear(sf::Color(10, 60, 15)); 
        } else {
            ventana.clear(sf::Color(40, 40, 40));
        }
        
        ventana.setView(vistaOficina);
        if (!jugador.esMonitorAbierto()) {
            if (spriteOficina.has_value()) {
                ventana.draw(spriteOficina.value());
            }

            if (jugador.esPuertaIzquierdaCerrada()) {
                ventana.draw(visualPuertaIzquierda);
            }
            if (jugador.esPuertaDerechaCerrada()) {
                ventana.draw(visualPuertaDerecha);
            }
        }
        
        if (jugador.esMonitorAbierto()) {
            ventana.setView(vistaInterfaz); 
            monitor.renderizar(ventana);
        }
        
        ventana.display();
    }

public:
    // Inicializamos las nuevas variables en el constructor
    Motor() : gobo("Gobo", 12), juegoTerminado(false), tiempoMuerteAcumulado(0.0f) {
        std::srand(static_cast<unsigned int>(std::time(nullptr))); 
        
        ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
        ventana.setFramerateLimit(60);
        
        posicionCamaraX = 640.0f;
        velocidadCamara = 400.0f; 
        vistaOficina.setSize({1280.0f, 720.0f});
        vistaOficina.setCenter({posicionCamaraX, 360.0f});

        vistaInterfaz.setSize({1280.0f, 720.0f});
        vistaInterfaz.setCenter({640.0f, 360.0f});

        visualPuertaIzquierda.setSize(sf::Vector2f(120.0f, 600.0f));
        visualPuertaIzquierda.setFillColor(sf::Color(80, 85, 95));
        visualPuertaIzquierda.setOutlineColor(sf::Color::Black);
        visualPuertaIzquierda.setOutlineThickness(4.0f);
        visualPuertaIzquierda.setPosition(sf::Vector2f(100.0f, 60.0f));

        visualPuertaDerecha.setSize(sf::Vector2f(120.0f, 600.0f));
        visualPuertaDerecha.setFillColor(sf::Color(80, 85, 95));
        visualPuertaDerecha.setOutlineColor(sf::Color::Black);
        visualPuertaDerecha.setOutlineThickness(4.0f);
        visualPuertaDerecha.setPosition(sf::Vector2f(1380.0f, 60.0f));

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