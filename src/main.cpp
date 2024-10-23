#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstring>

#include <oscpp/print.hpp>
#include "OscServer.hpp"

OscMsgHandlerType OscMsgHandler = [](OSCPP::Server::Message msg) {
    OSCPP::Server::ArgStream args = msg.args();
    std::cout << "Received: " << msg << std::endl;
};

int main()
{
    OscServer server(3333);

    try {
        while (true) {
            server.recv(OscMsgHandler);
            usleep(16000);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}