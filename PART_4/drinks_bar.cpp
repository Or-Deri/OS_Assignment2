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
<<<<<<< HEAD
#include <sys/un.h>
#include <fcntl.h>
#include "../WareHouse/WareHouse.hpp"
=======
#include "WareHouse.hpp"
#include <getopt.h>

>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e

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

<<<<<<< HEAD
    int opt;

    while ((opt = getopt(argc, argv, "o:c:h:t:T:U:")) != -1)
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
        default:
            std::cerr << "Usage is: " << argv[0] << " -o <oxygen-atoms> -c <carbon-atoms> -h <hydrogen-atoms> "
            << "-t <timeout> -T <tcp-port> -U <udp-port>\n";
            return 1;
        }
    }
    if (!must_use_tcp || !must_use_udp)
    {
=======

    static struct option long_options[] = {
        {"oxygen",    required_argument, nullptr, 'o'},
        {"carbon",    required_argument, nullptr, 'c'},
        {"hydrogen",  required_argument, nullptr, 'h'},
        {"timeout",   required_argument, nullptr, 't'},
        {"tcp-port",  required_argument, nullptr, 'T'},
        {"udp-port",  required_argument, nullptr, 'U'},
        {nullptr,     0,                 nullptr,   0 }
    };


    int opt;
    while ((opt = getopt(argc, argv, "o:c:h:t:T:U:")) != -1){
        switch (opt){
          case 'o':
            // OXYGEN
            try {
                unsigned long long val = std::stoull(optarg);
                OXYGEN_ATOMS = val;
            } catch (...) {
                std::cerr << "Invalid value for -o: " << optarg << "\n";
                return 1;
            }
            break;

          case 'c':
            // CARBON
            try {
                unsigned long long val = std::stoull(optarg);
                CARBON_ATOMS = val;
            } catch (...) {
                std::cerr << "Invalid value for -c: " << optarg << "\n";
                return 1;
            }
            break;

          case 'h':
            // HYDROGEN
            try {
                unsigned long long val = std::stoull(optarg);
                HYDROGEN_ATOMS = val;
            } catch (...) {
                std::cerr << "Invalid value for -h: " << optarg << "\n";
                return 1;
            }
            break;

          case 't':
            // TIMEOUT (in seconds)
            try {
                unsigned long long val = std::stoull(optarg);
                TIMEOUT = val;
            } catch (...) {
                std::cerr << "Invalid value for -t: " << optarg << "\n";
                return 1;
            }
            break;

          case 'T':
            // TCP port
            try {
                int val = std::stoi(optarg);
                if (val <= 0 || val > 65535) throw 0;
                tcp_p = val;
                must_use_tcp = true;
            } catch (...) {
                std::cerr << "Invalid port number for TCP: " << optarg << "\n";
                return 1;
            }
            break;

          case 'U':
            // UDP port
            try {
                int val = std::stoi(optarg);
                if (val <= 0 || val > 65535) throw 0;
                udp_p = val;
                must_use_udp = true;
            } catch (...) {
                std::cerr << "Invalid port number for UDP: " << optarg << "\n";
                return 1;
            }
            break;

          default:
            std::cerr << "Usage: " << argv[0]
                      << " -o <number> -c <number> -h <number> -t <timeout> -T <tcp-port> -U <udp-port>\n";
            return 1;
        }
    }

    if (!must_use_tcp || !must_use_udp) {
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
        std::cerr << "Invalid Usage - both -T <tcp-port> AND -U <udp-port> are required\n";
        return 1;
    }

    fd_set master_set;
    fd_set read_set;
    FD_ZERO(&master_set);

    // initialize file descriptors -
    // tcp - socket listener
    // udp - socket listener
    int listener = -1;
    int udp_listener = -1;

<<<<<<< HEAD
    udp_listener = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_listener < 0)
    {
=======
    //
    // 1) Create and bind UDP socket
    //
    int udp_listener = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_listener < 0){
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
        perror("socket(UDP)");
        return 1;
    }
    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(udp_p);
<<<<<<< HEAD

    if (bind(udp_listener, (sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
    {
=======
    if (bind(udp_listener, (sockaddr *)&udp_addr, sizeof(udp_addr)) < 0){
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
        perror("bind(UDP)");
        close(udp_listener);
        return 1;
    }
    FD_SET(udp_listener, &master_set);

<<<<<<< HEAD
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
    {
=======
    //
    // 2) Create, bind, and listen on TCP socket
    //

    Warehouse warehouse;
    if (CARBON_ATOMS > 0)   warehouse.add_atom("CARBON",   static_cast<unsigned int>(CARBON_ATOMS));
    if (OXYGEN_ATOMS > 0)   warehouse.add_atom("OXYGEN",   static_cast<unsigned int>(OXYGEN_ATOMS));
    if (HYDROGEN_ATOMS > 0) warehouse.add_atom("HYDROGEN", static_cast<unsigned int>(HYDROGEN_ATOMS));
    std::cout << "Inventory updated:\n";
    warehouse.print_state();


    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0){
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
        perror("socket(TCP)");
        if (udp_listener >= 0)
            close(udp_listener);
        return 1;
    }
    int reuse = 1;
<<<<<<< HEAD
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
=======
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
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
<<<<<<< HEAD

=======
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
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
<<<<<<< HEAD
=======

    std::cout << "Server listening on TCP port " << tcp_p << " and UDP port " << udp_p << " (timeout=" << TIMEOUT << "s)...\n";

    //
    // 3) Set up select()’s fd_sets, INCLUDING STDIN_FILENO
    //
    fd_set master_set;
    fd_set read_set;
    FD_ZERO(&master_set);

>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
    FD_SET(listener, &master_set);

    FD_SET(STDIN_FILENO, &master_set);
<<<<<<< HEAD

    int fd_max = STDIN_FILENO;
    if (listener > fd_max)
        fd_max = listener;
    if (udp_listener > fd_max)
        fd_max = udp_listener;

=======
    
    int fd_max = listener;
    if (udp_listener > fd_max){
        fd_max = udp_listener;
    }
    if (STDIN_FILENO > fd_max){    
        fd_max = STDIN_FILENO;
    }
    // Per‐TCP‐client partial‐line buffers
    
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
    std::unordered_map<int, std::string> bufmap;
    
    timeval timeout{};

<<<<<<< HEAD
    Warehouse warehouse;

    // if atoms were specified, set them in the warehouse
    //
    warehouse.add_atom("OXYGEN", OXYGEN_ATOMS);
    warehouse.add_atom("HYDROGEN", HYDROGEN_ATOMS);
    warehouse.add_atom("CARBON", CARBON_ATOMS);

    std::cout << "Server started";
    std::cout << " on TCP port " << tcp_p;
    std::cout << " and UDP port " << udp_p;

    timeval timeout{};

    while (true)
    {
        read_set = master_set;

        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        int ret;
        if (TIMEOUT > 0)
        {
            ret = select(fd_max + 1, &read_set, nullptr, nullptr, &timeout);
        }
        else
        {
            ret = select(fd_max + 1, &read_set, nullptr, nullptr, nullptr);
        }

        if (ret < 0)
        {
=======
    while (true){
        read_set = master_set;

        timeout.tv_sec  = TIMEOUT;
        timeout.tv_usec = 0;
        
        int ret;
        if (TIMEOUT > 0) {
            timeout.tv_sec  = TIMEOUT;
            timeout.tv_usec = 0;
            ret = select(fd_max + 1, &read_set, nullptr, nullptr, &timeout);
        } else {
            ret = select(fd_max + 1, &read_set, nullptr, nullptr, nullptr);
        }

        if (ret < 0) {
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
            perror("select");
            break;
        }
        
        if (ret == 0) {
            std::cout << "Timeout of " << TIMEOUT << " seconds reached. Shutting down.\n";
            break;
        }


<<<<<<< HEAD
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
            if (fd == listener)
            {
=======
        // 4) Check each fd
        for (int fd = 0; fd <= fd_max; ++fd){

            if (!FD_ISSET(fd, &read_set)){
                continue;
            }
            if (fd == STDIN_FILENO){
                continue;
            }
            
            // 4.a) UDP request arrived?

            if (fd == udp_listener){
            
                char buffer[MAX_LINE];
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);

                ssize_t n = recvfrom(udp_listener,buffer,sizeof(buffer) - 1,0,(sockaddr *)&client_addr,&client_len);
                
                if (n < 0){
                    perror("recvfrom(UDP)");
                    continue;
                }
                buffer[n] = '\0';
                std::string request = buffer;

                // Trim trailing newline/CR
                while (!request.empty() && (request.back() == '\n' || request.back() == '\r')){
                    request.pop_back();
                }

                // Expect: "DELIVER <MOLECULE> <amount>"
                if (request.rfind("DELIVER ", 0) != 0){

                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,(sockaddr *)&client_addr, client_len);
                    continue;
                }

                std::istringstream iss(request);
                std::vector<std::string> tokens;
                std::string tok;
                
                while (iss >> tok){
                    tokens.push_back(tok);
                }
                if (tokens.size() < 3){
                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,(sockaddr *)&client_addr, client_len);
                    continue;
                }

                // Parse amount last token
                unsigned long long amt = 0;
                try{
                    amt = std::stoull(tokens.back());
                }
                catch (...){
                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,(sockaddr *)&client_addr, client_len);
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

                for (unsigned long long i = 0; i < amt; ++i){
                    if (!warehouse.build_molecules(mol_type)){
                        success = false;
                        break;
                    }
                }

                if (success){
                    std::string resp = "DELIVERED " + mol_type + " " + std::to_string(amt) + "\n";
                    sendto(udp_listener, resp.c_str(), resp.size(), 0,(sockaddr *)&client_addr, client_len);
                    warehouse.print_state();
                }
                else{
                    const char *err = "ERROR\n";
                    sendto(udp_listener, err, strlen(err), 0,(sockaddr *)&client_addr, client_len);
                    warehouse.print_state();
                }
            }
    
            //if TCP listener ready Accept new connection
            else if (fd == listener){
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listener, (sockaddr *)&client_addr, &client_len);
                
                if (client_fd < 0){
                    perror("accept(TCP)");
                    continue;
                }
                
                FD_SET(client_fd, &master_set);
<<<<<<< HEAD
                if (client_fd > fd_max)
                    fd_max = client_fd;
                bufmap[client_fd] = "";
            }

            // --- EXISTING TCP CLIENTS: ONLY "ADD"
            else if (bufmap.count(fd))
            {
                char tmpbuf[MAX_LINE];
                ssize_t nbytes = recv(fd, tmpbuf, sizeof(tmpbuf) - 1, 0);
                if (nbytes <= 0)
                {
                    if (nbytes < 0)
                        perror("recv(client)");
=======
                
                if (client_fd > fd_max){
                    fd_max = client_fd;
                }
            
                bufmap[client_fd] = "";
            }
            //
            // 4.c) Existing TCP client ready? Read data
            //
            else{
                char tmpbuf[512];
                ssize_t nbytes = recv(fd, tmpbuf, sizeof(tmpbuf) - 1, 0);
                if (nbytes <= 0){

                    if (nbytes < 0){
                        perror("recv(TCP)");
                    }
                    
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
                    close(fd);
                    FD_CLR(fd, &master_set);
                    bufmap.erase(fd);
                }
                else{
                    tmpbuf[nbytes] = '\0';
                    bufmap[fd].append(tmpbuf);
                    size_t pos;
                    while ((pos = bufmap[fd].find('\n')) != std::string::npos){
                        std::string line = bufmap[fd].substr(0, pos);
                        bufmap[fd].erase(0, pos + 1);
<<<<<<< HEAD
                        if (!line.empty() && line.back() == '\r')
                            line.pop_back();

                        std::istringstream iss(line);
                        std::string cmd, atom_type;
                        unsigned long long amt;
                        bool success = false;
                        if ((iss >> cmd >> atom_type >> amt) && cmd == "ADD" && warehouse.is_valid_atom(atom_type))
                        {
                            success = warehouse.add_atom(atom_type, static_cast<unsigned int>(amt));
                            if (success)
                            {
                                warehouse.print_state();
=======

                        // Trim trailing CR if present
                        if (!line.empty() && line.back() == '\r'){
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
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
                            }
                            const char* resp = success ? "OK\n" : "ERROR\n";
                            send(fd, resp, std::strlen(resp), 0);
                            warehouse.print_state();
                        }
<<<<<<< HEAD
                        const char *resp = (success ? "OK\n" : "ERROR\n");
                        send(fd, resp, strlen(resp), 0);
=======
                        else if (cmd == "GEN") {
                            
                            std::string drink_type;
                            std::getline(iss, drink_type);
                            
                            if (!drink_type.empty() && drink_type.front() == ' '){
                                drink_type.erase(0, 1);
                            }

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
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
                    }
                }
            }
        }
    
            // // GEN PART
            // else if (fd == STDIN_FILENO)
            // {
            //     std::string line;
            //     if (!std::getline(std::cin, line))
            //         continue;

<<<<<<< HEAD
            // --- UDP
            else if (fd == udp_listener)
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

                    if (delivered)
                    {
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
        close(listener);
    if (udp_listener >= 0)
        close(udp_listener);

=======
            //     while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            //         line.pop_back();

            //     if (line.rfind("GEN ", 0) != 0)
            //     {
            //         std::cout << "Invalid command\n";
            //         continue;
            //     }
            //     std::string drink = line.substr(4);
            //     int count = warehouse.build_drink_amount(drink);
            //     std::cout << "Can make " << count << " " << drink << std::endl;
            // }
        
        if (FD_ISSET(STDIN_FILENO, &read_set)) {
            std::string line;
            if (std::getline(std::cin, line)) {
                // כאן נטפל בשורה שהוקלדה כ-ADD או GEN
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
                    std::cout << (success ? "OK\n" : "ERROR\n");
                    warehouse.print_state();
                }
                else if (cmd == "GEN") {
                    std::string drink_type;
                    std::getline(iss, drink_type);
                    if (!drink_type.empty() && drink_type.front() == ' '){
                        drink_type.erase(0,1);
                    }    
                    if (drink_type == "CHAMPAGNE" || drink_type == "VODKA" || drink_type == "SOFT DRINK") {
                        int maxd = warehouse.build_drink_amount(drink_type);
                        std::cout << maxd << "\n";
                    }
                    else {
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
>>>>>>> 45475baa99a557346cfe4ed41950f7df8e9fdd5e
    return 0;
}