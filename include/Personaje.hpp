#pragma once
#include <string>
#include <cstdlib>
#include "MonitorCamaras.hpp"

class Personaje {
private:
    std::string nombre;
    TipoCamara posicionActual;
    int dificultad;         // Rango de 1 a 20 (Como en FNAF)
    bool estaEnLaPuerta;    // true si ya llegó al marco de la oficina
    float tiempoAcumulado;  // Controla el temporizador de movimiento

public:
    Personaje(std::string nom, int dif) 
        : nombre(nom), posicionActual(TipoCamara::CAM_01_DULCERIA), 
          dificultad(dif), estaEnLaPuerta(false), tiempoAcumulado(0.0f) {}

    // Lógica para intentar moverse usando el Delta Time del juego
    void actualizarIA(float dt) {
        if (estaEnLaPuerta) return; // Si ya llegó a la puerta, no se mueve de ahí hasta atacar

        tiempoAcumulado += dt;
        
        // Cada 5 segundos hace un "Movment Opportunity"
        if (tiempoAcumulado >= 5.0f) {
            tiempoAcumulado = 0.0f;
            
            // Genera un número entre 1 y 20
            int intento = (std::rand() % 20) + 1;
            
            // Si el intento es menor o igual a su dificultad, el personaje avanza
            if (intento <= dificultad) {
                avanzarEnRuta();
            }
        }
    }

    // Define el camino específico de Gobo por el mapa
    void avanzarEnRuta() {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_04_SALAS;
                break;
            case TipoCamara::CAM_04_SALAS:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true; // Salió del pasillo y llegó a la puerta de la oficina
                break;
            default:
                break;
        }
    }

    // Métodos para reiniciar al personaje si lo bloqueas con la puerta
    void resetear() {
        posicionActual = TipoCamara::CAM_01_DULCERIA;
        estaEnLaPuerta = false;
        tiempoAcumulado = 0.0f;
    }

    // Getters
    std::string getNombre() const { return nombre; }
    TipoCamara getPosicionActual() const { return posicionActual; }
    bool esEnLaPuerta() const { return estaEnLaPuerta; }
};