// XBeeManager.cpp
#include "XBeeManager.h"
#include <iostream>
#include <cstring>
#include <cstdlib>  // Pour getenv

XBeeManager::XBeeManager() {
    // Constructor
}

XBeeManager::~XBeeManager() {
    // Destructor
    CloseXBee();
}

bool XBeeManager::InitXBee() {
    portName = "/dev/ttyUSB0";
    std::cout << "[LOG] Initialisation du XBee avec le port : " << portName << std::endl;

    // Vérifier si le mode test est activé via la variable d'environnement
    const char* env_p = std::getenv("TESTING_MODE");
    if (env_p != nullptr && std::string(env_p) == "true") {
        testing_mode = true;
        std::cout << "[LOG] Mode Test Activé. Les messages ne seront pas envoyés via le port série." << std::endl;
    }

    if(!testing_mode) {
        // Ouvrir le port série
        if(!openSerialPort()){
            std::cerr << "[ERREUR] Impossible d'ouvrir le port " << portName << std::endl;
            return false;
        }
    } else {
        std::cout << "[LOG] Mode Test : Simuler l'ouverture du port série." << std::endl;
    }

    return true;  
}

bool XBeeManager::openSerialPort() {
    std::cout << "[LOG] Ouverture du port série : " << portName << std::endl;
    serialPortFd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serialPortFd == -1) {
        perror("[ERREUR] Impossible d'ouvrir le port série");
        return false;
    }

    struct termios options;
    tcgetattr(serialPortFd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);  // Activer la réception et définir le mode local
    options.c_cflag &= ~PARENB;           // Pas de bit de parité
    options.c_cflag &= ~CSTOPB;           // 1 bit d'arrêt
    options.c_cflag &= ~CSIZE;            // Effacer le masque de taille des bits
    options.c_cflag |= CS8;               // 8 bits de données
    tcsetattr(serialPortFd, TCSANOW, &options);  // Appliquer les options immédiatement

    std::cout << "[LOG] Port série ouvert et configuré avec succès" << std::endl;
    return true;
}

void XBeeManager::CloseXBee() {
    std::cout << "[LOG] Fermeture du XBee..." << std::endl;
    closeSerialPort();
}

void XBeeManager::closeSerialPort() {
    if (serialPortFd != -1) {
        close(serialPortFd);
        std::cout << "[LOG] Port série fermé." << std::endl;
        serialPortFd = -1;
    } else if (testing_mode) {
        std::cout << "[LOG] Mode Test : Aucune action de fermeture nécessaire pour le port série." << std::endl;
    }
}

void XBeeManager::send_xbee_message(XBeeMessage& message) {
    if (testing_mode) {
        // En mode test, afficher le message au lieu de l'envoyer via le port série
        std::cout << "[TEST MODE] Message XBee Simulé Envoyé :" << std::endl;
        std::cout << "  Adresse MAC : ";
        for(int i = 0; i < 8; ++i) {
            printf("%02X ", message.mac_address[i]);
        }
        std::cout << std::endl;
        std::cout << "  Adresse     : 0x" << std::hex << message.address << std::endl;
        std::cout << "  Donnée      : 0x" << std::hex << message.data << std::endl;
        std::cout << "  Parité      : 0x" << std::hex << (int)message.parity << std::endl;
        return;
    }

    if (serialPortFd == -1) {
        std::cerr << "[ERREUR] Port série non ouvert." << std::endl;
        return;
    }
    // Préparer un tableau de 12 octets pour le calcul de la parité (8 octets pour l'adresse MAC + 2 pour l'adresse + 2 pour les données)
    uint8_t buffer[12];
    memcpy(buffer, message.mac_address, 8);  // Ajouter l'adresse MAC dans les 8 premiers octets
    buffer[8] = message.address & 0xFF;         // Octet de poids faible de l'adresse
    buffer[9] = (message.address >> 8) & 0xFF;  // Octet de poids fort de l'adresse
    buffer[10] = message.data & 0xFF;           // Octet de poids faible des données
    buffer[11] = (message.data >> 8) & 0xFF;    // Octet de poids fort des données

    std::cout << "[LOG] Calcul de la parité pour le message..." << std::endl;
    // Calculer la parité sur les 12 premiers octets
    message.parity = calculate_parity(buffer, 12);

    // Envoyer les données (12 octets + parité)
    int n = write(serialPortFd, buffer, 12);
    if (n < 0) {
        perror("[ERREUR] Echec de l'envoi du message");
    } else {
        std::cout << "[LOG] Message envoyé avec succès" << std::endl;
    }

    // Envoyer la parité
    n = write(serialPortFd, &message.parity, 1);
    if (n < 0) {
        perror("[ERREUR] Echec de l'envoi de la parité");
    } else {
        std::cout << "[LOG] Parité envoyée avec succès" << std::endl;
    }
}

// Calcul de la parité via XOR sur les octets
uint8_t XBeeManager::calculate_parity(const uint8_t* data, int length) {
    uint8_t parity = 0;
    for (int i = 0; i < length; i++) {
        parity ^= data[i];  // XOR sur les octets
    }
    std::cout << "[LOG] Parité calculée : 0x" << std::hex << (int)parity << std::endl;
    return parity;
}

void XBeeManager::setTestingMode(bool mode) {
    testing_mode = mode;
    if (testing_mode) {
        std::cout << "[LOG] Mode Test Activé via méthode setTestingMode." << std::endl;
    } else {
        std::cout << "[LOG] Mode Test Désactivé via méthode setTestingMode." << std::endl;
    }
}
