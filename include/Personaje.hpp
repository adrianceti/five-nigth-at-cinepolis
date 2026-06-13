#pragma once
#include <string>
#include <cstdlib>
#include <algorithm>
#include "MonitorCamaras.hpp"

// Clase base para todos los personajes
class Personaje {
protected:
    std::string nombre;
    TipoCamara posicionActual;
    int dificultad;
    bool estaEnLaPuerta;
    bool estaAdentro;
    
    float tiempoAcumuladoIA;
    float tiempoEnPuerta;

public:
    Personaje(std::string nom, int dif) 
        : nombre(nom), posicionActual(TipoCamara::CAM_01_DULCERIA), 
          dificultad(dif), estaEnLaPuerta(false), estaAdentro(false),
          tiempoAcumuladoIA(0.0f), tiempoEnPuerta(0.0f) {}

    virtual ~Personaje() = default;

    virtual void actualizarIA(float dt, bool puertaCerrada, bool puedeMoverse = true,
                              float intervaloIntentos = 9.0f, int bonoDificultad = 0) {
        if (estaAdentro) return;

        if (!puedeMoverse) {
            return;
        }

        if (estaEnLaPuerta) {
            if (puertaCerrada) {
                resetear();
                return;
            }
            tiempoEnPuerta += dt;
            if (tiempoEnPuerta >= 3.5f) {
                estaAdentro = true;
                estaEnLaPuerta = false;
            }
            return;
        }

        tiempoAcumuladoIA += dt;
        if (tiempoAcumuladoIA >= intervaloIntentos) {
            tiempoAcumuladoIA = 0.0f;
            int intento = (std::rand() % 20) + 1;
            int dificultadEfectiva = std::clamp(dificultad + bonoDificultad, 1, 20);
            if (intento <= dificultadEfectiva) {
                avanzarEnRuta();
            }
        }
    }

    virtual void avanzarEnRuta() = 0; // Cada personaje tiene su propia ruta

    virtual void resetear() {
        posicionActual = TipoCamara::CAM_01_DULCERIA;
        estaEnLaPuerta = false;
        estaAdentro = false;
        tiempoAcumuladoIA = 0.0f;
        tiempoEnPuerta = 0.0f;
    }

    std::string getNombre() const { return nombre; }
    TipoCamara getPosicionActual() const { return posicionActual; }
    bool esEnLaPuerta() const { return estaEnLaPuerta; }
    bool esEstaAdentro() const { return estaAdentro; }
};

// --- GOBO: Llega por la Puerta Izquierda ---
class Gobo : public Personaje {
public:
    Gobo(int dif) : Personaje("Gobo", dif) {}

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_04_SALAS;
                break;
            case TipoCamara::CAM_04_SALAS:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true;
                break;
            default:
                break;
        }
    }
};

// --- DIRECTOR: Llega por la Puerta Izquierda (ruta diferente) ---
class Director : public Personaje {
public:
    Director(int dif) : Personaje("Director", dif) {}

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_03_PASILLO_B;
                break;
            case TipoCamara::CAM_03_PASILLO_B:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true;
                break;
            default:
                break;
        }
    }
};

// --- POPY: Llega por la Puerta Derecha ---
class Popy : public Personaje {
public:
    Popy(int dif) : Personaje("Popy", dif) {}

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_05_BANOS;
                break;
            case TipoCamara::CAM_05_BANOS:
                posicionActual = TipoCamara::CAM_03_PASILLO_B;
                break;
            case TipoCamara::CAM_03_PASILLO_B:
                // NOTA: Esta es la puerta derecha (requeriría agregar esEnLaPuertaDerecha)
                estaEnLaPuerta = true; // Por ahora usa puerta izquierda
                break;
            default:
                break;
        }
    }
};

// --- THE USHER: Llega por la Puerta Izquierda (ruta larga) ---
class TheUsher : public Personaje {
public:
    TheUsher(int dif) : Personaje("The Usher", dif) {}

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_04_SALAS;
                break;
            case TipoCamara::CAM_04_SALAS:
                posicionActual = TipoCamara::CAM_05_BANOS;
                break;
            case TipoCamara::CAM_05_BANOS:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true;
                break;
            default:
                break;
        }
    }
};

// --- TICKETY STUB: Llega por la Puerta Izquierda (ruta rápida) ---
class TicketyStub : public Personaje {
public:
    TicketyStub(int dif) : Personaje("Tickety Stub", dif) {}

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true;
                break;
            default:
                break;
        }
    }
};
