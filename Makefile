# Makefile

# Variables du compilateur
CXX = g++
CXXFLAGS = -std=c++11 -O2 -pthread
INCLUDES = -I./include
LIBS = -lboost_system -lboost_thread -lpthread

# Répertoires
SRCDIR = src
INCDIR = include
BUILDDIR = build

# Fichiers sources et objets
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))

# Nom de l'exécutable
TARGET = domotique

# Règles par défaut
all: $(TARGET)

# Règle de linkage
$(TARGET): $(OBJ)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

# Règle de compilation des fichiers objets
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Règle de nettoyage
clean:
	rm -rf $(BUILDDIR) $(TARGET)

.PHONY: all clean
