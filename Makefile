# Définir les répertoires
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
BUILD_DIR = build

# Définir les fichiers source et les fichiers objets
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/NetworkComm.cpp
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/NetworkComm.o

# Nom de l'exécutable
TARGET = $(BIN_DIR)/my_program

# Options du compilateur
CXX = g++
CXXFLAGS = -I$(INC_DIR) -pthread

# Règle par défaut
all: $(TARGET)

# Règle pour créer l'exécutable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS)

# Règle pour compiler les fichiers source en fichiers objets dans build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Règle pour nettoyer les fichiers compilés
clean:
	@rm -rf $(BUILD_DIR)/*.o $(BIN_DIR)/*
