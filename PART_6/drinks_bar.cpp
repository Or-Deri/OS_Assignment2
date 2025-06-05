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
#include <sys/un.h>
#include <fcntl.h>
#include <sys/file.h>
#include "../WareHouse/WareHouse.hpp"

static const size_t MAX_LINE = 1024;

/// @brief
/// @param file_path
/// @param wh
bool save_file_path(const std::string &file_path, const Warehouse &wh)
{
    int fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        std::cerr << "Couldn't open the file!" << std::endl;
        return false;
    }

    // after opening the file, we will lock it
    if (flock(fd, LOCK_EX) == -1)
    {
        std::cerr << "Couldn't lock the file!" << std::endl;
        close(fd);
        return false;
    }

    unsigned long long oxy = wh.get_OXYGEN();
    unsigned long long car = wh.get_CARBON();
    unsigned long long hyd = wh.get_HYDROGEN();

    bool ok = true;

    if (write(fd, &oxy, sizeof(oxy)) != sizeof(oxy))
        ok = false;
    if (write(fd, &car, sizeof(car)) != sizeof(car))
        ok = false;
    if (write(fd, &hyd, sizeof(hyd)) != sizeof(hyd))
        ok = false;

    if (!ok)
        std::cerr << "Couldn't write to the file!" << std::endl;

    flock(fd, LOCK_UN);
    close(fd);
    return ok;
}

/// @brief
/// @param file_path
/// @param wh
bool load_file_path(const std::string &file_path, Warehouse &wh)
{
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        return false; // file can't be opened
    }
    if (flock(fd, LOCK_SH) == -1)
    {
        std::cerr << "Couldn't lock the file for reading!" << std::endl;
        close(fd);
        return false;
    }
    unsigned long long oxy = 0;
    unsigned long long car = 0;
    unsigned long long hyd = 0;

    bool ok = true;
    // read the numbers from the file -
    // each line contains one number
    if (read(fd, &oxy, sizeof(oxy)) != sizeof(oxy))
        ok = false;
    if (read(fd, &car, sizeof(car)) != sizeof(car))
        ok = false;
    if (read(fd, &hyd, sizeof(hyd)) != sizeof(hyd))
        ok = false;

    if (!ok)
    {
        std::cerr << "Couldn't read from the file!" << std::endl;
        flock(fd, LOCK_UN);
        close(fd);
        return false;
    }

    wh.set_atoms("OXYGEN", oxy);
    wh.set_atoms("CARBON", car);
    wh.set_atoms("HYDROGEN", hyd);

    flock(fd, LOCK_UN);
    close(fd);
    return true;
}

int create_stream_uds(const char *path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket(stream_uds)");
        return -1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    unlink(path);

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind(stream_uds)");
        close(fd);
        return -1;
    }

    if (listen(fd, SOMAXCONN) < 0)
    {
        perror("listen(stream_uds)");
        close(fd);
        return -1;
    }

    return fd;
}

int create_dgram_uds(const char *path)
{
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket(dgram_uds)");
        return -1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    unlink(path);

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind(dgram_uds)");
        close(fd);
        return -1;
    }

    return fd;
}

int main(int argc, char *argv[])
{
    unsigned long long OXYGEN_ATOMS = 0;
    unsigned long long CARBON_ATOMS = 0;
    unsigned long long HYDROGEN_ATOMS = 0;
    unsigned long long TIMEOUT = 0;

    const char *uds_stream_path = nullptr;
    const char *uds_dgram_path = nullptr;
    std::string save_file_path_str;

    int tcp_p = -1;
    int udp_p = -1;

    bool must_use_tcp = false;
    bool must_use_udp = false;

    int opt;
    while ((opt = getopt(argc, argv, "o:c:h:t:T:U:s:d:f:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            OXYGEN_ATOMS = std::stoull(optarg);
            break;
        case 'c':
            CARBON_ATOMS = std::stoull(optarg);
            break;
        case 'h':
            HYDROGEN_ATOMS = std::stoull(optarg);
            break;
        case 't':
            TIMEOUT = std::stoull(optarg);
            break;
        case 'T':
            tcp_p = std::stoi(optarg);
            if (tcp_p <= 0 || tcp_p > 65535)
            {
                std::cerr << "Invalid TCP port: " << optarg << "\n";
                return 1;
            }
            must_use_tcp = true;
            break;
        case 'U':
            udp_p = std::stoi(optarg);
            if (udp_p <= 0 || udp_p > 65535)
            {
                std::cerr << "Invalid UDP port: " << optarg << "\n";
                return 1;
            }
            must_use_udp = true;
            break;
        case 's':
            uds_stream_path = optarg;
            break;
        case 'd':
            uds_dgram_path = optarg;
            break;
        case 'f':
            save_file_path_str = optarg;
            break;
        default:
            std::cerr << "Unknown option: " << char(opt) << "\n";
            return 1;
        }
    }

    if ((must_use_tcp || must_use_udp) && (uds_stream_path || uds_dgram_path))
    {
        std::cerr << "ERROR: Cannot specify both IP sockets (TCP/UDP) and UDS paths.\n";
        return 1;
    }

    if (!must_use_tcp && !uds_stream_path)
    {
        std::cerr << "Invalid usage: either TCP port (-T) or UDS stream (-s) is required.\n";
        return 1;
    }

    if (!must_use_udp && !uds_dgram_path)
    {
        std::cerr << "Invalid usage: either UDP port (-U) or UDS datagram (-d) is required.\n";
        return 1;
    }

    fd_set master_set;
    fd_set read_set;
    FD_ZERO(&master_set);

    int listener = -1;
    int udp_listener = -1;
    int uds_stream_fd = -1;
    int uds_dgram_fd = -1;

    if (uds_stream_path)
    {
        uds_stream_fd = create_stream_uds(uds_stream_path);
        if (uds_stream_fd < 0)
            return 1;
        FD_SET(uds_stream_fd, &master_set);
    }
    if (uds_dgram_path)
    {
        uds_dgram_fd = create_dgram_uds(uds_dgram_path);
        if (uds_dgram_fd < 0)
            return 1;
        FD_SET(uds_dgram_fd, &master_set);
    }
    if (must_use_udp)
    {
        udp_listener = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_listener < 0)
        {
            perror("socket(UDP)");
            return 1;
        }
        sockaddr_in udp_addr{};
        udp_addr.sin_family = AF_INET;
        udp_addr.sin_addr.s_addr = INADDR_ANY;
        udp_addr.sin_port = htons(udp_p);
        if (bind(udp_listener, (sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
        {
            perror("bind(UDP)");
            close(udp_listener);
            return 1;
        }
        FD_SET(udp_listener, &master_set);
    }
    if (must_use_tcp)
    {
        listener = socket(AF_INET, SOCK_STREAM, 0);
        if (listener < 0)
        {
            perror("socket(TCP)");
            if (udp_listener >= 0)
                close(udp_listener);
            return 1;
        }
        int reuse = 1;
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        {
            perror("setsockopt");
            close(listener);
            if (udp_listener >= 0)
                close(udp_listener);
            return 1;
        }
        sockaddr_in tcp_addr{};
        tcp_addr.sin_family = AF_INET;
        tcp_addr.sin_addr.s_addr = INADDR_ANY;
        tcp_addr.sin_port = htons(tcp_p);
        if (bind(listener, (sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0)
        {
            perror("bind(TCP)");
            close(listener);
            if (udp_listener >= 0)
                close(udp_listener);
            return 1;
        }
        if (listen(listener, SOMAXCONN) < 0)
        {
            perror("listen");
            close(listener);
            if (udp_listener >= 0)
                close(udp_listener);
            return 1;
        }
        FD_SET(listener, &master_set);
    }
    FD_SET(STDIN_FILENO, &master_set);

    int fd_max = STDIN_FILENO;
    if (listener > fd_max)
        fd_max = listener;
    if (udp_listener > fd_max)
        fd_max = udp_listener;
    if (uds_stream_fd > fd_max)
        fd_max = uds_stream_fd;
    if (uds_dgram_fd > fd_max)
        fd_max = uds_dgram_fd;

    std::unordered_map<int, std::string> bufmap;

    Warehouse warehouse;

    if (!save_file_path_str.empty())
    {
        if (load_file_path(save_file_path_str, warehouse))
        {
            std::cout << "Inventory loaded from file: " << save_file_path_str << std::endl;
        }
        else
        {
            warehouse.set_atoms("OXYGEN", OXYGEN_ATOMS);
            warehouse.set_atoms("CARBON", CARBON_ATOMS);
            warehouse.set_atoms("HYDROGEN", HYDROGEN_ATOMS);
            save_file_path(save_file_path_str, warehouse);
        }
    }
    else
    {
        warehouse.set_atoms("OXYGEN", OXYGEN_ATOMS);
        warehouse.set_atoms("CARBON", CARBON_ATOMS);
        warehouse.set_atoms("HYDROGEN", HYDROGEN_ATOMS);
    }

    std::cout << "Server started";
    if (must_use_tcp)
        std::cout << " on TCP port " << tcp_p;
    if (must_use_udp)
        std::cout << " and UDP port " << udp_p;
    if (uds_stream_path)
        std::cout << " and UDS stream at \"" << uds_stream_path << "\"";
    if (uds_dgram_path)
        std::cout << " and UDS dgram at \"" << uds_dgram_path << "\"";
    if (!save_file_path_str.empty())
        std::cout << " (Inventory file: " << save_file_path_str << ")";
    std::cout << "\n";

    timeval timeout{};

    while (true)
    {
        read_set = master_set;
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;
        int ret;
        if (TIMEOUT > 0)
            ret = select(fd_max + 1, &read_set, nullptr, nullptr, &timeout);
        else
            ret = select(fd_max + 1, &read_set, nullptr, nullptr, nullptr);
        if (ret < 0)
        {
            perror("select");
            break;
        }
        if (ret == 0)
        {
            std::cout << "Timeout of " << TIMEOUT << " seconds reached. Shutting down.\n";
            break;
        }

        for (int fd = 0; fd <= fd_max; ++fd)
        {
            if (!FD_ISSET(fd, &read_set))
                continue;

            // --- TCP (INET) NEW CLIENTS
            if (must_use_tcp && fd == listener)
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
                    fd_max = client_fd;
                bufmap[client_fd] = "";
            }

            // --- UDS STREAM NEW CLIENTS
            else if (uds_stream_fd >= 0 && fd == uds_stream_fd)
            {
                int client_fd = accept(uds_stream_fd, nullptr, nullptr);
                if (client_fd < 0)
                {
                    perror("accept(UDS stream)");
                    continue;
                }
                FD_SET(client_fd, &master_set);
                if (client_fd > fd_max)
                    fd_max = client_fd;
                bufmap[client_fd] = "";
            }

            // --- EXISTING TCP/UDS STREAM CLIENTS: ONLY "ADD"
            else if (bufmap.count(fd))
            {
                char tmpbuf[MAX_LINE];
                ssize_t nbytes = recv(fd, tmpbuf, sizeof(tmpbuf) - 1, 0);
                if (nbytes <= 0)
                {
                    if (nbytes < 0)
                        perror("recv(client)");
                    close(fd);
                    FD_CLR(fd, &master_set);
                    bufmap.erase(fd);
                }
                else
                {
                    tmpbuf[nbytes] = '\0';
                    bufmap[fd].append(tmpbuf);
                    size_t pos;
                    while ((pos = bufmap[fd].find('\n')) != std::string::npos)
                    {
                        std::string line = bufmap[fd].substr(0, pos);
                        bufmap[fd].erase(0, pos + 1);
                        if (!line.empty() && line.back() == '\r')
                            line.pop_back();

                        std::istringstream iss(line);
                        std::string cmd, atom_type;
                        unsigned long long amt;
                        bool success = false;
                        if ((iss >> cmd >> atom_type >> amt) && cmd == "ADD" && warehouse.is_valid_atom(atom_type))
                        {
                            success = warehouse.add_atom(atom_type, static_cast<unsigned int>(amt));
                            if (success && !save_file_path_str.empty())
                            {
                                save_file_path(save_file_path_str, warehouse);
                                warehouse.print_state();
                            }
                        }
                        const char *resp = (success ? "OK\n" : "ERROR\n");
                        send(fd, resp, strlen(resp), 0);
                    }
                }
            }

            // --- UDP or UDS DGRAM: ONLY "DELIVER"
            else if ((must_use_udp && fd == udp_listener) || (uds_dgram_fd >= 0 && fd == uds_dgram_fd))
            {
                char buffer[MAX_LINE];
                sockaddr_storage client_addr{};
                socklen_t client_len = sizeof(client_addr);
                ssize_t n = recvfrom(fd, buffer, sizeof(buffer) - 1, 0, (sockaddr *)&client_addr, &client_len);
                if (n < 0)
                {
                    perror("recvfrom");
                    continue;
                }
                buffer[n] = '\0';
                std::string request = buffer;
                while (!request.empty() && (request.back() == '\n' || request.back() == '\r'))
                    request.pop_back();

                std::istringstream iss(request);
                std::string cmd, mol_type;
                unsigned long long amt;
                bool delivered = false;
                if ((iss >> cmd >> mol_type >> amt) && cmd == "DELIVER" && warehouse.is_valid_molecule(mol_type))
                {
                    delivered = true;
                    for (unsigned long long i = 0; i < amt; ++i)
                    {
                        if (!warehouse.build_molecules(mol_type))
                        {
                            delivered = false;
                            break;
                        }
                    }

                    if (delivered && !save_file_path_str.empty())
                    {
                        save_file_path(save_file_path_str, warehouse);
                        warehouse.print_state();
                    }
                }
                std::string resp = delivered ? ("DELIVERED " + mol_type + " " + std::to_string(amt) + "\n") : "ERROR\n";
                sendto(fd, resp.c_str(), resp.size(), 0, (sockaddr *)&client_addr, client_len);
            }

            // --- SERVER CONSOLE
            if (fd == STDIN_FILENO)
            {
                std::string line;
                if (!std::getline(std::cin, line))
                    continue;
                std::istringstream iss(line);
                std::string cmd;
                iss >> cmd;
                if (cmd == "GEN")
                {
                    std::string drink_type;
                    std::getline(iss, drink_type);
                    if (!drink_type.empty() && drink_type.front() == ' ')
                        drink_type.erase(0, 1);
                    if (drink_type == "CHAMPAGNE" || drink_type == "VODKA" || drink_type == "SOFT DRINK")
                    {
                        int maxd = warehouse.build_drink_amount(drink_type);
                        std::cout << maxd << "\n";
                    }
                    else
                        std::cout << "ERROR\n";
                }
                else
                    std::cout << "ERROR\n";
            }
        }
    }

    if (listener >= 0)
    {
        close(listener);
    }
    if (udp_listener >= 0)
    {
        close(udp_listener);
    }
    if (uds_stream_fd >= 0)
    {
        close(uds_stream_fd);
    }
    if (uds_dgram_fd >= 0)
    {
        close(uds_dgram_fd);
    }
    if (uds_stream_path)
    {
        unlink(uds_stream_path);
    }
    if (uds_dgram_path)
    {
        unlink(uds_dgram_path);
    }

    return 0;
}
