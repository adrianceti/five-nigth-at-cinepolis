#pragma once

class Guardia {
private:
    float energia;        
    int nivelConsumo;       
    
    bool puertaIzquierda;   
    bool puertaDerecha;     
    bool monitorAbierto;    
    bool luzIzquierda;      
    bool luzDerecha;        

public:
    Guardia() 
        : energia(100.0f), nivelConsumo(1), 
          puertaIzquierda(false), puertaDerecha(false), monitorAbierto(false),
          luzIzquierda(false), luzDerecha(false) {}

    
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
            
            energia -= (nivelConsumo * 0.3f) * tiempoTranscurrido;
            if (energia < 0.0f) energia = 0.0f;
        }
    }
    float getEnergia() const { return energia; }
    int getNivelConsumo() const { return nivelConsumo; }
    bool esPuertaIzquierdaCerrada() const { return puertaIzquierda; }
    bool esPuertaDerechaCerrada() const { return puertaDerecha; }
    bool esMonitorAbierto() const { return monitorAbierto; }
    bool esLuzIzquierdaEncendida() const { return luzIzquierda; }
    bool esLuzDerechaEncendida() const { return luzDerecha; }

private:
   
    void actualizarConsumo() {
        nivelConsumo = 1; 
        if (puertaIzquierda) nivelConsumo++;
        if (puertaDerecha) nivelConsumo++;
        if (monitorAbierto) nivelConsumo++;
        if (luzIzquierda) nivelConsumo++;
        if (luzDerecha) nivelConsumo++;
    }
};
