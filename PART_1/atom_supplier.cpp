// atom_supplier_client.cpp
//
// A simple interactive TCP client that connects to the "atom_warehouse" server.
// Usage: ./atom_supplier_client <hostname/IP> <port>
//
// Repeatedly asks the user to choose an atom type and amount, then sends
// "ADD <ATOM> <amount>\n" over the socket and prints the server's response.
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <limits>


static const size_t MAX_LINE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname/IP> <port>\n";
        return 1;
    }

    const char* host = argv[1];
    int port = std::atoi(argv[2]);


    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number.\n";
        return 1;
    }

    // 2) Create a TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // 3) Fill in server address structure
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    // 4) Connect to the server
    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    // 5) Interactive loop
    while (true) {
        std::cout << "\nSelect atom type:\n"
                  << " 1) HYDROGEN\n"
                  << " 2) OXYGEN\n"
                  << " 3) CARBON\n"
                  << " 4) Exit\n"
                  << "Choice: ";

        int choice;
        if (!(std::cin >> choice) || choice == 4) {
            // Either non‐integer input or the user chose “4 = Exit”
            break;
        }

        std::string type;
        switch (choice) {
            case 1: type = "HYDROGEN"; break;
            case 2: type = "OXYGEN";   break;
            case 3: type = "CARBON";   break;
            default:
                std::cout << "Invalid option\n";
                continue;
        }

        std::cout << "Enter amount: ";
        
        unsigned int amt;
        if (!(std::cin >> amt)) {
            std::cin.clear();                // clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid number\n";
            continue;
        }

        // Form the command string and send it
        std::string cmd = "ADD " + type + " " + std::to_string(amt) + "\n";
        if (send(sock, cmd.c_str(), cmd.size(), 0) < 0) {
            perror("send");
            break;
        }

        // Wait for the server’s response
        char resp[MAX_LINE];
        ssize_t n = recv(sock, resp, sizeof(resp) - 1, 0);
        if (n <= 0) {
            // n == 0 means server closed the connection
            if (n < 0) perror("recv");
            break;
        }
        resp[n] = '\0';
        std::cout << "Server response: " << resp;
    }

    close(sock);
    return 0;
}
