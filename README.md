# five-nigth-at-cinepolis
```plantuml
@startuml
skinparam classAttributeIconSize 0

class Juego {
    - motor : Motor
    + run() : void
}

class Motor {
    - ventana : sf::RenderWindow
    - guardia : Guardia
    - camaras : Camara
    - animatronicos : List<Animatronico>
    - horaActual : int
    - nocheActual : int
    + actualizar() : void
    + renderizar() : void
    + procesarEventos() : void
}

class Guardia {
    - energia : float
    - oficinaPuertaIzquierda : bool
    - oficinaPuertaDerecha : bool
    - mirandoMonitor : bool
    + alternarPuertaIzquierda() : void
    + alternarPuertaDerecha() : void
    + consumirEnergia(cantidad : float) : void
}

class Camara {
    - salaActiva : int
    - salas : List<Sala>
    + cambiarSala(idSala : int) : void
    + obtenerTexturaActual() : sf::Texture
}

class Sala {
    - id : int
    - nombre : String
    - texturaBase : sf::Texture
    + agregarAnimatronico() : void
}

class Animatronico {
    - nombre : String
    - nivelAgresividad : int
    - salaActual : int
    - ticMovimiento : sf::Clock
    + mover() : void
    + calcularSiguientePaso() : void
}

Juego o--> Motor
Motor o--> Guardia
Motor o--> Camara
Motor o--> Animatronico
Camara o--> Sala
@endum