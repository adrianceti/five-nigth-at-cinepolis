#pragma once

#include <algorithm>
#include <cstdlib>
#include <string>
#include "MonitorCamaras.hpp"

class Personaje {
protected:
    std::string nombre;
    TipoCamara posicionActual;
    int dificultad;
    bool estaEnLaPuerta;
    bool estaAdentro;
    float tiempoEnPuerta;

public:
    Personaje(std::string nom, int dif)
        : nombre(std::move(nom)),
          posicionActual(TipoCamara::CAM_01_DULCERIA),
          dificultad(dif),
          estaEnLaPuerta(false),
          estaAdentro(false),
          tiempoEnPuerta(0.0f) {}

    virtual ~Personaje() = default;

    virtual void actualizarEstadoPuerta(float dt, bool puertaCerrada, bool luzEncendida = true) {
        (void)luzEncendida;
        if (estaAdentro) {
            return;
        }

        if (estaEnLaPuerta) {
            if (puertaCerrada) {
                resetear();
                return;
            }

            tiempoEnPuerta += dt;
            if (tiempoEnPuerta >= tiempoPermanenciaPuerta()) {
                estaAdentro = true;
                estaEnLaPuerta = false;
            }
        }
    }

    virtual bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false) {
        (void)camaraObservada;
        if (estaAdentro || estaEnLaPuerta) {
            return false;
        }

        int intento = (std::rand() % 20) + 1;
        int dificultadClampeada = std::clamp(dificultadEfectiva, 1, 20);
        if (intento <= dificultadClampeada) {
            avanzarEnRuta();
            return true;
        }

        return false;
    }

    virtual void avanzarEnRuta() = 0;

    virtual void resetear() {
        posicionActual = TipoCamara::CAM_01_DULCERIA;
        estaEnLaPuerta = false;
        estaAdentro = false;
        tiempoEnPuerta = 0.0f;
    }

    virtual float tiempoPermanenciaPuerta() const {
        return 3.5f;
    }

    std::string getNombre() const { return nombre; }
    TipoCamara getPosicionActual() const { return posicionActual; }
    bool esEnLaPuerta() const { return estaEnLaPuerta; }
    bool esEstaAdentro() const { return estaAdentro; }
};

class Gobo : public Personaje {
public:
    Gobo(int dif) : Personaje("Gobo", dif) {}

    void actualizarEstadoPuerta(float dt, bool puertaCerrada, bool luzEncendida = true) override {
        if (estaAdentro) {
            return;
        }

        if (estaEnLaPuerta) {
            if (puertaCerrada) {
                resetear();
                return;
            }

            if (!luzEncendida) {
                estaAdentro = true;
                estaEnLaPuerta = false;
                return;
            }

            tiempoEnPuerta += dt;
            if (tiempoEnPuerta >= tiempoPermanenciaPuerta()) {
                estaAdentro = true;
                estaEnLaPuerta = false;
            }
        }
    }

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false) override {
        (void)camaraObservada;
        if (estaAdentro || estaEnLaPuerta) {
            return false;
        }

        int intento = (std::rand() % 20) + 1;
        int dificultadClampeada = std::clamp(dificultadEfectiva + 2, 1, 20);
        if (intento <= dificultadClampeada) {
            avanzarEnRuta();
            return true;
        }

        return false;
    }

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = (std::rand() % 2 == 0) ? TipoCamara::CAM_04_SALAS : TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_04_SALAS:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true;
                tiempoEnPuerta = 0.0f;
                break;
            default:
                break;
        }
    }
};

class Director : public Personaje {
public:
    Director(int dif) : Personaje("Director", dif) {}

    float tiempoPermanenciaPuerta() const override {
        return 5.0f;
    }

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false) override {
        (void)camaraObservada;
        if (estaAdentro || estaEnLaPuerta) {
            return false;
        }

        int intento = (std::rand() % 20) + 1;
        int dificultadClampeada = std::clamp(dificultadEfectiva + 1, 1, 20);
        if (intento <= dificultadClampeada) {
            avanzarEnRuta();
            return true;
        }

        return false;
    }

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_03_PASILLO_B;
                break;
            case TipoCamara::CAM_03_PASILLO_B:
                posicionActual = TipoCamara::CAM_05_BANOS;
                break;
            case TipoCamara::CAM_05_BANOS:
                estaEnLaPuerta = true;
                tiempoEnPuerta = 0.0f;
                break;
            default:
                break;
        }
    }
};

class Popy : public Personaje {
private:
    int faseAgitacion;

public:
    Popy(int dif) : Personaje("Popy", dif), faseAgitacion(0) {}

    void resetear() override {
        Personaje::resetear();
        faseAgitacion = 0;
    }

    float tiempoPermanenciaPuerta() const override {
        return 1.9f;
    }

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false) override {
        (void)camaraObservada;
        if (estaAdentro || estaEnLaPuerta) {
            return false;
        }

        if (posicionActual != TipoCamara::CAM_05_BANOS) {
            int intentoEntrada = (std::rand() % 20) + 1;
            int dificultadEntrada = std::clamp(dificultadEfectiva, 1, 20);
            if (intentoEntrada <= dificultadEntrada) {
                posicionActual = TipoCamara::CAM_05_BANOS;
                return true;
            }
            return false;
        }

        if (camaraObservada) {
            faseAgitacion = std::max(0, faseAgitacion - 1);
            return false;
        }

        int intento = (std::rand() % 20) + 1;
        int dificultadClampeada = std::clamp(dificultadEfectiva, 1, 20);
        if (intento <= dificultadClampeada) {
            if (faseAgitacion < 4) {
                faseAgitacion++;
            }

            if (faseAgitacion >= 4) {
                estaEnLaPuerta = true;
                tiempoEnPuerta = 0.0f;
            }
            return true;
        }

        return false;
    }

    void avanzarEnRuta() override {
        posicionActual = TipoCamara::CAM_05_BANOS;
    }
};

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
                tiempoEnPuerta = 0.0f;
                break;
            default:
                break;
        }
    }
};

class TicketyStub : public Personaje {
public:
    TicketyStub(int dif) : Personaje("Tickety Stub", dif) {}

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false) override {
        (void)camaraObservada;
        if (estaAdentro || estaEnLaPuerta) {
            return false;
        }

        int intento = (std::rand() % 20) + 1;
        int dificultadClampeada = std::clamp(dificultadEfectiva + 3, 1, 20);
        if (intento <= dificultadClampeada) {
            avanzarEnRuta();
            return true;
        }

        return false;
    }

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                break;
            case TipoCamara::CAM_02_PASILLO_A:
                estaEnLaPuerta = true;
                tiempoEnPuerta = 0.0f;
                break;
            default:
                break;
        }
    }
};
