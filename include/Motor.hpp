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
#include "Guardia.hpp"
#include "MonitorCamaras.hpp"
#include "Personaje.hpp" 

enum class EstadoJuego {
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
    sf::Font fuenteUI;
    bool fuenteUICargada;

    // Control de Cámara (Vista)
    sf::View vistaOficina;
    sf::View vistaInterfaz; 
    float posicionCamaraX;
    float velocidadCamara;
    float anchoVirtualOficina; // Ancho objetivo para forzar el paneo panorámico

    // Componentes Visuales de las Puertas
    sf::RectangleShape visualPuertaIzquierda;
    sf::RectangleShape visualPuertaDerecha;

    // Sistema de iluminación de pasillos (sprites y texturas)
    std::map<std::string, sf::Texture> texturasPersonajesPuerta;
    std::map<std::string, sf::Sprite> spritesPersonajesPuerta;
    std::map<std::string, sf::Texture> texturasJumpscare;
    sf::RectangleShape marcoLuzIzquierda;
    sf::RectangleShape marcoLuzDerecha;

    // Indicadores Gráficos del HUD
    sf::RectangleShape barraEnergiaFondo;
    sf::RectangleShape barraEnergiaFrente;
    sf::RectangleShape bloquesConsumo[4]; 
    sf::RectangleShape bloquesReloj[6];   

    // Entidades del juego
    Guardia jugador;
    MonitorCamaras monitor; 
    Gobo gobo;
    Director director;
    Popy popy;
    TheUsher usher;
    TicketyStub stub; 

    // Relojes de control
    sf::Clock relojEnergia;
    sf::Clock relojTerminal;
    sf::Clock relojEstado;
    sf::Clock relojIntroNoche1;
    
    // Control del Tiempo y Victoria
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

    std::map<std::string, sf::SoundBuffer> buffersAudio;
    std::optional<sf::Sound> sonidoIntroNoche1;
    std::optional<sf::Sound> sonidoAmbiente;
    std::vector<sf::Sound> sonidosActivos;
    bool audioDisponible;
    bool introNoche1Reproducido;
    bool introNoche1Activa;
    float duracionIntroNoche1;

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
        const std::string base = "assets/textures/musica/audio_juego/";
        const std::string baseAlterna = "../assets/textures/musica/audio_juego/";

        audioDisponible = true;
        if (cargarBufferAudio("ambiente", {base + "ambiente_tenebroso.wav", baseAlterna + "ambiente_tenebroso.wav"})) {
            sonidoAmbiente.emplace(buffersAudio.at("ambiente"));
            sonidoAmbiente->setLooping(true);
            sonidoAmbiente->setVolume(100.0f);
        } else {
            audioDisponible = false;
            std::cerr << "No se encontro musica ambiente" << std::endl;
        }

        std::vector<std::pair<std::string, std::string>> efectos = {
            {"intro", "intro_telefono.wav"},
            {"puerta", "puerta_metal.wav"},
            {"monitor", "monitor_static.wav"},
            {"camara", "cambio_camara.wav"},
            {"luz_roja", "alerta_luz_roja.wav"},
            {"ataque", "alerta_ataque.wav"},
            {"jumpscare", "jumpscare.wav"},
            {"gameover", "game_over.wav"},
            {"victoria", "victoria.wav"}
        };

        for (const auto& efecto : efectos) {
            if (!cargarBufferAudio(efecto.first, {base + efecto.second, baseAlterna + efecto.second})) {
                audioDisponible = false;
                std::cerr << "No se encontro audio: " << efecto.second << std::endl;
            }
        }

        if (buffersAudio.count("intro") > 0) {
            sonidoIntroNoche1.emplace(buffersAudio.at("intro"));
            sonidoIntroNoche1->setVolume(88.0f);
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
        relojEstado.restart();
        reproducirSonido("ataque", 42.0f);
        std::cerr << "[ATAQUE] " << atacanteActual << " entro. Jumpscare en "
                  << retrasoAtaque << " segundos." << std::endl;
    }

    void iniciarJumpscare() {
        estadoJuego = EstadoJuego::Jumpscare;
        juegoTerminado = true;
        victoria = false;
        relojEstado.restart();
        reproducirSonido("jumpscare", 86.0f);
    }

    void pasarAGameOver() {
        estadoJuego = EstadoJuego::GameOver;
        relojEstado.restart();
        reproducirSonido("gameover", 46.0f);
    }

    void resetearPartida() {
        jugador.resetear();
        gobo.resetear();
        director.resetear();
        popy.resetear();
        usher.resetear();
        stub.resetear();

        horaActual = 12;
        acumuladorHora = 0.0f;
        juegoTerminado = false;
        victoria = false;
        tiempoMuerteAcumulado = 0.0f;
        retrasoAtaque = 0.0f;
        atacanteActual.clear();
        estadoJuego = EstadoJuego::Jugando;

        posicionCamaraX = anchoVirtualOficina / 2.0f;
        vistaOficina.setCenter({posicionCamaraX, 360.0f});
        barraEnergiaFrente.setSize(sf::Vector2f(250.0f, 20.0f));
        barraEnergiaFrente.setFillColor(sf::Color::Green);

        for (int i = 0; i < 6; i++) {
            bloquesReloj[i].setFillColor(sf::Color(45, 45, 50));
        }

        if (spriteOficina.has_value()) {
            spriteOficina.value().setColor(sf::Color::White);
        }

        relojEnergia.restart();
        relojTerminal.restart();
        relojNoche.restart();
        relojEstado.restart();
        relojIntroNoche1.restart();

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
        // Cargar sprites de personajes para mostrar en las puertas
        std::vector<std::string> personajes = {"Gobo", "Director", "Popy", "TheUsher", "TicketyStub"};
        std::vector<std::string> carpetas = {"gobo", "director", "popy", "theusher", "ticketystub"};
        
        for (size_t i = 0; i < personajes.size(); i++) {
            std::string rutaBase = "assets/textures/personajes/" + carpetas[i] + "/sprite" + carpetas[i] + "/";
            
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
                    std::cerr << "✓ Cargada textura de puerta para " << personajes[i] << " desde " << nombre << std::endl;
                    break;
                }
            }
            
            if (cargada) {
                texturasPersonajesPuerta.insert({personajes[i], textura});
                sf::Sprite sprite(textura);
                sprite.setScale({0.5f, 0.5f}); // Escalar a tamaño visible en puerta
                spritesPersonajesPuerta.insert({personajes[i], sprite});
            } else {
                std::cerr << "⚠ No se encontró textura de puerta para " << personajes[i] << std::endl;
            }
        }
    }

    void procesarEventos() {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
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

                // Luces de pasillo - Q para izquierda, E para derecha
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
                    if (botonPresionado->code == sf::Keyboard::Key::Num1) { monitor.cambiarCamara(TipoCamara::CAM_01_DULCERIA); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num2) { monitor.cambiarCamara(TipoCamara::CAM_02_PASILLO_A); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num3) { monitor.cambiarCamara(TipoCamara::CAM_03_PASILLO_B); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num4) { monitor.cambiarCamara(TipoCamara::CAM_04_SALAS); reproducirSonido("camara", 24.0f); }
                    if (botonPresionado->code == sf::Keyboard::Key::Num5) { monitor.cambiarCamara(TipoCamara::CAM_05_BANOS); reproducirSonido("camara", 24.0f); }
                }
            }
        }
    }

    void actualizar() {
        float dt = relojEnergia.restart().asSeconds();

        if (estadoJuego == EstadoJuego::Jumpscare) {
            if (relojEstado.getElapsedTime().asSeconds() >= 2.5f) {
                pasarAGameOver();
            }
            return;
        }

        if (estadoJuego == EstadoJuego::GameOver || estadoJuego == EstadoJuego::Victoria) {
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

        // Lógica del Tiempo de la Noche
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
                victoria = true;
                estadoJuego = EstadoJuego::Victoria;
                tiempoMuerteAcumulado = 0.0f;
                reproducirSonido("victoria", 78.0f);
                return;
            }
        }

        jugador.bajarEnergia(dt);

        // Actualizar a todos los personajes
        bool puedeMoverIA = !introNoche1Activa;
        gobo.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA);
        director.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA);
        popy.actualizarIA(dt, jugador.esPuertaDerechaCerrada(), puedeMoverIA);
        usher.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA);
        stub.actualizarIA(dt, jugador.esPuertaIzquierdaCerrada(), puedeMoverIA);

        // Verificar si algún personaje logró entrar
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

        // Actualizar barra de energía
        float porcentajeEnergia = jugador.getEnergia() / 100.0f;
        if (porcentajeEnergia < 0.0f) porcentajeEnergia = 0.0f;
        barraEnergiaFrente.setSize(sf::Vector2f(250.0f * porcentajeEnergia, 20.0f));

        if (jugador.getEnergia() < 25.0f) {
            barraEnergiaFrente.setFillColor(sf::Color::Red);
        } else {
            barraEnergiaFrente.setFillColor(sf::Color::Green);
        }

        // Actualizar bloques del reloj
        int horasTranscurridas = (horaActual == 12) ? 0 : horaActual;
        for (int i = 0; i < 6; i++) {
            if (i < horasTranscurridas) {
                bloquesReloj[i].setFillColor(sf::Color::Cyan); 
            } else {
                bloquesReloj[i].setFillColor(sf::Color(45, 45, 50)); 
            }
        }

        // Movimiento de la cámara panorámica libre
        if (!jugador.esMonitorAbierto()) {
            sf::Vector2i posicionMouse = sf::Mouse::getPosition(ventana);
            
            if (posicionMouse.x >= 0 && posicionMouse.x <= 1280 && posicionMouse.y >= 0 && posicionMouse.y <= 720) {
                if (posicionMouse.x > 1000) posicionCamaraX += velocidadCamara * dt;
                else if (posicionMouse.x < 280) posicionCamaraX -= velocidadCamara * dt;

                // Límites basados en el ancho virtual de 1600 para permitir el paneo lateral
                if (posicionCamaraX < 640.0f) posicionCamaraX = 640.0f;
                if (posicionCamaraX > (anchoVirtualOficina - 640.0f)) posicionCamaraX = anchoVirtualOficina - 640.0f; 

                vistaOficina.setCenter({posicionCamaraX, 360.0f});
            }
        }

        if (relojTerminal.getElapsedTime().asSeconds() >= 0.1f) {
            #ifdef _WIN32
                std::system("cls");
            #else
                std::system("clear");
            #endif
            std::cout << "=========================================\n";
            std::cout << "  RELOJ DE LA NOCHE : " << horaActual << " AM \n";
            std::cout << "  ENERGIA RESTANTE  : " << static_cast<int>(jugador.getEnergia()) << "%\n";
            std::cout << "=========================================\n";
            
            // Mostrar amenazas activas
            std::cout << "  AMENAZAS ACTIVAS:\n";
            if (gobo.esEnLaPuerta()) std::cout << "    [!] " << gobo.getNombre() << " EN LA PUERTA IZQ\n";
            if (director.esEnLaPuerta()) std::cout << "    [!] " << director.getNombre() << " EN LA PUERTA IZQ\n";
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

        // Texto dibujado como bloques para evitar depender de una fuente externa.
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

    void renderizar() {
        if (estadoJuego == EstadoJuego::Victoria) {
            ventana.clear(sf::Color(20, 80, 20)); 
            ventana.display();
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
        
        // 1. DIBUJAR ENTORNO (Móvil)
        ventana.setView(vistaOficina);
        if (!jugador.esMonitorAbierto()) {
            if (spriteOficina.has_value()) {
                ventana.draw(spriteOficina.value());
            }

            // Dibujar puertas visuales
            if (jugador.esPuertaIzquierdaCerrada()) {
                ventana.draw(visualPuertaIzquierda);
            }
            if (jugador.esPuertaDerechaCerrada()) {
                ventana.draw(visualPuertaDerecha);
            }

            // Renderizar personajes en las puertas si tienen luz encendida
            if (jugador.esLuzIzquierdaEncendida()) {
                ventana.draw(marcoLuzIzquierda);
                if (gobo.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, gobo.getNombre(), true);
                if (director.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, director.getNombre(), true);
                if (usher.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, usher.getNombre(), true);
                if (stub.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, stub.getNombre(), true);
            }
            
            if (jugador.esLuzDerechaEncendida()) {
                ventana.draw(marcoLuzDerecha);
                if (popy.esEnLaPuerta()) renderizarPersonajeEnPuerta(ventana, popy.getNombre(), false);
            }
        }
        
        // 2. DIBUJAR INTERFAZ (Fija)
        ventana.setView(vistaInterfaz); 
        if (jugador.esMonitorAbierto()) {
            monitor.renderizar(ventana);
            
            // Mostrar personajes en la cámara actual del monitor
            if (gobo.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "Gobo");
            if (director.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "Director");
            if (popy.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "Popy");
            if (usher.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "TheUsher");
            if (stub.getPosicionActual() == monitor.getCamaraActual()) monitor.dibujarPersonaje(ventana, "TicketyStub");

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
                monitor.dibujarRutaPersonaje(
                    ventana, fuenteUI, "Tickety Stub",
                    {TipoCamara::CAM_01_DULCERIA, TipoCamara::CAM_02_PASILLO_A},
                    stub.getPosicionActual(), stub.esEnLaPuerta(), 4);
            }
        } else {
            ventana.draw(barraEnergiaFondo);
            ventana.draw(barraEnergiaFrente);

            for (int i = 0; i < jugador.getNivelConsumo() && i < 4; i++) {
                ventana.draw(bloquesConsumo[i]);
            }

            for (int i = 0; i < 6; i++) {
                ventana.draw(bloquesReloj[i]);
            }
        }
        
        ventana.display();
    }

    void renderizarPersonajeEnPuerta(sf::RenderWindow& ventana, const std::string& nombre, bool esIzquierda) {
        if (spritesPersonajesPuerta.find(nombre) != spritesPersonajesPuerta.end()) {
            sf::Sprite sprite = spritesPersonajesPuerta.at(nombre);
            
            if (esIzquierda) {
                sprite.setPosition(sf::Vector2f(50.0f, 200.0f));
            } else {
                sprite.setPosition(sf::Vector2f(anchoVirtualOficina - 150.0f, 200.0f));
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
              relojNoche(),
              estadoJuego(EstadoJuego::Jugando),
              horaActual(12),
              tiempoPorHora(120.0f),  // Cada hora dura 120 segundos (2 minutos) - Total: ~12 minutos para 6 noches
              acumuladorHora(0.0f),
              juegoTerminado(false),
              victoria(false),
              tiempoMuerteAcumulado(0.0f) { // Espacio panorámico ideal para que funcione el paneo
                  
        std::srand(static_cast<unsigned int>(std::time(nullptr))); 
        retrasoAtaque = 0.0f;
        atacanteActual.clear();
        audioDisponible = false;
        introNoche1Reproducido = false;
        introNoche1Activa = false;
        duracionIntroNoche1 = 20.0f;
        fuenteUICargada = cargarFuenteDesdeRutas(fuenteUI, {
            "assets/fonts/arial.ttf",
            "../assets/fonts/arial.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf"
        });
        
        ventana.create(sf::VideoMode({1280, 720}), "Five Nights at Cinepolis - Oficina");
        ventana.setFramerateLimit(60);

        // Configuración de las vistas
        posicionCamaraX = anchoVirtualOficina / 2.0f; // Centrado perfecto en 820.0f
        velocidadCamara = 600.0f; 
        
        vistaOficina.setSize({1280.0f, 720.0f});
        vistaOficina.setCenter({posicionCamaraX, 360.0f});

        vistaInterfaz.setSize({1280.0f, 720.0f});
        vistaInterfaz.setCenter({640.0f, 360.0f});

        if (!cargarTextureDesdeRutas(texturaOficina, {
            "assets/textures/oficina.png",
            "../assets/textures/oficina.png"
        })) {
            std::cerr << "Error: No se encontro assets/textures/oficina.png" << std::endl;
        } else {
            spriteOficina.emplace(texturaOficina);
            
            // Forzamos a la imagen a estirarse sutilmente a nivel panorámico (1640x720)
            // Esto garantiza que el mouse sí pueda mover la cámara a los lados pase lo que pase
            spriteOficina.value().setScale({
                anchoVirtualOficina / static_cast<float>(texturaOficina.getSize().x),
                720.0f / static_cast<float>(texturaOficina.getSize().y)
            });
        }

        // Cargar texturas de personajes para las puertas
        cargarTexturasPersonajesPuerta();
        cargarTexturasJumpscare();
        cargarAudio();

        // Configuración visual del medidor de batería
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
            bloquesConsumo[i].setPosition(sf::Vector2f(310.0f + (i * 20.0f), 650.0f));
        }

        // Configuración de los bloques del Reloj
        for (int i = 0; i < 6; i++) {
            bloquesReloj[i].setSize(sf::Vector2f(25.0f, 15.0f));
            bloquesReloj[i].setOutlineColor(sf::Color::White);
            bloquesReloj[i].setOutlineThickness(1.5f);
            bloquesReloj[i].setPosition(sf::Vector2f(1050.0f + (i * 32.0f), 30.0f));
        }

        // Coordenadas fijas de las puertas en los extremos del mapa virtual
        visualPuertaIzquierda.setSize(sf::Vector2f(240.0f, 720.0f));
        visualPuertaIzquierda.setFillColor(sf::Color(30, 30, 35, 240)); 
        visualPuertaIzquierda.setOutlineColor(sf::Color(80, 80, 80));
        visualPuertaIzquierda.setOutlineThickness(3.0f);
        visualPuertaIzquierda.setPosition(sf::Vector2f(0.0f, 0.0f)); // Extremo izquierdo absoluto

        visualPuertaDerecha.setSize(sf::Vector2f(240.0f, 720.0f));
        visualPuertaDerecha.setFillColor(sf::Color(30, 30, 35, 240));
        visualPuertaDerecha.setOutlineColor(sf::Color(80, 80, 80));
        visualPuertaDerecha.setOutlineThickness(3.0f);
        visualPuertaDerecha.setPosition(sf::Vector2f(anchoVirtualOficina - 240.0f, 0.0f)); // Extremo derecho absoluto

        // Configuración de marcos de luz de pasillo
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
