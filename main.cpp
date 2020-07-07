#include <iostream>
#include "INI.h"
#include "NetBridge.h"

//Version
#define MAJOR_VERSION 1
#define MINOR_VERSION 0

std::vector<std::shared_ptr<NetBridge>> gBridges;

bool startSystem(INI &rConfigs) {
    for (auto &rSection: rConfigs.sections) {
        std::string sectionName = rSection.first;
        if (sectionName.find("config") != std::string::npos) {
            NetBridge::Config lConfig;
            //Config section. get all parameters
            lConfig.mListenPort = std::stoi(rConfigs[rSection.first]["listen_port"]);
            lConfig.mListenIp = rConfigs[rSection.first]["listen_ip"];
            lConfig.mOutPort = std::stoi(rConfigs[rSection.first]["out_port"]);
            lConfig.mOutIp = rConfigs[rSection.first]["out_ip"];
            lConfig.mPsk = rConfigs[rSection.first]["key"];
            lConfig.mReorder = std::stoi(rConfigs[rSection.first]["reorder_distance"]);
            auto newBridge = std::make_shared<NetBridge>();
            if (!newBridge->startBridge(lConfig)) {
                std::cout << "Failed starting bridge using config: "  << rSection.first << std::endl;
                return false;
            }
            gBridges.push_back(newBridge);
        }
    }
    return true;
}


void printUsage() {
    std::cout << "Usage:" << std::endl << std::endl;
    std::cout << "(Executable) configuration.ini" << std::endl;
}

int main(int argc, char *argv[]) {
    std::cout << "SRT -> UDP Bridge V." << unsigned(MAJOR_VERSION) << "." <<  unsigned(MINOR_VERSION) << std::endl;
    NetBridge lNetBridge;

    if (argc != 2) {
        printUsage();
        return EXIT_FAILURE;
    }

    std::string lCommand = argv[1];
    if (!lCommand.compare("--help") || !lCommand.compare("-help")) {
        printUsage();
        return EXIT_SUCCESS;
    }

    std::cout << "Parsing configuration in file: " << lCommand << std::endl;
    INI ini(lCommand, true, INI::PARSE_COMMENTS_ALL | INI::PARSE_COMMENTS_SLASH | INI::PARSE_COMMENTS_HASH);

    if (ini.sections.size() < 2) {
        std::cout << "Failed parsing configuration." << lCommand << std::endl;
        return EXIT_FAILURE;
    }

    if (!startSystem(ini)) {
        std::cout << "Failed parsing configuration." << lCommand << std::endl;
        return EXIT_FAILURE;
    }

    bool running = true;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        uint64_t lConnectionCounter = 1;
        std::cout << std::endl;
        for (auto &rBridge: gBridges) {
            NetBridge::Stats lStats=rBridge->getStats();
            std::cout << "Connection " << unsigned(lConnectionCounter);
            std::cout << " Port: " << unsigned(rBridge->mCurrentConfig.mListenPort);
            std::cout << " Clients: " << unsigned(lStats.mConnections);
            std::cout << " packetCounter: " << unsigned(lStats.mPacketCounter) << std::endl;
            lConnectionCounter ++;
        }
    }

    return 0;
}