# Nom de l'exécutable
EXEC = trello

# Compilateur
CXX = g++

# Options de compilation
CXXFLAGS = -std=c++17 -Wall -I.

# Bibliothèques SFML
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Fichiers sources
SRC = main.cpp ui.cpp menu.cpp

# Fichiers objets générés
OBJ = $(SRC:.cpp=.o)

# Règle par défaut
all: $(EXEC)

# Compilation de l'exécutable
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)
	rm -f $(OBJ)

# Compilation des fichiers objets
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f $(OBJ) $(EXEC)
