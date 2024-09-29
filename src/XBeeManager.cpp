#include "XBeeManager.h"


XBeeManager::XBeeManager(){

}

XBeeManager::~XBeeManager() {

}



// Méthode pour initialiser la communication avec le XBee
bool XBeeManager::InitXBee() {
    portName = "/dev/ttyUSB0";
    std::cout << "[LOG] Initialisation du XBee avec le port : " << portName << std::endl;

    // Ouvrir le port série
    if(!openSerialPort()){
        std::cout << "[ERREUR] Impossibler ouvrir le port /dev/ttyUSB0 " << std::endl;
    }
    return true;  
}

// Méthode interne pour ouvrir le port série
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

// Méthode pour fermer la communication XBee
void XBeeManager::CloseXBee() {
    std::cout << "[LOG] Fermeture du XBee..." << std::endl;
    closeSerialPort();
}

// Méthode interne pour fermer le port série
void XBeeManager::closeSerialPort() {
    if (serialPortFd != -1) {
        close(serialPortFd);
        std::cout << "[LOG] Port série fermé." << std::endl;
        serialPortFd = -1;
    }
}


// Fonction pour envoyer un message XBee via le port série
void XBeeManager::send_xbee_message(XBeeMessage& message) {
    // Préparer un tableau de 12 octets pour le calcul de la parité (8 octets pour l'adresse MAC + 2 pour l'adresse + 2 pour les données)
    uint8_t buffer[12];
    memcpy(buffer, message.mac_address, 8);  // Ajouter l'adresse MAC dans les 8 premiers octets*
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