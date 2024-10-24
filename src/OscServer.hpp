#include <array>
#include <cstdio>
#include <vector>
#include <iostream>

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
#include <cerrno>
#include <fcntl.h>

#include <oscpp/server.hpp>

#include <MinimalSocket/udp/UdpSocket.h>

const size_t kMaxPacketSize = 8192;

typedef void (*OscMsgHandlerType)(OSCPP::Server::Message);

class OscServer
{
public:
    OscServer(const int& port) {
        printf("Starting OSC Server on port: %i\n", port);

        socket = MinimalSocket::udp::Udp<false>(
            MinimalSocket::Port(port),
            MinimalSocket::AddressFamily::IP_V6
        );
        socket.open();
    }

    ~OscServer() {
        socket.shutDown();
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
        ssize_t receivedBytes;
        bool dataAvailable = true;

        while (dataAvailable) {
            std::optional<MinimalSocket::ReceiveStringResult> received =
                socket.receive(kMaxPacketSize);

            if (!received) {
                dataAvailable = false;
                break;
            }

            std::string receivedData = received.value().received_message;
            // char* buffer = receivedData.data();
            size_t receivedBytes = receivedData.size();

            if (receivedBytes > 0) {
                lastPacket.assign(receivedData.begin(), receivedData.begin() + receivedBytes);
                handlePacket(OSCPP::Server::Packet(receivedData.data(), receivedBytes), handler);
            } else if (receivedBytes == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
                dataAvailable = false;
            } else if (receivedBytes == 0) {
                dataAvailable = false;
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
    MinimalSocket::udp::Udp<false> socket;
    std::vector<char> lastPacket;
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
