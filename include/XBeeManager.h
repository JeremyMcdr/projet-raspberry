/**
 * XBeeManager.h
 * 
 * Description:
 * Ce fichier définit la classe XBeeManager, utilisée pour gérer la communication avec un module XBee 
 * via un port série. La classe encapsule les fonctionnalités nécessaires pour l'envoi de messages
 * via XBee, telles que l'initialisation et la fermeture du port série, ainsi que la gestion des messages
 * XBee (adresse MAC, adresse courte, données).
 * 
 * Cette classe est conçue pour faciliter la communication entre un Raspberry Pi (ou autre appareil) et 
 * des modules XBee dans un réseau de capteurs ou d'actionneurs. Elle permet d'envoyer des commandes et
 * de transmettre des données à d'autres modules connectés en utilisant le protocole XBee.
 * 
 * Attributs principaux:
 * - Nom du port série (portName) : "/dev/ttyUSB0", spécifie le port série utilisé pour communiquer avec le module XBee.
 * - Descripteur de fichier du port série (serialPortFd) : int, permet l'accès bas niveau au port série.
 * 
 * Méthodes principales:
 * - InitXBee() : Initialise la communication XBee en configurant le port série.
 * - CloseXBee() : Ferme proprement la communication XBee et libère les ressources du port série.
 * - sendXBeeMessage() : Envoie un message structuré (adresse MAC, adresse courte, données, parité) via le module XBee.
 * - fillMessage() : Remplit manuellement un message XBee avec les informations nécessaires avant son envoi.
 * 
 * Auteur: Macadré Nicolas
 * Date: 23/09/2024
 * 
 */

#ifndef XBEEMANAGER_H
#define XBEEMANAGER_H

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string>

// Structure représentant le message à envoyer via XBee
struct XBeeMessage {
    uint8_t mac_address[8];   // Adresse MAC sous forme de tableau de 8 octets
    uint16_t address;         // Adresse codée sur 2 octets
    uint16_t data;            // Donnée codée sur 2 octets
    uint8_t parity;           // Byte de parité
};


class XBeeManager {
public:
    XBeeManager();  // Le constructeur initialise le port série avec une valeur codée en dur
    ~XBeeManager();

    // Méthode pour initialiser le XBee (ouvre le port série par défaut codé en dur)
    bool InitXBee();

    // Méthode pour fermer le XBee (inclut la fermeture du port série et autres désinitialisations)
    void CloseXBee();

    void send_xbee_message(XBeeMessage& message);



private:
    // Méthode interne pour ouvrir le port série
    bool openSerialPort();

    // Méthode interne pour fermer le port série
    void closeSerialPort();

    // Calcule le byte de parité du message
    uint8_t calculate_parity(const uint8_t* data, int length);

    std::string portName;     // Le nom du port série (codé en dur)
    int serialPortFd;         // Descripteur de fichier pour le port série


};

#endif // XBEEMANAGER_H