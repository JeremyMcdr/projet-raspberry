#ifndef NETWORK_COMM_H
#define NETWORK_COMM_H

#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // pour close()

class NetworkComm {
public:
    NetworkComm(int port);
    void acceptClient();
    void sendMessage(const std::string& message);
    std::string receiveMessage();
    ~NetworkComm();

private:
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
};

#endif
