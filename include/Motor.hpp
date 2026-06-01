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

    // Componentes Visuales de las Puertas (Ajustadas al centro)
    sf::RectangleShape visualPuertaIzquierda;
    sf::RectangleShape visualPuertaDerecha;

    // --- NUEVO: Indicadores Gráficos de HUD (Sin necesidad de Fuentes/Fonts) ---
    sf::RectangleShape barraEnergiaFondo;
    sf::RectangleShape barraEnergiaFrente;
    sf::RectangleShape bloquesConsumo[4]; // Hasta 4 niveles de consumo visual

    // Entidades del juego
    Guardia jugador;
    MonitorCamaras monitor; 
    Personaje gobo; 

    // Relojes de control
    sf::Clock relojEnergia;
    sf::Clock relojTerminal;
    
    // Control del Tiempo y Victoria
    sf::Clock relojNoche;
    int horaActual;             
    float tiempoPorHora;        
    float acumuladorHora;       
    bool juegoTerminado;
    bool victoria;              
    float tiempoMuerteAcumulado;

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            if (juegoTerminado || victoria) return;

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

        if (juegoTerminado || victoria) {
            tiempoMuerteAcumulado += dt;
            if (tiempoMuerteAcumulado >= 5.0f) { 
                ventana.close();
            }
            return; 
        }

        // Lógica del Tiempo de la Noche
        acumuladorHora += dt;
        if (acumuladorHora >= tiempoPorHora) {
            acumuladorHora = 0.0f;
            if (horaActual == 12) horaActual = 1;
            else horaActual++;

            if (horaActual == 6) {
                victoria = true;
                tiempoMuerteAcumulado = 0.0f;
                return;
            }
        }

        jugador.bajarEnergia(dt);
        gobo.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada());

        if (gobo.esEstaAdentro()) {
            juegoTerminado = true;
            tiempoMuerteAcumulado = 0.0f;
            return;
        }

        // --- ACTUALIZAR HUD GRÁFICO ---
        // Redimensionar la barra verde de energía según el porcentaje restante (0 a 250 píxeles de ancho)
        float porcentajeEnergia = jugador.getEnergia() / 100.0f;
        if (porcentajeEnergia < 0.0f) porcentajeEnergia = 0.0f;
        barraEnergiaFrente.setSize(sf::Vector2f(250.0f * porcentajeEnergia, 20.0f));

        // Cambiar color de la barra si le queda poca batería
        if (jugador.getEnergia() < 25.0f) {
            barraEnergiaFrente.setFillColor(sf::Color::Red);
        } else {
            barraEnergiaFrente.setFillColor(sf::Color::Green);
        }

        // El mouse mueve la oficina panorámica
        if (!jugador.esMonitorAbierto()) {
            sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);
            
            if (posicionMouse.x >= 0 && posicionMouse.x <= 1280 && posicionMouse.y >= 0 && posicionMouse.y <= 720) {
                if (posicionMouse.x > 1050) posicionCamaraX += velocidadCamara * dt;
                else if (posicionMouse.x < 230) posicionCamaraX -= velocidadCamara * dt;

                // Límites de paneo extendidos para que recorras toda tu imagen de fondo
                if (posicionCamaraX < 640.0f) posicionCamaraX = 640.0f;
                if (posicionCamaraX > 1100.0f) posicionCamaraX = 1100.0f; 

                vistaOficina.setCenter({posicionCamaraX, 360.0f});
            }
        }

        if (relojTerminal.getElapsedTime().asSeconds() >= 0.1f) {
            // Mantenemos las lecturas exactas en consola para que veas la hora exacta (1 AM, 2 AM...)
            #ifdef _WIN32
                std::system("cls");
            #else
                std::system("clear");
            #endif
            std::cout << "=========================================\n";
            std::cout << "  RELOJ DE LA NOCHE : " << horaActual << " AM \n";
            std::cout << "  ENERGIA RESTANTE  : " << static_cast<int>(jugador.getEnergia()) << "%\n";
            std::cout << "=========================================\n";
            relojTerminal.restart();
        }

        if (spriteOficina.has_value()) {
            if (jugador.getEnergia() <= 0.0f) {
                spriteOficina.value().setColor(sf::Color(10, 10, 30)); 
            } else if (gobo.esEnLaPuerta()) {
                spriteOficina.value().setColor(sf::Color(255, 180, 180)); 
            } else {
                spriteOficina.value().setColor(sf::Color::White);
            }
        }
    }

    void renderizar() {
        if (victoria) {
            ventana.clear(sf::Color(20, 80, 20)); // Pantalla verde de victoria
            ventana.display();
            return;
        }

        if (juegoTerminado) {
            ventana.clear(sf::Color::Red); // Pantalla roja de Jumpscare/Game Over
            ventana.display();
            return;
        }

        if (jugador.esMonitorAbierto()) {
            int tonoVerde = 30 + static_cast<int>(monitor.getCamaraActual()) * 15;
            ventana.clear(sf::Color(10, tonoVerde, 15)); 
        } else {
            ventana.clear(sf::Color::Black);
        }
        
        // 1. DIBUJAR ENTORNO (Se mueve con la cámara)
        ventana.setView(vistaOficina);
        if (!jugador.esMonitorAbierto()) {
            if (spriteOficina.has_value()) {
                ventana.draw(spriteOficina.value());
            }

            // CORREGIDO: Las puertas ahora aparecen directamente en las entradas laterales visibles
            if (jugador.esPuertaIzquierdaCerrada()) {
                ventana.draw(visualPuertaIzquierda);
            }
            if (jugador.esPuertaDerechaCerrada()) {
                ventana.draw(visualPuertaDerecha);
            }
        }
        
        // 2. DIBUJAR INTERFAZ (Fija en la pantalla)
        ventana.setView(vistaInterfaz); 
        if (jugador.esMonitorAbierto()) {
            monitor.renderizar(ventana);
        } else {
            // Dibujar la barra de batería gráfica en la esquina inferior izquierda
            ventana.draw(barraEnergiaFondo);
            ventana.draw(barraEnergiaFrente);

            // Dibujar los bloques indicadores de consumo
            for (int i = 0; i < jugador.getNivelConsumo() && i < 4; i++) {
                ventana.draw(bloquesConsumo[i]);
            }
        }
        
        ventana.display();
    }

public:
    Motor() : gobo("Gobo", 12), 
              horaActual(12), 
              tiempoPorHora(15.0f),  
              acumuladorHora(0.0f), 
              juegoTerminado(false), 
              victoria(false), 
              tiempoMuerteAcumulado(0.0f) {
                  
        std::srand(static_cast<unsigned int>(std::time(nullptr))); 
        
        ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
        ventana.setFramerateLimit(60);
        
        // Configuración inicial de posición de vista para abarcar el fondo panorámico
        posicionCamaraX = 640.0f;
        velocidadCamara = 550.0f; // Un poco más rápido el paneo
        vistaOficina.setSize({1280.0f, 720.0f});
        vistaOficina.setCenter({posicionCamaraX, 360.0f});

        vistaInterfaz.setSize({1280.0f, 720.0f});
        vistaInterfaz.setCenter({640.0f, 360.0f});

        // --- CONFIGURACIÓN DE BARRAS HUD (SIN FUENTES) ---
        // Fondo de la barra de energía (Borde gris vacío)
        barraEnergiaFondo.setSize(sf::Vector2f(250.0f, 20.0f));
        barraEnergiaFondo.setFillColor(sf::Color(50, 50, 50));
        barraEnergiaFondo.setOutlineColor(sf::Color::White);
        barraEnergiaFondo.setOutlineThickness(2.0f);
        barraEnergiaFondo.setPosition(sf::Vector2f(40.0f, 650.0f));

        // Frente de la barra (Relleno dinámico verde)
        barraEnergiaFrente.setSize(sf::Vector2f(250.0f, 20.0f));
        barraEnergiaFrente.setFillColor(sf::Color::Green);
        barraEnergiaFrente.setPosition(sf::Vector2f(40.0f, 650.0f));

        // Inicializar los 4 bloques indicadores de consumo de energía justo al lado
        for (int i = 0; i < 4; i++) {
            bloquesConsumo[i].setSize(sf::Vector2f(15.0f, 20.0f));
            bloquesConsumo[i].setFillColor(sf::Color::Yellow);
            bloquesConsumo[i].setOutlineColor(sf::Color::Black);
            bloquesConsumo[i].setOutlineThickness(1.0f);
            bloquesConsumo[i].setPosition(sf::Vector2f(310.0f + (i * 20.0f), 650.0f));
        }

        // --- COORDENADAS AJUSTADAS DE LAS PUERTAS ---
        // Ahora las colocamos en los extremos del rango de la pantalla para que las veas caer
        // al arrastrar el mouse por completo a la izquierda o derecha de tu consola de control.
        
        // Puertas Izquierda (Caerá en el extremo izquierdo de la oficina)
        visualPuertaIzquierda.setSize(sf::Vector2f(260.0f, 680.0f));
        visualPuertaIzquierda.setFillColor(sf::Color(35, 35, 40, 245)); // Metal oscuro grueso
        visualPuertaIzquierda.setOutlineColor(sf::Color(100, 100, 100));
        visualPuertaIzquierda.setOutlineThickness(4.0f);
        visualPuertaIzquierda.setPosition(sf::Vector2f(60.0f, 20.0f)); 

        // Puerta Derecha (Aparecerá al desplazar la mirada hacia los pósters de la derecha)
        visualPuertaDerecha.setSize(sf::Vector2f(260.0f, 680.0f));
        visualPuertaDerecha.setFillColor(sf::Color(35, 35, 40, 245));
        visualPuertaDerecha.setOutlineColor(sf::Color(100, 100, 100));
        visualPuertaDerecha.setOutlineThickness(4.0f);
        visualPuertaDerecha.setPosition(sf::Vector2f(1340.0f, 20.0f)); 

        if (!texturaOficina.loadFromFile("assets/textures/oficina.png")) {
            std::cerr << "Error: No se encontro assets/textures/oficina.png" << std::endl;
        } else {
            spriteOficina.emplace(texturaOficina);
        }

        relojEnergia.restart();
        relojTerminal.restart();
        relojNoche.restart();
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