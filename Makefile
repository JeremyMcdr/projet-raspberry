# Définitions du compilateur et des options
CXX = g++
INC_DIR = include
CXXFLAGS = -I$(INC_DIR) -I/usr/include/websocketpp -pthread -lboost_system -lboost_thread

# Liste des fichiers source
SRC = src/main.cpp src/App.cpp src/XBeeManager.cpp src/NetworkComm.cpp

# Générer une liste de fichiers objets correspondants
OBJ = $(SRC:.cpp=.o)

# Nom de l'exécutable final
TARGET = bin/my_program

# Règle principale
all: $(TARGET)

# Règle pour créer l'exécutable en liant tous les fichiers objets
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(CXXFLAGS)

# Règle pour compiler les fichiers objets
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Nettoyage des fichiers objets et de l'exécutable
clean:
	rm -f $(OBJ) $(TARGET)