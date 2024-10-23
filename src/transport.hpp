#include <array>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>

#include <oscpp/server.hpp>

const size_t kMaxPacketSize = 8192;

class Transport
{
public:
    Transport() {
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        // Configure the server address
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(3333);
        // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        // inet_pton(AF_INET, ip, &serverAddr.sin_addr);

        // Bind the socket
        if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            close(sockfd);
            throw std::runtime_error("Failed to bind socket");
        }
    }

    ~Transport() {
        close(sockfd);
    }

    // size_t send(const void* buffer, size_t size)
    // {
    //     size_t n = std::min(m_buffer.size(), size);
    //     std::memcpy(m_buffer.data(), buffer, n);
    //     m_message = n;
    //     return n;
    // }

    size_t send(const void* buffer, size_t size) {
        ssize_t sentBytes = sendto(sockfd, buffer, size, 0,
                                   (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (sentBytes < 0) {
            std::cerr << "Sending failed" << std::endl;
        }
        return static_cast<size_t>(sentBytes);
    }

    void recv(void (*callback) (char*, size_t)) {
        std::vector<char> buffer(kMaxPacketSize);
        struct sockaddr_in senderAddr;
        socklen_t senderAddrLen = sizeof(senderAddr);

        ssize_t receivedBytes;
        bool dataAvailable = true;

        while (dataAvailable) {
            receivedBytes = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                            (sockaddr*)&senderAddr, &senderAddrLen);
            std::cout << "try!\n";

            if (receivedBytes > 0) {
                std::cout << "YEAH!\n";
                lastPacket.assign(buffer.begin(), buffer.begin() + receivedBytes);
                // std::cout << "Received packet from " << inet_ntoa(senderAddr.sin_addr) << ":" << ntohs(senderAddr.sin_port) << std::endl;
                // processPacket(buffer.data(), receivedBytes);
                handlePacket(OSCPP::Server::Packet(buffer.data(), receivedBytes));
            } else if (receivedBytes < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    dataAvailable = false;  // No more data available
                } else {
                    std::cerr << "Error receiving data" << std::endl;
                }
            }
        }
        // callback(buffer.data());
        // if (!lastPacket.empty()) {
        //     handlePacket(OSCPP::Server::Packet(lastPacket.data(), lastPacket.size()));
        // }
    }

    void handlePacket(const OSCPP::Server::Packet& packet)
    {
        if (packet.isBundle()) {
            // Convert to bundle
            OSCPP::Server::Bundle bundle(packet);

            // Print the time
            std::cout << "#bundle " << bundle.time() << std::endl;

            // Get packet stream
            OSCPP::Server::PacketStream packets(bundle.packets());

            // Iterate over all the packets and call handlePacket recursively.
            // Cuidado: Might lead to stack overflow!
            while (!packets.atEnd()) {
                handlePacket(packets.next());
            }
        } else {
            // Convert to message
            OSCPP::Server::Message msg(packet);

            // Get argument stream
            OSCPP::Server::ArgStream args(msg.args());

            // const char* key = args.string();
            // const float val = args.float32();
            // std::cout << "-> " << key << " : " << val << "\n";
            // if (msg == "/rx") { printf("/rx: %f\n", args.float32()); } else
            // if (msg == "/ry") { printf("/ry: %f\n", args.float32()); } else
            // if (msg == "/rz") { printf("/rz: %f\n", args.float32()); } else
            {
                std::cout << "Other: " << msg << std::endl;
            }

            // Directly compare message address to string with operator==.
            // For handling larger address spaces you could use e.g. a
            // dispatch table based on std::unordered_map.
            // if (msg == "/s_new") {
            //     const char* name = args.string();
            //     const int32_t id = args.int32();
            //     std::cout << "/s_new" << " "
            //             << name << " "
            //             << id << " ";
            //     // Get the params array as an ArgStream
            //     OSCPP::Server::ArgStream params(args.array());
            //     while (!params.atEnd()) {
            //         const char* param = params.string();
            //         const float value = params.float32();
            //         std::cout << param << ":" << value << " ";
            //     }
            //     std::cout << std::endl;
            // } else if (msg == "/n_set") {
            //     const int32_t id = args.int32();
            //     const char* key = args.string();
            //     // Numeric arguments are converted automatically
            //     // to float32 (e.g. from int32).
            //     const float value = args.float32();
            //     std::cout << "/n_set" << " "
            //             << id << " "
            //             << key << " "
            //             << value << std::endl;
            // } else {
            //     // Simply print unknown messages
            //     std::cout << "Unknown message: " << msg << std::endl;
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

Transport* newTransport()
{
    return new Transport;
}