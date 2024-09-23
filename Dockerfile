# Utiliser une image de base qui contient un compilateur C++
FROM gcc:latest

# Créer un répertoire pour l'application
WORKDIR /app

# Copier le contenu du projet dans le conteneur
COPY ./src /app/src
COPY ./include /app/include

# Installer les dépendances et compiler le programme
RUN apt-get update && apt-get install -y \
    make \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Compiler le programme
RUN g++ -std=c++17 -I/app/include -o server /app/src/main.cpp /app/src/NetworkComm.cpp

# Exposer le port sur lequel votre application écoute
EXPOSE 8080
EXPOSE 9090

# Commande pour exécuter le programme
CMD ["./server"]
