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
#include "../WareHouse/WareHouse.hpp"

// Maximum line length we expect from a TCP client
static const size_t MAX_LINE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    // Parse port number
    int port = std::stoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number.\n";
        return 1;
    }

    //Create and bind UDP socket
    int udp_listener = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_listener < 0) {
        perror("socket(UDP)");
        return 1;
    }

    int opt = 1;  //
    setsockopt(udp_listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//


    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(port);
    if (bind(udp_listener, (sockaddr*)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("bind(UDP)");
        close(udp_listener);
        return 1;
    }


    // Create, bind, and listen on TCP socket

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket(TCP)");
        close(udp_listener);
        return 1;
    }
    
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listener);
        close(udp_listener);
        return 1;
    }
    sockaddr_in tcp_addr{};
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(port);
    if (bind(listener, (sockaddr*)&tcp_addr, sizeof(tcp_addr)) < 0) {
        perror("bind(TCP)");
        close(listener);
        close(udp_listener);
        return 1;
    }
    if (listen(listener, SOMAXCONN) < 0) {
        perror("listen");
        close(listener);
        close(udp_listener);
        return 1;
    }

    std::cout << "Server listening on port " << port << " (TCP+UDP)...\n";


    fd_set master_set;
    fd_set read_fds;
    FD_ZERO(&master_set);
    FD_SET(listener, &master_set);
    FD_SET(udp_listener, &master_set);

    FD_SET(STDIN_FILENO, &master_set);
    int fd_max = listener;
    if (udp_listener > fd_max){
        fd_max = udp_listener;
    }
    if (STDIN_FILENO > fd_max){
        fd_max = STDIN_FILENO;
    }
    std::unordered_map<int, std::string> bufmap;

    // Single shared Warehouse instance
    Warehouse warehouse;

    while (true) {
        read_fds = master_set;
        if (select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr) < 0) {
            perror("select");
            break;
        }

        //  Check each fd
        for (int fd = 0; fd <= fd_max; ++fd) {
            if (!FD_ISSET(fd, &read_fds)) {
                continue;
            }
            if (fd == STDIN_FILENO){
                continue;
            }
            //UDP request arrived
            if (fd == udp_listener) {

                char buffer[MAX_LINE];
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);

                ssize_t n = recvfrom(udp_listener,buffer,sizeof(buffer) - 1, 0,(sockaddr*)&client_addr,&client_len);
                
                if (n < 0) {
                    perror("recvfrom(UDP)");
                    continue;
                }
                buffer[n] = '\0';
                std::string request = buffer;
                
                // Trim trailing newline/CR
                while (!request.empty() && (request.back() == '\n' || request.back() == '\r')) {
                    request.pop_back();
                }

                // Expect: "DELIVER <MOLECULE> <amount>"
                if (request.rfind("DELIVER ", 0) != 0) {
                    const char* err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0, (sockaddr*)&client_addr, client_len);
                    continue;
                }

                std::istringstream iss(request);
                std::vector<std::string> tokens;

                std::string tok;
                while (iss >> tok) {
                    tokens.push_back(tok);
                }
                if (tokens.size() < 3) {
                    const char* err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,(sockaddr*)&client_addr, client_len);
                    continue;
                }

                // Parse amount last token
                unsigned long long amt = 0;
                try {
                    amt = std::stoull(tokens.back());
                } catch (...) {
                    const char* err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,(sockaddr*)&client_addr, client_len);
                    continue;
                }

                std::string mol_type = tokens[1];
                for (size_t i = 2; i + 1 < tokens.size(); ++i) {
                    mol_type += " " + tokens[i];
                }

                // Attempt to build 'amt' molecules
                bool success = true;

                for (unsigned long long i = 0; i < amt; ++i) {
                    if (!warehouse.build_molecules(mol_type)) {
                        success = false;
                        break;
                    }
                }

                if (success) {
                    std::string resp = "DELIVERED " + mol_type + " " + std::to_string(amt) + "\n";
                    sendto(udp_listener, resp.c_str(), resp.size(), 0,(sockaddr*)&client_addr, client_len);
                    warehouse.print_state();
                } else {
                    const char* err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,(sockaddr*)&client_addr, client_len);
                    warehouse.print_state();
                }
            }
<<<<<<< HEAD

            
=======
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
            //if TCP listener ready Accept new connection
            else if (fd == listener) {
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listener, (sockaddr*)&client_addr, &client_len);
                
                if (client_fd < 0) {
                    perror("accept(TCP)");
                    continue;
                }
                FD_SET(client_fd, &master_set);
                if (client_fd > fd_max) {
                    fd_max = client_fd;
                }
                bufmap[client_fd] = "";
            }
            //
            // 4.c) Existing TCP client ready? Read data
            //
            else {
                char tmpbuf[512];
                ssize_t nbytes = recv(fd, tmpbuf, sizeof(tmpbuf) - 1, 0);
                if (nbytes <= 0) {
                    if (nbytes < 0) {
                        perror("recv(TCP)");
                    }
                    close(fd);
                    FD_CLR(fd, &master_set);
                    bufmap.erase(fd);
                } else {
                    tmpbuf[nbytes] = '\0';
                    bufmap[fd].append(tmpbuf);

                    // Process all complete lines ending in '\n'
                    size_t pos;
                    while ((pos = bufmap[fd].find('\n')) != std::string::npos){
                        std::string line = bufmap[fd].substr(0, pos);
                        bufmap[fd].erase(0, pos + 1);

                        // Trim trailing CR if present
                        if (!line.empty() && line.back() == '\r') {
                            line.pop_back();
                        }


                        std::istringstream iss(line);
                        std::string cmd;
                        iss >> cmd;

                        if (cmd == "ADD") {
                            std::string atom_type;
                            unsigned long long amt = 0;
                            bool success = false;

                            if ((iss >> atom_type >> amt) && warehouse.is_valid_atom(atom_type)) {
                                success = warehouse.add_atom(atom_type, static_cast<unsigned int>(amt));
                            }
                            const char* resp = success ? "OK\n" : "ERROR\n";
                            send(fd, resp, std::strlen(resp), 0);
                            warehouse.print_state();
                        }
                        else if (cmd == "GEN") {
                            
                            std::string drink_type;
                            std::getline(iss, drink_type);
                            if (!drink_type.empty() && drink_type.front() == ' ')
                                drink_type.erase(0, 1);

                            if (drink_type == "CHAMPAGNE" || drink_type == "VODKA" || drink_type == "SOFT DRINK") {
                                int maxd = warehouse.build_drink_amount(drink_type);
                                std::string resp = std::to_string(maxd) + "\n";
                                send(fd, resp.c_str(), resp.size(), 0);
                            } else {
                                send(fd, "ERROR\n", 6, 0);
                            }

                        }
                        else {
                            send(fd, "ERROR\n", 6, 0);
                        }

                    }
                }
            }
        }


        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
<<<<<<< HEAD


            std::string line;
            if (std::getline(std::cin, line)) {
                std::istringstream iss(line);
                std::string cmd;
                iss >> cmd;

=======
            std::string line;
            if (std::getline(std::cin, line)) {
                // כאן נטפל בשורה שהוקלדה כ-ADD או GEN
                std::istringstream iss(line);
                std::string cmd;
                iss >> cmd;

>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
                if (cmd == "ADD") {
                    std::string atom_type;
                    unsigned long long amt = 0;
                    bool success = false;
                    if ((iss >> atom_type >> amt) && warehouse.is_valid_atom(atom_type)) {
                        success = warehouse.add_atom(atom_type, static_cast<unsigned int>(amt));
                    }
                    std::cout << (success ? "OK\n" : "ERROR\n");
                    warehouse.print_state();
                }
                else if (cmd == "GEN") {
                    std::string drink_type;
                    std::getline(iss, drink_type);
                    if (!drink_type.empty() && drink_type.front() == ' ')
                        drink_type.erase(0, 1);

                    if (drink_type == "CHAMPAGNE" || drink_type == "VODKA" || drink_type == "SOFT DRINK") {
                        int maxd = warehouse.build_drink_amount(drink_type);
                        std::cout << maxd << "\n";
                    } else {
                        std::cout << "ERROR\n";
                    }
                    
                }
                else {
                    std::cout << "ERROR\n";
                }
            }
        }

    }

    // Cleanup
    close(listener);
    close(udp_listener);
    return 0;
}