#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>

static const size_t MAX_LINE = 1024;

int main(int argc, char *argv[]) {
    std::string host;
    int tcp_port = -1;
    std::string uds_path;

    bool use_tcp = false;
    bool use_uds = false;

    int opt;

    while ((opt = getopt(argc, argv, "f:h:p:")) != -1) {
        switch (opt) {
            case 'f':
                if (use_tcp) {
                    std::cerr << "ERROR: Cannot specify both -f (UDS) and -h/-p (TCP) at the same time.\n";
                    return 1;
                }
                uds_path = optarg;
                use_uds = true;
                break;

            case 'h':
                if (use_uds) {
                    std::cerr << "ERROR: Cannot specify both -f (UDS) and -h/-p (TCP) at the same time.\n";
                    return 1;
                }
                host = optarg;
                use_tcp = true;
                break;

            case 'p':
                if (!use_tcp) {
                    std::cerr << "ERROR: -p <port> must be used together with -h <hostname/IP>.\n";
                    return 1;
                }
                tcp_port = std::atoi(optarg);
                break;

            default:
                std::cerr << "Usage:\n"
                          << "  " << argv[0] << " -f <UDS socket file path>\n"
                          << "  OR\n"
                          << "  " << argv[0] << " -h <hostname/IP> -p <port>\n";
                return 1;
        }
    }

    if (use_uds) {
        if (!host.empty() || tcp_port != -1) {
            std::cerr << "ERROR: Cannot specify both -f (UDS) and -h/-p (TCP) at the same time.\n";
            return 1;
        }
        if (uds_path.empty()) {
            std::cerr << "ERROR: -f requires a valid UDS socket file path.\n";
            return 1;
        }
    } else if (use_tcp) {
        if (host.empty() || tcp_port <= 0 || tcp_port > 65535) {
            std::cerr << "ERROR: For TCP you must specify both -h <hostname/IP> and -p <port>.\n";
            return 1;
        }
    } else {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " -f <UDS socket file path>\n"
                  << "  OR\n"
                  << "  " << argv[0] << " -h <hostname/IP> -p <port>\n";
        return 1;
    }

    int sock = -1;

    if (use_tcp) {
        struct addrinfo hints{}, *res = nullptr, *rp = nullptr;
        std::string port_str = std::to_string(tcp_port);

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        int err = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
        if (err != 0) {
            std::cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
            return 1;
        }

        for (rp = res; rp != nullptr; rp = rp->ai_next) {
            sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sock < 0) continue;

            if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;

            close(sock);
        }

        freeaddrinfo(res);

        if (rp == nullptr) {
            std::cerr << "Could not connect to " << host << ":" << tcp_port << "\n";
            return 1;
        }

    } else {
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("socket(AF_UNIX)");
            return 1;
        }

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, uds_path.c_str(), sizeof(addr.sun_path) - 1);

        if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            perror("connect(UDS)");
            close(sock);
            return 1;
        }
    }

    while (true) {
        std::cout << "\nSelect atom type:\n"
                  << " 1) HYDROGEN\n"
                  << " 2) OXYGEN\n"
                  << " 3) CARBON\n"
                  << " 4) Exit\n"
                  << "Choice: ";

        int choice;
        if (!(std::cin >> choice) || choice == 4) break;

        std::string type;
        switch (choice) {
            case 1: type = "HYDROGEN"; break;
            case 2: type = "OXYGEN"; break;
            case 3: type = "CARBON"; break;
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

        std::string cmd = "ADD " + type + " " + std::to_string(amt) + "\n";
        if (send(sock, cmd.c_str(), cmd.size(), 0) < 0) {
            perror("send");
            break;
        }

        char resp[MAX_LINE];
        ssize_t n = recv(sock, resp, sizeof(resp) - 1, 0);
        if (n <= 0) {
            if (n < 0) perror("recv");
            break;
        }

        resp[n] = '\0';
        std::cout << "Server response: " << resp;
    }

    close(sock);
    return 0;
}
