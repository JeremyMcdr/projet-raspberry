#include "NetworkComm.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

// Fonction pgour obtenir l'adresse IP locale (non localhost)
std::string getLocalIP() {
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    std::string localIP = "127.0.0.1"; // Valeur par défaut si aucun IP n'est trouvé

    getifaddrs(&ifap);
    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            localIP = inet_ntoa(sa->sin_addr);
            if (localIP != "127.0.0.1") // Exclure l'adresse localhost
                break;
        }
    }
    freeifaddrs(ifap);
    return localIP;
}

// Fonction pour écouter les messages de découverte
void listenForDiscovery() {
    int discovery_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024] = {0};

    // Créer un socket UDP pour écouter les messages de découverte
    discovery_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (discovery_socket == -1) {
        std::cerr << "Erreur lors de la création du socket de découverte" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Configurer le serveur pour écouter sur toutes les interfaces réseau sur le port 9090
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Écoute sur toutes les interfaces
    server_addr.sin_port = htons(9090); // Port de découverte

    // Attacher le socket à l'adresse et au port
    if (bind(discovery_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Erreur lors du bind du socket de découverte" << std::endl;
        close(discovery_socket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Serveur en attente de messages de découverte..." << std::endl;

    while (true) {
        // Recevoir un message de découverte
        int n = recvfrom(discovery_socket, buffer, 1024, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n > 0) {
            std::cout << "Message de découverte reçu, envoi de la réponse..." << std::endl;
            // Récupérer l'adresse IP locale
            std::string localIP = getLocalIP();
            // Répondre au client avec l'adresse IP
            sendto(discovery_socket, localIP.c_str(), localIP.size(), 0, (struct sockaddr *)&client_addr, addr_len);
            std::cout << "Réponse envoyée avec l'IP : " << localIP << std::endl;
        }
    }

    close(discovery_socket);
}

// Constructeur pour initialiser le serveur
NetworkComm::NetworkComm(int port) {
    // Création du socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Erreur lors de la création du socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr)); // Initialiser à 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accepte les connexions sur n'importe quelle interface réseau
    server_addr.sin_port = htons(port); // Port du serveur
    addr_len = sizeof(client_addr);

    // Attacher le socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Erreur lors du bind" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Mettre le socket en mode écoute
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Erreur lors de l'écoute" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Serveur en attente de connexions sur le port " << ntohs(server_addr.sin_port) << std::endl;
}

// Accepter les connexions client
void NetworkComm::acceptClient() {
    client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0) {
        std::cerr << "Erreur lors de l'acceptation de la connexion" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Client connecté" << std::endl;
}

// Envoyer un message au client
void NetworkComm::sendMessage(const std::string &message) {
    send(client_socket, message.c_str(), message.size(), 0);
}

// Recevoir un message du client
std::string NetworkComm::receiveMessage() {
    char buffer[1024] = {0};
    int valread = recv(client_socket, buffer, 1024, 0);
    return std::string(buffer, valread);
}

// Destructeur pour fermer les connexions
NetworkComm::~NetworkComm() {
    close(client_socket);
    close(server_fd);
}
