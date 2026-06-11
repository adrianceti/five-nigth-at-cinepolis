#pragma once

class Guardia {
private:
    float energia;          // Rango de 100.0f a 0.0f
    int nivelConsumo;       // Cuánta energía gasta (mínimo 1 por la oficina encendida)
    
    bool puertaIzquierda;   // true = cerrada, false = abierta
    bool puertaDerecha;     // true = cerrada, false = abierta
    bool monitorAbierto;    // true = viendo cámaras, false = en la oficina
    bool luzIzquierda;      // true = encendida, false = apagada (tecla Q)
    bool luzDerecha;        // true = encendida, false = apagada (tecla E)

public:
    Guardia() 
        : energia(100.0f), nivelConsumo(1), 
          puertaIzquierda(false), puertaDerecha(false), monitorAbierto(false),
          luzIzquierda(false), luzDerecha(false) {}

    // Métodos para cambiar los estados (Acciones del jugador)
    void alternarPuertaIzquierda() { 
        puertaIzquierda = !puertaIzquierda; 
        actualizarConsumo();
    }
    
    void alternarPuertaDerecha() { 
        puertaDerecha = !puertaDerecha; 
        actualizarConsumo();
    }
    
    void alternarMonitor() { 
        monitorAbierto = !monitorAbierto; 
        actualizarConsumo();
    }

    void alternarLuzIzquierda() {
        luzIzquierda = !luzIzquierda;
        actualizarConsumo();
    }

    void alternarLuzDerecha() {
        luzDerecha = !luzDerecha;
        actualizarConsumo();
    }

    void resetear() {
        energia = 100.0f;
        puertaIzquierda = false;
        puertaDerecha = false;
        monitorAbierto = false;
        luzIzquierda = false;
        luzDerecha = false;
        actualizarConsumo();
    }

    // Lógica por ciclo de juego
    void bajarEnergia(float tiempoTranscurrido) {
        if (energia > 0.0f) {
            // La velocidad de caída depende del nivel de consumo actual
            // Multiplicador: 0.3f permite que la energía dure para toda la noche
            // Consumo base (solo oficina): 1 * 0.3 * 120s = 36 puntos/hora
            // Con todas las cosas activadas: 5 * 0.3 * 120s = 180 puntos/hora (consume rápido si eres imprudente)
            energia -= (nivelConsumo * 0.3f) * tiempoTranscurrido;
            if (energia < 0.0f) energia = 0.0f;
        }
    }

    // Consultores de estado (Getters)
    float getEnergia() const { return energia; }
    int getNivelConsumo() const { return nivelConsumo; }
    bool esPuertaIzquierdaCerrada() const { return puertaIzquierda; }
    bool esPuertaDerechaCerrada() const { return puertaDerecha; }
    bool esMonitorAbierto() const { return monitorAbierto; }
    bool esLuzIzquierdaEncendida() const { return luzIzquierda; }
    bool esLuzDerechaEncendida() const { return luzDerecha; }

private:
    // Calcula cuánta energía se está drenando según los aparatos activos
    void actualizarConsumo() {
        nivelConsumo = 1; // Consumo base por tener la oficina prendida
        if (puertaIzquierda) nivelConsumo++;
        if (puertaDerecha) nivelConsumo++;
        if (monitorAbierto) nivelConsumo++;
        if (luzIzquierda) nivelConsumo++;
        if (luzDerecha) nivelConsumo++;
    }
};
