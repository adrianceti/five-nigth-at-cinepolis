#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <iostream>
#include <ctime>
#include <map>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdint>
#include <cmath>
#include "Guardia.hpp"
#include "MonitorCamaras.hpp"
#include "Personaje.hpp"

enum class EstadoJuego {
    Portada,
    MenuPrincipal,
    TransicionSala3,
    Instrucciones,
    Jugando,
    Pausa,
    AtaquePendiente,
    Jumpscare,
    GameOver,
    Victoria
};

class Motor {
private:
    sf::RenderWindow ventana;

    sf::Texture texturaOficina;
    std::optional<sf::Sprite> spriteOficina;
    sf::Texture texturaPasilloIzquierda;
    sf::Texture texturaPasilloDerecha;
    bool pasilloIzquierdoCargado;
    bool pasilloDerechoCargado;
    sf::Texture texturaPortada;
    std::optional<sf::Sprite> spritePortada;
    sf::Texture texturaMenuPrincipal;
    std::optional<sf::Sprite> spriteMenuPrincipal;
    sf::Texture texturaMenuPausa;
    std::optional<sf::Sprite> spriteMenuPausa;
    sf::Font fuenteUI;
    bool fuenteUICargada;


    sf::View vistaOficina;
    sf::View vistaInterfaz;
    float posicionCamaraX;
    float velocidadCamara;
    float anchoVirtualOficina;


    sf::RectangleShape visualPuertaIzquierda;
    sf::RectangleShape visualPuertaDerecha;


    std::map<std::string, sf::Texture> texturasPersonajesPuerta;
    std::map<std::string, sf::Sprite> spritesPersonajesPuerta;
    std::map<std::string, sf::Texture> texturasJumpscare;
    sf::RectangleShape marcoLuzIzquierda;
    sf::RectangleShape marcoLuzDerecha;


    sf::RectangleShape barraEnergiaFondo;
    sf::RectangleShape barraEnergiaFrente;
    std::optional<sf::Text> textoEnergiaHUD;
    std::optional<sf::Text> textoRelojHUD;
    sf::RectangleShape bloquesConsumo[4];
    sf::Texture texturaInterferenciaMonitor;


    Guardia jugador;
    MonitorCamaras monitor;
    Gobo gobo;
    Director director;
    Popy popy;
    TheUsher usher;


    sf::Clock relojEnergia;
    sf::Clock relojTerminal;
    sf::Clock relojEstado;
    sf::Clock relojIntroNoche1;
    sf::Clock relojVictoria;
    sf::Clock relojInterferenciaMonitor;
    sf::Clock relojParpadeoInterferencia;
    sf::Clock relojTickIA;
    sf::Clock relojTransicionSala3;
    sf::Clock relojEfectosMenu;
    float tiempoLuzIzquierdaActiva;
    float tiempoLuzDerechaActiva;


    sf::Clock relojNoche;
    EstadoJuego estadoJuego;
    int nocheActual;
    int horaActual;
    float tiempoPorHora;
    float acumuladorHora;
    bool juegoTerminado;
    bool victoria;
    float tiempoMuerteAcumulado;
    float retrasoAtaque;
    std::string atacanteActual;
    bool interferenciaMonitorActiva;
    float duracionInterferenciaMonitor;
    int fotogramaInterferenciaMonitor;
    float intervaloTickIA;
    float acumuladorTickIA;
    bool controlesBloqueados;

    std::map<std::string, sf::SoundBuffer> buffersAudio;
    std::optional<sf::Sound> sonidoIntroNoche1;
    std::optional<sf::Sound> sonidoAmbiente;
    std::optional<sf::Sound> sonidoMenuPrincipal;
    std::vector<sf::Sound> sonidosActivos;
    bool audioDisponible;
    bool introNoche1Reproducido;
    bool introNoche1Activa;
    float duracionIntroNoche1;
    float duracionVictoria;
    float tiempoIntroAntesPausa;

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

    bool cargarBufferAudio(const std::string& clave, const std::vector<std::string>& rutas) {
        sf::SoundBuffer buffer;
        for (const auto& ruta : rutas) {
            if (buffer.loadFromFile(ruta)) {
                buffersAudio.insert({clave, buffer});
                return true;
            }
        }
        return false;
    }

    void actualizarTextosHUD() {
        if (!textoEnergiaHUD.has_value() || !textoRelojHUD.has_value()) return;

        int energiaEntera = static_cast<int>(std::clamp(jugador.getEnergia(), 0.0f, 100.0f) + 0.5f);
        textoEnergiaHUD->setString(std::to_string(energiaEntera) + "%");

        textoRelojHUD->setString(std::to_string(horaActual) + " AM");
        sf::FloatRect limitesReloj = textoRelojHUD->getLocalBounds();
        textoRelojHUD->setOrigin({limitesReloj.position.x + limitesReloj.size.x, limitesReloj.position.y});
        textoRelojHUD->setPosition({1240.0f, 28.0f});
    }

    void actualizarBloquesConsumoHUD() {
        int dispositivosActivos = std::clamp(jugador.getDispositivosActivos(), 0, 4);
        sf::Color colorActivo = dispositivosActivos >= 3
            ? sf::Color(220, 70, 70)
            : sf::Color(90, 210, 120);
        sf::Color colorInactivo(35, 35, 35);

        for (int i = 0; i < 4; ++i) {
            if (i < dispositivosActivos) {
                bloquesConsumo[i].setFillColor(colorActivo);
            } else {
                bloquesConsumo[i].setFillColor(colorInactivo);
            }
        }
    }

    sf::IntRect obtenerRecortePuertaOficina(bool esIzquierda) const {
        return esIzquierda
            ? sf::IntRect(sf::Vector2i(0, 184), sf::Vector2i(222, 430))
            : sf::IntRect(sf::Vector2i(1360, 116), sf::Vector2i(187, 462));
    }

    sf::Vector2f obtenerEscalaOficina() const {
        sf::Vector2u tamanoTextura = texturaOficina.getSize();
        if (tamanoTextura.x == 0 || tamanoTextura.y == 0) {
            return {1.0f, 1.0f};
        }

        return {
            anchoVirtualOficina / static_cast<float>(tamanoTextura.x),
            720.0f / static_cast<float>(tamanoTextura.y)
        };
    }

    void actualizarVistaOficinaPorMouse(float dt) {
        sf::Vector2u tamanoVentana = ventana.getSize();
        if (tamanoVentana.x == 0 || tamanoVentana.y == 0) return;

        sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);
        if (posicionMouse.x < 0 ||
            posicionMouse.x > static_cast<int>(tamanoVentana.x) ||
            posicionMouse.y < 0 ||
            posicionMouse.y > static_cast<int>(tamanoVentana.y)) {
            return;
        }

        const float limiteIzquierdo = static_cast<float>(tamanoVentana.x) * (280.0f / 1280.0f);
        const float limiteDerecho = static_cast<float>(tamanoVentana.x) * (1000.0f / 1280.0f);

        if (static_cast<float>(posicionMouse.x) > limiteDerecho) {
            posicionCamaraX += velocidadCamara * dt;
        } else if (static_cast<float>(posicionMouse.x) < limiteIzquierdo) {
            posicionCamaraX -= velocidadCamara * dt;
        }

        posicionCamaraX = std::clamp(posicionCamaraX, 640.0f, anchoVirtualOficina - 640.0f);
        vistaOficina.setCenter({posicionCamaraX, 360.0f});
    }

    void ajustarZonaPuertaFisica(sf::RectangleShape& zonaPuerta, bool esIzquierda) {
        sf::IntRect recorte = obtenerRecortePuertaOficina(esIzquierda);
        sf::Vector2f escalaOficina = obtenerEscalaOficina();

        zonaPuerta.setSize({
            static_cast<float>(recorte.size.x) * escalaOficina.x,
            static_cast<float>(recorte.size.y) * escalaOficina.y
        });
        zonaPuerta.setFillColor(sf::Color::Transparent);
        zonaPuerta.setOutlineColor(sf::Color(8, 9, 10, 235));
        zonaPuerta.setOutlineThickness(3.0f);
        zonaPuerta.setPosition({
            static_cast<float>(recorte.position.x) * escalaOficina.x,
            static_cast<float>(recorte.position.y) * escalaOficina.y
        });
    }

    sf::Vector2f obtenerPosicionIndicadorPuerta(const sf::FloatRect& zona, bool esIzquierda) const {
        const float margenX = 10.0f;
        const float centroY = zona.position.y + zona.size.y * 0.56f;

        return {
            esIzquierda ? zona.position.x + zona.size.x - 34.0f : zona.position.x + margenX,
            centroY - 19.0f
        };
    }

    float obtenerPulsoLuzPasillo(bool esIzquierda) const {
        float tiempo = relojEstado.getElapsedTime().asSeconds();
        float fase = esIzquierda ? 0.0f : 1.35f;
        return 0.5f + 0.5f * std::sin((tiempo + fase) * 5.0f);
    }

    void dibujarResplandorLuzPuerta(sf::RenderWindow& ventana, const sf::FloatRect& zona, bool esIzquierda, bool cerrada) const {
        float pulso = obtenerPulsoLuzPasillo(esIzquierda);
        std::uint8_t alphaBorde = static_cast<std::uint8_t>((cerrada ? 70.0f : 110.0f) + pulso * 70.0f);

        sf::RectangleShape halo({zona.size.x, zona.size.y});
        halo.setPosition(zona.position);
        halo.setFillColor(sf::Color(255, 210, 105, cerrada ? 8 : 14));
        halo.setOutlineColor(sf::Color(255, 225, 140, static_cast<std::uint8_t>(alphaBorde * 0.68f)));
        halo.setOutlineThickness(4.0f);
        ventana.draw(halo);

        sf::RectangleShape bordeInterno({
            std::max(4.0f, zona.size.x - 16.0f),
            std::max(4.0f, zona.size.y - 16.0f)
        });
        bordeInterno.setPosition({zona.position.x + 8.0f, zona.position.y + 8.0f});
        bordeInterno.setFillColor(sf::Color::Transparent);
        bordeInterno.setOutlineColor(sf::Color(255, 238, 180, static_cast<std::uint8_t>(38.0f + pulso * 48.0f)));
        bordeInterno.setOutlineThickness(2.0f);
        ventana.draw(bordeInterno);
    }

    void generarTexturaInterferenciaMonitor() {
        if (cargarTextureDesdeRutas(texturaInterferenciaMonitor, {
                "assets/textures/monitor/estatica.png",
                "../assets/textures/monitor/estatica.png"
            })) {
            texturaInterferenciaMonitor.setRepeated(true);
            texturaInterferenciaMonitor.setSmooth(false);
            return;
        }

        sf::Image ruido(sf::Vector2u(128, 128), sf::Color::Black);

        for (unsigned int y = 0; y < 128; ++y) {
            for (unsigned int x = 0; x < 128; ++x) {
                int brillo = std::rand() % 256;
                int alpha = 135 + (std::rand() % 100);
                ruido.setPixel(sf::Vector2u(x, y), sf::Color(brillo, brillo, brillo, alpha));
            }
        }

        if (!texturaInterferenciaMonitor.loadFromImage(ruido)) {
            std::cerr << "No se pudo generar la textura de interferencia del monitor" << std::endl;
            return;
        }
        texturaInterferenciaMonitor.setRepeated(true);
        texturaInterferenciaMonitor.setSmooth(false);
    }

    void activarInterferenciaMonitor() {
        interferenciaMonitorActiva = true;
        relojInterferenciaMonitor.restart();
        relojParpadeoInterferencia.restart();
        fotogramaInterferenciaMonitor = std::rand();
    }

    void actualizarInterferenciaMonitor() {
        if (interferenciaMonitorActiva &&
            relojInterferenciaMonitor.getElapsedTime().asSeconds() >= duracionInterferenciaMonitor) {
            interferenciaMonitorActiva = false;
        }
    }

    void dibujarInterferenciaMonitor() {
        if (!interferenciaMonitorActiva) {
            return;
        }

        if (relojParpadeoInterferencia.getElapsedTime().asSeconds() >= 0.035f) {
            fotogramaInterferenciaMonitor++;
            relojParpadeoInterferencia.restart();
        }

        sf::Vector2u tamRuido = texturaInterferenciaMonitor.getSize();
        int anchoRuido = std::max(1, static_cast<int>(tamRuido.x));
        int altoRuido = std::max(1, static_cast<int>(tamRuido.y));
        int offsetX = (std::rand() + fotogramaInterferenciaMonitor * 17) % anchoRuido;
        int offsetY = (std::rand() + fotogramaInterferenciaMonitor * 31) % altoRuido;
        bool voltearX = fotogramaInterferenciaMonitor % 2 == 0;
        bool voltearY = (fotogramaInterferenciaMonitor / 2) % 2 == 0;

        sf::Sprite ruido(texturaInterferenciaMonitor);
        ruido.setTextureRect(sf::IntRect(sf::Vector2i(offsetX, offsetY), sf::Vector2i(1280, 720)));
        ruido.setScale({voltearX ? -1.0f : 1.0f, voltearY ? -1.0f : 1.0f});
        ruido.setPosition({voltearX ? 1280.0f : 0.0f, voltearY ? 720.0f : 0.0f});
        ruido.setColor(sf::Color(255, 255, 255, 215));
        ventana.draw(ruido);

        sf::RectangleShape flash(sf::Vector2f(1280.0f, 720.0f));
        flash.setFillColor(sf::Color(20, 255, 120, 35 + (std::rand() % 55)));
        ventana.draw(flash);
    }

    void reproducirSonido(const std::string& clave, float volumen = 75.0f) {
        auto buffer = buffersAudio.find(clave);
        if (buffer == buffersAudio.end()) {
            return;
        }

        sonidosActivos.erase(
            std::remove_if(sonidosActivos.begin(), sonidosActivos.end(),
                           [](const sf::Sound& sonido) {
                               return sonido.getStatus() == sf::SoundSource::Status::Stopped;
                           }),
            sonidosActivos.end());

        sonidosActivos.emplace_back(buffer->second);
        sonidosActivos.back().setRelativeToListener(true);
        sonidosActivos.back().setMinDistance(1.0f);
        sonidosActivos.back().setAttenuation(0.0f);
        sonidosActivos.back().setPosition({0.0f, 0.0f, 0.0f});
        sonidosActivos.back().setPan(0.0f);
        sonidosActivos.back().setVolume(volumen);
        sonidosActivos.back().play();
    }

    void reproducirSonidoEspacial(const std::string& clave, const sf::Vector3f& posicion, float volumen = 75.0f) {
        auto buffer = buffersAudio.find(clave);
        if (buffer == buffersAudio.end()) {
            return;
        }

        sonidosActivos.erase(
            std::remove_if(sonidosActivos.begin(), sonidosActivos.end(),
                           [](const sf::Sound& sonido) {
                               return sonido.getStatus() == sf::SoundSource::Status::Stopped;
                           }),
            sonidosActivos.end());

        sonidosActivos.emplace_back(buffer->second);
        sonidosActivos.back().setRelativeToListener(true);
        sonidosActivos.back().setMinDistance(1.0f);
        sonidosActivos.back().setAttenuation(0.0f);
        sonidosActivos.back().setPosition(posicion);
        sonidosActivos.back().setPan(posicion.x < 0.0f ? -1.0f : posicion.x > 0.0f ? 1.0f : 0.0f);
        sonidosActivos.back().setVolume(volumen);
        sonidosActivos.back().play();
    }

    void iniciarIntroNoche1() {
        if (!sonidoIntroNoche1.has_value()) {
            return;
        }

        sonidoIntroNoche1->setLooping(false);
        sonidoIntroNoche1->play();
        introNoche1Reproducido = true;
        introNoche1Activa = true;
        tiempoIntroAntesPausa = 0.0f;
        relojIntroNoche1.restart();
    }

    void cargarAudio() {
        const std::vector<std::string> bases = {
            "assets/audio/",
            "../assets/audio/",
            "assets/textures/musica/audio_juego/",
            "../assets/textures/musica/audio_juego/"
        };

        auto rutasDe = [&](const std::string& nombre) {
            std::vector<std::string> rutas;
            for (const auto& base : bases) {
                rutas.push_back(base + nombre);
            }
            return rutas;
        };

        audioDisponible = true;
        if (cargarBufferAudio("ambiente", rutasDe("ambiente_tenebroso.wav"))) {
            sonidoAmbiente.emplace(buffersAudio.at("ambiente"));
            sonidoAmbiente->setLooping(true);
            sonidoAmbiente->setVolume(100.0f);
        } else {
            audioDisponible = false;
            std::cerr << "No se encontro musica ambiente" << std::endl;
        }

        std::vector<std::pair<std::string, std::string>> efectos = {
            {"intro", "intro_telefono.wav"},
            {"puerta", "puerta.wav"},
            {"monitor", "monitor_static.wav"},
            {"camara", "cambio_camara.wav"},
            {"luz_roja", "alerta_luz_roja.wav"},
            {"ataque", "alerta_ataque.wav"},
            {"jumpscare", "jumpscare.wav"},
            {"gameover", "game_over.wav"},
            {"victoria", "victoria.wav"},
            {"paso_izq", "paso_izq.wav"},
            {"paso_der", "paso_der.wav"},
            {"alerta_puerta", "alerta_puerta.wav"},
            {"foxy_corriendo", "foxy_corriendo.wav"},
            {"golpe_puerta", "golpe_puerta.wav"},
            {"apagon", "apagón.wav"}
        };

        for (const auto& efecto : efectos) {
            if (!cargarBufferAudio(efecto.first, rutasDe(efecto.second))) {
                audioDisponible = false;
                std::cerr << "No se encontro audio: " << efecto.second << std::endl;
            }
        }

        if (buffersAudio.count("intro") > 0) {
            sonidoIntroNoche1.emplace(buffersAudio.at("intro"));
            sonidoIntroNoche1->setVolume(88.0f);
        }
    }

    struct CurvaDificultadHoraria {
        int bonoDado;
    };

    CurvaDificultadHoraria obtenerCurvaDificultadHoraria() const {
        switch (horaActual) {
            case 12: return {0};
            case 1:  return {1};
            case 2:  return {2};
            case 3:  return {3};
            case 4:  return {5};
            case 5:  return {7};
            default: return {0};
        }
    }

    float obtenerTasaConsumoBasePorNoche() const {
        switch (nocheActual) {
            case 1: return 1.0f / 9.6f;
            default: return 0.141f;
        }
    }

    int calcularDificultadBasePorPersonaje(const Personaje& personaje) const {
        int bonoHorario = obtenerCurvaDificultadHoraria().bonoDado;

        if (personaje.getNombre() == "Gobo") {
            return std::clamp(4 + bonoHorario, 1, 20);
        }
        if (personaje.getNombre() == "Director") {
            return std::clamp(3 + bonoHorario, 1, 20);
        }
        if (personaje.getNombre() == "Popy") {
            return std::clamp(2 + bonoHorario, 1, 20);
        }
        if (personaje.getNombre() == "The Usher") {
            return std::clamp(2 + bonoHorario, 1, 20);
        }
        return std::clamp(10 + bonoHorario, 1, 20);
    }

    void procesarTickIA() {
        if (estadoJuego != EstadoJuego::Jugando || introNoche1Activa) {
            return;
        }

        bool monitorAbierto = jugador.esMonitorAbierto();
        TipoCamara camaraActualMonitor = monitor.getCamaraActual();
        auto camaraObservada = [&](const Personaje& personaje) {
            return monitorAbierto && camaraActualMonitor == personaje.getPosicionActual();
        };

        bool movioGobo = gobo.procesarTickMovimiento(calcularDificultadBasePorPersonaje(gobo), camaraObservada(gobo), horaActual);
        bool movioDirector = director.procesarTickMovimiento(calcularDificultadBasePorPersonaje(director), camaraObservada(director), horaActual);
        bool movioPopy = popy.procesarTickMovimiento(calcularDificultadBasePorPersonaje(popy), monitorAbierto, horaActual);
        bool movioUsher = usher.procesarTickMovimiento(calcularDificultadBasePorPersonaje(usher), camaraObservada(usher), horaActual);
        if (movioGobo) {
            if (gobo.esEnLaPuerta()) reproducirSonidoEspacial("alerta_puerta", {-1.0f, 0.0f, 0.0f}, 60.0f);
            else reproducirSonidoEspacial("paso_izq", {-1.0f, 0.0f, 0.0f}, 52.0f);
        }
        if (movioDirector) {
            if (director.esEnLaPuerta()) reproducirSonidoEspacial("alerta_puerta", {1.0f, 0.0f, 0.0f}, 60.0f);
            else reproducirSonidoEspacial("paso_der", {1.0f, 0.0f, 0.0f}, 52.0f);
        }
        if (movioPopy) {
            if (popy.esEnLaPuerta()) reproducirSonidoEspacial("foxy_corriendo", {-1.0f, 0.0f, 0.0f}, 78.0f);
            else reproducirSonidoEspacial("paso_izq", {-1.0f, 0.0f, 0.0f}, 48.0f);
        }
        if (movioUsher) {
            if (usher.esEnLaPuerta()) reproducirSonidoEspacial("alerta_puerta", {1.0f, 0.0f, 0.0f}, 58.0f);
            else reproducirSonidoEspacial("paso_der", {1.0f, 0.0f, 0.0f}, 48.0f);
        }
        if (gobo.esEstaAdentro()) {
            iniciarAtaque(gobo.getNombre());
            return;
        }
        if (director.esEstaAdentro()) {
            iniciarAtaque(director.getNombre());
            return;
        }
        if (popy.esEstaAdentro()) {
            iniciarAtaque(popy.getNombre());
            return;
        }
        if (usher.esEstaAdentro()) {
            iniciarAtaque(usher.getNombre());
            return;
        }
    }

    bool cargarPrimerPngEnCarpeta(sf::Texture& textura, const std::vector<std::string>& carpetas) {
        for (const auto& carpeta : carpetas) {
            std::filesystem::path rutaCarpeta(carpeta);
            if (!std::filesystem::exists(rutaCarpeta) || !std::filesystem::is_directory(rutaCarpeta)) {
                continue;
            }

            for (const auto& entrada : std::filesystem::directory_iterator(rutaCarpeta)) {
                if (!entrada.is_regular_file() || entrada.path().extension() != ".png") {
                    continue;
                }

                if (textura.loadFromFile(entrada.path().string())) {
                    return true;
                }
            }
        }

        return false;
    }


    sf::IntRect obtenerBoundingBoxAlpha(const sf::Texture& textura) {
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

    void cargarTexturasJumpscare() {
        std::vector<std::pair<std::string, std::string>> personajes = {
            {"Gobo", "gobo"},
            {"Director", "director"},
            {"Popy", "popy"},
            {"The Usher", "theusher"},
        };

        for (const auto& personaje : personajes) {
            sf::Texture textura;
            std::vector<std::string> carpetas = {
                "assets/textures/personajes/" + personaje.second,
                "../assets/textures/personajes/" + personaje.second,
                "assets/textures/personajes/" + personaje.second + "/sprite" + personaje.second,
                "../assets/textures/personajes/" + personaje.second + "/sprite" + personaje.second
            };

            if (cargarPrimerPngEnCarpeta(textura, carpetas)) {
                texturasJumpscare.insert({personaje.first, textura});
                std::cerr << "✓ Cargado jumpscare de " << personaje.first << std::endl;
            } else {
                std::cerr << "⚠ No se encontró jumpscare para " << personaje.first << std::endl;
            }
        }
    }

    void iniciarAtaque(const std::string& nombreAtacante) {
        if (estadoJuego != EstadoJuego::Jugando) {
            return;
        }

        atacanteActual = nombreAtacante;
        retrasoAtaque = 1.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 2.0f;
        estadoJuego = EstadoJuego::AtaquePendiente;
        controlesBloqueados = true;
        relojEstado.restart();
        reproducirSonido("ataque", 42.0f);
        std::cerr << "[ATAQUE] " << atacanteActual << " entro. Jumpscare en "
                  << retrasoAtaque << " segundos." << std::endl;
    }

    void iniciarJumpscare() {
        estadoJuego = EstadoJuego::Jumpscare;
        juegoTerminado = true;
        victoria = false;
        controlesBloqueados = true;
        relojEstado.restart();
        reproducirSonido("jumpscare", 86.0f);
    }

    void iniciarVictoria() {
        estadoJuego = EstadoJuego::Victoria;
        juegoTerminado = true;
        victoria = true;
        controlesBloqueados = true;
        introNoche1Activa = false;
        relojVictoria.restart();

        if (sonidoIntroNoche1.has_value()) {
            sonidoIntroNoche1->stop();
        }
        if (sonidoAmbiente.has_value()) {
            sonidoAmbiente->stop();
        }
        for (auto& sonido : sonidosActivos) {
            sonido.stop();
        }
        sonidosActivos.clear();
        reproducirSonido("victoria", 78.0f);
    }

    void pasarAGameOver() {
        estadoJuego = EstadoJuego::GameOver;
        relojEstado.restart();
        introNoche1Activa = false;
        controlesBloqueados = true;
        if (sonidoIntroNoche1.has_value()) {
            sonidoIntroNoche1->stop();
        }
        if (sonidoAmbiente.has_value()) {
            sonidoAmbiente->stop();
        }
        for (auto& sonido : sonidosActivos) {
            sonido.stop();
        }
        sonidosActivos.clear();
        reproducirSonido("apagon", 46.0f);
    }

    void resetearPartida() {
        jugador.resetear();
        gobo.resetear();
        director.resetear();
        popy.resetear();
        usher.resetear();

        horaActual = 12;
        acumuladorHora = 0.0f;
        juegoTerminado = false;
        victoria = false;
        controlesBloqueados = false;
        tiempoMuerteAcumulado = 0.0f;
        retrasoAtaque = 0.0f;
        atacanteActual.clear();
        interferenciaMonitorActiva = false;
        fotogramaInterferenciaMonitor = 0;
        estadoJuego = EstadoJuego::Jugando;
        relojVictoria.restart();

        posicionCamaraX = anchoVirtualOficina / 2.0f;
        vistaOficina.setCenter({posicionCamaraX, 360.0f});
        barraEnergiaFrente.setSize(sf::Vector2f(250.0f, 20.0f));
        barraEnergiaFrente.setFillColor(sf::Color::Green);
        actualizarTextosHUD();

        if (spriteOficina.has_value()) {
            spriteOficina.value().setColor(sf::Color::White);
        }

        relojEnergia.restart();
        relojTerminal.restart();
        relojNoche.restart();
        relojEstado.restart();
        relojIntroNoche1.restart();
        relojVictoria.restart();
        relojInterferenciaMonitor.restart();
        relojParpadeoInterferencia.restart();
        tiempoLuzIzquierdaActiva = 0.0f;
        tiempoLuzDerechaActiva = 0.0f;

        if (sonidoIntroNoche1.has_value()) {
            sonidoIntroNoche1->stop();
        }
        if (sonidoAmbiente.has_value()) {
            sonidoAmbiente->stop();
        }
        for (auto& sonido : sonidosActivos) {
            sonido.stop();
        }
        sonidosActivos.clear();

        introNoche1Reproducido = false;
        introNoche1Activa = false;
        tiempoIntroAntesPausa = 0.0f;
    }

    void volverAlMenuPrincipal() {
        resetearPartida();
        estadoJuego = EstadoJuego::MenuPrincipal;
        controlesBloqueados = false;

        if (sonidoAmbiente.has_value()) {
            sonidoAmbiente->stop();
        }
        if (sonidoIntroNoche1.has_value()) {
            sonidoIntroNoche1->stop();
        }
        if (sonidoMenuPrincipal.has_value()) {
            sonidoMenuPrincipal->play();
        }
    }

    void pausarPartida() {
        if (estadoJuego != EstadoJuego::Jugando) {
            return;
        }

        estadoJuego = EstadoJuego::Pausa;
        tiempoIntroAntesPausa += relojIntroNoche1.getElapsedTime().asSeconds();

        if (sonidoIntroNoche1.has_value() &&
            sonidoIntroNoche1->getStatus() == sf::SoundSource::Status::Playing) {
            sonidoIntroNoche1->pause();
        }
        if (sonidoAmbiente.has_value() &&
            sonidoAmbiente->getStatus() == sf::SoundSource::Status::Playing) {
            sonidoAmbiente->pause();
        }
        for (auto& sonido : sonidosActivos) {
            if (sonido.getStatus() == sf::SoundSource::Status::Playing) {
                sonido.pause();
            }
        }
    }

    void continuarPartida() {
        if (estadoJuego != EstadoJuego::Pausa) {
            return;
        }

        estadoJuego = EstadoJuego::Jugando;
        relojEnergia.restart();
        relojTerminal.restart();
        relojTickIA.restart();
        relojInterferenciaMonitor.restart();
        relojParpadeoInterferencia.restart();
        relojIntroNoche1.restart();

        if (sonidoIntroNoche1.has_value() &&
            sonidoIntroNoche1->getStatus() == sf::SoundSource::Status::Paused) {
            sonidoIntroNoche1->play();
        }
        if (sonidoAmbiente.has_value() &&
            sonidoAmbiente->getStatus() == sf::SoundSource::Status::Paused) {
            sonidoAmbiente->play();
        }
        for (auto& sonido : sonidosActivos) {
            if (sonido.getStatus() == sf::SoundSource::Status::Paused) {
                sonido.play();
            }
        }
    }

    bool clicEnBotonPausa(sf::Vector2i posicionClick, int boton) const {
        if (!spriteMenuPausa.has_value()) {
            return false;
        }

        sf::Vector2f posicionInterfaz = ventana.mapPixelToCoords(posicionClick, vistaInterfaz);
        sf::Vector2f posicionTextura = spriteMenuPausa->getInverseTransform().transformPoint(posicionInterfaz);
        sf::FloatRect zona;

        if (boton == 0) {
            zona = sf::FloatRect({575.0f, 370.0f}, {540.0f, 92.0f});
        } else if (boton == 1) {
            zona = sf::FloatRect({575.0f, 472.0f}, {540.0f, 82.0f});
        } else {
            zona = sf::FloatRect({575.0f, 572.0f}, {540.0f, 82.0f});
        }

        return zona.contains(posicionTextura);
    }

    bool clicEnSalirGameOver(sf::Vector2i posicionClick) const {
        sf::Vector2f posicionInterfaz = ventana.mapPixelToCoords(posicionClick, vistaInterfaz);
        sf::FloatRect botonSalir({455.0f, 470.0f}, {370.0f, 70.0f});
        return botonSalir.contains(posicionInterfaz);
    }

    void cargarTexturasPersonajesPuerta() {

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
                    std::cerr << "✓ Cargada textura de puerta para " << personajes[i] << " desde " << nombre << std::endl;
                    break;
                }
            }

            if (cargada) {
                auto texturaInsertada = texturasPersonajesPuerta.insert_or_assign(personajes[i], textura);
                sf::Sprite sprite(texturaInsertada.first->second);

                sf::IntRect bbox = obtenerBoundingBoxAlpha(texturaInsertada.first->second);
                float alturaObjetivoPuerta = 280.0f;
                if (personajes[i] == "Gobo") alturaObjetivoPuerta = 320.0f;
                else if (personajes[i] == "Director") alturaObjetivoPuerta = 260.0f;
                else if (personajes[i] == "Popy") alturaObjetivoPuerta = 225.0f;
                else if (personajes[i] == "TheUsher") alturaObjetivoPuerta = 250.0f;
                float escala = 1.0f;
                if (bbox.size.y > 0) escala = alturaObjetivoPuerta / static_cast<float>(bbox.size.y);
                sprite.setScale({escala, escala});

                sprite.setOrigin({static_cast<float>(bbox.position.x) + bbox.size.x / 2.0f, static_cast<float>(bbox.position.y + bbox.size.y)});
                spritesPersonajesPuerta.insert({personajes[i], sprite});
            } else {
                std::cerr << "⚠ No se encontró textura de puerta para " << personajes[i] << std::endl;
            }
        }
    }

    std::string getClavePersonajePuerta(const std::string& nombre) const {
        if (nombre == "The Usher") return "TheUsher";
        return nombre;
    }

    int obtenerBotonMenuEn(sf::Vector2i posicionPixel) const {
        if (!spriteMenuPrincipal.has_value()) {
            return -1;
        }

        sf::Vector2f posicionInterfaz = ventana.mapPixelToCoords(posicionPixel, vistaInterfaz);
        sf::Vector2f posicionTextura = spriteMenuPrincipal->getInverseTransform().transformPoint(posicionInterfaz);
        const std::vector<sf::FloatRect> botones = {
            {{40.0f, 355.0f}, {475.0f, 105.0f}},
            {{40.0f, 470.0f}, {475.0f, 70.0f}},
            {{40.0f, 550.0f}, {475.0f, 70.0f}}
        };

        for (size_t i = 0; i < botones.size(); ++i) {
            if (botones[i].contains(posicionTextura)) {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    bool clicEnJugar(sf::Vector2i posicionClick) const {
        return obtenerBotonMenuEn(posicionClick) == 0;
    }

    bool clicEnInstrucciones(sf::Vector2i posicionClick) const {
        return obtenerBotonMenuEn(posicionClick) == 1;
    }

    bool clicEnSalirMenu(sf::Vector2i posicionClick) const {
        return obtenerBotonMenuEn(posicionClick) == 2;
    }

    void dibujarBotonesAdicionalesMenu() {
        if (!spriteMenuPrincipal.has_value()) {
            return;
        }

        const sf::Transform transformacion = spriteMenuPrincipal->getTransform();
        dibujarZonaOscuraTransformada(
            transformacion,
            {{32.0f, 465.0f}, {495.0f, 240.0f}},
            sf::Color(1, 2, 3, 245)
        );

        const int botonActivo = obtenerBotonMenuEn(sf::Mouse::getPosition(ventana));
        const std::vector<std::pair<std::string, sf::FloatRect>> botones = {
            {"JUGAR", {{40.0f, 355.0f}, {475.0f, 105.0f}}},
            {"INSTRUCCIONES", {{40.0f, 470.0f}, {475.0f, 70.0f}}},
            {"SALIR", {{40.0f, 550.0f}, {475.0f, 70.0f}}}
        };

        for (size_t i = 0; i < botones.size(); ++i) {
            const auto& boton = botones[i];
            const bool activo = static_cast<int>(i) == botonActivo;
            sf::Vector2f inicio = transformacion.transformPoint(boton.second.position);
            sf::Vector2f final = transformacion.transformPoint({
                boton.second.position.x + boton.second.size.x,
                boton.second.position.y + boton.second.size.y
            });
            sf::RectangleShape forma(final - inicio);
            forma.setPosition(inicio);
            forma.setFillColor(activo ? sf::Color(12, 58, 15, 205) : sf::Color(2, 3, 3, 210));
            forma.setOutlineColor(activo ? sf::Color(103, 255, 65, 245) : sf::Color(92, 94, 88, 210));
            forma.setOutlineThickness(activo ? 3.0f : 2.0f);
            ventana.draw(forma);

            if (fuenteUICargada) {
                sf::Text texto(fuenteUI, boton.first, i == 0 ? 38 : 27);
                texto.setStyle(sf::Text::Bold);
                texto.setFillColor(activo ? sf::Color(142, 255, 100) : sf::Color(205, 202, 188));
                texto.setOutlineColor(sf::Color::Black);
                texto.setOutlineThickness(activo ? 3.0f : 2.0f);
                sf::FloatRect limites = texto.getLocalBounds();
                texto.setOrigin({
                    limites.position.x + limites.size.x / 2.0f,
                    limites.position.y + limites.size.y / 2.0f
                });
                texto.setPosition({
                    inicio.x + (final.x - inicio.x) * (i == 0 ? 0.43f : 0.5f),
                    inicio.y + (final.y - inicio.y) / 2.0f
                });
                ventana.draw(texto);

                if (i == 0) {
                    sf::CircleShape icono(19.0f, 3);
                    icono.setFillColor(activo ? sf::Color(120, 255, 85) : sf::Color(190, 190, 178));
                    icono.setOutlineColor(sf::Color::Black);
                    icono.setOutlineThickness(2.0f);
                    icono.setRotation(sf::degrees(90.0f));
                    icono.setPosition({
                        inicio.x + 72.0f,
                        inicio.y + (final.y - inicio.y) / 2.0f - 19.0f
                    });
                    ventana.draw(icono);
                }
            }
        }
    }

    void renderizarInstrucciones() {
        ventana.clear(sf::Color::Black);
        ventana.setView(vistaInterfaz);

        if (spriteMenuPrincipal.has_value()) {
            ventana.draw(spriteMenuPrincipal.value());
        }

        sf::RectangleShape sombra({1280.0f, 720.0f});
        sombra.setFillColor(sf::Color(0, 0, 0, 185));
        ventana.draw(sombra);

        sf::RectangleShape panel({760.0f, 590.0f});
        panel.setPosition({260.0f, 65.0f});
        panel.setFillColor(sf::Color(7, 9, 10, 245));
        panel.setOutlineColor(sf::Color(120, 105, 76));
        panel.setOutlineThickness(4.0f);
        ventana.draw(panel);

        if (fuenteUICargada) {
            sf::Text titulo(fuenteUI, "INSTRUCCIONES", 50);
            titulo.setStyle(sf::Text::Bold);
            titulo.setFillColor(sf::Color(220, 205, 170));
            sf::FloatRect limitesTitulo = titulo.getLocalBounds();
            titulo.setOrigin({
                limitesTitulo.position.x + limitesTitulo.size.x / 2.0f,
                limitesTitulo.position.y + limitesTitulo.size.y / 2.0f
            });
            titulo.setPosition({640.0f, 125.0f});
            ventana.draw(titulo);

            const std::vector<std::string> lineas = {
                "MUEVE EL RATON A LOS LADOS  -  MIRAR LA OFICINA",
                "A / D  -  ABRIR O CERRAR PUERTAS",
                "Q / E  -  ENCENDER LUCES",
                "BARRA ESPACIADORA  -  ABRIR O CERRAR MONITOR",
                "TECLAS 1 - 5  -  CAMBIAR CAMARAS",
                "ESC  -  PAUSAR EL JUEGO",
                "SOBREVIVE HASTA LAS 6 AM Y CUIDA LA ENERGIA"
            };

            float y = 215.0f;
            for (const auto& linea : lineas) {
                sf::Text texto(fuenteUI, linea, 23);
                texto.setFillColor(sf::Color(215, 215, 205));
                sf::FloatRect limites = texto.getLocalBounds();
                texto.setOrigin({
                    limites.position.x + limites.size.x / 2.0f,
                    limites.position.y + limites.size.y / 2.0f
                });
                texto.setPosition({640.0f, y});
                ventana.draw(texto);
                y += 57.0f;
            }

            sf::Text regresar(fuenteUI, "PULSA ESC PARA REGRESAR", 19);
            regresar.setFillColor(sf::Color(150, 160, 150));
            sf::FloatRect limitesRegresar = regresar.getLocalBounds();
            regresar.setOrigin({
                limitesRegresar.position.x + limitesRegresar.size.x / 2.0f,
                limitesRegresar.position.y + limitesRegresar.size.y / 2.0f
            });
            regresar.setPosition({640.0f, 615.0f});
            ventana.draw(regresar);
        }

        ventana.display();
    }

    void iniciarPartidaDesdeMenu() {
        if (sonidoMenuPrincipal.has_value()) {
            sonidoMenuPrincipal->stop();
        }
        jugador.resetear();
        gobo.resetear();
        director.resetear();
        popy.resetear();
        usher.resetear();
        horaActual = 12;
        acumuladorHora = 0.0f;
        acumuladorTickIA = 0.0f;
        introNoche1Reproducido = false;
        introNoche1Activa = false;
        tiempoIntroAntesPausa = 0.0f;
        estadoJuego = EstadoJuego::Jugando;
        relojNoche.restart();
        relojEnergia.restart();
        relojIntroNoche1.restart();
        controlesBloqueados = false;
    }

    void iniciarTransicionSala3() {
        if (sonidoMenuPrincipal.has_value()) {
            sonidoMenuPrincipal->stop();
        }
        estadoJuego = EstadoJuego::TransicionSala3;
        relojTransicionSala3.restart();
        reproducirSonido("golpe_puerta", 82.0f);
    }

    void dibujarZonaOscuraTransformada(const sf::Transform& transformacion, sf::FloatRect zona, sf::Color color) {
        sf::ConvexShape forma(4);
        forma.setPoint(0, transformacion.transformPoint(zona.position));
        forma.setPoint(1, transformacion.transformPoint({zona.position.x + zona.size.x, zona.position.y}));
        forma.setPoint(2, transformacion.transformPoint({zona.position.x + zona.size.x, zona.position.y + zona.size.y}));
        forma.setPoint(3, transformacion.transformPoint({zona.position.x, zona.position.y + zona.size.y}));
        forma.setFillColor(color);
        ventana.draw(forma);
    }

    void dibujarSegmentoCadena(sf::Vector2f inicio, sf::Vector2f fin, float escala, float alpha) {
        sf::Vector2f diferencia = fin - inicio;
        float longitud = std::sqrt(diferencia.x * diferencia.x + diferencia.y * diferencia.y);
        int cantidad = std::max(2, static_cast<int>(longitud / std::max(10.0f, 18.0f * escala)));
        float angulo = std::atan2(diferencia.y, diferencia.x) * 180.0f / 3.14159265f;

        for (int i = 0; i <= cantidad; ++i) {
            float avance = static_cast<float>(i) / static_cast<float>(cantidad);
            sf::Vector2f posicion = inicio + diferencia * avance;
            sf::RectangleShape eslabon({15.0f * escala, 7.0f * escala});
            eslabon.setOrigin({7.5f * escala, 3.5f * escala});
            eslabon.setPosition(posicion);
            eslabon.setRotation(sf::degrees(angulo + (i % 2 == 0 ? 0.0f : 90.0f)));
            eslabon.setFillColor(sf::Color(23, 22, 20, static_cast<std::uint8_t>(alpha)));
            eslabon.setOutlineColor(sf::Color(92, 88, 78, static_cast<std::uint8_t>(alpha)));
            eslabon.setOutlineThickness(std::max(1.0f, 1.3f * escala));
            ventana.draw(eslabon);
        }
    }

    void dibujarEfectosTenebrososMenu(const sf::Transform& transformacion, float escala, float tiempo, float intensidad) {
        float ciclo = std::fmod(tiempo, 5.7f);
        bool ojosEncendidos = !(ciclo > 1.25f && ciclo < 1.48f) &&
                              !(ciclo > 1.62f && ciclo < 2.05f) &&
                              !(ciclo > 4.35f && ciclo < 4.92f);
        float pulso = ojosEncendidos
            ? 0.72f + std::sin(tiempo * 5.8f) * 0.18f + std::sin(tiempo * 13.1f) * 0.10f
            : 0.0f;

        for (const sf::Vector2f ojoTextura : {sf::Vector2f(944.0f, 382.0f), sf::Vector2f(991.0f, 382.0f)}) {
            sf::Vector2f ojo = transformacion.transformPoint(ojoTextura);
            if (!ojosEncendidos) {
                sf::CircleShape apagado(12.0f * escala);
                apagado.setOrigin({12.0f * escala, 12.0f * escala});
                apagado.setScale({0.72f, 1.0f});
                apagado.setPosition(ojo);
                apagado.setFillColor(sf::Color(12, 1, 1, 235));
                ventana.draw(apagado);
                continue;
            }

            sf::CircleShape halo(30.0f * escala);
            halo.setOrigin({30.0f * escala, 30.0f * escala});
            halo.setScale({0.8f, 1.0f});
            halo.setPosition(ojo);
            halo.setFillColor(sf::Color(215, 0, 0, static_cast<std::uint8_t>(42.0f * pulso * intensidad)));
            ventana.draw(halo);

            sf::CircleShape brillo(8.0f * escala);
            brillo.setOrigin({8.0f * escala, 8.0f * escala});
            brillo.setPosition(ojo);
            brillo.setFillColor(sf::Color(255, 32, 20, static_cast<std::uint8_t>(170.0f * pulso * intensidad)));
            ventana.draw(brillo);
        }

        float falla = 0.0f;
        if ((ciclo > 1.22f && ciclo < 1.52f) ||
            (ciclo > 3.08f && ciclo < 3.18f) ||
            (ciclo > 4.32f && ciclo < 4.95f)) {
            falla = 1.0f;
        } else {
            falla = 0.18f + 0.08f * std::sin(tiempo * 2.3f);
        }

        sf::RectangleShape oscuridad({1280.0f, 720.0f});
        oscuridad.setFillColor(sf::Color(0, 3, 8, static_cast<std::uint8_t>((24.0f + falla * 72.0f) * intensidad)));
        ventana.draw(oscuridad);

        float destello = std::max(0.0f, std::sin(tiempo * 17.0f) - 0.94f) / 0.06f;
        if (destello > 0.0f && falla < 0.9f) {
            sf::RectangleShape luzFria({1280.0f, 720.0f});
            luzFria.setFillColor(sf::Color(28, 48, 62, static_cast<std::uint8_t>(16.0f * destello * intensidad)));
            ventana.draw(luzFria);
        }
    }

    void dibujarEfectosTenebrososPortada(const sf::Transform& transformacion, float escala, float tiempo) {
        float cicloLuz = std::fmod(tiempo, 6.4f);
        bool luzEncendida = !(cicloLuz > 1.18f && cicloLuz < 1.44f) &&
                            !(cicloLuz > 1.62f && cicloLuz < 2.08f) &&
                            !(cicloLuz > 4.72f && cicloLuz < 5.26f);
        float oscuridadBase = luzEncendida ? 26.0f : 105.0f;
        oscuridadBase += luzEncendida ? 8.0f * std::sin(tiempo * 2.1f) : 0.0f;

        sf::RectangleShape oscuridad({1280.0f, 720.0f});
        oscuridad.setFillColor(sf::Color(0, 2, 7, static_cast<std::uint8_t>(oscuridadBase)));
        ventana.draw(oscuridad);

        float cicloOjos = std::fmod(tiempo + 0.37f, 4.9f);
        bool ojosEncendidos = !(cicloOjos > 0.92f && cicloOjos < 1.18f) &&
                              !(cicloOjos > 2.74f && cicloOjos < 3.22f) &&
                              !(cicloOjos > 4.15f && cicloOjos < 4.34f);
        float pulsoOjos = ojosEncendidos
            ? 0.72f + 0.19f * std::sin(tiempo * 6.2f) + 0.09f * std::sin(tiempo * 15.0f)
            : 0.0f;

        for (const sf::Vector2f ojoTextura : {sf::Vector2f(617.0f, 379.0f), sf::Vector2f(678.0f, 379.0f)}) {
            sf::Vector2f ojo = transformacion.transformPoint(ojoTextura);

            if (!ojosEncendidos) {
                sf::CircleShape sombra(15.0f * escala);
                sombra.setOrigin({15.0f * escala, 15.0f * escala});
                sombra.setScale({0.8f, 1.0f});
                sombra.setPosition(ojo);
                sombra.setFillColor(sf::Color(11, 0, 0, 238));
                ventana.draw(sombra);
                continue;
            }

            sf::CircleShape halo(38.0f * escala);
            halo.setOrigin({38.0f * escala, 38.0f * escala});
            halo.setScale({0.82f, 1.0f});
            halo.setPosition(ojo);
            halo.setFillColor(sf::Color(230, 0, 0, static_cast<std::uint8_t>(47.0f * pulsoOjos)));
            ventana.draw(halo);

            sf::CircleShape nucleo(10.0f * escala);
            nucleo.setOrigin({10.0f * escala, 10.0f * escala});
            nucleo.setPosition(ojo);
            nucleo.setFillColor(sf::Color(255, 35, 24, static_cast<std::uint8_t>(190.0f * pulsoOjos)));
            ventana.draw(nucleo);
        }

        float cicloLetrero = std::fmod(tiempo + 1.1f, 5.35f);
        bool letreroEncendido = !(cicloLetrero > 1.36f && cicloLetrero < 1.68f) &&
                                !(cicloLetrero > 3.18f && cicloLetrero < 3.72f) &&
                                !(cicloLetrero > 4.66f && cicloLetrero < 4.82f);
        sf::Vector2f centroLetrero = transformacion.transformPoint({1520.0f, 565.0f});

        if (letreroEncendido) {
            float pulso = 0.65f + 0.22f * std::sin(tiempo * 5.5f);
            sf::RectangleShape resplandor({270.0f * escala, 105.0f * escala});
            resplandor.setOrigin({135.0f * escala, 52.5f * escala});
            resplandor.setPosition(centroLetrero);
            resplandor.setRotation(sf::degrees(6.0f));
            resplandor.setFillColor(sf::Color(30, 230, 75, static_cast<std::uint8_t>(24.0f * pulso)));
            ventana.draw(resplandor);
        } else {
            sf::RectangleShape apagado({270.0f * escala, 105.0f * escala});
            apagado.setOrigin({135.0f * escala, 52.5f * escala});
            apagado.setPosition(centroLetrero);
            apagado.setRotation(sf::degrees(6.0f));
            apagado.setFillColor(sf::Color(0, 4, 1, 145));
            ventana.draw(apagado);
        }

        float destello = std::max(0.0f, std::sin(tiempo * 19.0f) - 0.965f) / 0.035f;
        if (destello > 0.0f && luzEncendida) {
            sf::RectangleShape luzFria({1280.0f, 720.0f});
            luzFria.setFillColor(sf::Color(35, 55, 72, static_cast<std::uint8_t>(18.0f * destello)));
            ventana.draw(luzFria);
        }
    }

    void renderizarTransicionSala3() {
        ventana.clear(sf::Color::Black);
        ventana.setView(vistaInterfaz);

        if (!spriteMenuPrincipal.has_value()) {
            iniciarPartidaDesdeMenu();
            ventana.display();
            return;
        }

        float tiempo = relojTransicionSala3.getElapsedTime().asSeconds();
        float progreso = std::clamp(tiempo / 2.75f, 0.0f, 1.0f);
        float progresoZoom = std::clamp((tiempo - 0.62f) / 1.88f, 0.0f, 1.0f);
        progresoZoom = progresoZoom * progresoZoom * (3.0f - 2.0f * progresoZoom);
        float progresoRuptura = std::clamp((tiempo - 0.16f) / 1.05f, 0.0f, 1.0f);
        progresoRuptura = 1.0f - std::pow(1.0f - progresoRuptura, 3.0f);

        sf::Sprite escena(texturaMenuPrincipal);
        sf::Vector2f escalaBase = spriteMenuPrincipal->getScale();
        sf::Vector2f posicionBase = spriteMenuPrincipal->getPosition();
        float aumento = 1.0f + 2.25f * progresoZoom;
        sf::Vector2f escalaActual = escalaBase * aumento;
        sf::Vector2f objetivoTextura(1480.0f, 425.0f);
        sf::Vector2f posicionObjetivo(
            640.0f - objetivoTextura.x * escalaActual.x,
            360.0f - objetivoTextura.y * escalaActual.y
        );
        sf::Vector2f posicionActual = posicionBase + (posicionObjetivo - posicionBase) * progresoZoom;

        float sacudida = std::max(0.0f, 1.0f - tiempo / 0.28f);
        posicionActual.x += std::sin(tiempo * 76.0f) * 4.0f * sacudida;
        posicionActual.y += std::cos(tiempo * 61.0f) * 2.5f * sacudida;
        escena.setScale(escalaActual);
        escena.setPosition(posicionActual);
        ventana.draw(escena);

        const sf::Transform transformacion = escena.getTransform();
        dibujarZonaOscuraTransformada(transformacion, {{1280.0f, 392.0f}, {392.0f, 80.0f}}, sf::Color(3, 4, 5, 225));
        dibujarZonaOscuraTransformada(transformacion, {{1395.0f, 478.0f}, {277.0f, 185.0f}}, sf::Color(4, 5, 6, 238));

        sf::Vector2f anclaIzquierda = transformacion.transformPoint({1215.0f, 385.0f});
        sf::Vector2f anclaDerecha = transformacion.transformPoint({1665.0f, 405.0f});
        sf::Vector2f centro = transformacion.transformPoint({1445.0f, 418.0f});
        float caida = progresoRuptura * 76.0f * escalaActual.y;
        sf::Vector2f extremoIzquierdo = centro + sf::Vector2f(-18.0f * escalaActual.x, caida);
        sf::Vector2f extremoDerecho = centro + sf::Vector2f(22.0f * escalaActual.x, caida * 0.82f);
        dibujarSegmentoCadena(anclaIzquierda, extremoIzquierdo, escalaActual.x, 245.0f);
        dibujarSegmentoCadena(anclaDerecha, extremoDerecho, escalaActual.x, 245.0f);

        float balanceo = std::sin((tiempo - 0.12f) * 5.2f) * 8.5f * std::exp(-tiempo * 0.62f);
        sf::Vector2f anclaLetrero = transformacion.transformPoint({1533.0f, 486.0f});
        sf::Sprite letrero(texturaMenuPrincipal);
        letrero.setTextureRect(sf::IntRect(sf::Vector2i(1398, 477), sf::Vector2i(274, 188)));
        letrero.setOrigin({137.0f, 9.0f});
        letrero.setScale(escalaActual);
        letrero.setPosition(anclaLetrero + sf::Vector2f(0.0f, caida * 0.38f));
        letrero.setRotation(sf::degrees(balanceo));
        ventana.draw(letrero);

        if (tiempo < 0.32f) {
            float intensidad = 1.0f - tiempo / 0.32f;
            sf::Vector2f puntoRuptura = transformacion.transformPoint({1445.0f, 418.0f});
            for (int i = 0; i < 5; ++i) {
                float angulo = static_cast<float>(i) * 1.31f + tiempo * 5.0f;
                float distancia = (12.0f + i * 4.0f) * escalaActual.x;
                sf::CircleShape chispa(std::max(1.0f, 1.7f * escalaActual.x));
                chispa.setPosition(puntoRuptura + sf::Vector2f(std::cos(angulo), std::sin(angulo)) * distancia);
                chispa.setFillColor(sf::Color(220, 175, 92, static_cast<std::uint8_t>(150.0f * intensidad)));
                ventana.draw(chispa);
            }
        }

        dibujarEfectosTenebrososMenu(transformacion, escalaActual.x, tiempo + 1.2f, 1.0f);

        if (progreso > 0.82f) {
            float alpha = std::clamp((progreso - 0.82f) / 0.18f, 0.0f, 1.0f);
            sf::RectangleShape fundido({1280.0f, 720.0f});
            fundido.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(255.0f * alpha)));
            ventana.draw(fundido);
        }

        ventana.display();
    }

    bool clicEnBotonPuerta(sf::Vector2i posicionClick, bool esIzquierda) const {
        sf::FloatRect zona = esIzquierda
            ? visualPuertaIzquierda.getGlobalBounds()
            : visualPuertaDerecha.getGlobalBounds();

        sf::Vector2f posicionBoton = obtenerPosicionIndicadorPuerta(zona, esIzquierda);

        sf::Vector2i esquinaSuperior = ventana.mapCoordsToPixel(
            posicionBoton - sf::Vector2f(10.0f, 10.0f),
            vistaOficina
        );
        sf::Vector2i esquinaInferior = ventana.mapCoordsToPixel(
            posicionBoton + sf::Vector2f(34.0f, 48.0f),
            vistaOficina
        );

        return posicionClick.x >= esquinaSuperior.x &&
               posicionClick.x <= esquinaInferior.x &&
               posicionClick.y >= esquinaSuperior.y &&
               posicionClick.y <= esquinaInferior.y;
    }

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
                continue;
            }

            if (estadoJuego == EstadoJuego::Portada) {
                if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                    if (tecla->code == sf::Keyboard::Key::Space) {
                        estadoJuego = EstadoJuego::MenuPrincipal;
                    }
                }
                continue;
            }

            if (estadoJuego == EstadoJuego::MenuPrincipal) {
                if (const auto* click = evento->getIf<sf::Event::MouseButtonPressed>()) {
                    if (click->button == sf::Mouse::Button::Left && clicEnJugar(click->position)) {
                        iniciarTransicionSala3();
                    } else if (click->button == sf::Mouse::Button::Left && clicEnInstrucciones(click->position)) {
                        if (sonidoMenuPrincipal.has_value()) {
                            sonidoMenuPrincipal->stop();
                        }
                        estadoJuego = EstadoJuego::Instrucciones;
                    } else if (click->button == sf::Mouse::Button::Left && clicEnSalirMenu(click->position)) {
                        ventana.close();
                    }
                } else if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                    if (tecla->code == sf::Keyboard::Key::Enter ||
                        tecla->code == sf::Keyboard::Key::Space) {
                        iniciarTransicionSala3();
                    }
                }
                continue;
            }

            if (estadoJuego == EstadoJuego::TransicionSala3) {
                continue;
            }

            if (estadoJuego == EstadoJuego::Instrucciones) {
                if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                    if (tecla->code == sf::Keyboard::Key::Escape) {
                        estadoJuego = EstadoJuego::MenuPrincipal;
                    }
                }
                continue;
            }

            if (estadoJuego == EstadoJuego::Pausa) {
                if (const auto* click = evento->getIf<sf::Event::MouseButtonPressed>()) {
                    if (click->button == sf::Mouse::Button::Left && clicEnBotonPausa(click->position, 0)) {
                        continuarPartida();
                    } else if (click->button == sf::Mouse::Button::Left && clicEnBotonPausa(click->position, 1)) {
                        resetearPartida();
                    } else if (click->button == sf::Mouse::Button::Left && clicEnBotonPausa(click->position, 2)) {
                        volverAlMenuPrincipal();
                    }
                } else if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                    if (tecla->code == sf::Keyboard::Key::Escape) {
                        continuarPartida();
                    }
                }
                continue;
            }

            if (estadoJuego == EstadoJuego::GameOver) {
                if (const auto* click = evento->getIf<sf::Event::MouseButtonPressed>()) {
                    if (click->button == sf::Mouse::Button::Left && clicEnSalirGameOver(click->position)) {
                        volverAlMenuPrincipal();
                    }
                } else if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                    if (tecla->code == sf::Keyboard::Key::R) {
                        resetearPartida();
                    }
                }
                continue;
            }

            if (const auto* click = evento->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button == sf::Mouse::Button::Left &&
                    estadoJuego == EstadoJuego::Jugando &&
                    !jugador.esMonitorAbierto() &&
                    !controlesBloqueados) {
                    if (clicEnBotonPuerta(click->position, true)) {
                        jugador.alternarPuertaIzquierda();
                        reproducirSonido("puerta", 34.0f);
                        continue;
                    }

                    if (clicEnBotonPuerta(click->position, false)) {
                        jugador.alternarPuertaDerecha();
                        reproducirSonido("puerta", 34.0f);
                        continue;
                    }
                }
            }

            if (const auto* botonPresionado = evento->getIf<sf::Event::KeyPressed>()) {
                if (estadoJuego == EstadoJuego::Jumpscare || estadoJuego == EstadoJuego::Victoria) {
                    continue;
                }

                if (botonPresionado->code == sf::Keyboard::Key::Escape &&
                    estadoJuego == EstadoJuego::Jugando) {
                    pausarPartida();
                    continue;
                }

                if (controlesBloqueados) {
                    continue;
                }

                if (botonPresionado->code == sf::Keyboard::Key::A) {
                    jugador.alternarPuertaIzquierda();
                    reproducirSonido("puerta", 34.0f);
                }
                if (botonPresionado->code == sf::Keyboard::Key::D) {
                    jugador.alternarPuertaDerecha();
                    reproducirSonido("puerta", 34.0f);
                }

                if (botonPresionado->code == sf::Keyboard::Key::Space) {
                    jugador.alternarMonitor();
                    reproducirSonido("monitor", 26.0f);
                }


                if (botonPresionado->code == sf::Keyboard::Key::Q) {
                    bool estabaApagada = !jugador.esLuzIzquierdaEncendida();
                    jugador.alternarLuzIzquierda();
                    if (estabaApagada && jugador.esLuzIzquierdaEncendida()) {
                        reproducirSonido("luz_roja", 40.0f);
                    }
                }
                if (botonPresionado->code == sf::Keyboard::Key::E) {
                    bool estabaApagada = !jugador.esLuzDerechaEncendida();
                    jugador.alternarLuzDerecha();
                    if (estabaApagada && jugador.esLuzDerechaEncendida()) {
                        reproducirSonido("luz_roja", 40.0f);
                    }
                }

                if (jugador.esMonitorAbierto()) {
                    if (botonPresionado->code == sf::Keyboard::Key::Num1) { monitor.cambiarCamara(TipoCamara::CAM_01_DULCERIA); activarInterferenciaMonitor(); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num2) { monitor.cambiarCamara(TipoCamara::CAM_02_PASILLO_A); activarInterferenciaMonitor(); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num3) { monitor.cambiarCamara(TipoCamara::CAM_03_PASILLO_B); activarInterferenciaMonitor(); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num4) { monitor.cambiarCamara(TipoCamara::CAM_04_SALAS); activarInterferenciaMonitor(); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num5) { monitor.cambiarCamara(TipoCamara::CAM_05_BANOS); activarInterferenciaMonitor(); reproducirSonido("camara", 24.0f); }
                }
            }
        }
    }

    void actualizar() {

        if (estadoJuego == EstadoJuego::Portada ||
            estadoJuego == EstadoJuego::MenuPrincipal) {
            if (sonidoMenuPrincipal.has_value() && sonidoMenuPrincipal->getStatus() == sf::SoundSource::Status::Stopped) {
                sonidoMenuPrincipal->play();
            }
            return;
        }

        if (estadoJuego == EstadoJuego::Instrucciones) {
            if (sonidoMenuPrincipal.has_value()) {
                sonidoMenuPrincipal->stop();
            }
            return;
        }

        if (estadoJuego == EstadoJuego::Pausa) {
            return;
        }

        if (estadoJuego == EstadoJuego::TransicionSala3) {
            if (relojTransicionSala3.getElapsedTime().asSeconds() >= 2.75f) {
                iniciarPartidaDesdeMenu();
            }
            return;
        }

        float dt = relojEnergia.restart().asSeconds();

        if (estadoJuego == EstadoJuego::Jumpscare) {
            if (relojEstado.getElapsedTime().asSeconds() >= 2.5f) {
                pasarAGameOver();
            }
            return;
        }

        if (estadoJuego == EstadoJuego::Victoria) {
            if (relojVictoria.getElapsedTime().asSeconds() >= duracionVictoria) {
                ventana.close();
            }
            return;
        }

        if (estadoJuego == EstadoJuego::GameOver) {
            return;
        }

        if (estadoJuego == EstadoJuego::AtaquePendiente) {
            if (relojEstado.getElapsedTime().asSeconds() >= retrasoAtaque) {
                iniciarJumpscare();
                return;
            }

            if (!jugador.esMonitorAbierto()) {
                actualizarVistaOficinaPorMouse(dt);
            }
            return;
        }


        if (!introNoche1Reproducido && horaActual == 12) {
            iniciarIntroNoche1();
        }

        if (introNoche1Activa &&
            tiempoIntroAntesPausa + relojIntroNoche1.getElapsedTime().asSeconds() >= duracionIntroNoche1) {
            introNoche1Activa = false;
            if (sonidoIntroNoche1.has_value() &&
                sonidoIntroNoche1->getStatus() == sf::SoundSource::Status::Playing) {
                sonidoIntroNoche1->stop();
            }
            if (sonidoAmbiente.has_value() &&
                sonidoAmbiente->getStatus() != sf::SoundSource::Status::Playing) {
                sonidoAmbiente->play();
            }
        }

        acumuladorHora += dt;
        if (acumuladorHora >= tiempoPorHora) {
            acumuladorHora = 0.0f;
            if (horaActual == 12) horaActual = 1;
            else horaActual++;

            if (horaActual == 6) {
                iniciarVictoria();
                return;
            }
        }

        jugador.bajarEnergia(dt, obtenerTasaConsumoBasePorNoche());
        actualizarInterferenciaMonitor();

        EventoPuerta eventoGobo = gobo.actualizarEstadoPuerta(dt, jugador.esPuertaIzquierdaCerrada(), jugador.esMonitorAbierto(), jugador.esLuzIzquierdaEncendida());
        EventoPuerta eventoDirector = director.actualizarEstadoPuerta(dt, jugador.esPuertaDerechaCerrada(), jugador.esMonitorAbierto(), jugador.esLuzDerechaEncendida());
        EventoPuerta eventoPopy = popy.actualizarEstadoPuerta(dt, jugador.esPuertaIzquierdaCerrada(), jugador.esMonitorAbierto(), jugador.esLuzIzquierdaEncendida());
        EventoPuerta eventoUsher = usher.actualizarEstadoPuerta(dt, jugador.esPuertaDerechaCerrada(), jugador.esMonitorAbierto(), false);

        if (eventoGobo == EventoPuerta::Golpe) reproducirSonidoEspacial("golpe_puerta", {-1.0f, 0.0f, 0.0f}, 68.0f);
        if (eventoDirector == EventoPuerta::Golpe) reproducirSonidoEspacial("golpe_puerta", {1.0f, 0.0f, 0.0f}, 68.0f);
        if (eventoPopy == EventoPuerta::Golpe) {
            jugador.descontarEnergiaPorcentaje(6.0f);
            reproducirSonidoEspacial("golpe_puerta", {-1.0f, 0.0f, 0.0f}, 70.0f);
        }
        if (eventoUsher == EventoPuerta::Golpe) reproducirSonidoEspacial("golpe_puerta", {1.0f, 0.0f, 0.0f}, 66.0f);
        if (eventoGobo == EventoPuerta::Entrada) { iniciarAtaque(gobo.getNombre()); return; }
        if (eventoDirector == EventoPuerta::Entrada) { iniciarAtaque(director.getNombre()); return; }
        if (eventoPopy == EventoPuerta::Entrada) { iniciarAtaque(popy.getNombre()); return; }
        if (eventoUsher == EventoPuerta::Entrada) { iniciarAtaque(usher.getNombre()); return; }
        if (jugador.esLuzIzquierdaEncendida() && gobo.esEnLaPuerta()) {
            tiempoLuzIzquierdaActiva += dt;
            if (tiempoLuzIzquierdaActiva >= 0.8f) {
                gobo.resetear();
                tiempoLuzIzquierdaActiva = 0.0f;
            }
        } else {
            tiempoLuzIzquierdaActiva = 0.0f;
        }

        if (jugador.esLuzDerechaEncendida() && director.esEnLaPuerta()) {
            tiempoLuzDerechaActiva += dt;
            if (tiempoLuzDerechaActiva >= 0.8f) {
                director.resetear();
                tiempoLuzDerechaActiva = 0.0f;
            }
        } else {
            tiempoLuzDerechaActiva = 0.0f;
        }

        if (jugador.getEnergia() <= 0.0f) {
            pasarAGameOver();
            return;
        }

        if (introNoche1Activa) {
            acumuladorTickIA = 0.0f;
        } else {
            acumuladorTickIA += dt;
            while (acumuladorTickIA >= intervaloTickIA) {
                acumuladorTickIA -= intervaloTickIA;
                procesarTickIA();
                if (estadoJuego != EstadoJuego::Jugando) {
                    return;
                }
            }
        }

        #if 0

        bool puedeMoverIA = !introNoche1Activa;
        CurvaDificultadHoraria curvaIA = obtenerCurvaDificultadHoraria();
        bool huboMovimientoIA = false;
        huboMovimientoIA |= gobo.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA, curvaIA.intervaloIntentos, curvaIA.bonoDado);
        huboMovimientoIA |= director.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA, curvaIA.intervaloIntentos, curvaIA.bonoDado);
        huboMovimientoIA |= popy.actualizarIA(dt, jugador.esPuertaDerechaCerrada(), puedeMoverIA, curvaIA.intervaloIntentos, curvaIA.bonoDado);
        huboMovimientoIA |= usher.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA, curvaIA.intervaloIntentos, curvaIA.bonoDado);
        huboMovimientoIA |= stub.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA, curvaIA.intervaloIntentos, curvaIA.bonoDado);

        if (huboMovimientoIA) {
            activarInterferenciaMonitor();
        }


        if (gobo.esEstaAdentro()) {
            iniciarAtaque(gobo.getNombre());
            return;
        }
        if (director.esEstaAdentro()) {
            iniciarAtaque(director.getNombre());
            return;
        }
        if (popy.esEstaAdentro()) {
            iniciarAtaque(popy.getNombre());
            return;
        }
        if (usher.esEstaAdentro()) {
            iniciarAtaque(usher.getNombre());
            return;
        }
        #endif


        float porcentajeEnergia = jugador.getEnergia() / 100.0f;
        if (porcentajeEnergia < 0.0f) porcentajeEnergia = 0.0f;
        barraEnergiaFrente.setSize(sf::Vector2f(250.0f * porcentajeEnergia, 20.0f));

        if (jugador.getEnergia() < 25.0f) {
            barraEnergiaFrente.setFillColor(sf::Color::Red);
        } else {
            barraEnergiaFrente.setFillColor(sf::Color::Green);
        }


        actualizarTextosHUD();


        if (!jugador.esMonitorAbierto()) {
            actualizarVistaOficinaPorMouse(dt);
        }

        if (relojTerminal.getElapsedTime().asSeconds() >= 1.0f) {
            std::cout << "=========================================\n";
            std::cout << "  RELOJ DE LA NOCHE : " << horaActual << " AM \n";
            std::cout << "  ENERGIA RESTANTE  : " << static_cast<int>(jugador.getEnergia()) << "%\n";
            std::cout << "=========================================\n";


            std::cout << "  AMENAZAS ACTIVAS:\n";
            if (gobo.esEnLaPuerta()) std::cout << "    [!] " << gobo.getNombre() << " EN LA PUERTA IZQ\n";
            if (director.esEnLaPuerta()) std::cout << "    [!] " << director.getNombre() << " EN LA PUERTA DER\n";
            if (usher.esEnLaPuerta()) std::cout << "    [!] " << usher.getNombre() << " EN LA PUERTA IZQ\n";
            if (popy.esEnLaPuerta()) std::cout << "    [!] " << popy.getNombre() << " EN LA PUERTA DER\n";

            if (!gobo.esEnLaPuerta() && !director.esEnLaPuerta() && !popy.esEnLaPuerta() &&
                !usher.esEnLaPuerta()) {
                std::cout << "    [OK] Ninguna amenaza activa\n";
            }
            std::cout << "=========================================\n";
            relojTerminal.restart();
        }

        if (spriteOficina.has_value()) {
            if (jugador.getEnergia() <= 0.0f) {
                spriteOficina.value().setColor(sf::Color(10, 10, 30));
            } else if (gobo.esEnLaPuerta() || director.esEnLaPuerta() || usher.esEnLaPuerta()) {
                spriteOficina.value().setColor(sf::Color(255, 180, 180));
            } else {
                spriteOficina.value().setColor(sf::Color::White);
            }
        }
    }

    void dibujarTexturaPantallaCompleta(sf::Texture& textura, bool ajustarCompleta = false) {
        ventana.setView(vistaInterfaz);
        sf::Sprite sprite(textura);
        const auto tamTextura = textura.getSize();
        const float anchoDestino = ajustarCompleta ? 1152.0f : 1280.0f;
        const float altoDestino = ajustarCompleta ? 648.0f : 720.0f;
        float escalaX = anchoDestino / static_cast<float>(tamTextura.x);
        float escalaY = altoDestino / static_cast<float>(tamTextura.y);
        float escala = ajustarCompleta ? std::min(escalaX, escalaY) : std::max(escalaX, escalaY);
        sprite.setScale({escala, escala});
        sprite.setPosition({
            (1280.0f - static_cast<float>(tamTextura.x) * escala) / 2.0f,
            (720.0f - static_cast<float>(tamTextura.y) * escala) / 2.0f
        });
        ventana.draw(sprite);
    }

    void renderizarJumpscare() {
        ventana.clear(sf::Color::Black);
        auto textura = texturasJumpscare.find(atacanteActual);
        if (textura != texturasJumpscare.end()) {
            dibujarTexturaPantallaCompleta(textura->second, true);
        } else {
            sf::RectangleShape fondo(sf::Vector2f(1280.0f, 720.0f));
            fondo.setFillColor(sf::Color(120, 0, 0));
            ventana.setView(vistaInterfaz);
            ventana.draw(fondo);
        }
        ventana.display();
    }

    void renderizarGameOver() {
        ventana.clear(sf::Color::Black);
        ventana.setView(vistaInterfaz);

        for (int y = 0; y < 720; y += 10) {
            sf::RectangleShape linea(sf::Vector2f(1280.0f, 2.0f));
            linea.setPosition({0.0f, static_cast<float>(y)});
            int brillo = 12 + (std::rand() % 32);
            linea.setFillColor(sf::Color(brillo, 0, 0, 180));
            ventana.draw(linea);
        }

        sf::RectangleShape resplandor(sf::Vector2f(1280.0f, 720.0f));
        resplandor.setFillColor(sf::Color(80, 0, 0, 55));
        ventana.draw(resplandor);

        sf::RectangleShape panel(sf::Vector2f(650.0f, 390.0f));
        panel.setPosition({315.0f, 155.0f});
        panel.setFillColor(sf::Color(5, 5, 8, 242));
        panel.setOutlineColor(sf::Color(190, 15, 15));
        panel.setOutlineThickness(5.0f);
        ventana.draw(panel);

        sf::RectangleShape botonSalir(sf::Vector2f(370.0f, 70.0f));
        botonSalir.setPosition({455.0f, 470.0f});
        botonSalir.setFillColor(sf::Color(45, 5, 8, 245));
        botonSalir.setOutlineColor(sf::Color(220, 45, 45));
        botonSalir.setOutlineThickness(3.0f);
        ventana.draw(botonSalir);

        if (fuenteUICargada) {
            sf::Text titulo(fuenteUI, "GAME OVER", 88);
            titulo.setFillColor(sf::Color(235, 25, 25));
            titulo.setOutlineColor(sf::Color::Black);
            titulo.setOutlineThickness(4.0f);
            titulo.setStyle(sf::Text::Bold);
            sf::FloatRect limitesTitulo = titulo.getLocalBounds();
            titulo.setOrigin({limitesTitulo.position.x + limitesTitulo.size.x / 2.0f, limitesTitulo.position.y});
            titulo.setPosition({640.0f, 190.0f});
            ventana.draw(titulo);

            sf::Text reiniciar(fuenteUI, "REINICIAR JUEGO", 34);
            reiniciar.setFillColor(sf::Color::White);
            reiniciar.setStyle(sf::Text::Bold);
            sf::FloatRect limitesReiniciar = reiniciar.getLocalBounds();
            reiniciar.setOrigin({limitesReiniciar.position.x + limitesReiniciar.size.x / 2.0f, limitesReiniciar.position.y});
            reiniciar.setPosition({640.0f, 325.0f});
            ventana.draw(reiniciar);

            sf::Text tecla(fuenteUI, "Presiona la letra R", 25);
            tecla.setFillColor(sf::Color(205, 205, 205));
            sf::FloatRect limitesTecla = tecla.getLocalBounds();
            tecla.setOrigin({limitesTecla.position.x + limitesTecla.size.x / 2.0f, limitesTecla.position.y});
            tecla.setPosition({640.0f, 380.0f});
            ventana.draw(tecla);

            sf::Text salir(fuenteUI, "SALIR AL MENU", 30);
            salir.setFillColor(sf::Color(255, 225, 225));
            salir.setStyle(sf::Text::Bold);
            sf::FloatRect limitesSalir = salir.getLocalBounds();
            salir.setOrigin({limitesSalir.position.x + limitesSalir.size.x / 2.0f, limitesSalir.position.y + limitesSalir.size.y / 2.0f});
            salir.setPosition({640.0f, 505.0f});
            ventana.draw(salir);
        }

        ventana.display();
    }

    void renderizarVictoria() {
        float tiempo = relojVictoria.getElapsedTime().asSeconds();
        ventana.clear(sf::Color(8, 10, 12));
        ventana.setView(vistaInterfaz);

        for (int y = 0; y < 720; y += 10) {
            sf::RectangleShape linea(sf::Vector2f(1280.0f, 2.0f));
            linea.setPosition({0.0f, static_cast<float>(y)});
            int brillo = 18 + ((y / 10) % 2) * 10;
            linea.setFillColor(sf::Color(brillo, brillo, brillo + 6, 120));
            ventana.draw(linea);
        }

        sf::RectangleShape resplandor(sf::Vector2f(1280.0f, 720.0f));
        std::uint8_t alpha = tiempo < 2.0f ? static_cast<std::uint8_t>(80.0f * (tiempo / 2.0f)) : 80;
        resplandor.setFillColor(sf::Color(240, 225, 170, alpha));
        ventana.draw(resplandor);

        sf::RectangleShape panelReloj(sf::Vector2f(520.0f, 210.0f));
        panelReloj.setPosition({380.0f, 160.0f});
        panelReloj.setFillColor(sf::Color(4, 6, 8, 235));
        panelReloj.setOutlineColor(tiempo < 1.8f ? sf::Color(210, 210, 210) : sf::Color(255, 215, 90));
        panelReloj.setOutlineThickness(5.0f);
        ventana.draw(panelReloj);

        std::string horaVictoria = tiempo < 1.6f ? "5 AM" : "6 AM";
        if (tiempo >= 1.45f && tiempo < 1.8f && static_cast<int>(tiempo * 18.0f) % 2 == 0) {
            horaVictoria = "";
        }

        if (fuenteUICargada) {
            sf::Text reloj(fuenteUI, horaVictoria, 96);
            reloj.setFillColor(tiempo < 1.8f ? sf::Color(240, 240, 240) : sf::Color(255, 230, 120));
            reloj.setOutlineColor(sf::Color::Black);
            reloj.setOutlineThickness(4.0f);
            reloj.setStyle(sf::Text::Bold);
            sf::FloatRect limitesReloj = reloj.getLocalBounds();
            reloj.setOrigin({limitesReloj.position.x + limitesReloj.size.x / 2.0f, limitesReloj.position.y + limitesReloj.size.y / 2.0f});
            reloj.setPosition({640.0f, 260.0f});
            ventana.draw(reloj);

            if (tiempo >= 2.2f) {
                sf::Text turno(fuenteUI, "TURNO SUPERADO", 54);
                turno.setFillColor(sf::Color(255, 245, 210));
                turno.setOutlineColor(sf::Color(80, 55, 0));
                turno.setOutlineThickness(3.0f);
                turno.setStyle(sf::Text::Bold);
                sf::FloatRect limitesTurno = turno.getLocalBounds();
                turno.setOrigin({limitesTurno.position.x + limitesTurno.size.x / 2.0f, limitesTurno.position.y + limitesTurno.size.y / 2.0f});
                turno.setPosition({640.0f, 455.0f});
                ventana.draw(turno);
            }

            if (tiempo >= 3.1f) {
                sf::Text detalle(fuenteUI, "6:00 AM", 30);
                detalle.setFillColor(sf::Color(210, 230, 220));
                sf::FloatRect limitesDetalle = detalle.getLocalBounds();
                detalle.setOrigin({limitesDetalle.position.x + limitesDetalle.size.x / 2.0f, limitesDetalle.position.y + limitesDetalle.size.y / 2.0f});
                detalle.setPosition({640.0f, 520.0f});
                ventana.draw(detalle);
            }
        } else {
            sf::RectangleShape bloqueHora(sf::Vector2f(tiempo < 1.6f ? 220.0f : 300.0f, 44.0f));
            bloqueHora.setFillColor(tiempo < 1.8f ? sf::Color::White : sf::Color(255, 220, 90));
            bloqueHora.setPosition({tiempo < 1.6f ? 530.0f : 490.0f, 238.0f});
            ventana.draw(bloqueHora);

            if (tiempo >= 2.2f) {
                sf::RectangleShape bloqueTurno(sf::Vector2f(520.0f, 32.0f));
                bloqueTurno.setFillColor(sf::Color(255, 245, 210));
                bloqueTurno.setPosition({380.0f, 445.0f});
                ventana.draw(bloqueTurno);
            }
        }

        ventana.display();
    }

    void renderizar() {

        if (estadoJuego == EstadoJuego::Portada) {
            ventana.clear(sf::Color::Black);
            ventana.setView(vistaInterfaz);
            if (spritePortada.has_value()) {
                ventana.draw(spritePortada.value());
                dibujarEfectosTenebrososPortada(
                    spritePortada->getTransform(),
                    spritePortada->getScale().x,
                    relojEfectosMenu.getElapsedTime().asSeconds()
                );
            }
            if (fuenteUICargada) {
                float tiempo = relojEfectosMenu.getElapsedTime().asSeconds();
                float pulso = 0.62f + 0.38f * (0.5f + 0.5f * std::sin(tiempo * 3.0f));

                sf::RectangleShape fondoMensaje({500.0f, 38.0f});
                fondoMensaje.setOrigin({250.0f, 19.0f});
                fondoMensaje.setPosition({640.0f, 691.0f});
                fondoMensaje.setFillColor(sf::Color(0, 0, 0, 135));
                fondoMensaje.setOutlineColor(sf::Color(145, 155, 150, static_cast<std::uint8_t>(80.0f * pulso)));
                fondoMensaje.setOutlineThickness(1.0f);
                ventana.draw(fondoMensaje);

                sf::Text continuar(fuenteUI, "PULSA LA BARRA ESPACIADORA PARA CONTINUAR", 16);
                continuar.setStyle(sf::Text::Bold);
                continuar.setFillColor(sf::Color(225, 230, 225, static_cast<std::uint8_t>(255.0f * pulso)));
                continuar.setOutlineColor(sf::Color::Black);
                continuar.setOutlineThickness(2.0f);
                sf::FloatRect limites = continuar.getLocalBounds();
                continuar.setOrigin({
                    limites.position.x + limites.size.x / 2.0f,
                    limites.position.y + limites.size.y / 2.0f
                });
                continuar.setPosition({640.0f, 690.0f});
                ventana.draw(continuar);
            }
            ventana.display();
            return;
        }

        if (estadoJuego == EstadoJuego::MenuPrincipal) {
            ventana.clear(sf::Color::Black);
            ventana.setView(vistaInterfaz);
            if (spriteMenuPrincipal.has_value()) {
                ventana.draw(spriteMenuPrincipal.value());
                dibujarEfectosTenebrososMenu(
                    spriteMenuPrincipal->getTransform(),
                    spriteMenuPrincipal->getScale().x,
                    relojEfectosMenu.getElapsedTime().asSeconds(),
                    1.0f
                );
                dibujarBotonesAdicionalesMenu();
            }
            ventana.display();
            return;
        }

        if (estadoJuego == EstadoJuego::TransicionSala3) {
            renderizarTransicionSala3();
            return;
        }

        if (estadoJuego == EstadoJuego::Instrucciones) {
            renderizarInstrucciones();
            return;
        }

        if (estadoJuego == EstadoJuego::Pausa) {
            ventana.clear(sf::Color::Black);
            ventana.setView(vistaInterfaz);
            if (spriteMenuPausa.has_value()) {
                ventana.draw(spriteMenuPausa.value());
            }
            ventana.display();
            return;
        }

        if (estadoJuego == EstadoJuego::Victoria) {
            renderizarVictoria();
            return;
        }

        if (estadoJuego == EstadoJuego::Jumpscare) {
            renderizarJumpscare();
            return;
        }

        if (estadoJuego == EstadoJuego::GameOver) {
            renderizarGameOver();
            return;
        }

        if (jugador.esMonitorAbierto()) {
            int tonoVerde = 30 + static_cast<int>(monitor.getCamaraActual()) * 15;
            ventana.clear(sf::Color(10, tonoVerde, 15));
        } else {
            ventana.clear(sf::Color::Black);
        }


        ventana.setView(vistaOficina);
        if (!jugador.esMonitorAbierto()) {
            if (spriteOficina.has_value()) {
                ventana.draw(spriteOficina.value());
            }

            dibujarPuertaOficina(
                ventana,
                true,
                jugador.esPuertaIzquierdaCerrada(),
                jugador.esLuzIzquierdaEncendida());
            dibujarPuertaOficina(
                ventana,
                false,
                jugador.esPuertaDerechaCerrada(),
                jugador.esLuzDerechaEncendida());


            if (!jugador.esPuertaIzquierdaCerrada()) {
                if (gobo.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, gobo.getNombre(), true, jugador.esLuzIzquierdaEncendida());
                if (popy.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, popy.getNombre(), true, jugador.esLuzIzquierdaEncendida());
            }

            if (!jugador.esPuertaDerechaCerrada()) {
                if (director.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, director.getNombre(), false, jugador.esLuzDerechaEncendida());
                if (usher.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, usher.getNombre(), false, jugador.esLuzDerechaEncendida());
            }
        }


        ventana.setView(vistaInterfaz);
        if (jugador.esMonitorAbierto()) {
            monitor.renderizar(ventana);

            auto dibujarPersonajesEnCamara = [&](TipoCamara camara, float desplazamientoX) {
                if (!gobo.esEnLaPuerta() && gobo.getPosicionActual() == camara) monitor.dibujarPersonaje(ventana, "Gobo", camara, desplazamientoX);
                if (!director.esEnLaPuerta() && director.getPosicionActual() == camara) monitor.dibujarPersonaje(ventana, "Director", camara, desplazamientoX);
                if (!popy.esEnLaPuerta() && popy.getPosicionActual() == camara) monitor.dibujarPersonaje(ventana, "Popy", camara, desplazamientoX);
                if (!usher.esEnLaPuerta() && usher.getPosicionActual() == camara) monitor.dibujarPersonaje(ventana, "TheUsher", camara, desplazamientoX);
            };

            dibujarPersonajesEnCamara(
                monitor.getCamaraActual(),
                monitor.getDesplazamientoHorizontalActual());

            if (interferenciaMonitorActiva) {
                dibujarInterferenciaMonitor();
            }
        } else {
            ventana.draw(barraEnergiaFondo);
            ventana.draw(barraEnergiaFrente);

            actualizarBloquesConsumoHUD();
            for (int i = 0; i < 4; i++) {
                ventana.draw(bloquesConsumo[i]);
            }

            if (textoEnergiaHUD.has_value()) ventana.draw(textoEnergiaHUD.value());
            if (textoRelojHUD.has_value()) ventana.draw(textoRelojHUD.value());
        }

        ventana.display();
    }

    void dibujarPuertaOficina(sf::RenderWindow& ventana, bool esIzquierda, bool cerrada, bool luzEncendida) {
        sf::FloatRect zona = esIzquierda
            ? visualPuertaIzquierda.getGlobalBounds()
            : visualPuertaDerecha.getGlobalBounds();

        float x = zona.position.x;
        float y = zona.position.y;
        float w = zona.size.x;
        float h = zona.size.y;

        if (!cerrada) {
            sf::RectangleShape hueco({w, h});
            hueco.setPosition({x, y});
            hueco.setFillColor(sf::Color(10, 12, 16, 235));
            hueco.setOutlineThickness(0.0f);
            ventana.draw(hueco);

            dibujarFondoPasilloPuerta(ventana, zona, esIzquierda, luzEncendida);
        }

        sf::IntRect recorte = obtenerRecortePuertaOficina(esIzquierda);

        sf::Sprite puerta(texturaOficina);
        puerta.setTextureRect(recorte);
        sf::Vector2f escalaOficina = obtenerEscalaOficina();
        float anchoPuerta = static_cast<float>(recorte.size.x) * escalaOficina.x;

        puerta.setScale(escalaOficina);

        float puertaX = static_cast<float>(recorte.position.x) * escalaOficina.x;
        float puertaY = static_cast<float>(recorte.position.y) * escalaOficina.y;
        float desplazamientoAbierta = anchoPuerta * 0.82f;
        if (!cerrada) {
            puertaX += esIzquierda ? -desplazamientoAbierta : desplazamientoAbierta;
        }
        puerta.setPosition({puertaX, puertaY});
        ventana.draw(puerta);

        sf::RectangleShape marcoExterior({w, h});
        marcoExterior.setPosition({x, y});
        marcoExterior.setFillColor(sf::Color::Transparent);
        marcoExterior.setOutlineColor(sf::Color(5, 6, 7, 245));
        marcoExterior.setOutlineThickness(5.0f);
        ventana.draw(marcoExterior);

        if (luzEncendida) dibujarResplandorLuzPuerta(ventana, zona, esIzquierda, cerrada);

        sf::RectangleShape boton({24.0f, 38.0f});
        boton.setPosition(obtenerPosicionIndicadorPuerta(zona, esIzquierda));
        boton.setFillColor(sf::Color(9, 10, 12, 238));
        boton.setOutlineColor(sf::Color(34, 36, 40, 235));
        boton.setOutlineThickness(2.0f);
        ventana.draw(boton);

        sf::RectangleShape luzBoton({12.0f, 12.0f});
        luzBoton.setPosition({
            boton.getPosition().x + 6.0f,
            boton.getPosition().y + 6.0f
        });
        luzBoton.setFillColor(cerrada ? sf::Color(220, 48, 36, 245) : sf::Color(58, 210, 86, 245));
        ventana.draw(luzBoton);
    }

    void dibujarFondoPasilloPuerta(sf::RenderWindow& ventana, const sf::FloatRect& zona, bool esIzquierda, bool luzEncendida) {
        const sf::Texture* texturaPasillo = nullptr;
        if (esIzquierda && pasilloIzquierdoCargado) {
            texturaPasillo = &texturaPasilloIzquierda;
        } else if (!esIzquierda && pasilloDerechoCargado) {
            texturaPasillo = &texturaPasilloDerecha;
        }

        if (texturaPasillo == nullptr) {
            return;
        }

        float pulso = luzEncendida ? obtenerPulsoLuzPasillo(esIzquierda) : 0.0f;
        sf::Sprite fondo(*texturaPasillo);
        sf::Vector2u tamano = texturaPasillo->getSize();
        float proporcionZona = zona.size.x / zona.size.y;
        float proporcionTextura = static_cast<float>(tamano.x) / static_cast<float>(tamano.y);
        sf::IntRect recorte(
            sf::Vector2i(0, 0),
            sf::Vector2i(static_cast<int>(tamano.x), static_cast<int>(tamano.y))
        );

        if (proporcionTextura > proporcionZona) {
            int anchoRecorte = static_cast<int>(static_cast<float>(tamano.y) * proporcionZona);
            recorte.position.x = (static_cast<int>(tamano.x) - anchoRecorte) / 2;
            recorte.size.x = anchoRecorte;
        } else {
            int altoRecorte = static_cast<int>(static_cast<float>(tamano.x) / proporcionZona);
            recorte.position.y = (static_cast<int>(tamano.y) - altoRecorte) / 2;
            recorte.size.y = altoRecorte;
        }

        fondo.setTextureRect(recorte);
        fondo.setScale({
            zona.size.x / static_cast<float>(recorte.size.x),
            zona.size.y / static_cast<float>(recorte.size.y)
        });
        fondo.setPosition(zona.position);
        fondo.setColor(luzEncendida
            ? sf::Color(255, 255, 248, 255)
            : sf::Color(170, 174, 182, 255));
        ventana.draw(fondo);

        if (luzEncendida) {
            sf::RectangleShape haz({zona.size.x, zona.size.y});
            haz.setPosition(zona.position);
            haz.setFillColor(sf::Color(255, 224, 142, static_cast<std::uint8_t>(14.0f + pulso * 14.0f)));
            ventana.draw(haz);

            sf::RectangleShape nucleo({
                std::max(10.0f, zona.size.x * 0.34f),
                zona.size.y
            });
            nucleo.setPosition({
                esIzquierda ? zona.position.x + zona.size.x * 0.54f : zona.position.x + zona.size.x * 0.08f,
                zona.position.y
            });
            nucleo.setFillColor(sf::Color(255, 245, 195, static_cast<std::uint8_t>(24.0f + pulso * 24.0f)));
            ventana.draw(nucleo);

            for (int i = 0; i < 3; ++i) {
                float t = static_cast<float>(i);
                float ancho = zona.size.x * (0.06f + t * 0.025f);
                float x = esIzquierda
                    ? zona.position.x + zona.size.x - ancho - t * 16.0f
                    : zona.position.x + t * 16.0f;

                sf::RectangleShape banda({ancho, zona.size.y});
                banda.setPosition({x, zona.position.y});
                banda.setFillColor(sf::Color(
                    255,
                    static_cast<std::uint8_t>(224 + i * 7),
                    static_cast<std::uint8_t>(155 + i * 12),
                    static_cast<std::uint8_t>(8.0f + pulso * 10.0f)));
                ventana.draw(banda);
            }
        }
    }

    void renderizarPersonajeEnPuerta(sf::RenderWindow& ventana, const std::string& nombre, bool esIzquierda, bool luzEncendida) {
        static const std::vector<std::string> permitidos = {"Gobo", "Director", "Popy", "TheUsher"};
        std::string clave = getClavePersonajePuerta(nombre);
        if (std::find(permitidos.begin(), permitidos.end(), clave) == permitidos.end()) return;

        if (spritesPersonajesPuerta.find(clave) != spritesPersonajesPuerta.end()) {
            sf::Sprite sprite = spritesPersonajesPuerta.at(clave);
            sprite.setColor(luzEncendida ? sf::Color(235, 235, 235, 240) : sf::Color(55, 55, 55, 220));

            if (esIzquierda) {
                sprite.setPosition(sf::Vector2f(160.0f, 650.0f));
            } else {
                sprite.setPosition(sf::Vector2f(anchoVirtualOficina - 160.0f, 650.0f));
            }

            ventana.draw(sprite);
        }
    }

public:
    Motor() : ventana(),
              vistaOficina(),
              vistaInterfaz(),
              posicionCamaraX(0.0f),
              velocidadCamara(600.0f),
              anchoVirtualOficina(1640.0f),
              pasilloIzquierdoCargado(false),
              pasilloDerechoCargado(false),
              visualPuertaIzquierda(),
              visualPuertaDerecha(),
              marcoLuzIzquierda(),
              marcoLuzDerecha(),
              barraEnergiaFondo(),
              barraEnergiaFrente(),
              gobo(12),
              director(14),
              popy(10),
              usher(16),
              relojEnergia(),
              relojTerminal(),
              relojEstado(),
              relojIntroNoche1(),
              relojVictoria(),
              relojInterferenciaMonitor(),
              relojParpadeoInterferencia(),
              relojTransicionSala3(),
              relojEfectosMenu(),
              relojNoche(),
              tiempoLuzIzquierdaActiva(0.0f),
              tiempoLuzDerechaActiva(0.0f),
              estadoJuego(EstadoJuego::Portada),
              nocheActual(1),
              horaActual(12),
              tiempoPorHora(86.0f),
              acumuladorHora(0.0f),
              juegoTerminado(false),
              victoria(false),
              tiempoMuerteAcumulado(0.0f),
              interferenciaMonitorActiva(false),
              duracionInterferenciaMonitor(0.4f),
              fotogramaInterferenciaMonitor(0),
              intervaloTickIA(5.0f),
              acumuladorTickIA(0.0f),
              controlesBloqueados(false),
              tiempoIntroAntesPausa(0.0f) {

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        sf::Listener::setPosition({0.0f, 0.0f, 0.0f});
        sf::Listener::setDirection({0.0f, 0.0f, -1.0f});
        generarTexturaInterferenciaMonitor();
        retrasoAtaque = 0.0f;
        atacanteActual.clear();
        audioDisponible = false;
        introNoche1Reproducido = false;
        introNoche1Activa = false;
        duracionIntroNoche1 = 20.0f;
        duracionVictoria = 6.5f;
        fuenteUICargada = cargarFuenteDesdeRutas(fuenteUI, {
            "assets/fonts/arial.ttf",
            "../assets/fonts/arial.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf"
        });
        if (fuenteUICargada) {
            textoEnergiaHUD.emplace(fuenteUI, "100%", 24);
            textoEnergiaHUD->setFillColor(sf::Color::White);
            textoEnergiaHUD->setOutlineColor(sf::Color::Black);
            textoEnergiaHUD->setOutlineThickness(2.0f);
            textoEnergiaHUD->setStyle(sf::Text::Bold);
            textoEnergiaHUD->setPosition({305.0f, 641.0f});

            textoRelojHUD.emplace(fuenteUI, "12 AM", 32);
            textoRelojHUD->setFillColor(sf::Color::White);
            textoRelojHUD->setOutlineColor(sf::Color::Black);
            textoRelojHUD->setOutlineThickness(2.0f);
            textoRelojHUD->setStyle(sf::Text::Bold);
            actualizarTextosHUD();
        }

        ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
        ventana.setFramerateLimit(60);
        ventana.requestFocus();


        posicionCamaraX = anchoVirtualOficina / 2.0f;
        velocidadCamara = 600.0f;

        vistaOficina.setSize({1280.0f, 720.0f});
        vistaOficina.setCenter({posicionCamaraX, 360.0f});

        vistaInterfaz.setSize({1280.0f, 720.0f});
        vistaInterfaz.setCenter({640.0f, 360.0f});


        if (!cargarTextureDesdeRutas(texturaPortada, {
            "assets/textures/menuprincipal/portadaprincipal/portada.png",
            "../assets/textures/menuprincipal/portadaprincipal/portada.png"
        })) {
            std::cerr << "Advertencia: No se encontró textura de portada" << std::endl;
        } else {
            spritePortada.emplace(texturaPortada);

            const auto tamTextura = texturaPortada.getSize();
            float escalaX = 1280.0f / static_cast<float>(tamTextura.x);
            float escalaY = 720.0f / static_cast<float>(tamTextura.y);
            float escala = std::max(escalaX, escalaY);
            spritePortada->setScale({escala, escala});
            spritePortada->setPosition({
                (1280.0f - static_cast<float>(tamTextura.x) * escala) / 2.0f,
                (720.0f - static_cast<float>(tamTextura.y) * escala) / 2.0f
            });
        }

        if (!cargarTextureDesdeRutas(texturaMenuPrincipal, {
            "assets/textures/menuprincipal/menuprincipal .png",
            "../assets/textures/menuprincipal/menuprincipal .png",
            "assets/textures/menuprincipal/menuprincipal.png",
            "../assets/textures/menuprincipal/menuprincipal.png"
        })) {
            std::cerr << "Advertencia: No se encontró textura de menú principal" << std::endl;
        } else {
            spriteMenuPrincipal.emplace(texturaMenuPrincipal);

            const auto tamTextura = texturaMenuPrincipal.getSize();
            float escalaX = 1280.0f / static_cast<float>(tamTextura.x);
            float escalaY = 720.0f / static_cast<float>(tamTextura.y);
            float escala = std::max(escalaX, escalaY);
            spriteMenuPrincipal->setScale({escala, escala});
            spriteMenuPrincipal->setPosition({
                (1280.0f - static_cast<float>(tamTextura.x) * escala) / 2.0f,
                (720.0f - static_cast<float>(tamTextura.y) * escala) / 2.0f
            });
        }

        if (!cargarTextureDesdeRutas(texturaMenuPausa, {
            "assets/textures/menuprincipal/menudepausa/menudepausa.png",
            "../assets/textures/menuprincipal/menudepausa/menudepausa.png"
        })) {
            std::cerr << "Advertencia: No se encontró textura del menú de pausa" << std::endl;
        } else {
            spriteMenuPausa.emplace(texturaMenuPausa);

            const auto tamTextura = texturaMenuPausa.getSize();
            float escalaX = 1280.0f / static_cast<float>(tamTextura.x);
            float escalaY = 720.0f / static_cast<float>(tamTextura.y);
            float escala = std::max(escalaX, escalaY);
            spriteMenuPausa->setScale({escala, escala});
            spriteMenuPausa->setPosition({
                (1280.0f - static_cast<float>(tamTextura.x) * escala) / 2.0f,
                (720.0f - static_cast<float>(tamTextura.y) * escala) / 2.0f
            });
        }

        if (!cargarTextureDesdeRutas(texturaOficina, {
            "assets/textures/oficina/oficina2.png",
            "../assets/textures/oficina/oficina2.png",
            "assets/textures/oficina/oficina.png",
            "../assets/textures/oficina/oficina.png",
            "assets/textures/oficina.png",
            "../assets/textures/oficina.png"
        })) {
            std::cerr << "Error: No se encontro assets/textures/oficina.png" << std::endl;
        } else {
            spriteOficina.emplace(texturaOficina);



            spriteOficina.value().setScale({
                anchoVirtualOficina / static_cast<float>(texturaOficina.getSize().x),
                720.0f / static_cast<float>(texturaOficina.getSize().y)
            });
        }


        pasilloIzquierdoCargado = cargarTextureDesdeRutas(texturaPasilloIzquierda, {
            "assets/textures/oficina/saladeproyeccion1/saledefondo1.png",
            "../assets/textures/oficina/saladeproyeccion1/saledefondo1.png"
        });
        if (!pasilloIzquierdoCargado && spriteOficina.has_value()) {
            texturaPasilloIzquierda = texturaOficina;
            pasilloIzquierdoCargado = true;
        }
        pasilloDerechoCargado = cargarTextureDesdeRutas(texturaPasilloDerecha, {
            "assets/textures/oficina/saladeproyeccion1/saladeproyeccion2/saladefondo2.png",
            "../assets/textures/oficina/saladeproyeccion1/saladeproyeccion2/saladefondo2.png"
        });
        if (!pasilloDerechoCargado && spriteOficina.has_value()) {
            texturaPasilloDerecha = texturaOficina;
            pasilloDerechoCargado = true;
        }


        cargarTexturasPersonajesPuerta();
        cargarTexturasJumpscare();
        cargarAudio();


        if (cargarBufferAudio("menuambiente", {
            "assets/audio/menu_principal.ogg",
            "../assets/audio/menu_principal.ogg",
            "assets/audio/menu_principal.wav",
            "../assets/audio/menu_principal.wav",
            "assets/textures/musica/audio_juego/ambiente_tenebroso.wav",
            "../assets/textures/musica/audio_juego/ambiente_tenebroso.wav"
        })) {
            sonidoMenuPrincipal.emplace(buffersAudio.at("menuambiente"));
            sonidoMenuPrincipal->setLooping(true);
            sonidoMenuPrincipal->setVolume(70.0f);
            sonidoMenuPrincipal->play();
            std::cerr << "✓ Música del menú principal cargada" << std::endl;
        } else {
            std::cerr << "⚠ No se encontró audio tenebroso para el menú" << std::endl;
        }


        barraEnergiaFondo.setSize(sf::Vector2f(250.0f, 20.0f));
        barraEnergiaFondo.setFillColor(sf::Color(50, 50, 50));
        barraEnergiaFondo.setOutlineColor(sf::Color::White);
        barraEnergiaFondo.setOutlineThickness(2.0f);
        barraEnergiaFondo.setPosition(sf::Vector2f(40.0f, 650.0f));

        barraEnergiaFrente.setSize(sf::Vector2f(250.0f, 20.0f));
        barraEnergiaFrente.setFillColor(sf::Color::Green);
        barraEnergiaFrente.setPosition(sf::Vector2f(40.0f, 650.0f));

        for (int i = 0; i < 4; i++) {
            bloquesConsumo[i].setSize(sf::Vector2f(15.0f, 20.0f));
            bloquesConsumo[i].setFillColor(sf::Color::Yellow);
            bloquesConsumo[i].setOutlineColor(sf::Color::Black);
            bloquesConsumo[i].setOutlineThickness(1.0f);
            bloquesConsumo[i].setPosition(sf::Vector2f(390.0f + (i * 20.0f), 650.0f));
        }


        ajustarZonaPuertaFisica(visualPuertaIzquierda, true);
        ajustarZonaPuertaFisica(visualPuertaDerecha, false);


        marcoLuzIzquierda.setSize(sf::Vector2f(220.0f, 400.0f));
        marcoLuzIzquierda.setFillColor(sf::Color::Transparent);
        marcoLuzIzquierda.setOutlineColor(sf::Color(200, 200, 100, 100));
        marcoLuzIzquierda.setOutlineThickness(2.0f);
        marcoLuzIzquierda.setPosition(sf::Vector2f(10.0f, 160.0f));

        marcoLuzDerecha.setSize(sf::Vector2f(220.0f, 400.0f));
        marcoLuzDerecha.setFillColor(sf::Color::Transparent);
        marcoLuzDerecha.setOutlineColor(sf::Color(200, 200, 100, 100));
        marcoLuzDerecha.setOutlineThickness(2.0f);
        marcoLuzDerecha.setPosition(sf::Vector2f(anchoVirtualOficina - 230.0f, 160.0f));

        relojEnergia.restart();
        relojTerminal.restart();
        relojNoche.restart();
        relojEstado.restart();
        relojVictoria.restart();
    }

    ~Motor() {}

    void ejecutarCiclo() {
        procesarEventos();
        actualizar();
        renderizar();
    }

    bool estaAbierto() const {
        return ventana.isOpen();
    }
};
