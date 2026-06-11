# Makefile para Five Nights at Cinepolis
# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

# Directorios
SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = bin
OBJ_DIR = obj

# Archivos fuente y objetos
# Nota: Juego.cpp y Motor.cpp tienen implementaciones inline en headers
SOURCES = $(SRC_DIR)/Main.cpp
OBJECTS = $(OBJ_DIR)/Main.o

# Nombre del ejecutable
EXECUTABLE = $(BIN_DIR)/FiveNightsAtCinepolis.exe

# Targets principales
.PHONY: all build clean run help

# Objetivo por defecto
all: build

# Compilar el proyecto
build: $(EXECUTABLE)

# Compilar el objeto principal
$(OBJ_DIR)/Main.o: $(SRC_DIR)/Main.cpp $(INCLUDE_DIR)/Juego.hpp $(INCLUDE_DIR)/Motor.hpp $(INCLUDE_DIR)/MonitorCamaras.hpp $(INCLUDE_DIR)/Guardia.hpp $(INCLUDE_DIR)/Personaje.hpp
	@mkdir -p $(OBJ_DIR)
	@echo "[COMPILE] Main.cpp"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Crear el ejecutable
$(EXECUTABLE): $(OBJECTS)
	@echo "[LINKING] Creando ejecutable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[SUCCESS] Ejecutable creado en: $@"

# Limpiar archivos compilados
clean:
	@echo "[CLEAN] Eliminando archivos compilados..."
	@rm -rf $(OBJ_DIR)
	@rm -f $(EXECUTABLE)
	@echo "[SUCCESS] Limpieza completada"

# Ejecutar el programa en terminal externa
run: build
	@echo "[RUN] Iniciando aplicacion..."
	@cd $(BIN_DIR) && start FiveNightsAtCinepolis.exe

# Recompilar desde cero
rebuild: clean build

# Ayuda
help:
	@echo "=== COMANDOS DISPONIBLES ==="
	@echo "make build   - Compila el proyecto"
	@echo "make clean   - Elimina archivos compilados"
	@echo "make run     - Compila y ejecuta en terminal externa"
	@echo "make rebuild - Limpia y recompila"
	@echo "make help    - Muestra esta ayuda"
