#include <csignal>
#include <thread>
#include <chrono>
#include <iostream>

#include "NetworkComm.h"
#include "App.h"

volatile bool serverRunning = true;

void signalHandler(int signum) {
    std::cout << "Interruption reçue, fermeture du serveur..." << std::endl;
    serverRunning = false;
}

int main() {

    // Créer une instance de l'application
    App app;

    // Initialiser l'application
    if (!app.Init()) {
        std::cerr << "Erreur lors de l'initialisation de l'application." << std::endl;
        return 1; // Quitter avec une erreur
    }
    std::cout << "Application initialisée avec succés." << std::endl;

    // Exécuter l'application
    if (!app.Run()) {

        std::cerr << "Erreur lors de l'exécution de l'application." << std::endl;
        return 1; // Quitter avec une erreur
    }
    std::cout << "Application exécutée avec succés." << std::endl;

    // Quitter l'application
    app.Quit();


    /**************************************************** 
    A déplacer par jérémy
    ****************************************************

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

    **************************************************** 
    A déplacer par jérémy
    ****************************************************/

    return true;
}
