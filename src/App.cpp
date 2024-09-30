// App.cpp
#include "App.h"
#include <cstdlib>  // Pour getenv

App::App() : ws_server(xb_manager) { // Initialiser ws_server avec xb_manager
    // Construction de l'objet App
}

App::~App() {
    // Ajoutez ici toute désinitialisation nécessaire
}

// Méthode pour initialiser l'application (gestion des signaux, etc.)
bool App::Init(){
    std::cout << "[LOG] Init "  << std::endl;
    
    if(xb_manager.InitXBee()){
        std::cout << "[LOG] XBee initialisé "  << std::endl;
    }
    else {
        std::cerr << "Erreur lors de l'initialisation du XBeeManager" << std::endl;
        return false;
    }

    state = ProgramState::RUNNING;
    
    return true;
}

// Méthode pour démarrer et exécuter l'application (serveur WebSocket)
bool App::Run(){
    std::cout << "[LOG] Run "  << std::endl;

    // Lancer le serveur WebSocket dans un thread séparé
    std::thread ws_thread(&WebSocketServer::run, &ws_server, 8080);
    
    while(state == ProgramState::RUNNING){
        // Votre code ici, par exemple gérer les événements ou surveiller l'état
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Arrêter le serveur WebSocket
    ws_server.stop();
    ws_thread.join();

    return true;
}

// Méthode pour quitter proprement le programme
void App::Quit(){
    xb_manager.CloseXBee();
}

// Récupère les évènement disponible sur le XBee
bool App::PollXBeeEvent(){
    return true;
}

/*
*
*   a modifier par la suite
*
*
*/
// Fonction pour demander à l'utilisateur de remplir un message
void App::fill_message(XBeeMessage& message) {
    std::string mac_str;

    std::cout << "[LOG] Demande de saisie des informations pour le message." << std::endl;

    // Saisie de l'adresse MAC par l'utilisateur
    do {
        std::cout << "Entrez l'adresse MAC (16 caractères hexadécimaux sans espaces) : ";
        std::cin >> mac_str;

        // Vérifier que la chaîne contient bien 16 caractères
        if (mac_str.length() != 16) {
            std::cerr << "[ERREUR] L'adresse MAC doit contenir exactement 16 caractères hexadécimaux." << std::endl;
        }
    } while (mac_str.length() != 16);

    // Convertir l'adresse MAC en tableau d'octets
    mac_string_to_bytes(mac_str, message.mac_address);

    // Saisie de l'adresse (2 octets)
    std::cout << "Entrez l'adresse (2-byte integer) : ";
    std::cin >> message.address;

    // Saisie des données (2 octets)
    std::cout << "Entrez la donnée (2-byte integer) : ";
    std::cin >> message.data;

    // Affichage des données saisies pour vérification
    std::cout << "[LOG] Données saisies par l'utilisateur : " << std::endl;
    std::cout << "Adresse MAC : " << mac_str << std::endl;
    std::cout << "Adresse : 0x" << std::hex << message.address << std::endl;
    std::cout << "Donnée : 0x" << std::hex << message.data << std::endl;
}

// Fonction pour convertir une chaîne hexadécimale en tableau d'octets
void App::mac_string_to_bytes(const std::string& mac_str, uint8_t* mac_bytes) {
    for (int i = 0; i < 8; i++) {  // Se limiter à 8 octets, soit 16 caractères hexadécimaux
        sscanf(&mac_str[i * 2], "%2hhx", &mac_bytes[i]);
    }
}
