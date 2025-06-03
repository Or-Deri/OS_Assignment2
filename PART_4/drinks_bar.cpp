//
// drinks_bar.cpp
//
// Part 4: A TCP+UDP server on the same port.
//   • TCP (SOCK_STREAM): clients send "ADD <ATOM> <amount>\n".
//       Valid ATOMs are CARBON, OXYGEN, HYDROGEN.
//       On success, reply "OK\n"; otherwise "ERROR\n".
//   • UDP (SOCK_DGRAM): clients send "DELIVER <MOLECULE> <amount>\n".
//       Valid MOLECULEs are WATER, CARBON DIOXIDE, ALCOHOL, GLUCOSE.
//       On success, deduct atoms (one molecule per unit) and reply
//       "DELIVERED <MOLECULE> <amount>\n"; otherwise "ERROR\n".
//
// Uses select() to multiplex the TCP listening socket, each TCP client
// socket, and the UDP socket.
//

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "Warehouse.hpp"

// Maximum line length we expect from a TCP client
static const size_t MAX_LINE = 1024;

int main(int argc, char *argv[])
{

    unsigned long long OXYGEN_ATOMS = 0;
    unsigned long long CARBON_ATOMS = 0;
    unsigned long long HYDROGEN_ATOMS = 0;
    unsigned long long TIMEOUT = 0;
    int tcp_p = -1;
    int udp_p = -1;

    bool must_use_tcp = false;
    bool must_use_udp = false;

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int opt;
    while ((opt = getopt(argc, argv, "o:c:h:t:T:U")) != -1)
    {

        if (opt = 'o')
        {
            if (std::stoull(optarg) < 0)
            {
                std::cerr << "Invalid value for -o: " << optarg << "\n";
                return 1;
            }
            OXYGEN_ATOMS = std::stoull(optarg);
            break;
        }

        if (opt = 'c')
        {
            if (std::stoull(optarg) < 0)
            {
                std::cerr << "Invalid value for -c: " << optarg << "\n";
                return 1;
            }
            CARBON_ATOMS = std::stoull(optarg);
            break;
        }
        if (opt = 'h')
        {
            if (std::stoull(optarg) < 0)
            {
                std::cerr << "Invalid value for -c: " << optarg << "\n";
                return 1;
            }
            CARBON_ATOMS = std::stoull(optarg);
            break;
        }
        if (opt = 't')
        {
            if (std::stoull(optarg) < 0)
            {
                std::cerr << "Invalid value for -c: " << optarg << "\n";
                return 1;
            }
            CARBON_ATOMS = std::stoull(optarg);
            break;
        }
        if (opt = 'T')
        {
            if (std::stoi(optarg) <= 0 || std::stoi(optarg) > 65535)
            {
                std::cerr << "Invalid port number for TCP.\n";
                return 1;
            }
            tcp_p = std::stoi(optarg);

            !must_use_tcp;
        }

        if (opt = 'U')
        {
            if (std::stoi(optarg) <= 0 || std::stoi(optarg) > 65535)
            {
                std::cerr << "Invalid port number for TCP.\n";
                return 1;
            }
            udp_p = std::stoi(optarg);

            !must_use_udp;
        }
    }

    if (!must_use_tcp || !must_use_udp)
    {
        std::cerr << "Invalid Usage - tcp port (-T) AND udp port (-U) are required)\n";
        return 1;
    }

    if (tcp_p <= 0 || tcp_p > 65535)
    {
        std::cerr << "Invalid port number for TCP.\n";
        return 1;
    }

    if (udp_p <= 0 || udp_p > 65535)
    {
        std::cerr << "Invalid port number for UDP.\n";
        return 1;
    }

    //
    // 1) Create and bind UDP socket
    //
    int udp_listener = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_listener < 0)
    {
        perror("socket(UDP)");
        return 1;
    }
    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(port);
    if (bind(udp_listener, (sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
    {
        perror("bind(UDP)");
        close(udp_listener);
        return 1;
    }

    //
    // 2) Create, bind, and listen on TCP socket
    //
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
    {
        perror("socket(TCP)");
        close(udp_listener);
        return 1;
    }
    int opt = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(listener);
        close(udp_listener);
        return 1;
    }
    sockaddr_in tcp_addr{};
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(port);
    if (bind(listener, (sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0)
    {
        perror("bind(TCP)");
        close(listener);
        close(udp_listener);
        return 1;
    }
    if (listen(listener, SOMAXCONN) < 0)
    {
        perror("listen");
        close(listener);
        close(udp_listener);
        return 1;
    }

    std::cout << "Server listening on port " << port << " (TCP+UDP)...\n";

    //
    // 3) Set up select()’s fd_sets, INCLUDING STDIN_FILENO
    //
    fd_set master_set;
    fd_set read_set;
    FD_ZERO(&master_set);

    FD_SET(listener, &master_set);
    FD_SET(udp_listener, &master_set);
    FD_SET(STDIN_FILENO, &master_set);

    int fd_max = std::max(listener, udp_listener, STDIN_FILENO);

    // Per‐TCP‐client partial‐line buffers
    std::unordered_map<int, std::string> bufmap;

    // Single shared Warehouse instance
    Warehouse warehouse;

    while (true)
    {
        read_set = master_set;
        if (select(fd_max + 1, &read_set, nullptr, nullptr, nullptr) < 0)
        {
            perror("select");
            break;
        }

        // 4) Check each fd
        for (int fd = 0; fd <= fd_max; ++fd)
        {
            if (!FD_ISSET(fd, &read_set))
            {
                continue;
            }

            //
            // 4.a) UDP request arrived?
            //
            if (fd == udp_listener)
            {
                char buffer[MAX_LINE];
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);

                ssize_t n = recvfrom(
                    udp_listener,
                    buffer,
                    sizeof(buffer) - 1,
                    0,
                    (sockaddr *)&client_addr,
                    &client_len);
                if (n < 0)
                {
                    perror("recvfrom(UDP)");
                    continue;
                }
                buffer[n] = '\0';
                std::string request = buffer;

                // Trim trailing newline/CR
                while (!request.empty() &&
                       (request.back() == '\n' || request.back() == '\r'))
                {
                    request.pop_back();
                }

                // Expect: "DELIVER <MOLECULE> <amount>"
                if (request.rfind("DELIVER ", 0) != 0)
                {
                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,
                           (sockaddr *)&client_addr, client_len);
                    continue;
                }

                // Split into tokens; last token = amount, rest = molecule
                std::istringstream iss(request);
                std::vector<std::string> tokens;

                std::string tok;
                while (iss >> tok)
                {
                    tokens.push_back(tok);
                }
                if (tokens.size() < 2)
                {
                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,
                           (sockaddr *)&client_addr, client_len);
                    continue;
                }

                // Parse amount (last token)
                unsigned long long amt = 0;
                try
                {
                    amt = std::stoull(tokens.back());
                }
                catch (...)
                {
                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,
                           (sockaddr *)&client_addr, client_len);
                    continue;
                }

                // Reconstruct full molecule name from tokens[1]..tokens[size-2]
                std::string mol_type = tokens[1];
                for (size_t i = 2; i + 1 < tokens.size(); ++i)
                {
                    mol_type += " " + tokens[i];
                }

                // Attempt to build 'amt' molecules
                bool success = true;

                for (unsigned long long i = 0; i < amt; ++i)
                {
                    if (!warehouse.build_molecule(mol_type))
                    {
                        success = false;
                        break;
                    }
                }

                if (success)
                {
                    std::string resp = "DELIVERED " + mol_type + " " + std::to_string(amt) + "\n";
                    sendto(udp_listener, resp.c_str(), resp.size(), 0,
                           (sockaddr *)&client_addr, client_len);
                }
                else
                {
                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,
                           (sockaddr *)&client_addr, client_len);
                }
            }
            //
            // 4.b) TCP listener ready? Accept new connection
            //
            else if (fd == listener)
            {
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listener, (sockaddr *)&client_addr, &client_len);
                if (client_fd < 0)
                {
                    perror("accept(TCP)");
                    continue;
                }
                FD_SET(client_fd, &master_set);
                if (client_fd > fd_max)
                {
                    fd_max = client_fd;
                }
                bufmap[client_fd] = "";
            }
            //
            // 4.c) Existing TCP client ready? Read data
            //
            else if (fd != STDIN_FILENO)
            {
                char tmpbuf[512];
                ssize_t nbytes = recv(fd, tmpbuf, sizeof(tmpbuf) - 1, 0);
                if (nbytes <= 0)
                {
                    if (nbytes < 0)
                    {
                        perror("recv(TCP)");
                    }
                    close(fd);
                    FD_CLR(fd, &master_set);
                    bufmap.erase(fd);
                }
                else
                {
                    tmpbuf[nbytes] = '\0';
                    bufmap[fd].append(tmpbuf);

                    // Process all complete lines ending in '\n'
                    size_t pos;
                    while ((pos = bufmap[fd].find('\n')) != std::string::npos)
                    {
                        std::string line = bufmap[fd].substr(0, pos);
                        bufmap[fd].erase(0, pos + 1);

                        // Trim trailing CR if present
                        if (!line.empty() && line.back() == '\r')
                        {
                            line.pop_back();
                        }

                        // Parse "ADD <ATOM> <amount>"
                        std::istringstream iss(line);
                        std::string cmd, atom_type;
                        unsigned long long amt;
                        bool success = false;

                        if ((iss >> cmd >> atom_type >> amt) && cmd == "ADD")
                        {
                            if (warehouse.is_valid_atom(atom_type))
                            {
                                if (warehouse.add_atom(atom_type, static_cast<unsigned int>(amt)))
                                {
                                    success = true;
                                }
                            }
                        }

                        const char *resp = success ? "OK\n" : "ERROR\n";
                        send(fd, resp, std::strlen(resp), 0);
                    }
                }
            }

            // GEN PART
            else if (fd == STDIN_FILENO)
            {
                std::string line;
                if (!std::getline(std::cin, line))
                    continue;

                while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
                    line.pop_back();

                if (line.rfind("GEN ", 0) != 0)
                {
                    std::cout << "Invalid command\n";
                    continue;
                }
                std::string drink = line.substr(4);
                int count = warehouse.build_drink_amount(drink);
                std::cout << "Can make " << count << " " << drink << std::endl;
            }
        }
    }

    // Cleanup
    close(listener);
    close(udp_listener);
    return 0;
}
