#pragma once
#include <string>
#include <cstdlib>
#include "MonitorCamaras.hpp"

class Personaje {
private:
    std::string nombre;
    TipoCamara posicionActual;
    int dificultad;         // Rango de 1 a 20
    bool estaEnLaPuerta;    // true si está esperando afuera en el marco
    bool estaAdentro;       // true si ya logró entrar a la oficina (Peligro de Jumpscare)
    
    float tiempoAcumuladoIA;  // Controla el temporizador de los 5 segundos para moverse
    float tiempoEnPuerta;     // Controla cuánto tiempo lleva esperando en la puerta abierta

public:
    Personaje(std::string nom, int dif) 
        : nombre(nom), posicionActual(TipoCamara::CAM_01_DULCERIA), 
          dificultad(dif), estaEnLaPuerta(false), estaAdentro(false),
          tiempoAcumuladoIA(0.0f), tiempoEnPuerta(0.0f) {}

    // Lógica para actualizar la IA usando el Delta Time del juego
    void actualizarIA(float dt, bool puertaCerrada) {
        // Si ya entró a la oficina, ya no sigue la ruta de cámaras ni espera afuera
        if (estaAdentro) return; 

        // ESTADO 1: El personaje está esperando afuera en la puerta
        if (estaEnLaPuerta) {
            if (puertaCerrada) {
                // Si el jugador cerró la puerta a tiempo, el personaje rebota y regresa al inicio
                resetear();
                return;
            }

            // Si la puerta sigue abierta, aumenta el temporizador de ataque
            tiempoEnPuerta += dt;
            if (tiempoEnPuerta >= 3.5f) { // Tiene 3.5 segundos para cerrar antes de que entre
                estaAdentro = true;
                estaEnLaPuerta = false;
            }
            return;
        }

        // ESTADO 2: El personaje está en las cámaras intentando avanzar
        tiempoAcumuladoIA += dt;
        
        // Cada 5 segundos hace un intento de movimiento
        if (tiempoAcumuladoIA >= 5.0f) {
            tiempoAcumuladoIA = 0.0f;
            
            int intento = (std::rand() % 20) + 1;
            if (intento <= dificultad) {
                avanzarEnRuta();
            }
        }
    }

    void avanzarEnRuta() {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_04_SALAS;
                break;
            case TipoCamara::CAM_04_SALAS:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true; // Llegó al marco exterior de la puerta izquierda
                break;
            default:
                break;
        }
    }

    void resetear() {
        posicionActual = TipoCamara::CAM_01_DULCERIA;
        estaEnLaPuerta = false;
        estaAdentro = false;
        tiempoAcumuladoIA = 0.0f;
        tiempoEnPuerta = 0.0f;
    }

    // Getters
    std::string getNombre() const { return nombre; }
    TipoCamara getPosicionActual() const { return posicionActual; }
    bool esEnLaPuerta() const { return estaEnLaPuerta; }
    bool esEstaAdentro() const { return estaAdentro; }
};