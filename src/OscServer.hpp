#include <array>
#include <cstdio>
#include <vector>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>

#include <oscpp/server.hpp>

const size_t kMaxPacketSize = 8192;

typedef void (*OscMsgHandlerType)(OSCPP::Server::Message);

class OscServer
{
public:
    OscServer(const int& port) {
        printf("Starting OSC Server on port: %i\n", port);
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        // Configure the server address
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        // inet_pton(AF_INET, ip, &serverAddr.sin_addr);

        // Bind the socket
        if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            close(sockfd);
            throw std::runtime_error("Failed to bind socket");
        }
    }

    ~OscServer() {
        close(sockfd);
    }

    // size_t send(const void* buffer, size_t size) {
    //     ssize_t sentBytes = sendto(sockfd, buffer, size, 0,
    //                                (sockaddr*)&serverAddr, sizeof(serverAddr));
    //     if (sentBytes < 0) {
    //         std::cerr << "Sending failed" << std::endl;
    //     }
    //     return static_cast<size_t>(sentBytes);
    // }

    void recv(OscMsgHandlerType handler) {
        std::vector<char> buffer(kMaxPacketSize);
        struct sockaddr_in senderAddr;
        socklen_t senderAddrLen = sizeof(senderAddr);

        // Set the socket to non-blocking mode
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        ssize_t receivedBytes;
        bool dataAvailable = true;

        while (dataAvailable) {
            receivedBytes = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                            (sockaddr*)&senderAddr, &senderAddrLen);

            if (receivedBytes > 0) {
                lastPacket.assign(buffer.begin(), buffer.begin() + receivedBytes);
                handlePacket(OSCPP::Server::Packet(buffer.data(), receivedBytes), handler);
            } else if (receivedBytes == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
                dataAvailable = false;  // No more data available
            } else if (receivedBytes == 0) {
                dataAvailable = false;  // No more data available
            } else {
                std::cerr << "Error receiving data" << std::endl;
                dataAvailable = false;
            }
        }
    }

    void handlePacket(const OSCPP::Server::Packet& packet, OscMsgHandlerType handler)
    {
        if (packet.isBundle()) {
            OSCPP::Server::Bundle bundle(packet);
            std::cout << "#bundle " << bundle.time() << std::endl;

            OSCPP::Server::PacketStream packets(bundle.packets());

            while (!packets.atEnd()) {
                handlePacket(packets.next(), handler);
            }
        } else {
            OSCPP::Server::Message msg(packet);
            // OSCPP::Server::ArgStream args(msg.args());

            // const char* key = args.string();
            // const float val = args.float32();
            // std::cout << "-> " << key << " : " << val << "\n";
            // if (msg == "/rx") { printf("/rx: %f\n", args.float32()); } else
            // if (msg == "/ry") { printf("/ry: %f\n", args.float32()); } else
            // if (msg == "/rz") { printf("/rz: %f\n", args.float32()); } else
            handler(msg);
            // {
            //     std::cout << "Input: " << msg << std::endl;
            // }
        }
    }

private:
    int sockfd;
    sockaddr_in serverAddr;
    std::vector<char> lastPacket;

    std::array<char,kMaxPacketSize> m_buffer;
    size_t m_message;
};

size_t makeOscPacket(void* buffer, size_t size)
{
    OSCPP::Client::Packet packet(buffer, size);

    packet
        .openBundle(1234ULL)
            .openMessage("/s_new", 2 + OSCPP::Tags::array(6))
                .string("sinesweep")
                .int32(2)
                .openArray()
                    .string("start-freq")
                    .float32(330.0f)
                    .string("end-freq")
                    .float32(990.0f)
                    .string("amp")
                    .float32(0.4f)
                .closeArray()
            .closeMessage()
            .openMessage("/n_free", 1)
                .int32(1)
            .closeMessage()
            .openMessage("/n_set", 3)
                .int32(1)
                .string("wobble")
                .int32(31)
            .closeMessage()
        .closeBundle();
    return packet.size();
}
