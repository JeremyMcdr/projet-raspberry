// NetworkComm.cpp
#include "NetworkComm.h"               // Inclusion de l'en-tête de la classe
#include <iostream>                    // Pour les entrées/sorties standard
#include <functional>                  // Pour std::bind et std::placeholders
#include <cstring>                     // Pour les fonctions de manipulation de chaînes
#include <stdexcept>                   // Pour les exceptions standard
#include <ifaddrs.h>                   // Pour obtenir les adresses réseau
#include <arpa/inet.h>                 // Pour les fonctions de conversion d'adresses
#include <unistd.h>                    // Pour les fonctions POSIX (comme close)
#include <sys/socket.h>                // Pour les sockets
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Constructeur de la classe WebSocketServer
WebSocketServer::WebSocketServer(XBeeManager& xbee_manager)
    : m_xbee_manager(xbee_manager), m_discovery_running(false) {
    // Initialisation d'Asio pour la gestion des IO
    m_server.init_asio();

    // Définition des gestionnaires d'événements pour les connexions WebSocket
    m_server.set_open_handler(std::bind(&WebSocketServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(std::bind(&WebSocketServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(std::bind(&WebSocketServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

// Méthode pour démarrer le serveur WebSocket sur le port spécifié
void WebSocketServer::run(uint16_t port) {
    // Démarrage du thread de découverte UDP pour permettre la découverte du serveur
    startDiscoveryListener();

    // Configuration du serveur pour écouter sur le port spécifié
    m_server.listen(port);
    m_server.start_accept();  // Démarrage de l'acceptation des connexions entrantes
    m_server.run();           // Démarrage de la boucle d'événements Asio

    // Arrêt du thread de découverte une fois que le serveur WebSocket est arrêté
    stopDiscoveryListener();
}

// Méthode pour arrêter le serveur WebSocket proprement
void WebSocketServer::stop() {
    // Arrêt de l'écoute des nouvelles connexions
    m_server.stop_listening();

    // Verrouillage du mutex pour accéder de manière sécurisée aux connexions
    std::lock_guard<std::mutex> guard(m_connection_lock);
    for (auto hdl : m_connections) {
        // Fermeture de chaque connexion avec un statut "going_away"
        m_server.close(hdl, websocketpp::close::status::going_away, "");
        // Pause pour laisser le temps à chaque connexion de se fermer proprement
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    m_server.stop();  // Arrêt complet du serveur WebSocket

    // Arrêt du thread de découverte
    stopDiscoveryListener();
}

// Gestionnaire d'ouverture de connexion WebSocket
void WebSocketServer::on_open(websocketpp::connection_hdl hdl) {
    // Verrouillage du mutex pour accéder de manière sécurisée aux connexions
    std::lock_guard<std::mutex> guard(m_connection_lock);
    m_connections.insert(hdl);  // Ajout du handle de connexion au set des connexions actives
    std::cout << "Client connecté." << std::endl;
}

// Gestionnaire de fermeture de connexion WebSocket
void WebSocketServer::on_close(websocketpp::connection_hdl hdl) {
    // Verrouillage du mutex pour accéder de manière sécurisée aux connexions
    std::lock_guard<std::mutex> guard(m_connection_lock);
    m_connections.erase(hdl);  // Suppression du handle de connexion du set des connexions actives
    std::cout << "Client déconnecté." << std::endl;
}

// Gestionnaire de réception de messages WebSocket
void WebSocketServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::string message = msg->get_payload();
    std::cout << "Message reçu: " << message << std::endl;

    try {
        auto json_msg = json::parse(message);
        std::string action = json_msg["action"];
        auto params = json_msg["parameters"];

        if (action == "toggle_light") {
            std::string module_id = params["module_id"];
            std::string state = params["state"];

            // Convertir l'état en code approprié
            uint16_t address = get_address_from_module_id(module_id);
            uint16_t data = (state == "on") ? 0x01 : 0x00;

            XBeeMessage xbee_message;
            memset(&xbee_message, 0, sizeof(XBeeMessage));
            
            // Récupérer l'adresse MAC dynamique basée sur le module_id
            if (!get_mac_address_from_module_id(module_id, xbee_message.mac_address)) {
                m_server.send(hdl, "Module ID invalide.", websocketpp::frame::opcode::text);
                return;
            }

            xbee_message.address = address;
            xbee_message.data = data;

            // Calcul de la parité est déjà géré dans send_xbee_message
            m_xbee_manager.send_xbee_message(xbee_message);
            m_server.send(hdl, "Commande exécutée.", websocketpp::frame::opcode::text);
        }
        // Ajoutez d'autres actions ici
        else {
            m_server.send(hdl, "Action inconnue.", websocketpp::frame::opcode::text);
        }
    }
    catch (json::parse_error& e) {
        std::cerr << "Erreur de parsing JSON: " << e.what() << std::endl;
        m_server.send(hdl, "Format de message invalide.", websocketpp::frame::opcode::text);
    }
    catch (std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        m_server.send(hdl, "Erreur serveur.", websocketpp::frame::opcode::text);
    }
}

// Définition de la méthode pour obtenir l'adresse courte XBee basée sur le module_id
uint16_t WebSocketServer::get_address_from_module_id(const std::string& module_id) {
    // Implémentez la logique pour convertir module_id en adresse courte XBee
    // Cela pourrait impliquer une table de correspondance ou une autre méthode
    if (module_id == "module_1") {
        return 0x0001;
    }
    else if (module_id == "module_2") {
        return 0x0002;
    }
    // Ajoutez d'autres modules si nécessaire
    else {
        return 0xFFFF; // Adresse invalide
    }
}

// Définition de la méthode pour obtenir l'adresse MAC basée sur le module_id
bool WebSocketServer::get_mac_address_from_module_id(const std::string& module_id, uint8_t* mac_address) {
    // Implémentez la logique pour obtenir l'adresse MAC basée sur le module_id
    if (module_id == "module_1") {
        uint8_t mac[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0x52, 0x3A, 0x4B};
        memcpy(mac_address, mac, 8);
        return true;
    }
    else if (module_id == "module_2") {
        uint8_t mac[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0x52, 0x3A, 0x4C};
        memcpy(mac_address, mac, 8);
        return true;
    }
    // Ajoutez d'autres modules si nécessaire
    else {
        return false;
    }
}

// Méthode pour démarrer le thread de découverte UDP
void WebSocketServer::startDiscoveryListener() {
    m_discovery_running = true; // Indique que le thread de découverte doit fonctionner
    // Démarrage du thread qui écoute les messages de découverte
    m_discovery_thread = std::thread(&WebSocketServer::listenForDiscovery, this);
}

// Méthode pour arrêter le thread de découverte UDP
void WebSocketServer::stopDiscoveryListener() {
    m_discovery_running = false; // Indique que le thread de découverte doit s'arrêter
    if (m_discovery_thread.joinable()) { // Vérifie si le thread peut être rejoint
        m_discovery_thread.join();       // Attend la fin du thread de découverte
    }
}

// Fonction pour obtenir l'adresse IP locale de la machine
std::string getLocalIP() {
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    std::string localIP;

    // Obtention des adresses réseau
    if (getifaddrs(&ifap) != 0) {
        throw std::runtime_error("Erreur lors de l'obtention des adresses IP locales.");
    }

    // Parcours des interfaces réseau
    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        // Vérifie si l'interface a une adresse et si elle est de type IPv4
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            char* addr = inet_ntoa(sa->sin_addr);
            // Exclut l'interface loopback
            if (std::string(ifa->ifa_name) != "lo") {
                localIP = std::string(addr);
                break; // Arrête le parcours une fois une adresse trouvée
            }
        }
    }
    freeifaddrs(ifap); // Libère la mémoire allouée par getifaddrs

    if (localIP.empty()) {
        throw std::runtime_error("Impossible de déterminer l'adresse IP locale.");
    }

    return localIP; // Retourne l'adresse IP locale trouvée
}

// Méthode pour écouter les messages de découverte UDP
void WebSocketServer::listenForDiscovery() {
    try {
        int discovery_socket;
        struct sockaddr_in server_addr, client_addr;
        socklen_t addr_len = sizeof(client_addr);
        char buffer[1024] = {0};

        // Création d'un socket UDP pour écouter les messages de découverte
        discovery_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (discovery_socket == -1) {
            throw std::runtime_error("Erreur lors de la création du socket de découverte.");
        }

        // Configuration de l'adresse du serveur pour écouter sur toutes les interfaces réseau sur le port 9090
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Écoute sur toutes les interfaces
        server_addr.sin_port = htons(9090);              // Port de découverte

        // Attachement du socket à l'adresse et au port spécifiés
        if (bind(discovery_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            close(discovery_socket); // Fermeture du socket en cas d'échec
            throw std::runtime_error("Erreur lors du bind du socket de découverte.");
        }

        std::cout << "Serveur en attente de messages de découverte..." << std::endl;

        // Boucle principale pour écouter les messages de découverte
        while (m_discovery_running) {
            // Configuration d'un timeout pour recvfrom afin de ne pas bloquer indéfiniment
            struct timeval tv;
            tv.tv_sec = 1; // Timeout de 1 seconde
            tv.tv_usec = 0;
            setsockopt(discovery_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            // Réception d'un message de découverte
            int n = recvfrom(discovery_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
            if (n > 0) {
                std::cout << "Message de découverte reçu, envoi de la réponse..." << std::endl;
                // Obtention de l'adresse IP locale
                std::string localIP = getLocalIP();
                // Réponse au client avec l'adresse IP locale
                sendto(discovery_socket, localIP.c_str(), localIP.size(), 0, (struct sockaddr *)&client_addr, addr_len);
                std::cout << "Réponse envoyée avec l'IP : " << localIP << std::endl;
            } else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                // Gestion des erreurs autres que les timeouts
                std::cerr << "Erreur de réception: " << strerror(errno) << std::endl;
            }
        }

        close(discovery_socket);  // Fermeture propre du socket une fois la boucle terminée
    } catch (const std::exception& e) {
        // Capture et affichage des exceptions
        std::cerr << "Erreur dans listenForDiscovery : " << e.what() << std::endl;
    }
}
