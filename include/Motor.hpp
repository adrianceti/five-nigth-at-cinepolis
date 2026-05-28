#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

class Motor {
private:
    sf::RenderWindow ventana;

    sf::Texture texturaOficina;
    std::optional<sf::Sprite> spriteOficina;

    void procesarEventos();
    void actualizar();
    void renderizar();

public:
    Motor();
    ~Motor();
    void ejecutarCiclo();
    bool estaAbierto() const;
};