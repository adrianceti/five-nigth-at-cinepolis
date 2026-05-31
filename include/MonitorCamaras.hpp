#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <random>
#include <cstdlib>

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
    }

    void cambiarCamara(TipoCamara nuevaCamara) {
        camaraActual = nuevaCamara;
    }

    TipoCamara getCamaraActual() const {
        return camaraActual;
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
        
        // Dibuja líneas horizontales (efecto de scanlines)
        for (float y = 110.0f; y < 600.0f; y += 20.0f) {
            sf::RectangleShape linea({360.0f, 1.0f});
            linea.setPosition({860.0f, y});
            linea.setFillColor(sf::Color(0, 100, 0));
            ventana.draw(linea);
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