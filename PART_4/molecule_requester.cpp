#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

static const size_t MAX_LINE = 1024;

int main(int argc, char* argv[]) {

    std::string host;
    std::string port;
    int opt;

    while((opt = getopt(argc,argv, "h:p:")) != -1)
    {
        switch(opt)
        {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -h <hostname> -p <udp_port>\n";
                return 1;
        }
    }
    if (host.empty() || port.empty()) {
        std::cerr << "Hostname and port must be specified.\n";
        return 1;
    }

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number.\n";
        return 1;
    }

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number.\n";
        return 1;
    }

    // UDP 
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket"); return 1;
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);

    if (inet_pton(AF_INET, hostname, &serv.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        close(sock);
        return 1;
    }

    while (true) {
        std::string molecule;
        unsigned int amount;

        /*
        WATER NEED 2 H + 1 C
        CARBON DIOXIDE NEECDS 1 C + 2 O
        ALCOHOL NEEDS 2 C + 6 H + 1 O
        GLUCOSE NEEDS 6 C + 12 H + 6 O
        */
        std::cout << "Molecule to request (WATER / CARBON DIOXIDE / ALCOHOL / GLUCOSE): ";
        //std::cin >> molecule;
        std::cin >> std::ws;//
        std::getline(std::cin, molecule);//

        std::cout << "Amount: ";
        //std::cin >> amount;

        if (!(std::cin >> amount)) {//
        
            std::cin.clear();
            std::cin.ignore(MAX_LINE, '\n');
            std::cout << "Invalid number\n";
            continue;
        }
        
        std::cin.ignore(MAX_LINE, '\n');

        if (molecule.empty()){
        
            std::cout << "Invalid molecule name\n";
            continue;
        }

        std::string request = "DELIVER " + molecule + " " + std::to_string(amount) + "\n";
        if (sendto(sock, request.c_str(), request.size(), 0,(sockaddr*)&serv, sizeof(serv)) < 0) {
            
            perror("sendto");
            break;
        }

        char buffer[MAX_LINE];
        socklen_t len = sizeof(serv);
        ssize_t n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,(sockaddr*)&serv, &len);
        
        if (n > 0) {
            
            buffer[n] = '\0';
            std::cout << "Response: " << buffer;
        } 
        else if (n < 0) {
            
            perror("recvfrom");
            break;
        }
    }
    close(sock);
    return 0;
}