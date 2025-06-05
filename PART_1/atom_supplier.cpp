#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <limits>
#include <netdb.h>
#include <cstring>

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

    // Create a TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }


    struct addrinfo hints;
    struct addrinfo *res, *rp;
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family= AF_INET;     
    hints.ai_socktype = SOCK_STREAM;   

    err = getaddrinfo(host, argv[2], &hints, &res);
    if (err != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
        close(sock);
        return 1;
    }


    for (rp = res; rp != nullptr; rp = rp->ai_next) {


        // what is the last argument in th
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) {
            continue;
        }

        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }

        close(sock);
    }

    if (rp == nullptr) {
        std::cerr << "Could not connect to " << host << ":" << port << "\n";
        freeaddrinfo(res);
        return 1;
    }

    freeaddrinfo(res);
    
    //Interactive loop
    while (true) {
        std::cout << "\nSelect atom type:\n"
        << " 1) HYDROGEN\n"
        << " 2) OXYGEN\n"
        << " 3) CARBON\n"
        << " 4) Exit\n"
        << "Choice: ";

        int choice;
        if (!(std::cin >> choice) || choice == 4) {
            break;
        }

        std::string type;
        switch (choice) {
            case 1: type = "HYDROGEN"; 
                break;
            case 2: type = "OXYGEN";   
                break;
            case 3: type = "CARBON";   
                break;
            default:
                std::cout << "Invalid option\n";
                continue;
        }

        std::cout << "Enter amount: ";
        
        unsigned int amt;
        if (!(std::cin >> amt)) {
            std::cin.clear();
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

        // Wait for the server response
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