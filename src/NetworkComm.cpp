#include "NetworkComm.h"
#include <iostream>
#include <functional>
#include <cstring>
#include <stdexcept>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

WebSocketServer::WebSocketServer() : m_discovery_running(false) {
    m_server.init_asio();

    m_server.set_open_handler(std::bind(&WebSocketServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(std::bind(&WebSocketServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(std::bind(&WebSocketServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void WebSocketServer::run(uint16_t port) {
    // Démarrer le thread de découverte
    startDiscoveryListener();

    m_server.listen(port);
    m_server.start_accept();
    m_server.run();

    // Arrêter le thread de découverte une fois que le serveur WebSocket est arrêté
    stopDiscoveryListener();
}

void WebSocketServer::stop() {
    m_server.stop_listening();

    std::lock_guard<std::mutex> guard(m_connection_lock);
    for (auto hdl : m_connections) {
        m_server.close(hdl, websocketpp::close::status::going_away, "");
        //Pause pour laisser le temps à chaque connexion de se fermer proprement 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  
    }
    m_server.stop();

    // Arrêter le thread de découverte
    stopDiscoveryListener();
}

void WebSocketServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> guard(m_connection_lock);
    m_connections.insert(hdl);
    std::cout << "Client connecté." << std::endl;
}

void WebSocketServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> guard(m_connection_lock);
    m_connections.erase(hdl);
    std::cout << "Client déconnecté." << std::endl;
}

void WebSocketServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::string message = msg->get_payload();
    std::cout << "Message reçu: " << message << std::endl;

    // Traiter le message reçu
    if (message == "Bouton 1 Appuyé") {
        std::cout << "Action pour le Bouton 1" << std::endl;
    } else if (message == "Bouton 2 Appuyé") {
        std::cout << "Action pour le Bouton 2" << std::endl;
    } else {
        std::cout << "Message inconnu." << std::endl;
    }

    // Envoyer une réponse au client
    m_server.send(hdl, "Message reçu et traité !", websocketpp::frame::opcode::text);
}

void WebSocketServer::startDiscoveryListener() {
    m_discovery_running = true;
    m_discovery_thread = std::thread(&WebSocketServer::listenForDiscovery, this);
}

void WebSocketServer::stopDiscoveryListener() {
    m_discovery_running = false;
    if (m_discovery_thread.joinable()) {
        m_discovery_thread.join();
    }
}

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

void WebSocketServer::listenForDiscovery() {
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

        std::cout << "Serveur en attente de messages de découverte..." << std::endl;

        while (m_discovery_running) {
            // Configurer un timeout pour recvfrom
            struct timeval tv;
            tv.tv_sec = 1; // Timeout de 1 seconde
            tv.tv_usec = 0;
            setsockopt(discovery_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            // Recevoir un message de découverte
            int n = recvfrom(discovery_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
            if (n > 0) {
                std::cout << "Message de découverte reçu, envoi de la réponse..." << std::endl;
                // Récupérer l'adresse IP locale
                std::string localIP = getLocalIP();
                // Répondre au client avec l'adresse IP
                sendto(discovery_socket, localIP.c_str(), localIP.size(), 0, (struct sockaddr *)&client_addr, addr_len);
                std::cout << "Réponse envoyée avec l'IP : " << localIP << std::endl;
            } else if (n==-1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "Erreur de réception: " << strerror(errno) << std::endl;
            }
        }

        close(discovery_socket);  // Fermer le socket proprement
    } catch (const std::exception& e) {
        std::cerr << "Erreur dans listenForDiscovery : " << e.what() << std::endl;
    }
}
