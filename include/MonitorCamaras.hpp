#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <random>
#include <cstdlib>
#include <map>
#include <vector>
#include <filesystem>

// Enumerado para las distintas zonas de Cinépolis
enum class TipoCamara {
    CAM_01_DULCERIA,
    CAM_02_PASILLO_A,
    CAM_03_PASILLO_B,
    CAM_04_SALAS,
    CAM_05_BANOS
};

class MonitorCamaras {
private:
    TipoCamara camaraActual;
    sf::RectangleShape cajaMapa;
    mutable std::mt19937 generadorAleatorio;
    mutable std::uniform_int_distribution<int> distribucionEstadistica;

    // Sistema de texturas y renderización
    std::map<TipoCamara, sf::Texture> texturasFondo;
    std::map<TipoCamara, sf::Color> colorFondoFallback;
    std::map<std::string, sf::Texture> texturasPersonajes;
    
    // Registro de personajes en cada cámara (nombre -> presente)
    std::map<std::string, bool> personajesPorCamara;
    std::map<std::string, sf::Sprite> spritesPersonajes;

    bool cargarTextureDesdeRutas(sf::Texture& textura, const std::vector<std::string>& rutas) {
        for (const auto& ruta : rutas) {
            if (textura.loadFromFile(ruta)) {
                return true;
            }
        }
        return false;
    }

    void generarEstatica() const {
        // Genera estática visual en la terminal (simulación de ruido de cámara)
        for (int y = 0; y < 12; y++) {
            std::string fila;
            for (int x = 0; x < 60; x++) {
                int caracter = distribucionEstadistica(generadorAleatorio);
                if (caracter < 30) fila += " ";
                else if (caracter < 60) fila += ".";
                else if (caracter < 80) fila += ":";
                else if (caracter < 95) fila += "#";
                else fila += "@";
            }
            std::cout << "║ " << fila << " ║\n";
        }
    }

    void cargarTexturasFondo() {
        // Intentar cargar fondos de cámaras desde assets/textures/camaras/
        std::vector<std::pair<TipoCamara, std::string>> camaras = {
            {TipoCamara::CAM_01_DULCERIA, "dulceria.png"},
            {TipoCamara::CAM_02_PASILLO_A, "pasillo_a.png"},
            {TipoCamara::CAM_03_PASILLO_B, "pasillo_b.png"},
            {TipoCamara::CAM_04_SALAS, "sala.png"},
            {TipoCamara::CAM_05_BANOS, "banos.png"}
        };
        
        for (const auto& camara : camaras) {
            sf::Texture textura;
            std::vector<std::string> rutas = {
                "assets/textures/camaras/" + camara.second,
                "../assets/textures/camaras/" + camara.second
            };
            
            if (cargarTextureDesdeRutas(textura, rutas)) {
                texturasFondo[camara.first] = textura;
                std::cerr << "✓ Cargado fondo de cámara desde " << camara.second << std::endl;
            } else {
                std::cerr << "⚠ No se encontró " << camara.second << " - usando color de fallback" << std::endl;
            }
        }
        
        // Configurar colores de fallback para cada cámara
        colorFondoFallback[TipoCamara::CAM_01_DULCERIA] = sf::Color(60, 80, 40);    // Verde oscuro (dulcería)
        colorFondoFallback[TipoCamara::CAM_02_PASILLO_A] = sf::Color(50, 60, 70);   // Gris azulado (pasillo izq)
        colorFondoFallback[TipoCamara::CAM_03_PASILLO_B] = sf::Color(50, 60, 70);   // Gris azulado (pasillo der)
        colorFondoFallback[TipoCamara::CAM_04_SALAS] = sf::Color(70, 50, 50);       // Marrón rojizo (sala)
        colorFondoFallback[TipoCamara::CAM_05_BANOS] = sf::Color(60, 70, 80);       // Gris frío (baños)
    }

    void cargarTexturasPersonajes() {
        // Cargar sprites de todos los personajes
        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
        std::vector<std::string> carpetas = {"gobo", "director", "popy", "theusher", "ticketystub"};
        
        for (size_t i = 0; i < personajes.size(); i++) {
            std::string rutaBase = "assets/textures/personajes/" + carpetas[i] + "/sprite" + carpetas[i] + "/";
            // Intentar cargar PNG
            std::string rutaImagen = rutaBase + "*.png";
            
            // Construir ruta corregida (buscando el archivo específico)
            sf::Texture textura;
            bool cargada = false;
            
            // Intentar múltiples variantes de nombre de archivo
            std::vector<std::string> nombresPosibles = {
                "sprite" + carpetas[i] + ".png",
                personajes[i] + ".png",
                "sprite.png",
                carpetas[i] + ".png"
            };
            
            for (const auto& nombre : nombresPosibles) {
                std::vector<std::string> rutas = {
                    rutaBase + nombre,
                    "../" + rutaBase + nombre
                };
                if (cargarTextureDesdeRutas(textura, rutas)) {
                    cargada = true;
                    std::cerr << "✓ Cargada textura de " << personajes[i] << " desde " << nombre << std::endl;
                    break;
                }
            }
            
            if (cargada) {
                texturasPersonajes.insert({personajes[i], textura});
                sf::Sprite sprite(textura);
                sprite.setScale({0.3f, 0.3f}); // Escalar a tamaño visible en monitor
                spritesPersonajes.insert({personajes[i], sprite});
            } else {
                std::cerr << "⚠ No se encontró textura para " << personajes[i] << std::endl;
            }
        }
    }

public:

    MonitorCamaras() 
        : camaraActual(TipoCamara::CAM_01_DULCERIA),
          generadorAleatorio(std::random_device{}()),
          distribucionEstadistica(0, 100) {
        
        // Creamos un rectángulo con marco verde retro
        cajaMapa.setSize(sf::Vector2f(380.f, 520.f));
        cajaMapa.setFillColor(sf::Color(10, 40, 10, 230)); // Verde oscuro semi-transparente
        cajaMapa.setOutlineColor(sf::Color(0, 255, 0));
        cajaMapa.setOutlineThickness(3.f);
        cajaMapa.setPosition(sf::Vector2f(850.f, 90.f)); // Esquina derecha de la pantalla
        
        // Cargar texturas y fondos
        cargarTexturasFondo();
        cargarTexturasPersonajes();
        
        // Inicializar registro de personajes
        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
        for (const auto& p : personajes) {
            personajesPorCamara[p] = false;
        }
    }

    void cambiarCamara(TipoCamara nuevaCamara) {
        camaraActual = nuevaCamara;
    }

    TipoCamara getCamaraActual() const {
        return camaraActual;
    }

    // Registrar presencia de personaje en cámara
    void registrarPersonajeEnCamara(const std::string& nombre, bool presente) {
        if (personajesPorCamara.find(nombre) != personajesPorCamara.end()) {
            personajesPorCamara[nombre] = presente;
        }
    }

    // Consultar si un personaje está en la cámara actual
    bool hayPersonajesEnCamara() const {
        return false; // Por ahora, será actualizado desde Motor
    }

    // Devuelve el nombre de la cámara como texto para nuestra terminal
    std::string getNombreCamaraActual() const {
        switch (camaraActual) {
            case TipoCamara::CAM_01_DULCERIA:  return "CAM_01 - Dulceria Principal";
            case TipoCamara::CAM_02_PASILLO_A: return "CAM_02 - Pasillo Izquierdo";
            case TipoCamara::CAM_03_PASILLO_B: return "CAM_03 - Pasillo Derecho";
            case TipoCamara::CAM_04_SALAS:     return "CAM_04 - Acceso a Salas";
            case TipoCamara::CAM_05_BANOS:     return "CAM_05 - Zona de Banos";
        }
        return "Desconocida";
    }

    // Dibuja la interfaz del monitor sobre la ventana
    void renderizar(sf::RenderWindow& ventana) {
        ventana.draw(cajaMapa);
        
        // Dibujar fondo de la cámara (textura o color)
        if (texturasFondo.find(camaraActual) != texturasFondo.end()) {
            // Si existe textura, dibujarla
            sf::Sprite fondoSprite(texturasFondo.at(camaraActual));
            fondoSprite.setPosition(sf::Vector2f(870.f, 110.f));
            // Escalar para que quepa en el monitor (360x480)
            float escalaX = 360.0f / texturasFondo.at(camaraActual).getSize().x;
            float escalaY = 480.0f / texturasFondo.at(camaraActual).getSize().y;
            fondoSprite.setScale({escalaX, escalaY});
            ventana.draw(fondoSprite);
        } else {
            // Si no existe textura, usar color de fallback
            sf::Color colorFondo = colorFondoFallback.count(camaraActual) > 0 
                ? colorFondoFallback.at(camaraActual)
                : sf::Color(40, 60, 30);
            
            sf::RectangleShape fondoMonitor(sf::Vector2f(360.f, 480.f));
            fondoMonitor.setFillColor(colorFondo);
            fondoMonitor.setPosition(sf::Vector2f(870.f, 110.f));
            ventana.draw(fondoMonitor);
        }
        
        // Dibuja líneas horizontales (efecto de scanlines)
        for (float y = 110.0f; y < 600.0f; y += 20.0f) {
            sf::RectangleShape linea({360.0f, 1.0f});
            linea.setPosition({860.0f, y});
            linea.setFillColor(sf::Color(0, 100, 0));
            ventana.draw(linea);
        }
    }

private:
    sf::Color obtenerColorFondo() const {
        // Esta función ya no es necesaria pero la mantenemos por compatibilidad
        if (colorFondoFallback.count(camaraActual) > 0) {
            return colorFondoFallback.at(camaraActual);
        }
        return sf::Color(20, 40, 20);
    }

    void dibujarPersonajesEnMonitor(sf::RenderWindow& ventana) {
        // Este método será llamado desde Motor pasando los personajes activos
        // Por ahora solo dibuja si hay personajes detectados en la cámara
    }

public:
    // Dibuja un personaje específico en el monitor en la posición central
    void dibujarPersonaje(sf::RenderWindow& ventana, const std::string& nombre) {
        if (spritesPersonajes.find(nombre) != spritesPersonajes.end()) {
            sf::Sprite sprite = spritesPersonajes.at(nombre);
            // Posicionar en centro del monitor
            sprite.setPosition(sf::Vector2f(920.f, 280.f));
            ventana.draw(sprite);
        }
    }

    // Muestra la estática y la información en la terminal de Windows
    void mostrarEnTerminal() const {
        #ifdef _WIN32
            std::system("cls");
        #else
            std::system("clear");
        #endif

        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║          SISTEMA DE VIGILANCIA POR CAMARA - CINEPOLIS          ║\n";
        std::cout << "║                                                                ║\n";
        std::cout << "║  >>> TRANSMISION ACTIVA: " << getNombreCamaraActual() << "\n";
        std::cout << "║                                                                ║\n";
        std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
        
        // Genera estática
        generarEstatica();
        
        std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ CANALES DISPONIBLES:                                           ║\n";
        std::cout << "║ [1] DULCERIA    [2] PASILLO A   [3] PASILLO B                  ║\n";
        std::cout << "║ [4] SALAS       [5] BANOS                                       ║\n";
        std::cout << "║                                                                ║\n";
        std::cout << "║ [ESPACIO] Cerrar Monitor - Volver a Oficina                    ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    }
};
