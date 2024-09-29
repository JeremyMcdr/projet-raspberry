#ifndef APP_H
#define APP_H

#include <csignal>
#include <thread>
#include <chrono>
#include <iostream>

#include "NetworkComm.h"
#include "XBeeManager.h"

enum class ProgramState {
    RUNNING, 
    STOPPED

};

class App {
public:
    App();
    ~App();

    // Méthode pour initialiser l'application (gestion des signaux, etc.)
    bool Init();

    // Méthode pour démarrer et exécuter l'application (serveur WebSocket)
    bool Run();

    void Quit();

private:
    ProgramState state;

    XBeeManager xb_manager;
    WebSocketServer ws_server;

    bool PollXBeeEvent();

    void fill_message(XBeeMessage& message);
    void mac_string_to_bytes(const std::string& mac_str, uint8_t* mac_bytes);

};

#endif // APP_H