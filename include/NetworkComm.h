#ifndef NETWORK_COMM_H
#define NETWORK_COMM_H

#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // pour close()

class NetworkComm {
public:
    NetworkComm(int port);
    NetworkComm(); // Constructeur par défaut
    int acceptClient();
    void sendMessage(int client_socket, const std::string& message);
    std::string receiveMessage(int client_socket);
    ~NetworkComm();

private:
    int server_fd;
    struct sockaddr_in server_addr;
};

#endif
