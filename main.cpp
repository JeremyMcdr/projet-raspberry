#include "NetworkComm.h"
#include <iostream>
#include <thread>

// Déclaration des fonctions
void handleClient(NetworkComm& server);
void listenForDiscovery();

int main() {
    // Créer un thread séparé pour écouter les messages de découverte
    std::thread discoveryThread(listenForDiscovery);

    // Démarrer le serveur pour gérer les connexions réseau normales
    NetworkComm server(8080);

    while (true) {
        // Accepter un client
        server.acceptClient();

        // Gérer le client connecté
        handleClient(server);
    }

    // Attendre que le thread de découverte se termine (il ne se terminera pas)
    discoveryThread.join();

    return 0;
}

// Fonction pour gérer les interactions avec le client
void handleClient(NetworkComm& server) {
    // Recevoir un message du client
    std::string message = server.receiveMessage();
    
    // Afficher le message reçu et effectuer une action en fonction de celui-ci
    if (message == "Bouton 1 Appuyé") {
        std::cout << "Action pour le Bouton 1" << std::endl;
    } else if (message == "Bouton 2 Appuyé") {
        std::cout << "Action pour le Bouton 2" << std::endl;
    } else {
        std::cout << "Message reçu : " << message << std::endl;
    }

    // Répondre au client
    server.sendMessage("Message reçu et traité !");
}
