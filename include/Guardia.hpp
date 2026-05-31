#pragma once

class Guardia {
private:
    float energia;          // Rango de 100.0f a 0.0f
    int nivelConsumo;       // Cuánta energía gasta (mínimo 1 por la oficina encendida)
    
    bool puertaIzquierda;   // true = cerrada, false = abierta
    bool puertaDerecha;     // true = cerrada, false = abierta
    bool monitorAbierto;    // true = viendo cámaras, false = en la oficina

public:
    Guardia() 
        : energia(100.0f), nivelConsumo(1), 
          puertaIzquierda(false), puertaDerecha(false), monitorAbierto(false) {}

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

    // Lógica por ciclo de juego
    void bajarEnergia(float tiempoTranscurrido) {
        if (energia > 0.0f) {
            // La velocidad de caída depende del nivel de consumo actual
            energia -= (nivelConsumo * 1.5f) * tiempoTranscurrido;
            if (energia < 0.0f) energia = 0.0f;
        }
    }

    // Consultores de estado (Getters)
    float getEnergia() const { return energia; }
    int getNivelConsumo() const { return nivelConsumo; }
    bool esPuertaIzquierdaCerrada() const { return puertaIzquierda; }
    bool esPuertaDerechaCerrada() const { return puertaDerecha; }
    bool esMonitorAbierto() const { return monitorAbierto; }

private:
    // Calcula cuánta energía se está drenando según los aparatos activos
    void actualizarConsumo() {
        nivelConsumo = 1; // Consumo base por tener la oficina prendida
        if (puertaIzquierda) nivelConsumo++;
        if (puertaDerecha) nivelConsumo++;
        if (monitorAbierto) nivelConsumo++;
    }
};