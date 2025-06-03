// molecule_requester.cpp
// compile with: g++ -o molecule_requester molecule_requester.cpp
// usage: ./molecule_requester <hostname> <udp_port>

#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

static const size_t MAX_LINE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <udp_port>\n";
        return 1;
    }

    const char* hostname = argv[1];
    int port = std::stoi(argv[2]);

    // UDP 
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket"); return 1;
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, hostname, &serv.sin_addr);

    while (true) {
        std::string molecule;
        unsigned int amount;

        /*
        WATER NEED 2 H + 1 C
        CARBON DIOXIDE NEECDS 1 C + 2 O
        ALCOHOL NEEDS 2 C + 6 H + 1 O
        GLUCOSE NEEDS 6 C + 12 H + 6 O
        */
        std::cout << "Molecule to request (WATER / CO2 / ALCOHOL / GLUCOSE): ";
        std::cin >> molecule;

        std::cout << "Amount: ";
        std::cin >> amount;

        std::string request = "DELIVER " + molecule + " " + std::to_string(amount) + "\n";

        sendto(sock, request.c_str(), request.size(), 0, (sockaddr*)&serv, sizeof(serv));

        char buffer[MAX_LEN];
        socklen_t len = sizeof(serv);
        ssize_t n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&serv, &len);
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "Response: " << buffer;
        }
    }

    close(sock);
    return 0;
}
