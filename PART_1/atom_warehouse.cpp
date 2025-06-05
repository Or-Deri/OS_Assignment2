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
#include <signal.h>
// Maximum line length we expect from a client
static const size_t MAX_LINE = 1024;

void handle_sigint(int sig)
{
    std::exit(0);
}

int main(int argc, char *argv[])
{

    signal(SIGINT, handle_sigint);
    
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        std::exit(1);
    }

    // Parse port
    int port = std::stoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Invalid port number.\n";
        std::exit(1);
    }

    // 1) Create listening socket
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
    {
        perror("socket");
        std::exit(1);
    }

    int opt = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(listener);
        std::exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listener, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(listener);
        std::exit(1);
    }

    if (listen(listener, SOMAXCONN) < 0)
    {
        perror("listen");
        close(listener);
        std::exit(1);
    }

    std::cout << "Server listening on port " << port << "...\n";

    //
    // 2) set up select()’s fd_sets
    //
    fd_set master_set;
    fd_set read_set;
    FD_ZERO(&master_set);
    FD_SET(listener, &master_set);
    int fd_max = listener;

    FD_SET(STDIN_FILENO, &master_set); // Add stdin for interactive commands

    if (STDIN_FILENO > fd_max)
    {
        fd_max = STDIN_FILENO;
    }

    // Per‐client partial‐line buffers
    //   key = client_fd, value = buffered data not yet parsed into a full line
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

        // 3) Check each fd
        for (int fd = 0; fd <= fd_max; ++fd)
        {
            if (!FD_ISSET(fd, &read_set))
            {
                continue;
            }

            if (fd == listener)
            {
                // New incoming connection
                int client_fd = accept(listener, nullptr, nullptr);
                if (client_fd < 0)
                {
                    perror("accept");
                    continue;
                }
                FD_SET(client_fd, &master_set);
                if (client_fd > fd_max)
                {
                    fd_max = client_fd;
                }
                // Initialize its buffer
                bufmap[client_fd] = "";
            }

            else
            {
                // Existing client is readable
                char tmpbuf[512];
                ssize_t nbytes = recv(fd, tmpbuf, sizeof(tmpbuf) - 1, 0);
                if (nbytes <= 0)
                {
                    // Client closed or error
                    if (nbytes < 0)
                    {
                        perror("recv");
                    }
                    close(fd);
                    FD_CLR(fd, &master_set);
                    bufmap.erase(fd);
                }
                else
                {
                    // Append to this client's buffer
                    tmpbuf[nbytes] = '\0';
                    bufmap[fd].append(tmpbuf);

                    // Process complete lines
                    size_t pos;
                    while ((pos = bufmap[fd].find('\n')) != std::string::npos)
                    {
                        std::string line = bufmap[fd].substr(0, pos);
                        // Remove the line + newline from buffer
                        bufmap[fd].erase(0, pos + 1);

                        // Trim any CR if sent by a Windows client
                        if (!line.empty() && line.back() == '\r')
                        {
                            line.pop_back();
                        }

                        // Parse: ADD <ATOM> <amount>
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

                        if (success)
                        {
                            warehouse.print_state();
                            const char *resp = "OK\n";
                            send(fd, resp, std::strlen(resp), 0);
                        }
                        else
                        {
                            warehouse.print_state();
                            const char *resp = "ERROR\n";
                            send(fd, resp, std::strlen(resp), 0);
                        }
                    }
                }
            }
        }
    }

    // Cleanup
    close(listener);
    return 0;
}