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
    MenuPrincipal,
    Jugando,
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
    sf::Texture texturaMenuPrincipal;
    std::optional<sf::Sprite> spriteMenuPrincipal;
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
    TicketyStub stub;


    sf::Clock relojEnergia;
    sf::Clock relojTerminal;
    sf::Clock relojEstado;
    sf::Clock relojIntroNoche1;
    sf::Clock relojVictoria;
    sf::Clock relojInterferenciaMonitor;
    sf::Clock relojParpadeoInterferencia;
    sf::Clock relojTickIA;
    float tiempoLuzIzquierdaActiva;
    float tiempoLuzDerechaActiva;


    sf::Clock relojNoche;
    EstadoJuego estadoJuego;
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

    int calcularDificultadBasePorPersonaje(const Personaje& personaje) const {
        int bonoHorario = obtenerCurvaDificultadHoraria().bonoDado;

        if (personaje.getNombre() == "Gobo") {
            return std::clamp(12 + bonoHorario + 2, 1, 20);
        }
        if (personaje.getNombre() == "Director") {
            return std::clamp(14 + bonoHorario + 1, 1, 20);
        }
        if (personaje.getNombre() == "Popy") {
            return std::clamp(10 + bonoHorario, 1, 20);
        }
        if (personaje.getNombre() == "The Usher") {
            return std::clamp(16 + bonoHorario - 1, 1, 20);
        }
        if (personaje.getNombre() == "Tickety Stub") {
            return std::clamp(8 + bonoHorario + 3, 1, 20);
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
            {"Tickety Stub", "ticketystub"}
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
    }

    void cargarTexturasPersonajesPuerta() {

        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
        std::vector<std::string> carpetas = {"gobo", "director", "popy", "theusher", "ticketystub"};

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
                const float alturaObjetivoPuerta = 300.0f;
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
        if (nombre == "Tickety Stub") return "TicketyStub";
        return nombre;
    }

    void iniciarPartidaDesdeMenu() {
        if (sonidoMenuPrincipal.has_value()) {
            sonidoMenuPrincipal->stop();
        }
        estadoJuego = EstadoJuego::Jugando;
        relojNoche.restart();
        relojEnergia.restart();
        controlesBloqueados = false;
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

            if (estadoJuego == EstadoJuego::MenuPrincipal) {
                if (const auto* click = evento->getIf<sf::Event::MouseButtonReleased>()) {
                    if (click->button == sf::Mouse::Button::Left) {
                        iniciarPartidaDesdeMenu();
                    }
                } else if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                    if (tecla->code == sf::Keyboard::Key::Enter ||
                        tecla->code == sf::Keyboard::Key::Space) {
                        iniciarPartidaDesdeMenu();
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
                if (estadoJuego == EstadoJuego::GameOver) {
                    if (botonPresionado->code == sf::Keyboard::Key::R) {
                        resetearPartida();
                    }
                    continue;
                }

                if (estadoJuego == EstadoJuego::Jumpscare || estadoJuego == EstadoJuego::Victoria) {
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

        if (estadoJuego == EstadoJuego::MenuPrincipal) {
            if (sonidoMenuPrincipal.has_value() && sonidoMenuPrincipal->getStatus() == sf::SoundSource::Status::Stopped) {
                sonidoMenuPrincipal->play();
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
                sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);

                if (posicionMouse.x >= 0 && posicionMouse.x <= 1280 && posicionMouse.y >= 0 && posicionMouse.y <= 720) {
                    if (posicionMouse.x > 1000) posicionCamaraX += velocidadCamara * dt;
                    else if (posicionMouse.x < 280) posicionCamaraX -= velocidadCamara * dt;

                    if (posicionCamaraX < 640.0f) posicionCamaraX = 640.0f;
                    if (posicionCamaraX > (anchoVirtualOficina - 640.0f)) posicionCamaraX = anchoVirtualOficina - 640.0f;

                    vistaOficina.setCenter({posicionCamaraX, 360.0f});
                }
            }
            return;
        }


        if (!introNoche1Reproducido && horaActual == 12) {
            iniciarIntroNoche1();
        }

        if (introNoche1Activa && relojIntroNoche1.getElapsedTime().asSeconds() >= duracionIntroNoche1) {
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

        jugador.bajarEnergia(dt);
        actualizarInterferenciaMonitor();

        EventoPuerta eventoGobo = gobo.actualizarEstadoPuerta(dt, jugador.esPuertaIzquierdaCerrada(), jugador.esMonitorAbierto(), jugador.esLuzIzquierdaEncendida());
        EventoPuerta eventoDirector = director.actualizarEstadoPuerta(dt, jugador.esPuertaDerechaCerrada(), jugador.esMonitorAbierto(), jugador.esLuzDerechaEncendida());
        EventoPuerta eventoPopy = popy.actualizarEstadoPuerta(dt, jugador.esPuertaIzquierdaCerrada(), jugador.esMonitorAbierto(), jugador.esLuzIzquierdaEncendida());
        EventoPuerta eventoUsher = usher.actualizarEstadoPuerta(dt, jugador.esPuertaDerechaCerrada(), jugador.esMonitorAbierto(), false);

        if (eventoGobo == EventoPuerta::Golpe) reproducirSonidoEspacial("golpe_puerta", {-1.0f, 0.0f, 0.0f}, 68.0f);
        if (eventoDirector == EventoPuerta::Golpe) reproducirSonidoEspacial("golpe_puerta", {1.0f, 0.0f, 0.0f}, 68.0f);
        if (eventoPopy == EventoPuerta::Golpe) reproducirSonidoEspacial("golpe_puerta", {-1.0f, 0.0f, 0.0f}, 70.0f);
        if (eventoUsher == EventoPuerta::Golpe) reproducirSonidoEspacial("golpe_puerta", {1.0f, 0.0f, 0.0f}, 66.0f);

        if (eventoGobo == EventoPuerta::Entrada) { iniciarAtaque(gobo.getNombre()); return; }
        if (eventoDirector == EventoPuerta::Entrada) { iniciarAtaque(director.getNombre()); return; }
        if (eventoPopy == EventoPuerta::Entrada) { iniciarAtaque(popy.getNombre()); return; }
        if (eventoUsher == EventoPuerta::Entrada) { iniciarAtaque(usher.getNombre()); return; }
        if (jugador.esMonitorAbierto() &&
            !jugador.esPuertaDerechaCerrada() &&
            usher.getPosicionActual() == TipoCamara::CAM_03_PASILLO_B &&
            !usher.esEnLaPuerta()) {
            iniciarJumpscare();
            return;
        }

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
        if (stub.esEstaAdentro()) {
            iniciarAtaque(stub.getNombre());
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
            sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);

            if (posicionMouse.x >= 0 && posicionMouse.x <= 1280 && posicionMouse.y >= 0 && posicionMouse.y <= 720) {
                if (posicionMouse.x > 1000) posicionCamaraX += velocidadCamara * dt;
                else if (posicionMouse.x < 280) posicionCamaraX -= velocidadCamara * dt;


                if (posicionCamaraX < 640.0f) posicionCamaraX = 640.0f;
                if (posicionCamaraX > (anchoVirtualOficina - 640.0f)) posicionCamaraX = anchoVirtualOficina - 640.0f;

                vistaOficina.setCenter({posicionCamaraX, 360.0f});
            }
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
            if (stub.esEnLaPuerta()) std::cout << "    [!] " << stub.getNombre() << " EN LA PUERTA IZQ\n";
            if (popy.esEnLaPuerta()) std::cout << "    [!] " << popy.getNombre() << " EN LA PUERTA DER\n";

            if (!gobo.esEnLaPuerta() && !director.esEnLaPuerta() && !popy.esEnLaPuerta() &&
                !usher.esEnLaPuerta() && !stub.esEnLaPuerta()) {
                std::cout << "    [OK] Ninguna amenaza activa\n";
            }
            std::cout << "=========================================\n";
            relojTerminal.restart();
        }

        if (spriteOficina.has_value()) {
            if (jugador.getEnergia() <= 0.0f) {
                spriteOficina.value().setColor(sf::Color(10, 10, 30));
            } else if (gobo.esEnLaPuerta() || director.esEnLaPuerta() || usher.esEnLaPuerta() || stub.esEnLaPuerta()) {
                spriteOficina.value().setColor(sf::Color(255, 180, 180));
            } else {
                spriteOficina.value().setColor(sf::Color::White);
            }
        }
    }

    void dibujarTexturaPantallaCompleta(sf::Texture& textura) {
        ventana.setView(vistaInterfaz);
        sf::Sprite sprite(textura);
        const auto tamTextura = textura.getSize();
        float escalaX = 1280.0f / static_cast<float>(tamTextura.x);
        float escalaY = 720.0f / static_cast<float>(tamTextura.y);
        float escala = std::max(escalaX, escalaY);
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
            dibujarTexturaPantallaCompleta(textura->second);
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

        for (int y = 0; y < 720; y += 12) {
            sf::RectangleShape linea(sf::Vector2f(1280.0f, 4.0f));
            linea.setPosition({0.0f, static_cast<float>(y)});
            int brillo = 20 + (std::rand() % 55);
            linea.setFillColor(sf::Color(brillo, 0, 0));
            ventana.draw(linea);
        }

        sf::RectangleShape panel(sf::Vector2f(620.0f, 170.0f));
        panel.setPosition({330.0f, 265.0f});
        panel.setFillColor(sf::Color(10, 10, 10, 230));
        panel.setOutlineColor(sf::Color(160, 0, 0));
        panel.setOutlineThickness(4.0f);
        ventana.draw(panel);


        sf::RectangleShape barra(sf::Vector2f(460.0f, 18.0f));
        barra.setFillColor(sf::Color(190, 0, 0));
        barra.setPosition({410.0f, 315.0f});
        ventana.draw(barra);
        barra.setPosition({410.0f, 365.0f});
        ventana.draw(barra);

        sf::RectangleShape reinicio(sf::Vector2f(280.0f, 12.0f));
        reinicio.setFillColor(sf::Color(220, 220, 220));
        reinicio.setPosition({500.0f, 405.0f});
        ventana.draw(reinicio);

        if (fuenteUICargada) {
            sf::Text titulo(fuenteUI, "GAME OVER", 84);
            titulo.setFillColor(sf::Color(220, 20, 20));
            titulo.setStyle(sf::Text::Bold);
            titulo.setPosition({405.0f, 285.0f});
            ventana.draw(titulo);

            sf::Text instruccion(fuenteUI, "Presiona R para reiniciar", 30);
            instruccion.setFillColor(sf::Color(230, 230, 230));
            instruccion.setPosition({460.0f, 390.0f});
            ventana.draw(instruccion);
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

        if (estadoJuego == EstadoJuego::MenuPrincipal) {
            ventana.clear(sf::Color::Black);
            ventana.setView(vistaInterfaz);
            if (spriteMenuPrincipal.has_value()) {
                ventana.draw(spriteMenuPrincipal.value());
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
            if (interferenciaMonitorActiva) {
                dibujarInterferenciaMonitor();
            } else {


            if (gobo.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "Gobo");
            if (director.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "Director");
            if (popy.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "Popy");
            if (usher.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "TheUsher");
            if (fuenteUICargada) {
                monitor.dibujarRutaPersonaje(
                    ventana, fuenteUI, "Gobo",
                    {TipoCamara::CAM_01_DULCERIA, TipoCamara::CAM_04_SALAS, TipoCamara::CAM_02_PASILLO_A},
                    gobo.getPosicionActual(), gobo.esEnLaPuerta(), 0);
                monitor.dibujarRutaPersonaje(
                    ventana, fuenteUI, "Director",
                    {TipoCamara::CAM_01_DULCERIA, TipoCamara::CAM_03_PASILLO_B, TipoCamara::CAM_02_PASILLO_A},
                    director.getPosicionActual(), director.esEnLaPuerta(), 1);
                monitor.dibujarRutaPersonaje(
                    ventana, fuenteUI, "Popy",
                    {TipoCamara::CAM_01_DULCERIA, TipoCamara::CAM_05_BANOS, TipoCamara::CAM_03_PASILLO_B},
                    popy.getPosicionActual(), popy.esEnLaPuerta(), 2);
                monitor.dibujarRutaPersonaje(
                    ventana, fuenteUI, "The Usher",
                    {TipoCamara::CAM_01_DULCERIA, TipoCamara::CAM_04_SALAS, TipoCamara::CAM_05_BANOS, TipoCamara::CAM_02_PASILLO_A},
                    usher.getPosicionActual(), usher.esEnLaPuerta(), 3);
            }
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
        static const std::vector<std::string> permitidos = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
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
              stub(8),
              relojEnergia(),
              relojTerminal(),
              relojEstado(),
              relojIntroNoche1(),
              relojVictoria(),
              relojInterferenciaMonitor(),
              relojParpadeoInterferencia(),
              relojNoche(),
              tiempoLuzIzquierdaActiva(0.0f),
              tiempoLuzDerechaActiva(0.0f),
              estadoJuego(EstadoJuego::MenuPrincipal),
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
              controlesBloqueados(false) {

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


        posicionCamaraX = anchoVirtualOficina / 2.0f;
        velocidadCamara = 600.0f;

        vistaOficina.setSize({1280.0f, 720.0f});
        vistaOficina.setCenter({posicionCamaraX, 360.0f});

        vistaInterfaz.setSize({1280.0f, 720.0f});
        vistaInterfaz.setCenter({640.0f, 360.0f});


        if (!cargarTextureDesdeRutas(texturaMenuPrincipal, {
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
