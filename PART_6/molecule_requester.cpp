#include <iostream>
#include <string>
#include <limits>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>

static const size_t MAX_LINE = 1024;

int main(int argc, char* argv[]) {

    std::string uds_path;
    std::string host;
    int udp_port = -1;

    bool use_udp = false;
    bool use_uds = false;

    int opt;

    while ((opt = getopt(argc, argv, "f:h:p:")) != -1) {
        switch (opt) {
            case 'f':
                if (use_udp) {
                    std::cerr << "ERROR: Cannot specify both -f (UDS) and -h/-p (UDP) simultaneously.\n";
                    return 1;
                }
                uds_path = optarg;
                use_uds = true;
                break;

            case 'h':
                if (use_uds) {
                    std::cerr << "ERROR: Cannot specify both -f (UDS) and -h/-p (UDP) simultaneously.\n";
                    return 1;
                }
                host = optarg;
                use_udp = true;
                break;

            case 'p':
                if (!use_udp) {
                    std::cerr << "ERROR: -p <port> must be used together with -h <hostname/IP>.\n";
                    return 1;
                }
                udp_port = std::atoi(optarg);
                break;

            default:
                std::cerr << "Usage:\n"
                << "  " << argv[0] << " -f <UDS socket file path>\n"
                << "  OR\n"
                << "  " << argv[0] << " -h <hostname/IP> -p <port>\n";
                return 1;
        }
    }

    // Validate combinations:
    if (use_uds) {
        if (!host.empty() || udp_port != -1) {
            std::cerr << "ERROR: Cannot specify both -f (UDS) and -h/-p (UDP) simultaneously.\n";
            return 1;
        }
        if (uds_path.empty()) {
            std::cerr << "ERROR: -f requires a valid UDS socket file path.\n";
            return 1;
        }
    }
    else if (use_udp) {
        if (host.empty() || udp_port <= 0 || udp_port > 65535) {
            std::cerr << "ERROR: For UDP you must specify both -h <hostname/IP> and -p <port> (1..65535).\n";
            return 1;
        }
    }
    else {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " -f <UDS socket file path>\n"
                  << "  OR\n"
                  << "  " << argv[0] << " -h <hostname/IP> -p <port>\n";
        return 1;
    }


    // Create socket:
    int sock;
    if (use_udp) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("socket(AF_INET)");
            return 1;
        }
    } else {
        sock = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("socket(AF_UNIX)");
            return 1;
        }
    }

    // Main interactive loop:
    while (true) {
        std::string molecule;
        unsigned int amount;

        std::cout << "Molecule to request (WATER / CARBON DIOXIDE / ALCOHOL / GLUCOSE): ";
        std::getline(std::cin, molecule);
        if (molecule.empty()) continue;

        std::cout << "Amount: ";
        if (!(std::cin >> amount)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid number\n";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string request = "DELIVER " + molecule + " " + std::to_string(amount) + "\n";

        if (use_udp) {
            sockaddr_in serv{};
            serv.sin_family = AF_INET;
            serv.sin_port = htons(udp_port);
            if (inet_pton(AF_INET, host.c_str(), &serv.sin_addr) <= 0) {
                perror("inet_pton");
                close(sock);
                return 1;
            }

            if (sendto(sock, request.c_str(), request.size(), 0,
                       reinterpret_cast<sockaddr*>(&serv), sizeof(serv)) < 0) {
                perror("sendto(UDP)");
                break;
            }

            char buffer[MAX_LINE];
            sockaddr_in from{};
            socklen_t from_len = sizeof(from);
            ssize_t n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                 reinterpret_cast<sockaddr*>(&from), &from_len);
            if (n > 0) {
                buffer[n] = '\0';
                std::cout << "Response: " << buffer;
            }
        }
        else {
            sockaddr_un serv{};
            serv.sun_family = AF_UNIX;
            strncpy(serv.sun_path, uds_path.c_str(), sizeof(serv.sun_path) - 1);

            if (sendto(sock, request.c_str(), request.size(), 0,
                       reinterpret_cast<sockaddr*>(&serv), sizeof(serv)) < 0) {
                perror("sendto(UDS)");
                break;
            }

            char buffer[MAX_LINE];
            sockaddr_un from{};
            socklen_t from_len = sizeof(from);
            ssize_t n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                 reinterpret_cast<sockaddr*>(&from), &from_len);
            if (n > 0) {
                buffer[n] = '\0';
                std::cout << "Response: " << buffer;
            }
        }
    }
    close(sock);
    return 0;
}