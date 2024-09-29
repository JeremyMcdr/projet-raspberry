#ifndef NETWORK_COMM_H
#define NETWORK_COMM_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "XBeeManager.h"

#include <set>
#include <mutex>
#include <string>
#include <thread>
#include <atomic>

typedef websocketpp::server<websocketpp::config::asio> server;

class WebSocketServer {
public:
    WebSocketServer();
    void run(uint16_t port);
    void stop();
    WebSocketServer(XBeeManager& xbee_manager);

    // Fonction pour démarrer l'écoute de découverte
    void startDiscoveryListener();
    void stopDiscoveryListener();

private:
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
    
    XBeeManager& m_xbee_manager;
    // Fonction pour écouter les messages de découverte
    void listenForDiscovery();

    server m_server;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> m_connections;
    std::mutex m_connection_lock;

    // Variables pour la découverte
    std::thread m_discovery_thread;
    std::atomic<bool> m_discovery_running;
};

#endif // NETWORK_COMM_H
