#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <vector>

static const size_t MAX_LINE = 1024;

int main(int argc, char *argv[])
{
    std::string host;
    std::string uds_path;
    int udp_port = -1;
    bool use_udp = false;
    bool use_uds = false;

    int opt;
    while ((opt = getopt(argc, argv, "f:h:p:")) != -1)
    {
        switch (opt)
        {
        case 'f':
            if (use_udp)
            {
                std::cerr << "ERROR: Cannot use both -f and -h/-p\n";
                return 1;
            }
            uds_path = optarg;
            use_uds = true;
            break;
        case 'h':
            if (use_uds)
            {
                std::cerr << "ERROR: Cannot use both -f and -h/-p\n";
                return 1;
            }
            host = optarg;
            use_udp = true;
            break;
        case 'p':
            if (!use_udp)
            {
                std::cerr << "ERROR: -p must be used with -h\n";
                return 1;
            }
            udp_port = std::atoi(optarg);
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " -f <path> | -h <host> -p <port>\n";
            return 1;
        }
    }

    if (!use_udp && !use_uds)
    {
        std::cerr << "Must specify -f or -h/-p\n";
        return 1;
    }

    int sock = -1;
    sockaddr_storage addr{};
    socklen_t addrlen = 0;
    if (use_udp)
    {
        if (host.empty() || udp_port <= 0 || udp_port > 65535)
        {
            std::cerr << "Invalid host or port\n";
            return 1;
        }

        struct addrinfo hints{}, *res = nullptr;

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        std::string port_str = std::to_string(udp_port);
        int err = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
        if (err != 0 || !res)
        {
            std::cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
            return 1;
        }
        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0)
        {
            perror("socket(UDP)");
            freeaddrinfo(res);
            return 1;
        }
        memcpy(&addr, res->ai_addr, res->ai_addrlen);
        addrlen = res->ai_addrlen;
        freeaddrinfo(res);
    }
    else
    {
        sock = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            perror("socket(AF_UNIX)");
            return 1;
        }
        sockaddr_un *sun = (sockaddr_un *)&addr;
        sun->sun_family = AF_UNIX;
        strncpy(sun->sun_path, uds_path.c_str(), sizeof(sun->sun_path) - 1);
        addrlen = sizeof(sockaddr_un);
    }

    while (true)
    {
        std::cout << "\nSelect molecule:\n"
                  << " 1) WATER\n"
                  << " 2) CARBON DIOXIDE\n"
                  << " 3) ALCOHOL\n"
                  << " 4) GLUCOSE\n"
                  << " 5) Exit\n"
                  << "Choice: ";
        int choice;
        if (!(std::cin >> choice) || choice == 5)
            break;
        std::string mol;
        switch (choice)
        {
        case 1:
            mol = "WATER";
            break;
        case 2:
            mol = "CARBON DIOXIDE";
            break;
        case 3:
            mol = "ALCOHOL";
            break;
        case 4:
            mol = "GLUCOSE";
            break;
        default:
            std::cout << "Invalid choice\n";
            continue;
        }
        std::cout << "Enter amount: ";
        unsigned int amt;
        if (!(std::cin >> amt))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid number\n";
            continue;
        }
        
        std::string cmd = "DELIVER " + mol + " " + std::to_string(amt) + "\n";
        if (sendto(sock, cmd.c_str(), cmd.size(), 0, (sockaddr *)&addr, addrlen) < 0)
        {
            perror("sendto");
            break;
        }
        char resp[MAX_LINE];
        ssize_t n = recvfrom(sock, resp, sizeof(resp) - 1, 0, nullptr, nullptr);
        if (n <= 0)
        {
            if (n < 0)
                perror("recvfrom");
            break;
        }
        resp[n] = '\0';
        std::cout << "Server response: " << resp;
    }
    close(sock);
    return 0;
}
