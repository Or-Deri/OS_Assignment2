<<<<<<< HEAD
=======
// atom_supplier_client.cpp
//
// A simple interactive TCP client that connects to the "atom_warehouse" server.
// Usage: ./atom_supplier_client <hostname/IP> <port>
//
// Repeatedly asks the user to choose an atom type and amount, then sends
// "ADD <ATOM> <amount>\n" over the socket and prints the server's response.
//
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
#include <limits>
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <netdb.h>

static const size_t MAX_LINE = 1024;

int main(int argc, char* argv[]) {

    std::string host;
    std::string port;

    int opt;

    while ((opt = getopt(argc, argv, "h:p:")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -h <hostname/IP> -p <port>\n";
                return 1;
        }
    }

<<<<<<< HEAD
=======
    // 1) Parse hostname/IP and port
    const char* host = argv[1];
    int port = std::atoi(argv[2]);
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number.\n";
        return 1;
    }

<<<<<<< HEAD
    if (host.empty()) {
        std::cerr << "Hostname/IP must be specified.\n";
        return 1;
    }


=======
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
    int sock = -1;

    // 2) Create a TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

<<<<<<< HEAD
=======
    // 3) Resolve hostname + port בעזרת getaddrinfo
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    int err;

    memset(&hints, 0, sizeof(hints));
<<<<<<< HEAD
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;  


    err = getaddrinfo(host, argv[2], &hints, &res);

=======
    hints.ai_family   = AF_UNSPEC;      // IPv4 או IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP

    // argv[2] מכיל כבר את הטקסט של הפורט (כמו "5000")
    err = getaddrinfo(host, argv[2], &hints, &res);
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
    if (err != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
        close(sock);
        return 1;
    }

    
    for (rp = res; rp != nullptr; rp = rp->ai_next) {
        
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
<<<<<<< HEAD

=======
    // עכשיו sock הוא סוקט מחובר לשרת המתאים

    // // 3) Fill in server address structure
    // sockaddr_in serv{};
    // serv.sin_family = AF_INET;
    // serv.sin_port = htons(port);
    // if (inet_pton(AF_INET, host, &serv.sin_addr) <= 0) {
    //     perror("inet_pton");
    //     close(sock);
    //     return 1;
    // }

    // // 4) Connect to the server
    // if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) {
    //     perror("connect");
    //     close(sock);
    //     return 1;
    // }

    // 5) Interactive loop
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
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