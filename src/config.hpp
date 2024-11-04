#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct Config {
    int port;
    int monitor;
    std::string pointerX_chan;
    float pointerX_range[2];
    std::string pointerY_chan;
    float pointerY_range[2];

    std::string version;
};

inline void from_json(const nlohmann::json& j, Config& c) {
    j.at("port").get_to(c.port);
    j.at("monitor").get_to(c.monitor);
    j.at("pointerX_chan").get_to(c.pointerX_chan);
    j.at("pointerY_chan").get_to(c.pointerY_chan);
    j.at("pointerX_range").at(0).get_to(c.pointerX_range[0]);
    j.at("pointerX_range").at(1).get_to(c.pointerX_range[1]);
    j.at("pointerY_range").at(0).get_to(c.pointerY_range[0]);
    j.at("pointerY_range").at(1).get_to(c.pointerY_range[1]);

    j.at("version").get_to(c.version);
}

inline Config LoadConfig () {
    std::ifstream configJsonFile("../resources/config.json");

    if (!configJsonFile.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
    }

    try {
        nlohmann::json configJson;
        configJsonFile >> configJson;
        return configJson.get<Config>();
    } catch (const std::exception& e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
    }

    configJsonFile.close();
}