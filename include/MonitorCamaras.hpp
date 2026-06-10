#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <random>
#include <cstdlib>
#include <map>
#include <vector>

// Enumerado para las distintas zonas de Cinépolis
enum class TipoCamara {
    CAM_01_DULCERIA,
    CAM_02_PASILLO_A,
    CAM_03_PASILLO_B,
    CAM_04_SALAS,
    CAM_05_BANOS
};

class MonitorCamaras {
private:
    TipoCamara camaraActual;
    sf::RectangleShape cajaMapa;
    mutable std::mt19937 generadorAleatorio;
    mutable std::uniform_int_distribution<int> distribucionEstadistica;

    // Sistema de texturas y renderización
    std::map<TipoCamara, sf::Texture> texturasFondo;
    std::map<std::string, sf::Texture> texturasPersonajes;
    
    // Registro de personajes en cada cámara (nombre -> presente)
    std::map<std::string, bool> personajesPorCamara;
    std::map<std::string, sf::Sprite> spritesPersonajes;

    void generarEstatica() const {
        // Genera estática visual en la terminal (simulación de ruido de cámara)
        for (int y = 0; y < 12; y++) {
            std::string fila;
            for (int x = 0; x < 60; x++) {
                int caracter = distribucionEstadistica(generadorAleatorio);
                if (caracter < 30) fila += " ";
                else if (caracter < 60) fila += ".";
                else if (caracter < 80) fila += ":";
                else if (caracter < 95) fila += "#";
                else fila += "@";
            }
            std::cout << "║ " << fila << " ║\n";
        }
    }

    void cargarTexturasFondo() {
        // Cargar fondos de cámaras (si existen, opcional)
        // Por ahora usaremos colores dinámicos, pero aquí podría haber fondos reales
    }

    void cargarTexturasPersonajes() {
        // Cargar sprites de todos los personajes
        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
        std::vector<std::string> carpetas = {"gobo", "director", "popy", "theusher", "ticketystub"};
        
        for (size_t i = 0; i < personajes.size(); i++) {
            std::string rutaBase = "assets/textures/personajes/" + carpetas[i] + "/sprite" + carpetas[i] + "/";
            // Intentar cargar PNG
            std::string rutaImagen = rutaBase + "*.png";
            
            // Construir ruta corregida (buscando el archivo específico)
            sf::Texture textura;
            bool cargada = false;
            
            // Intentar múltiples variantes de nombre de archivo
            std::vector<std::string> nombresPosibles = {
                "sprite" + carpetas[i] + ".png",
                personajes[i] + ".png",
                "sprite.png",
                carpetas[i] + ".png"
            };
            
            for (const auto& nombre : nombresPosibles) {
                if (textura.loadFromFile(rutaBase + nombre)) {
                    cargada = true;
                    std::cerr << "✓ Cargada textura de " << personajes[i] << " desde " << rutaBase + nombre << std::endl;
                    break;
                }
            }
            
            if (cargada) {
                texturasPersonajes.insert({personajes[i], textura});
                sf::Sprite sprite(textura);
                sprite.setScale({0.3f, 0.3f}); // Escalar a tamaño visible en monitor
                spritesPersonajes.insert({personajes[i], sprite});
            } else {
                std::cerr << "⚠ No se encontró textura para " << personajes[i] << std::endl;
            }
        }
    }

public:

    MonitorCamaras() 
        : camaraActual(TipoCamara::CAM_01_DULCERIA),
          generadorAleatorio(std::random_device{}()),
          distribucionEstadistica(0, 100) {
        
        // Creamos un rectángulo con marco verde retro
        cajaMapa.setSize(sf::Vector2f(380.f, 520.f));
        cajaMapa.setFillColor(sf::Color(10, 40, 10, 230)); // Verde oscuro semi-transparente
        cajaMapa.setOutlineColor(sf::Color(0, 255, 0));
        cajaMapa.setOutlineThickness(3.f);
        cajaMapa.setPosition(sf::Vector2f(850.f, 90.f)); // Esquina derecha de la pantalla
        
        // Cargar texturas
        cargarTexturasPersonajes();
        
        // Inicializar registro de personajes
        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
        for (const auto& p : personajes) {
            personajesPorCamara[p] = false;
        }
    }

    void cambiarCamara(TipoCamara nuevaCamara) {
        camaraActual = nuevaCamara;
    }

    TipoCamara getCamaraActual() const {
        return camaraActual;
    }

    // Registrar presencia de personaje en cámara
    void registrarPersonajeEnCamara(const std::string& nombre, bool presente) {
        if (personajesPorCamara.find(nombre) != personajesPorCamara.end()) {
            personajesPorCamara[nombre] = presente;
        }
    }

    // Consultar si un personaje está en la cámara actual
    bool hayPersonajesEnCamara() const {
        return false; // Por ahora, será actualizado desde Motor
    }

    // Devuelve el nombre de la cámara como texto para nuestra terminal
    std::string getNombreCamaraActual() const {
        switch (camaraActual) {
            case TipoCamara::CAM_01_DULCERIA:  return "CAM_01 - Dulceria Principal";
            case TipoCamara::CAM_02_PASILLO_A: return "CAM_02 - Pasillo Izquierdo";
            case TipoCamara::CAM_03_PASILLO_B: return "CAM_03 - Pasillo Derecho";
            case TipoCamara::CAM_04_SALAS:     return "CAM_04 - Acceso a Salas";
            case TipoCamara::CAM_05_BANOS:     return "CAM_05 - Zona de Banos";
        }
        return "Desconocida";
    }

    // Dibuja la interfaz del monitor sobre la ventana
    void renderizar(sf::RenderWindow& ventana) {
        ventana.draw(cajaMapa);
        
        // Dibujar fondos según cámara
        sf::Color colorFondo = obtenerColorFondo();
        sf::RectangleShape fondoMonitor(sf::Vector2f(360.f, 480.f));
        fondoMonitor.setFillColor(colorFondo);
        fondoMonitor.setPosition(sf::Vector2f(870.f, 110.f));
        ventana.draw(fondoMonitor);
        
        // Dibujar sprites de personajes si hay en la cámara actual
        dibujarPersonajesEnMonitor(ventana);
        
        // Dibuja líneas horizontales (efecto de scanlines)
        for (float y = 110.0f; y < 600.0f; y += 20.0f) {
            sf::RectangleShape linea({360.0f, 1.0f});
            linea.setPosition({860.0f, y});
            linea.setFillColor(sf::Color(0, 100, 0));
            ventana.draw(linea);
        }
    }

private:
    sf::Color obtenerColorFondo() const {
        // Colores dinámicos según la cámara (fondo limpio)
        switch (camaraActual) {
            case TipoCamara::CAM_01_DULCERIA:  return sf::Color(40, 60, 30);
            case TipoCamara::CAM_02_PASILLO_A: return sf::Color(30, 50, 40);
            case TipoCamara::CAM_03_PASILLO_B: return sf::Color(30, 50, 40);
            case TipoCamara::CAM_04_SALAS:     return sf::Color(35, 55, 35);
            case TipoCamara::CAM_05_BANOS:     return sf::Color(40, 50, 50);
            default:                           return sf::Color(20, 40, 20);
        }
    }

    void dibujarPersonajesEnMonitor(sf::RenderWindow& ventana) {
        // Este método será llamado desde Motor pasando los personajes activos
        // Por ahora solo dibuja si hay personajes detectados en la cámara
    }

public:
    // Dibuja un personaje específico en el monitor en la posición central
    void dibujarPersonaje(sf::RenderWindow& ventana, const std::string& nombre) {
        if (spritesPersonajes.find(nombre) != spritesPersonajes.end()) {
            sf::Sprite sprite = spritesPersonajes.at(nombre);
            // Posicionar en centro del monitor
            sprite.setPosition(sf::Vector2f(920.f, 280.f));
            ventana.draw(sprite);
        }
    }

    // Muestra la estática y la información en la terminal de Windows
    void mostrarEnTerminal() const {
        #ifdef _WIN32
            std::system("cls");
        #else
            std::system("clear");
        #endif

        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║          SISTEMA DE VIGILANCIA POR CAMARA - CINEPOLIS          ║\n";
        std::cout << "║                                                                ║\n";
        std::cout << "║  >>> TRANSMISION ACTIVA: " << getNombreCamaraActual() << "\n";
        std::cout << "║                                                                ║\n";
        std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
        
        // Genera estática
        generarEstatica();
        
        std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ CANALES DISPONIBLES:                                           ║\n";
        std::cout << "║ [1] DULCERIA    [2] PASILLO A   [3] PASILLO B                  ║\n";
        std::cout << "║ [4] SALAS       [5] BANOS                                       ║\n";
        std::cout << "║                                                                ║\n";
        std::cout << "║ [ESPACIO] Cerrar Monitor - Volver a Oficina                    ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    }
};