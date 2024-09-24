#include "NetworkComm.h"
#include <csignal>
#include <thread>
#include <chrono>
#include <iostream>

volatile bool serverRunning = true;

void signalHandler(int signum) {
    std::cout << "Interruption reçue, fermeture du serveur..." << std::endl;
    serverRunning = false;
}

int main() {
    // Gérer les signaux pour fermer proprement le serveur
    std::signal(SIGINT, signalHandler);

    // Créer une instance du serveur WebSocket
    WebSocketServer ws_server;

    // Démarrer le serveur dans un thread séparé
    std::thread server_thread([&ws_server]() {
        ws_server.run(8080); // Port sur lequel le serveur écoute
    });

    // Boucle principale pour maintenir le programme actif
    while (serverRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Arrêter le serveur proprement
    ws_server.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }

    return 0;
}
