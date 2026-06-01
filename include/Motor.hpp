#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <ctime>      // Para inicializar la semilla aleatoria de la IA
#include "Guardia.hpp"
#include "MonitorCamaras.hpp"
#include "Personaje.hpp" // Asegúrate de tener este archivo creado

class Motor {
private:
    sf::RenderWindow ventana;

    sf::Texture texturaOficina;
    std::optional<sf::Sprite> spriteOficina;

    // --- Control de Cámara (Vista) ---
    sf::View vistaOficina;
    sf::View vistaInterfaz; 
    float posicionCamaraX;
    float velocidadCamara;

    // Instancia del jugador, el monitor, el enemigo y los relojes de control
    Guardia jugador;
    MonitorCamaras monitor; 
    Personaje gobo; // NUEVO: Instancia del animatrónico enemigo
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
                
                // Abrir/Cerrar monitor con la Barra Espaciadora
                if (botonPresionado->code == sf::Keyboard::Key::Space) {
                    jugador.alternarMonitor();
                }

                // Cambiar canales de cámara con números del 1 al 5 si el monitor está abierto
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
        jugador.bajarEnergia(dt);

        // NUEVO: Actualizamos la Inteligencia Artificial de Gobo en cada frame
        gobo.actualizarIA(dt);

        // REGLA DE JUEGO: Si Gobo está en la puerta y el jugador cierra la puerta izquierda, Gobo regresa a la dulcería
        if (gobo.esEnLaPuerta() && jugador.esPuertaIzquierdaCerrada()) {
            gobo.resetear();
            std::cout << "\a"; // Pitido de advertencia en consola indicando que rebotó con éxito
        }

        // El mouse solo mueve la cámara si el monitor está CERRADO
        if (!jugador.esMonitorAbierto()) {
            sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);
            
            if (posicionMouse.x >= 0 && posicionMouse.x <= 1280 && posicionMouse.y >= 0 && posicionMouse.y <= 720) {
                if (posicionMouse.x > 1000) {
                    posicionCamaraX += velocidadCamara * dt;
                }
                else if (posicionMouse.x < 280) {
                    posicionCamaraX -= velocidadCamara * dt;
                }

                if (posicionCamaraX < 640.0f) posicionCamaraX = 640.0f;
                if (posicionCamaraX > 960.0f) posicionCamaraX = 960.0f;

                vistaOficina.setCenter({posicionCamaraX, 360.0f});
            }
        }

        // --- MONITOR DE TELEMETRÍA EN TERMINAL CONTROLADO ---
        if (relojTerminal.getElapsedTime().asSeconds() >= 0.1f) {
            if (jugador.esMonitorAbierto()) {
                // Si el monitor está abierto, delegamos la impresión al monitor
                monitor.mostrarEnTerminal();
            } else {
                // Si el monitor está cerrado, mostramos el HUD de la oficina
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
                
                // ALERTA DE PELIGRO: Te avisa en la oficina si Gobo está en el marco esperando a que te descuides
                if (gobo.esEnLaPuerta()) {
                    std::cout << " PELIGRO INMINENTE: [!] ALGUIEN EN LA PUERTA IZQUIERDA\n";
                } else {
                    std::cout << " STATUS MONITOR   : [PANTALLA APAGADA]\n";
                    std::cout << " VISTA ACTUAL     : Oficina Principal\n";
                }
                std::cout << "=========================================\n";
                std::cout << " Controles: 'A' Puerta Izq | 'D' Puerta Der | 'ESPACIO' Monitor\n";
            }

            relojTerminal.restart();
        }

        // Efecto visual de apagón, estática o alerta en la oficina
        if (spriteOficina.has_value()) {
            if (jugador.getEnergia() <= 0.0f) {
                spriteOficina.value().setColor(sf::Color(10, 10, 30)); 
            } else if (gobo.esEnLaPuerta()) {
                spriteOficina.value().setColor(sf::Color(255, 120, 120)); // Parpadeo rojizo de peligro si está afuera
            } else if (jugador.esMonitorAbierto()) {
                spriteOficina.value().setColor(sf::Color(40, 60, 40)); 
            } else {
                spriteOficina.value().setColor(sf::Color::White);
            }
        }
    }

    void renderizar() {
        ventana.clear(sf::Color(40, 40, 40));
        
        // 1. Dibujamos la oficina usando su cámara con desplazamiento
        ventana.setView(vistaOficina);
        if (spriteOficina.has_value()) {
            ventana.draw(spriteOficina.value());
        }
        
        // 2. Si el monitor está abierto, cambiamos a la vista fija y superponemos el mapa
        if (jugador.esMonitorAbierto()) {
            ventana.setView(vistaInterfaz); 
            monitor.renderizar(ventana);
        }
        
        ventana.display();
    }

public:
    // Inicializamos a Gobo con un nivel de dificultad de 12 para que sea activo en el testeo
    Motor() : gobo("Gobo", 12) {
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // Semilla para la aleatoriedad de la IA
        
        ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
        ventana.setFramerateLimit(60);
        
        // Inicializamos la vista móvil
        posicionCamaraX = 640.0f;
        velocidadCamara = 400.0f; 
        vistaOficina.setSize({1280.0f, 720.0f});
        vistaOficina.setCenter({posicionCamaraX, 360.0f});

        // Inicializamos la vista fija de interfaz
        vistaInterfaz.setSize({1280.0f, 720.0f});
        vistaInterfaz.setCenter({640.0f, 360.0f});

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