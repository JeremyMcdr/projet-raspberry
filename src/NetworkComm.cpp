#include "NetworkComm.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <stdexcept>

// Mutex pour protéger l'accès à la console
std::mutex consoleMutex;

// Fonction pour obtenir l'adresse IP locale (non localhost)
std::string getLocalIP() {
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    std::string localIP;

    if (getifaddrs(&ifap) != 0) {
        throw std::runtime_error("Erreur lors de l'obtention des adresses IP locales.");
    }

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            char* addr = inet_ntoa(sa->sin_addr);
            if (std::string(ifa->ifa_name) != "lo") { // Exclure l'interface loopback
                localIP = std::string(addr);
                break;
            }
        }
    }
    freeifaddrs(ifap);

    if (localIP.empty()) {
        throw std::runtime_error("Impossible de déterminer l'adresse IP locale.");
    }

    return localIP;
}

// Fonction pour écouter les messages de découverte
void listenForDiscovery() {
    try {
        int discovery_socket;
        struct sockaddr_in server_addr, client_addr;
        socklen_t addr_len = sizeof(client_addr);
        char buffer[1024] = {0};

        // Créer un socket UDP pour écouter les messages de découverte
        discovery_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (discovery_socket == -1) {
            throw std::runtime_error("Erreur lors de la création du socket de découverte.");
        }

        // Configurer le serveur pour écouter sur toutes les interfaces réseau sur le port 9090
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Écoute sur toutes les interfaces
        server_addr.sin_port = htons(9090); // Port de découverte

        // Attacher le socket à l'adresse et au port
        if (bind(discovery_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            close(discovery_socket);
            throw std::runtime_error("Erreur lors du bind du socket de découverte.");
        }

        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "Serveur en attente de messages de découverte..." << std::endl;
        }

        while (serverRunning) { // Ajout de la condition d'arrêt
            // Configurer un timeout pour recvfrom
            struct timeval tv;
            tv.tv_sec = 1; // Timeout de 1 seconde
            tv.tv_usec = 0;
            setsockopt(discovery_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            // Recevoir un message de découverte
            int n = recvfrom(discovery_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
            if (n > 0) {
                {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "Message de découverte reçu, envoi de la réponse..." << std::endl;
                }
                // Récupérer l'adresse IP locale
                std::string localIP = getLocalIP();
                // Répondre au client avec l'adresse IP
                sendto(discovery_socket, localIP.c_str(), localIP.size(), 0, (struct sockaddr *)&client_addr, addr_len);
                {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "Réponse envoyée avec l'IP : " << localIP << std::endl;
                }
            }
        }

        close(discovery_socket);  // Fermer le socket proprement
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cerr << "Erreur dans listenForDiscovery : " << e.what() << std::endl;
    }
}


// Constructeur pour initialiser le serveur
NetworkComm::NetworkComm(int port) : server_fd(-1) {
    // Création du socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Erreur lors de la création du socket.");
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr)); // Initialiser à 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accepte les connexions sur n'importe quelle interface réseau
    server_addr.sin_port = htons(port); // Port du serveur

    // Attacher le socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(server_fd);
        throw std::runtime_error("Erreur lors du bind.");
    }

    // Mettre le socket en mode écoute
    if (listen(server_fd, 10) < 0) {
        close(server_fd);
        throw std::runtime_error("Erreur lors de l'écoute.");
    }

    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Serveur en attente de connexions sur le port " << port << std::endl;
    }
}

// Constructeur par défaut pour utilisation dans handleClient
NetworkComm::NetworkComm() : server_fd(-1) {}


// Accepter les connexions client
int NetworkComm::acceptClient() {
    struct timeval tv;
    tv.tv_sec = 1; // Timeout de 1 seconde
    tv.tv_usec = 0;
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int client_socket = accept(server_fd, nullptr, nullptr);
    if (client_socket < 0 && errno != EWOULDBLOCK) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cerr << "Erreur lors de l'acceptation de la connexion." << std::endl;
    } else if (client_socket >= 0) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Client connecté." << std::endl;
    }
    return client_socket;
}


// Envoyer un message au client
void NetworkComm::sendMessage(int client_socket, const std::string &message) {
    if (send(client_socket, message.c_str(), message.size(), 0) == -1) {
        throw std::runtime_error("Erreur lors de l'envoi du message au client.");
    }
}

// Recevoir un message du client
std::string NetworkComm::receiveMessage(int client_socket) {
    char buffer[1024] = {0};
    int valread = recv(client_socket, buffer, sizeof(buffer), 0);
    if (valread <= 0) {
        throw std::runtime_error("Erreur lors de la réception du message du client.");
    }
    return std::string(buffer, valread);
}

// Destructeur pour fermer les connexions
NetworkComm::~NetworkComm() {
    if (server_fd != -1) {
        close(server_fd);
    }
}
