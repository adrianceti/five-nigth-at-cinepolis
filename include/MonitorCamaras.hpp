#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <random>
#include <cstdlib>
#include <map>
#include <vector>
#include <filesystem>
#include <cctype>
#include <algorithm>

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

    bool esImagenSoportada(const std::filesystem::path& ruta) const {
        if (!ruta.has_extension()) {
            return false;
        }

        std::string extension = ruta.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        return extension == ".png" ||
               extension == ".jpg" ||
               extension == ".jpeg" ||
               extension == ".bmp";
    }

    bool cargarPrimeraImagenEnCarpeta(sf::Texture& textura, const std::vector<std::string>& carpetas) const {
        for (const auto& carpeta : carpetas) {
            std::filesystem::path rutaCarpeta(carpeta);
            if (!std::filesystem::exists(rutaCarpeta) || !std::filesystem::is_directory(rutaCarpeta)) {
                continue;
            }

            for (const auto& entrada : std::filesystem::recursive_directory_iterator(rutaCarpeta)) {
                if (!entrada.is_regular_file() || !esImagenSoportada(entrada.path())) {
                    continue;
                }

                if (textura.loadFromFile(entrada.path().u8string())) {
                    return true;
                }
            }
        }

        return false;
    }

    sf::Vector2f tamanoPantalla() const {
        return sf::Vector2f(1280.f, 720.f);
    }

    sf::Vector2f centroPantalla() const {
        return sf::Vector2f(640.f, 360.f);
    }

    bool rutaIncluyeCamara(const std::vector<TipoCamara>& ruta, TipoCamara camara) const {
        return std::find(ruta.begin(), ruta.end(), camara) != ruta.end();
    }

    std::string getNombreCamaraCorto(TipoCamara camara) const {
        switch (camara) {
            case TipoCamara::CAM_01_DULCERIA:  return "DUL";
            case TipoCamara::CAM_02_PASILLO_A: return "PAS A";
            case TipoCamara::CAM_03_PASILLO_B: return "PAS B";
            case TipoCamara::CAM_04_SALAS:     return "SALAS";
            case TipoCamara::CAM_05_BANOS:     return "BANOS";
        }
        return "?";
    }

    std::string getClaveTexturaPersonaje(const std::string& nombre) const {
        if (nombre == "The Usher") {
            return "TheUsher";
        }
        if (nombre == "Tickety Stub") {
            return "TicketyStub";
        }
        return nombre;
    }

    std::string getCarpetaPersonaje(const std::string& nombre) const {
        std::string clave = getClaveTexturaPersonaje(nombre);
        if (clave == "Gobo") return "gobo";
        if (clave == "Director") return "director";
        if (clave == "Popy") return "popy";
        if (clave == "TheUsher") return "theusher";
        if (clave == "TicketyStub") return "ticketystub";
        return "";
    }

    void dibujarMiniaturaPersonaje(sf::RenderWindow& ventana, const std::string& nombre, sf::Vector2f centro) const {
        auto textura = texturasPersonajes.find(getClaveTexturaPersonaje(nombre));
        sf::Texture texturaTemporal;
        const sf::Texture* texturaParaDibujar = nullptr;

        if (textura != texturasPersonajes.end()) {
            texturaParaDibujar = &textura->second;
        } else {
            std::string carpeta = getCarpetaPersonaje(nombre);
            if (!carpeta.empty()) {
                std::vector<std::string> rutas = {
                    "assets/textures/personajes/" + carpeta,
                    "../assets/textures/personajes/" + carpeta
                };

                if (cargarPrimeraImagenEnCarpeta(texturaTemporal, rutas)) {
                    texturaParaDibujar = &texturaTemporal;
                }
            }
        }

        if (texturaParaDibujar == nullptr) {
            return;
        }

        sf::Sprite sprite(*texturaParaDibujar);
        const auto tam = texturaParaDibujar->getSize();
        float escalaX = 86.f / static_cast<float>(tam.x);
        float escalaY = 86.f / static_cast<float>(tam.y);
        float escala = std::min(escalaX, escalaY);

        sprite.setScale({escala, escala});
        sprite.setOrigin({static_cast<float>(tam.x) / 2.f, static_cast<float>(tam.y) / 2.f});
        sprite.setPosition(centro + sf::Vector2f(0.f, -32.f));
        ventana.draw(sprite);
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
        std::vector<std::pair<TipoCamara, std::vector<std::string>>> camaras = {
            {TipoCamara::CAM_01_DULCERIA, {"assets/textures/camaras/dulceria", "../assets/textures/camaras/dulceria"}},
            {TipoCamara::CAM_02_PASILLO_A, {"assets/textures/camaras/pasillo-a", "../assets/textures/camaras/pasillo-a", "assets/textures/camaras/pasillo_a", "../assets/textures/camaras/pasillo_a"}},
            {TipoCamara::CAM_03_PASILLO_B, {"assets/textures/camaras/pasillo-b", "../assets/textures/camaras/pasillo-b", "assets/textures/camaras/pasillo_b", "../assets/textures/camaras/pasillo_b"}},
            {TipoCamara::CAM_04_SALAS, {"assets/textures/camaras/sala1", "../assets/textures/camaras/sala1", "assets/textures/camaras/sala2", "../assets/textures/camaras/sala2"}},
            {TipoCamara::CAM_05_BANOS, {"assets/textures/camaras/baños", "../assets/textures/camaras/baños", "assets/textures/camaras/banos", "../assets/textures/camaras/banos", "assets/textures/camaras/lobby", "../assets/textures/camaras/lobby"}}
        };
        
        for (const auto& camara : camaras) {
            sf::Texture textura;
            
            if (cargarPrimeraImagenEnCarpeta(textura, camara.second)) {
                texturasFondo[camara.first] = textura;
                std::cerr << "✓ Cargado fondo de cámara" << std::endl;
            } else {
                std::cerr << "⚠ No se encontró fondo de cámara - usando color de fallback" << std::endl;
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
        cajaMapa.setSize(tamanoPantalla());
        cajaMapa.setFillColor(sf::Color(0, 0, 0, 0));
        cajaMapa.setOutlineColor(sf::Color(0, 255, 0));
        cajaMapa.setOutlineThickness(2.f);
        cajaMapa.setPosition(sf::Vector2f(0.f, 0.f));
        
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
        sf::Vector2f pantalla = tamanoPantalla();
        
        // Dibujar fondo de la cámara (textura o color)
        if (texturasFondo.find(camaraActual) != texturasFondo.end()) {
            // Si existe textura, dibujarla
            sf::Sprite fondoSprite(texturasFondo.at(camaraActual));
            fondoSprite.setPosition(sf::Vector2f(0.f, 0.f));
            // Escalar para que quepa en el monitor (360x480)
            float escalaX = pantalla.x / texturasFondo.at(camaraActual).getSize().x;
            float escalaY = pantalla.y / texturasFondo.at(camaraActual).getSize().y;
            fondoSprite.setScale({escalaX, escalaY});
            ventana.draw(fondoSprite);
        } else {
            // Si no existe textura, usar color de fallback
            sf::Color colorFondo = colorFondoFallback.count(camaraActual) > 0 
                ? colorFondoFallback.at(camaraActual)
                : sf::Color(40, 60, 30);
            
            sf::RectangleShape fondoMonitor(pantalla);
            fondoMonitor.setFillColor(colorFondo);
            fondoMonitor.setPosition(sf::Vector2f(0.f, 0.f));
            ventana.draw(fondoMonitor);
        }
        
        // Dibuja líneas horizontales (efecto de scanlines)
        for (float y = 0.0f; y < pantalla.y; y += 20.0f) {
            sf::RectangleShape linea({pantalla.x, 1.0f});
            linea.setPosition({0.0f, y});
            linea.setFillColor(sf::Color(0, 80, 0, 120));
            ventana.draw(linea);
        }

        ventana.draw(cajaMapa);
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
        auto textura = texturasPersonajes.find(nombre);
        if (textura != texturasPersonajes.end()) {
            sf::Sprite sprite(textura->second);
            const auto tam = textura->second.getSize();
            float escalaX = 420.f / static_cast<float>(tam.x);
            float escalaY = 560.f / static_cast<float>(tam.y);
            float escala = std::min(escalaX, escalaY);

            sprite.setScale({escala, escala});
            sprite.setOrigin({static_cast<float>(tam.x) / 2.f, static_cast<float>(tam.y) / 2.f});
            sprite.setPosition(centroPantalla() + sf::Vector2f(0.f, 60.f));
            ventana.draw(sprite);
        }
    }

    // Muestra la estática y la información en la terminal de Windows
    void dibujarRutaPersonaje(sf::RenderWindow& ventana,
                              const sf::Font& fuente,
                              const std::string& nombre,
                              const std::vector<TipoCamara>& ruta,
                              TipoCamara posicionActual,
                              bool enPuerta,
                              int fila) const {
        if (!rutaIncluyeCamara(ruta, camaraActual) && posicionActual != camaraActual) {
            return;
        }

        float baseX = 24.f;
        float baseY = 24.f + static_cast<float>(fila) * 58.f;
        float nodoAncho = 74.f;
        float nodoAlto = 26.f;
        float espacio = 12.f;

        sf::RectangleShape fondo({520.f, 46.f});
        fondo.setPosition({baseX - 10.f, baseY - 8.f});
        fondo.setFillColor(sf::Color(0, 0, 0, 150));
        fondo.setOutlineThickness(1.f);
        fondo.setOutlineColor(posicionActual == camaraActual ? sf::Color(255, 220, 80) : sf::Color(0, 180, 80));
        ventana.draw(fondo);

        sf::Text etiqueta(fuente, nombre + (enPuerta ? "  PUERTA" : ""), 15);
        etiqueta.setFillColor(posicionActual == camaraActual ? sf::Color(255, 230, 120) : sf::Color(200, 255, 210));
        etiqueta.setPosition({baseX, baseY - 4.f});
        ventana.draw(etiqueta);

        float rutaX = baseX + 145.f;
        float rutaY = baseY;
        for (size_t i = 0; i < ruta.size(); i++) {
            float x = rutaX + static_cast<float>(i) * (nodoAncho + espacio);
            bool esCamaraActual = ruta[i] == camaraActual;
            bool estaAqui = ruta[i] == posicionActual && !enPuerta;

            sf::RectangleShape nodo({nodoAncho, nodoAlto});
            nodo.setPosition({x, rutaY});
            nodo.setFillColor(estaAqui ? sf::Color(255, 220, 80, 220)
                                       : esCamaraActual ? sf::Color(30, 120, 70, 210)
                                                       : sf::Color(10, 35, 20, 210));
            nodo.setOutlineThickness(1.f);
            nodo.setOutlineColor(esCamaraActual ? sf::Color(255, 255, 255) : sf::Color(0, 170, 80));
            ventana.draw(nodo);

            sf::Text textoNodo(fuente, getNombreCamaraCorto(ruta[i]), 13);
            textoNodo.setFillColor(estaAqui ? sf::Color::Black : sf::Color(220, 255, 220));
            textoNodo.setPosition({x + 8.f, rutaY + 5.f});
            ventana.draw(textoNodo);

            if (estaAqui) {
                dibujarMiniaturaPersonaje(ventana, nombre, {x + nodoAncho / 2.f, rutaY});
            }

            if (i + 1 < ruta.size()) {
                sf::RectangleShape conector({espacio, 2.f});
                conector.setPosition({x + nodoAncho, rutaY + nodoAlto / 2.f});
                conector.setFillColor(sf::Color(0, 170, 80));
                ventana.draw(conector);
            }
        }
    }

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
