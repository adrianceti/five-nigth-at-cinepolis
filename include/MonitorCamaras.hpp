#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

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
    sf::RectangleShape cajaMapa; // Contenedor temporal para simular el mapa en pantalla

public:
    MonitorCamaras() : camaraActual(TipoCamara::CAM_01_DULCERIA) {
        // Creamos un rectángulo simple que representará el mapa en la esquina
        cajaMapa.setSize(sf::Vector2f(300.f, 400.f));
        cajaMapa.setFillColor(sf::Color(20, 20, 20, 200)); // Gris semi-transparente
        cajaMapa.setOutlineColor(sf::Color::Green);
        cajaMapa.setOutlineThickness(2.f);
        cajaMapa.setPosition(sf::Vector2f(900.f, 200.f)); // Esquina derecha de la pantalla
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
    }
};