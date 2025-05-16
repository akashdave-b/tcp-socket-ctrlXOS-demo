//
// Created by akash at boschrexroth on 5/9/25.
//
#include <iostream>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

// Graceful shutdown flag
static volatile sig_atomic_t g_endProcess = false;
void sigIntHandler(int /*sig*/) {
    std::cout << "Signal received, shutting down..." << std::endl;
    g_endProcess = true;
}

constexpr const char* SERVER_IP   = "192.168.1.113";   //"10.0.2.2" (for virtual ctrlX Core);
constexpr int         SERVER_PORT = 5500;
constexpr int         HEARTBEAT_INTERVAL_SEC = 5;

int main() {
    // Register SIGINT handler for Ctrl-C
    std::signal(SIGINT, sigIntHandler);

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // Configure server address
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    // Connect to server
    if (connect(sock, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    // Main loop: send heartbeat, wait for commands
    while (!g_endProcess) {
        // Send heartbeat
        const char* heartbeat = "CTRLX - alive\n";
        if (send(sock, heartbeat, std::strlen(heartbeat), 0) < 0) {
            perror("send");
            break;
        }

        // Wait for response with timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        struct timeval tv{HEARTBEAT_INTERVAL_SEC, 0};
        int ret = select(sock + 1, &readfds, nullptr, nullptr, &tv);

        // If data does not arrive
        if (ret < 0) {
            if (errno == EINTR) continue;  // interrupted by signal
            perror("select");
            break;
        } else if (ret > 0 && FD_ISSET(sock, &readfds)) {   // if data arrived, read it
            char buffer[128];
            ssize_t len = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (len < 0) {
                perror("recv");
                break;
            } else if (len == 0) {
                std::cout << "Server closed connection" << std::endl;
                break;
            }
            //handle disonnect or erros
            buffer[len] = '\0';
            std::cout << "Received command: " << buffer;
            // TODO: parse commands like "LED_ON" / "LED_OFF"
        }
    }

    // Cleanup
    close(sock);
    return 0;
}
