# Utiliser une image de base qui contient un compilateur C++
FROM gcc:latest

# Installer les dépendances nécessaires
RUN apt-get update && apt-get install -y \
    make \
    cmake \
    git \
    libc6-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libssl-dev \
    libwebsocketpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Créer un répertoire pour l'application
WORKDIR /app

# Copier le contenu du projet dans le conteneur
COPY ./src /app/src
COPY ./include /app/include
COPY ./Makefile /app/

# Compiler le programme en utilisant le Makefile
RUN make

# Exposer les ports sur lesquels votre application écoute
EXPOSE 8080 9090

# Définir la variable d'environnement pour le mode test (par défaut désactivé)
ENV TESTING_MODE=true

# Commande pour exécuter le programme
CMD ["./domotique"]
