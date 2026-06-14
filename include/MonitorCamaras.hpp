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
    sf::Texture texturaMapaCamaras;
    bool mapaCamarasCargado;
    
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

    bool cargarMapaCamaras() {
        return cargarTextureDesdeRutas(texturaMapaCamaras, {
            "assets/textures/monitor/mapa_camaras.png",
            "../assets/textures/monitor/mapa_camaras.png"
        });
    }

    // Devuelve el bounding box (rectángulo) de los pixeles no transparentes en la textura
    sf::IntRect obtenerBoundingBoxAlpha(const sf::Texture& textura) const {
        sf::Image img = textura.copyToImage();
        unsigned int w = img.getSize().x;
        unsigned int h = img.getSize().y;
        int minX = static_cast<int>(w), minY = static_cast<int>(h), maxX = -1, maxY = -1;

        for (unsigned int y = 0; y < h; ++y) {
            for (unsigned int x = 0; x < w; ++x) {
                if (img.getPixel(sf::Vector2u(x, y)).a > 8) {
                    if (static_cast<int>(x) < minX) minX = x;
                    if (static_cast<int>(y) < minY) minY = y;
                    if (static_cast<int>(x) > maxX) maxX = x;
                    if (static_cast<int>(y) > maxY) maxY = y;
                }
            }
        }

        if (maxX < 0 || maxY < 0) {
            return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(w), static_cast<int>(h)));
        }

        return sf::IntRect(sf::Vector2i(minX, minY), sf::Vector2i(maxX - minX + 1, maxY - minY + 1));
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

    bool personajePermitido(const std::string& nombre) const {
        static const std::vector<std::string> permitidos = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
        return std::find(permitidos.begin(), permitidos.end(), nombre) != permitidos.end();
    }

    void dibujarMiniaturaPersonaje(sf::RenderWindow& ventana, const std::string& nombre, sf::Vector2f centro) const {
        if (!personajePermitido(nombre)) return;
        // En Dulcería no se muestran miniaturas: sólo el fondo debe verse
        if (camaraActual == TipoCamara::CAM_01_DULCERIA) return;
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
        // Usar bounding box para ignorar márgenes transparentes al calcular la escala y el origen
        sf::IntRect bbox = obtenerBoundingBoxAlpha(*texturaParaDibujar);
        float escalaX = 86.f / static_cast<float>(bbox.size.x);
        float escalaY = 86.f / static_cast<float>(bbox.size.y);
        float escala = std::min(escalaX, escalaY);

        sprite.setScale({escala, escala});
        sprite.setOrigin({static_cast<float>(bbox.position.x) + bbox.size.x / 2.f, static_cast<float>(bbox.position.y + bbox.size.y) / 2.f});
        sprite.setPosition(centro + sf::Vector2f(0.f, -32.f));
        ventana.draw(sprite);
    }

    void obtenerPlanoPersonaje(const std::string& nombre, sf::Vector2f& posicion, sf::Vector2f& maximo) const {
        std::string clave = getClaveTexturaPersonaje(nombre);
        const float suelo = 720.f;
        posicion = {640.f, suelo};
        maximo = {330.f, 610.f};

        if (camaraActual == TipoCamara::CAM_01_DULCERIA) {
            if (clave == "Gobo") {
                posicion = {250.f, suelo};
                maximo = {280.f, 590.f};
            } else if (clave == "Director") {
                // Colocar detrás del panel del mapa de cámaras (zona superior derecha)
                posicion = {920.f, 460.f};
                maximo = {300.f, 420.f};
            } else if (clave == "Popy") {
                posicion = {785.f, suelo};
                maximo = {310.f, 610.f};
            } else if (clave == "TheUsher") {
                posicion = {1015.f, suelo};
                maximo = {300.f, 600.f};
            } else if (clave == "TicketyStub") {
                posicion = {1135.f, suelo};
                maximo = {260.f, 560.f};
            }
            return;
        }

        if (camaraActual == TipoCamara::CAM_02_PASILLO_A) {
            if (clave == "TicketyStub") {
                posicion = {410.f, suelo};
                maximo = {320.f, 610.f};
            } else if (clave == "Director") {
                posicion = {665.f, suelo};
                maximo = {340.f, 640.f};
            } else {
                posicion = {900.f, suelo};
                maximo = {320.f, 610.f};
            }
            return;
        }

        if (camaraActual == TipoCamara::CAM_03_PASILLO_B) {
            if (clave == "Popy") {
                posicion = {760.f, suelo};
                maximo = {350.f, 640.f};
            } else {
                posicion = {520.f, suelo};
                maximo = {320.f, 610.f};
            }
            return;
        }

        if (camaraActual == TipoCamara::CAM_04_SALAS) {
            if (clave == "Gobo") {
                posicion = {500.f, suelo};
                maximo = {330.f, 620.f};
            } else {
                posicion = {775.f, suelo};
                maximo = {330.f, 620.f};
            }
            return;
        }

        if (camaraActual == TipoCamara::CAM_05_BANOS) {
            if (clave == "Popy") {
                posicion = {760.f, suelo};
                maximo = {340.f, 630.f};
            } else {
                posicion = {520.f, suelo};
                maximo = {320.f, 610.f};
            }
        }
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
            // Primero, intentar cargar un archivo con nombre específico dentro de la carpeta
            bool cargada = false;
            std::vector<std::string> candidatos;
            for (const auto& carpeta : camara.second) {
                std::filesystem::path p(carpeta);
                std::string base = p.filename().string();
                if (!base.empty()) {
                    candidatos.push_back(carpeta + "/" + base + ".png");
                    candidatos.push_back(std::string("../") + carpeta + "/" + base + ".png");
                }
            }

            if (!candidatos.empty()) {
                if (cargarPrimeraImagenEnCarpeta(textura, candidatos)) {
                    cargada = true;
                }
            }

            // Si no se encontró un archivo con nombre específico, buscar el primer png en la carpeta
            if (!cargada) {
                if (cargarPrimeraImagenEnCarpeta(textura, camara.second)) {
                    cargada = true;
                }
            }

            if (cargada) {
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
          distribucionEstadistica(0, 100),
          mapaCamarasCargado(false) {
        
        // Creamos un rectángulo con marco verde retro
        cajaMapa.setSize(tamanoPantalla());
        cajaMapa.setFillColor(sf::Color(0, 0, 0, 0));
        cajaMapa.setOutlineColor(sf::Color(0, 255, 0));
        cajaMapa.setOutlineThickness(2.f);
        cajaMapa.setPosition(sf::Vector2f(0.f, 0.f));
        
        // Cargar texturas y fondos
        cargarTexturasFondo();
        cargarTexturasPersonajes();
        mapaCamarasCargado = cargarMapaCamaras();
        
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

            // Si estamos en Dulcería, dibujar al Director detrás de las palomitas
            if (camaraActual == TipoCamara::CAM_01_DULCERIA) {
                // Intentar obtener textura del Director
                const sf::Texture* texDirector = nullptr;
                auto it = texturasPersonajes.find(getClaveTexturaPersonaje("Director"));
                sf::Texture texTemp;
                if (it != texturasPersonajes.end()) {
                    texDirector = &it->second;
                } else {
                    // Priorizar la imagen exacta proporcionada por el usuario
                    std::vector<std::string> preferidas = {
                        "assets/textures/personajes/director/7fe9fc08-2881-4e84-92a4-0efea3ba737a_removalai_preview.png",
                        "../assets/textures/personajes/director/7fe9fc08-2881-4e84-92a4-0efea3ba737a_removalai_preview.png"
                    };
                    for (const auto& p : preferidas) {
                        if (texTemp.loadFromFile(p)) { texDirector = &texTemp; break; }
                    }

                    // Si no existe la preferida, buscar en carpeta de cámara primero
                    if (texDirector == nullptr) {
                        std::vector<std::string> posibles = {
                            "assets/textures/camaras/dulceria/director.png",
                            "assets/textures/camaras/dulceria/Director.png",
                            "../assets/textures/camaras/dulceria/director.png",
                            "../assets/textures/camaras/dulceria/Director.png"
                        };
                        for (const auto& p : posibles) {
                            if (texTemp.loadFromFile(p)) { texDirector = &texTemp; break; }
                        }
                    }
                    // Si aún no se encontró, intentar en carpeta de personaje
                    if (texDirector == nullptr) {
                        std::vector<std::string> rutas = {
                            "assets/textures/personajes/director",
                            "../assets/textures/personajes/director"
                        };
                        cargarPrimeraImagenEnCarpeta(texTemp, rutas);
                        if (texTemp.getSize().x > 0) texDirector = &texTemp;
                    }
                }

                if (texDirector != nullptr) {
                    // Calcular bounding box y escala según el plano para Dulcería
                    sf::IntRect bbox = obtenerBoundingBoxAlpha(*texDirector);
                    sf::Vector2f posicion, maximo;
                    obtenerPlanoPersonaje("Director", posicion, maximo);
                    float escalaXdir = maximo.x / static_cast<float>(bbox.size.x);
                    float escalaYdir = maximo.y / static_cast<float>(bbox.size.y);
                    float escalaDir = std::min(escalaXdir, escalaYdir);

                    sf::Sprite spriteDir(*texDirector);
                    spriteDir.setScale({escalaDir, escalaDir});
                    spriteDir.setOrigin({static_cast<float>(bbox.position.x) + bbox.size.x / 2.f,
                                         static_cast<float>(bbox.position.y + bbox.size.y)});

                    // Colocar los pies del Director al ras del suelo del monitor (sin movimiento)
                    // Como el origen está en la base del sprite, poner la posición Y en el fondo
                    spriteDir.setPosition(sf::Vector2f(posicion.x, pantalla.y));
                    ventana.draw(spriteDir);

                    // Volver a dibujar la región de las palomitas encima para que el Director quede atrás
                    // Definimos una región relativa (ajustable) que cubre la caja de palomitas en la imagen
                    const sf::Texture& texFondo = texturasFondo.at(camaraActual);
                    unsigned int w = texFondo.getSize().x;
                    unsigned int h = texFondo.getSize().y;
                    // Rectángulo aproximado en coordenadas de textura que cubre la máquina de popcorn (ampliado)
                    sf::IntRect rectPop(sf::Vector2i(static_cast<int>(w * 0.00f), static_cast<int>(h * 0.45f)),
                                        sf::Vector2i(static_cast<int>(w * 0.35f), static_cast<int>(h * 0.40f)));

                    // Crear sprite que use sólo esa sub-región y posicionarlo escalado
                    sf::Sprite overlay(texFondo);
                    overlay.setTextureRect(rectPop);
                    overlay.setPosition(sf::Vector2f(static_cast<float>(rectPop.position.x) * escalaX,
                                                     static_cast<float>(rectPop.position.y) * escalaY));
                    overlay.setScale({escalaX, escalaY});
                    ventana.draw(overlay);
                }
            }
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

        if (mapaCamarasCargado) {
            sf::RectangleShape panelMapa({392.f, 302.f});
            panelMapa.setPosition({870.f, 388.f});
            panelMapa.setFillColor(sf::Color(0, 0, 0, 170));
            panelMapa.setOutlineThickness(2.f);
            panelMapa.setOutlineColor(sf::Color(235, 235, 235, 180));
            ventana.draw(panelMapa);

            sf::Sprite spriteMapa(texturaMapaCamaras);
            const sf::Vector2u tamMapa = texturaMapaCamaras.getSize();
            float escalaX = 380.f / static_cast<float>(tamMapa.x);
            float escalaY = 290.f / static_cast<float>(tamMapa.y);
            float escala = std::min(escalaX, escalaY);
            spriteMapa.setScale({escala, escala});
            spriteMapa.setPosition({876.f, 394.f});
            spriteMapa.setColor(sf::Color(255, 255, 255, 240));
            ventana.draw(spriteMapa);
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

    void dibujarPersonajesEnMonitor() {
        // Este método será llamado desde Motor pasando los personajes activos
        // Por ahora solo dibuja si hay personajes detectados en la cámara
    }

public:
    // Dibuja un personaje específico en el monitor en la posición central
    void dibujarPersonaje(sf::RenderWindow& ventana, const std::string& nombre) {
        if (!personajePermitido(nombre)) return;
        // En Dulcería no se deben dibujar personajes (solo el fondo)
        if (camaraActual == TipoCamara::CAM_01_DULCERIA) return;
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

        // Si no existe textura en `texturasPersonajes`, intentar cargar una imagen específica
        if (texturaParaDibujar == nullptr) {
            std::string carpeta = getCarpetaPersonaje(nombre);
            if (!carpeta.empty()) {
                // Primero intentar carpeta específica de la cámara (dulcería)
                if (camaraActual == TipoCamara::CAM_01_DULCERIA) {
                    std::vector<std::string> posibles = {
                        "assets/textures/camaras/dulceria/" + carpeta + ".png",
                        "assets/textures/camaras/dulceria/" + nombre + ".png",
                        "../assets/textures/camaras/dulceria/" + carpeta + ".png",
                        "../assets/textures/camaras/dulceria/" + nombre + ".png"
                    };
                    for (const auto& p : posibles) {
                        if (texturaTemporal.loadFromFile(p)) {
                            texturaParaDibujar = &texturaTemporal;
                            break;
                        }
                    }
                }

                // Si no se encontró en la carpeta de cámara, intentar en la carpeta de personaje
                if (texturaParaDibujar == nullptr) {
                    std::vector<std::string> rutas = {
                        "assets/textures/personajes/" + carpeta,
                        "../assets/textures/personajes/" + carpeta
                    };

                    if (cargarPrimeraImagenEnCarpeta(texturaTemporal, rutas)) {
                        texturaParaDibujar = &texturaTemporal;
                    }
                }
            }
        }

        if (texturaParaDibujar != nullptr) {
            sf::Sprite sprite(*texturaParaDibujar);
            sf::IntRect bbox = obtenerBoundingBoxAlpha(*texturaParaDibujar);
            sf::Vector2f posicion;
            sf::Vector2f maximo;
            obtenerPlanoPersonaje(nombre, posicion, maximo);
            float escalaX = maximo.x / static_cast<float>(bbox.size.x);
            float escalaY = maximo.y / static_cast<float>(bbox.size.y);
            float escala = std::min(escalaX, escalaY);

            sprite.setScale({escala, escala});
            sprite.setOrigin({static_cast<float>(bbox.position.x) + bbox.size.x / 2.f, static_cast<float>(bbox.position.y + bbox.size.y)});
            sprite.setPosition(posicion);
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
        if (camaraActual == TipoCamara::CAM_01_DULCERIA) return; // No mostrar rutas en dulcería
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
