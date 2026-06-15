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
#include <cmath>

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


    std::map<TipoCamara, sf::Texture> texturasFondo;
    std::map<TipoCamara, sf::Color> colorFondoFallback;
    std::map<std::string, sf::Texture> texturasPersonajes;
    sf::Texture texturaMapaCamaras;
    bool mapaCamarasCargado;
    sf::Font fuenteMapa;
    bool fuenteMapaCargada;
    sf::Clock relojAnimacionCamara;

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

    bool cargarFuenteDesdeRutas(sf::Font& fuente, const std::vector<std::string>& rutas) {
        for (const auto& ruta : rutas) {
            if (fuente.openFromFile(ruta)) {
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

    bool camaraUsaAnimacionHorizontal(TipoCamara camara) const {
        switch (camara) {
            case TipoCamara::CAM_01_DULCERIA:
            case TipoCamara::CAM_02_PASILLO_A:
            case TipoCamara::CAM_04_SALAS:
                return true;
            case TipoCamara::CAM_03_PASILLO_B:
            case TipoCamara::CAM_05_BANOS:
                return false;
        }
        return false;
    }

    float obtenerDesplazamientoHorizontal(TipoCamara camara) const {
        if (!camaraUsaAnimacionHorizontal(camara)) {
            return 0.0f;
        }

        float tiempo = relojAnimacionCamara.getElapsedTime().asSeconds();
        return std::sin(tiempo * 1.35f) * 42.0f;
    }

    void renderizarBaseCamara(sf::RenderWindow& ventana, TipoCamara camara, float offsetX) {
        sf::Vector2f pantalla = tamanoPantalla();

        if (texturasFondo.find(camara) != texturasFondo.end()) {
            const sf::Texture& texturaFondo = texturasFondo.at(camara);
            if (texturaFondo.getSize().x == 0 || texturaFondo.getSize().y == 0) {
                return;
            }
            sf::Sprite fondoSprite(texturaFondo);

            float margenHorizontal = camaraUsaAnimacionHorizontal(camara) ? 58.0f : 0.0f;
            float escalaX = (pantalla.x + margenHorizontal * 2.0f) / texturaFondo.getSize().x;
            float escalaY = pantalla.y / texturaFondo.getSize().y;
            fondoSprite.setPosition({offsetX - margenHorizontal, 0.f});
            fondoSprite.setScale({escalaX, escalaY});
            ventana.draw(fondoSprite);
        } else {
            sf::Color colorFondo = colorFondoFallback.count(camara) > 0
                ? colorFondoFallback.at(camara)
                : sf::Color(40, 60, 30);
            sf::RectangleShape fondoMonitor(pantalla);
            fondoMonitor.setFillColor(colorFondo);
            fondoMonitor.setPosition({offsetX, 0.f});
            ventana.draw(fondoMonitor);
        }

        for (float y = 0.0f; y < pantalla.y; y += 20.0f) {
            sf::RectangleShape linea({pantalla.x, 1.0f});
            linea.setPosition({0.0f, y});
            linea.setFillColor(sf::Color(0, 80, 0, 120));
            ventana.draw(linea);
        }
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

    void dibujarNavegacionCamaras(sf::RenderWindow& ventana) const {
        const sf::Vector2f tamanoPantalla = this->tamanoPantalla();
        const sf::Vector2f tamano(360.f, 304.f);
        const sf::Vector2f origen(tamanoPantalla.x - tamano.x - 16.f, 16.f);

        if (!mapaCamarasCargado) {
            return;
        }

        sf::Sprite mapa(texturaMapaCamaras);
        const sf::Vector2u tamTex = texturaMapaCamaras.getSize();
        if (tamTex.x == 0 || tamTex.y == 0) {
            return;
        }

        sf::IntRect recorte(sf::Vector2i(700, 90), sf::Vector2i(900, 760));
        float escala = std::min(
            tamano.x / static_cast<float>(recorte.size.x),
            tamano.y / static_cast<float>(recorte.size.y)
        );

        sf::Color base = obtenerColorFondo(camaraActual);
        sf::RectangleShape fondoMapa(tamano);
        fondoMapa.setPosition(origen);
        fondoMapa.setFillColor(sf::Color(
            static_cast<std::uint8_t>(base.r * 0.30f),
            static_cast<std::uint8_t>(base.g * 0.30f),
            static_cast<std::uint8_t>(base.b * 0.30f),
            225
        ));
        ventana.draw(fondoMapa);

        mapa.setTextureRect(recorte);
        mapa.setPosition(origen);
        mapa.setScale({escala, escala});
        mapa.setColor(sf::Color(255, 255, 255, 238));
        ventana.draw(mapa);

        sf::RectangleShape cubrirTextoSuperiorDerecha({86.f, 34.f});
        cubrirTextoSuperiorDerecha.setPosition(origen + sf::Vector2f(274.f, 0.f));
        cubrirTextoSuperiorDerecha.setFillColor(fondoMapa.getFillColor());
        ventana.draw(cubrirTextoSuperiorDerecha);

    }

    std::string getClaveTexturaPersonaje(const std::string& nombre) const {
        if (nombre == "The Usher") {
            return "TheUsher";
        }
        return nombre;
    }

    std::string getCarpetaPersonaje(const std::string& nombre) const {
        std::string clave = getClaveTexturaPersonaje(nombre);
        if (clave == "Gobo") return "gobo";
        if (clave == "Director") return "director";
        if (clave == "Popy") return "popy";
        if (clave == "TheUsher") return "theusher";
        return "";
    }

    bool personajePermitido(const std::string& nombre) const {
        static const std::vector<std::string> permitidos = {"Gobo", "Director", "Popy", "TheUsher"};
        return std::find(permitidos.begin(), permitidos.end(), nombre) != permitidos.end();
    }

    bool personajeVisibleEnCamara(TipoCamara camara, const std::string& nombre) const {
        if (camara == TipoCamara::CAM_01_DULCERIA) {
            return nombre == "Gobo" || nombre == "Director" || nombre == "TheUsher";
        }
        if (camara == TipoCamara::CAM_02_PASILLO_A) {
            return nombre == "Gobo" || nombre == "Popy";
        }
        if (camara == TipoCamara::CAM_03_PASILLO_B) {
            return nombre == "Director" || nombre == "TheUsher";
        }
        if (camara == TipoCamara::CAM_04_SALAS) {
            return nombre == "Gobo" || nombre == "Director" || nombre == "TheUsher";
        }
        if (camara == TipoCamara::CAM_05_BANOS) {
            return nombre == "Popy" || nombre == "TheUsher";
        }
        return false;
    }

    bool personajeVisibleEnCamara(const std::string& nombre) const {
        return personajeVisibleEnCamara(camaraActual, nombre);
    }

    void dibujarMiniaturaPersonaje(sf::RenderWindow& ventana, const std::string& nombre, sf::Vector2f centro) const {
        if (!personajePermitido(nombre)) return;
        if (!personajeVisibleEnCamara(nombre)) return;
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

        sf::IntRect bbox = obtenerBoundingBoxAlpha(*texturaParaDibujar);
        float escalaX = 80.f / static_cast<float>(bbox.size.x);
        float escalaY = 80.f / static_cast<float>(bbox.size.y);
        float escala = std::min(escalaX, escalaY);

        sprite.setScale({escala, escala});
        sprite.setOrigin({static_cast<float>(bbox.position.x) + bbox.size.x / 2.f, static_cast<float>(bbox.position.y + bbox.size.y) / 2.f});
        sprite.setPosition(centro + sf::Vector2f(0.f, -32.f));
        ventana.draw(sprite);
    }

    void obtenerPlanoPersonaje(TipoCamara camara, const std::string& nombre, sf::Vector2f& posicion, sf::Vector2f& maximo) const {
        std::string clave = getClaveTexturaPersonaje(nombre);
        const float suelo = 720.f;
        posicion = {640.f, suelo};
        maximo = {330.f, 610.f};

        if (camara == TipoCamara::CAM_01_DULCERIA) {
            if (clave == "Gobo") {
                posicion = {215.f, 680.f};
                maximo = {155.f, 315.f};
            } else if (clave == "Director") {
                posicion = {1030.f, 625.f};
                maximo = {102.f, 220.f};
            } else if (clave == "Popy") {
                posicion = {760.f, 655.f};
                maximo = {105.f, 210.f};
            } else if (clave == "TheUsher") {
                posicion = {880.f, 610.f};
                maximo = {98.f, 210.f};
            }
            return;
        }

        if (camara == TipoCamara::CAM_02_PASILLO_A) {
            if (clave == "Director") {
                posicion = {665.f, 670.f};
                maximo = {110.f, 210.f};
            } else {
                posicion = {900.f, 670.f};
                maximo = {125.f, 220.f};
            }
            return;
        }

        if (camara == TipoCamara::CAM_03_PASILLO_B) {
            if (clave == "Popy") {
                posicion = {760.f, 670.f};
                maximo = {140.f, 230.f};
            } else if (clave == "Director") {
                posicion = {880.f, 670.f};
                maximo = {112.f, 220.f};
            } else if (clave == "TheUsher") {
                posicion = {520.f, 645.f};
                maximo = {100.f, 200.f};
            } else {
                posicion = {520.f, 670.f};
                maximo = {120.f, 220.f};
            }
            return;
        }

        if (camara == TipoCamara::CAM_04_SALAS) {
            if (clave == "Gobo") {
                posicion = {500.f, 670.f};
                maximo = {130.f, 230.f};
            } else if (clave == "Director") {
                posicion = {780.f, 670.f};
                maximo = {110.f, 210.f};
            } else if (clave == "TheUsher") {
                posicion = {940.f, 640.f};
                maximo = {98.f, 190.f};
            } else {
                posicion = {775.f, 670.f};
                maximo = {120.f, 220.f};
            }
            return;
        }

        if (camara == TipoCamara::CAM_05_BANOS) {
            if (clave == "Popy") {
                posicion = {760.f, 675.f};
                maximo = {132.f, 235.f};
            } else if (clave == "TheUsher") {
                posicion = {520.f, 650.f};
                maximo = {100.f, 200.f};
            } else {
                posicion = {520.f, 675.f};
                maximo = {120.f, 220.f};
            }
        }
    }

    void generarEstatica() const {

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

        std::vector<std::pair<TipoCamara, std::vector<std::string>>> camaras = {
            {TipoCamara::CAM_01_DULCERIA, {"assets/textures/camaras/dulceria", "../assets/textures/camaras/dulceria"}},
            {TipoCamara::CAM_02_PASILLO_A, {"assets/textures/camaras/pasillo-a", "../assets/textures/camaras/pasillo-a", "assets/textures/camaras/pasillo_a", "../assets/textures/camaras/pasillo_a"}},
            {TipoCamara::CAM_03_PASILLO_B, {"assets/textures/camaras/pasillo-b", "../assets/textures/camaras/pasillo-b", "assets/textures/camaras/pasillo_b", "../assets/textures/camaras/pasillo_b"}},
            {TipoCamara::CAM_04_SALAS, {"assets/textures/camaras/sala1", "../assets/textures/camaras/sala1", "assets/textures/camaras/sala2", "../assets/textures/camaras/sala2"}},
            {TipoCamara::CAM_05_BANOS, {"assets/textures/camaras/baños", "../assets/textures/camaras/baños", "assets/textures/camaras/banos", "../assets/textures/camaras/banos", "assets/textures/camaras/lobby", "../assets/textures/camaras/lobby"}}
        };

        for (const auto& camara : camaras) {
            sf::Texture textura;

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


        colorFondoFallback[TipoCamara::CAM_01_DULCERIA] = sf::Color(60, 80, 40);
        colorFondoFallback[TipoCamara::CAM_02_PASILLO_A] = sf::Color(50, 60, 70);
        colorFondoFallback[TipoCamara::CAM_03_PASILLO_B] = sf::Color(50, 60, 70);
        colorFondoFallback[TipoCamara::CAM_04_SALAS] = sf::Color(70, 50, 50);
        colorFondoFallback[TipoCamara::CAM_05_BANOS] = sf::Color(60, 70, 80);
    }

    void cargarTexturasPersonajes() {

        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher"};
        std::vector<std::string> carpetas = {"gobo", "director", "popy", "theusher"};

        for (size_t i = 0; i < personajes.size(); i++) {
            std::string rutaPersonaje = "assets/textures/personajes/" + carpetas[i] + "/";
            std::string rutaSpriteAnterior = rutaPersonaje + "sprite" + carpetas[i] + "/";


            sf::Texture textura;
            bool cargada = false;


            std::vector<std::string> nombresPosibles = {
                carpetas[i] + ".png",
                "sprite.png",
                personajes[i] + ".png",
                "sprite" + carpetas[i] + ".png"
            };

            for (const auto& nombre : nombresPosibles) {
                std::vector<std::string> rutas = {
                    rutaPersonaje + nombre,
                    "../" + rutaPersonaje + nombre,
                    rutaSpriteAnterior + nombre,
                    "../" + rutaSpriteAnterior + nombre
                };
                if (cargarTextureDesdeRutas(textura, rutas)) {
                    cargada = true;
                    std::cerr << "✓ Cargada textura de " << personajes[i] << " desde " << nombre << std::endl;
                    break;
                }
            }

            if (cargada) {
                auto texturaInsertada = texturasPersonajes.insert_or_assign(personajes[i], textura);
                sf::Sprite sprite(texturaInsertada.first->second);
                sprite.setScale({0.3f, 0.3f});
                spritesPersonajes.insert_or_assign(personajes[i], sprite);
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
          mapaCamarasCargado(false),
          fuenteMapaCargada(false),
          relojAnimacionCamara() {


        cajaMapa.setSize(tamanoPantalla());
        cajaMapa.setFillColor(sf::Color(0, 0, 0, 0));
        cajaMapa.setOutlineColor(sf::Color(0, 255, 0));
        cajaMapa.setOutlineThickness(2.f);
        cajaMapa.setPosition(sf::Vector2f(0.f, 0.f));


        cargarTexturasFondo();
        cargarTexturasPersonajes();
        mapaCamarasCargado = cargarMapaCamaras();
        fuenteMapaCargada = cargarFuenteDesdeRutas(fuenteMapa, {
            "assets/fonts/arial.ttf",
            "../assets/fonts/arial.ttf",
            "assets/fonts/Roboto-Regular.ttf",
            "../assets/fonts/Roboto-Regular.ttf"
        });


        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher"};
        for (const auto& p : personajes) {
            personajesPorCamara[p] = false;
        }
    }

    void cambiarCamara(TipoCamara nuevaCamara) {
        if (camaraActual == nuevaCamara) {
            return;
        }

        camaraActual = nuevaCamara;
        relojAnimacionCamara.restart();
    }

    TipoCamara getCamaraActual() const {
        return camaraActual;
    }

    float getDesplazamientoHorizontalActual() const {
        return obtenerDesplazamientoHorizontal(camaraActual);
    }


    void registrarPersonajeEnCamara(const std::string& nombre, bool presente) {
        if (personajesPorCamara.find(nombre) != personajesPorCamara.end()) {
            personajesPorCamara[nombre] = presente;
        }
    }


    bool hayPersonajesEnCamara() const {
        return false;
    }


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


    void renderizar(sf::RenderWindow& ventana) {
        renderizarBaseCamara(ventana, camaraActual, getDesplazamientoHorizontalActual());

        dibujarNavegacionCamaras(ventana);
    }

private:
    sf::Color obtenerColorFondo(TipoCamara camara) const {

        if (colorFondoFallback.count(camara) > 0) {
            return colorFondoFallback.at(camara);
        }
        return sf::Color(20, 40, 20);
    }

    void dibujarPersonajesEnMonitor() {


    }

public:

    void dibujarPersonaje(sf::RenderWindow& ventana, const std::string& nombre) {
        dibujarPersonaje(ventana, nombre, camaraActual, 0.0f);
    }

    void dibujarPersonaje(sf::RenderWindow& ventana, const std::string& nombre, TipoCamara camara, float offsetX = 0.0f) {
        if (!personajePermitido(nombre)) return;
        if (!personajeVisibleEnCamara(camara, nombre)) return;
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
            std::string carpeta = getCarpetaPersonaje(nombre);
            if (!carpeta.empty()) {

                if (camara == TipoCamara::CAM_01_DULCERIA) {
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
            obtenerPlanoPersonaje(camara, nombre, posicion, maximo);
            maximo *= 1.65f;
            float escalaX = maximo.x / static_cast<float>(bbox.size.x);
            float escalaY = maximo.y / static_cast<float>(bbox.size.y);
            float escala = std::min(escalaX, escalaY) * 1.15f;

            sprite.setScale({escala, escala});
            sprite.setOrigin({static_cast<float>(bbox.position.x) + bbox.size.x / 2.f, static_cast<float>(bbox.position.y + bbox.size.y)});
            sprite.setPosition(posicion + sf::Vector2f(offsetX, 0.0f));
            ventana.draw(sprite);
        }
    }

    void obtenerPlanoPersonaje(const std::string& nombre, sf::Vector2f& posicion, sf::Vector2f& maximo) const {
        obtenerPlanoPersonaje(camaraActual, nombre, posicion, maximo);
    }


    void dibujarRutaPersonaje(sf::RenderWindow& ventana,
                              const sf::Font& fuente,
                              const std::string& nombre,
                              const std::vector<TipoCamara>& ruta,
                              TipoCamara posicionActual,
                              bool enPuerta,
                              int fila) const {
        if (camaraActual == TipoCamara::CAM_01_DULCERIA) return;
        if (!rutaIncluyeCamara(ruta, camaraActual) && posicionActual != camaraActual) {
            return;
        }

        float baseX = 198.f;
        float baseY = 22.f + static_cast<float>(fila) * 20.f;

        sf::Text etiqueta(fuente, nombre + (enPuerta ? " PUERTA" : ""), 12);
        etiqueta.setFillColor(enPuerta ? sf::Color(255, 120, 90) :
                              posicionActual == camaraActual ? sf::Color(255, 230, 120) : sf::Color(200, 255, 210));
        etiqueta.setOutlineColor(sf::Color::Black);
        etiqueta.setOutlineThickness(1.f);
        etiqueta.setPosition({baseX, baseY});
        ventana.draw(etiqueta);
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
