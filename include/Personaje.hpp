#pragma once

#include <algorithm>
#include <cstdlib>
#include <string>
#include "MonitorCamaras.hpp"

enum class EventoPuerta {
    Ninguno,
    Golpe,
    Entrada
};

class Personaje {
protected:
    std::string nombre;
    TipoCamara posicionActual;
    int dificultad;
    bool estaEnLaPuerta;
    bool estaAdentro;
    float tiempoEnPuerta;
    float tiempoBajoLuz;
    float tiempoMonitorAbierto;

public:
    Personaje(std::string nom, int dif)
        : nombre(std::move(nom)),
          posicionActual(TipoCamara::CAM_01_DULCERIA),
          dificultad(dif),
          estaEnLaPuerta(false),
          estaAdentro(false),
          tiempoEnPuerta(0.0f),
          tiempoBajoLuz(0.0f),
          tiempoMonitorAbierto(0.0f) {}

    virtual ~Personaje() = default;

    virtual EventoPuerta actualizarEstadoPuerta(float dt, bool puertaCerrada, bool monitorAbierto, bool luzEncendida = true) {
        if (estaAdentro || !estaEnLaPuerta) {
            return EventoPuerta::Ninguno;
        }

        if (luzEncendida) {
            tiempoBajoLuz += dt;
            if (tiempoBajoLuz >= 0.8f) {
                resetear();
                return EventoPuerta::Golpe;
            }
        } else {
            tiempoBajoLuz = 0.0f;
        }

        if (puertaCerrada) {
            if (monitorAbierto) {
                tiempoEnPuerta += dt;
                if (tiempoEnPuerta >= tiempoPermanenciaPuerta()) {
                    estaAdentro = true;
                    estaEnLaPuerta = false;
                    return EventoPuerta::Entrada;
                }
            } else {
                tiempoEnPuerta = 0.0f;
            }
            return EventoPuerta::Ninguno;
        }

        if (monitorAbierto) {
            tiempoMonitorAbierto += dt;
            if (tiempoMonitorAbierto >= 1.2f) {
                estaAdentro = true;
                estaEnLaPuerta = false;
                return EventoPuerta::Entrada;
            }
        } else {
            tiempoMonitorAbierto = 0.0f;
        }

        return EventoPuerta::Ninguno;
    }

    virtual bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false, int horaActual = 12) {
        (void)horaActual;
        if (estaAdentro || estaEnLaPuerta || camaraObservada) {
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
        tiempoBajoLuz = 0.0f;
        tiempoMonitorAbierto = 0.0f;
    }

    virtual float tiempoPermanenciaPuerta() const {
        return 1.2f;
    }

    std::string getNombre() const { return nombre; }
    TipoCamara getPosicionActual() const { return posicionActual; }
    bool esEnLaPuerta() const { return estaEnLaPuerta; }
    bool esEstaAdentro() const { return estaAdentro; }
};

class Gobo : public Personaje {
public:
    Gobo(int dif) : Personaje("Gobo", dif) {}

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false, int horaActual = 12) override {
        (void)horaActual;
        if (estaAdentro || estaEnLaPuerta || camaraObservada) {
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

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false, int horaActual = 12) override {
        (void)horaActual;
        if (estaAdentro || estaEnLaPuerta || camaraObservada) {
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
                posicionActual = TipoCamara::CAM_04_SALAS;
                break;
            case TipoCamara::CAM_04_SALAS:
                posicionActual = TipoCamara::CAM_03_PASILLO_B;
                break;
            case TipoCamara::CAM_03_PASILLO_B:
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
    Popy(int dif)
        : Personaje("Popy", dif),
          faseAgitacion(0) {
        posicionActual = TipoCamara::CAM_05_BANOS;
    }

    void resetear() override {
        posicionActual = TipoCamara::CAM_05_BANOS;
        estaEnLaPuerta = false;
        estaAdentro = false;
        tiempoEnPuerta = 0.0f;
        tiempoBajoLuz = 0.0f;
        tiempoMonitorAbierto = 0.0f;
        faseAgitacion = 0;
    }

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false, int horaActual = 12) override {
        (void)horaActual;
        if (estaAdentro || estaEnLaPuerta || camaraObservada) {
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

        int intento = (std::rand() % 20) + 1;
        int dificultadClampeada = std::clamp(dificultadEfectiva, 1, 20);
        if (intento <= dificultadClampeada) {
            if (faseAgitacion < 4) {
                faseAgitacion++;
            }

            if (faseAgitacion >= 4) {
                posicionActual = TipoCamara::CAM_02_PASILLO_A;
                estaEnLaPuerta = true;
                tiempoEnPuerta = 0.0f;
            }
            return true;
        }

        return false;
    }

    EventoPuerta actualizarEstadoPuerta(float dt, bool puertaCerrada, bool monitorAbierto, bool luzEncendida = true) override {
        (void)luzEncendida;
        if (estaAdentro || !estaEnLaPuerta) {
            return EventoPuerta::Ninguno;
        }

        if (puertaCerrada) {
            if (monitorAbierto) {
                tiempoEnPuerta += dt;
                if (tiempoEnPuerta >= 3.0f) {
                    estaAdentro = true;
                    estaEnLaPuerta = false;
                    return EventoPuerta::Entrada;
                }
            } else {
                tiempoEnPuerta = 0.0f;
            }
            return EventoPuerta::Ninguno;
        }

        if (luzEncendida) {
            tiempoBajoLuz += dt;
            if (tiempoBajoLuz >= 0.8f) {
                resetear();
                return EventoPuerta::Golpe;
            }
        } else {
            tiempoBajoLuz = 0.0f;
        }

        if (monitorAbierto) {
            tiempoEnPuerta += dt;
            if (tiempoEnPuerta >= 3.0f) {
                estaAdentro = true;
                estaEnLaPuerta = false;
                return EventoPuerta::Entrada;
            }
        } else {
            tiempoEnPuerta = 0.0f;
        }

        return EventoPuerta::Ninguno;
    }

    void avanzarEnRuta() override {
        posicionActual = TipoCamara::CAM_05_BANOS;
    }
};

class TheUsher : public Personaje {
public:
    TheUsher(int dif) : Personaje("The Usher", dif) {}

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false, int horaActual = 12) override {
        if (horaActual < 3) {
            return false;
        }

        return Personaje::procesarTickMovimiento(dificultadEfectiva, camaraObservada, horaActual);
    }

    EventoPuerta actualizarEstadoPuerta(float dt, bool puertaCerrada, bool monitorAbierto, bool luzEncendida = true) override {
        if (estaAdentro || !estaEnLaPuerta) {
            return EventoPuerta::Ninguno;
        }

        if (puertaCerrada) {
            if (monitorAbierto) {
                tiempoEnPuerta += dt;
                if (tiempoEnPuerta >= tiempoPermanenciaPuerta()) {
                    estaAdentro = true;
                    estaEnLaPuerta = false;
                    return EventoPuerta::Entrada;
                }
            } else {
                tiempoEnPuerta = 0.0f;
            }
            return EventoPuerta::Ninguno;
        }

        if (luzEncendida) {
            tiempoBajoLuz += dt;
            if (tiempoBajoLuz >= 0.8f) {
                resetear();
                return EventoPuerta::Golpe;
            }
        } else {
            tiempoBajoLuz = 0.0f;
        }

        if (monitorAbierto) {
            estaAdentro = true;
            estaEnLaPuerta = false;
            return EventoPuerta::Entrada;
        }

        return EventoPuerta::Ninguno;
    }

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = TipoCamara::CAM_04_SALAS;
                break;
            case TipoCamara::CAM_04_SALAS:
                posicionActual = TipoCamara::CAM_05_BANOS;
                break;
            case TipoCamara::CAM_05_BANOS:
                posicionActual = TipoCamara::CAM_03_PASILLO_B;
                break;
            case TipoCamara::CAM_03_PASILLO_B:
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

    bool procesarTickMovimiento(int dificultadEfectiva, bool camaraObservada = false, int horaActual = 12) override {
        (void)horaActual;
        if (estaAdentro || estaEnLaPuerta || camaraObservada) {
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

    EventoPuerta actualizarEstadoPuerta(float dt, bool puertaCerrada, bool monitorAbierto, bool luzEncendida = true) override {
        if (estaAdentro || !estaEnLaPuerta) {
            return EventoPuerta::Ninguno;
        }

        if (luzEncendida) {
            tiempoBajoLuz += dt;
            if (tiempoBajoLuz >= 0.7f) {
                posicionActual = TipoCamara::CAM_04_SALAS;
                estaEnLaPuerta = false;
                tiempoEnPuerta = 0.0f;
                tiempoBajoLuz = 0.0f;
                tiempoMonitorAbierto = 0.0f;
                return EventoPuerta::Golpe;
            }
        } else {
            tiempoBajoLuz = 0.0f;
        }

        if (puertaCerrada) {
            tiempoEnPuerta += dt;
            if (tiempoEnPuerta >= 1.8f) {
                estaAdentro = true;
                estaEnLaPuerta = false;
                return EventoPuerta::Entrada;
            }
            return EventoPuerta::Ninguno;
        }

        if (monitorAbierto) {
            tiempoMonitorAbierto += dt;
            if (tiempoMonitorAbierto >= 1.0f) {
                estaAdentro = true;
                estaEnLaPuerta = false;
                return EventoPuerta::Entrada;
            }
        } else {
            tiempoMonitorAbierto = 0.0f;
        }

        return EventoPuerta::Ninguno;
    }

    void avanzarEnRuta() override {
        switch (posicionActual) {
            case TipoCamara::CAM_01_DULCERIA:
                posicionActual = (std::rand() % 2 == 0) ? TipoCamara::CAM_02_PASILLO_A : TipoCamara::CAM_04_SALAS;
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
