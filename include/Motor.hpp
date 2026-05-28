#pragma once

#include <SFML/Graphics.hpp>

class Motor {
private:
    sf::RenderWindow ventana;

    sf::Texture texturaOficina;
    sf::Sprite spriteOficina;

    void procesarEventos();
    void actualizar();
    void renderizar();

public:
    Motor();
    ~Motor();
    void ejecutarCiclo();
    bool estaAbierto() const;
};