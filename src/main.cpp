#include "NetworkComm.h"
#include <iostream>
#include <thread>
#include <vector>
#include <csignal>

// Déclaration des fonctions
void handleClient(int client_socket);
void listenForDiscovery();

bool serverRunning = true;

void signalHandler(int signum) {
    std::cout << "Interruption reçue, fermeture du serveur..." << std::endl;
    serverRunning = false;
}

int main() {
    // Gérer les signaux pour fermer proprement le serveur
    std::signal(SIGINT, signalHandler);

    // Créer un thread séparé pour écouter les messages de découverte
    std::thread discoveryThread(listenForDiscovery);

    // Démarrer le serveur pour gérer les connexions réseau normales
    NetworkComm server(8080);

    // Vecteur pour stocker les threads des clients
    std::vector<std::thread> clientThreads;

    while (serverRunning) {
        // Accepter un client
        int client_socket = server.acceptClient();
        if (client_socket >= 0) {
            // Créer un nouveau thread pour gérer le client
            clientThreads.emplace_back(std::thread(handleClient, client_socket));
        }
    }

    // Attendre que le thread de découverte se termine
    discoveryThread.join();

    // Attendre que tous les threads clients se terminent
    for (auto& th : clientThreads) {
        if (th.joinable()) {
            th.join();
        }
    }

    return 0;
}

// Fonction pour gérer les interactions avec le client
void handleClient(int client_socket) {
    try {
        NetworkComm server;
        std::string message = server.receiveMessage(client_socket);
        
        // Validation de l'entrée
        if (message.empty()) {
            throw std::runtime_error("Message vide reçu.");
        }

        // Afficher le message reçu et effectuer une action en fonction de celui-ci
        if (message == "Bouton 1 Appuyé") {
            std::cout << "Action pour le Bouton 1" << std::endl;
        } else if (message == "Bouton 2 Appuyé") {
            std::cout << "Action pour le Bouton 2" << std::endl;
        } else {
            std::cout << "Message reçu : " << message << std::endl;
        }

        // Répondre au client
        server.sendMessage(client_socket, "Message reçu et traité !");
    } catch (const std::exception& e) {
        std::cerr << "Erreur dans handleClient : " << e.what() << std::endl;
    }

    // Fermer le socket client
    close(client_socket);
}
