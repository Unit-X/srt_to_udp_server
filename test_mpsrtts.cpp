//
// Created by Anders Cedronius on 2020-07-08.
//

#include <iostream>

#include "INI.h"
#include "NetBridge.h"


std::map<std::string, std::shared_ptr<NetBridge>> gBridges;

//Create the client
SRTNet gSRTNetClient;

//Print mutex
std::mutex gPrintMtx;


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

            std::string tagString = rConfigs[rSection.first]["tag"];
            if (!tagString.empty()) {
                lConfig.mMode = NetBridge::Mode::MPSRTTS;
                lConfig.mTag = std::stoi(tagString);
            } else {
                lConfig.mMode = NetBridge::Mode::MPEGTS;
            }

            auto newBridge = std::make_shared<NetBridge>();
            if (!newBridge->startBridge(lConfig)) {
                std::cout << "Failed starting bridge using config: "  << rSection.first << std::endl;
                return false;
            }
            gBridges[rSection.first] = newBridge;
        } else if (sectionName.find("flow") != std::string::npos) {
            std::string lBindKey = rConfigs[rSection.first]["bind_to"];
            if ( gBridges.find(lBindKey) != gBridges.end() && !lBindKey.empty() ) {
                NetBridge::Config lConfig;
                lConfig.mOutPort = std::stoi(rConfigs[rSection.first]["out_port"]);
                lConfig.mOutIp = rConfigs[rSection.first]["out_ip"];
                lConfig.mPsk = rConfigs[rSection.first]["key"];

                std::string tagString = rConfigs[rSection.first]["tag"];
                if (!tagString.empty()) {
                    lConfig.mTag = std::stoi(tagString);
                } else {
                    std::cout << "tag missing for flow: "  << rSection.first << std::endl;
                    return false;
                }

                gBridges[lBindKey]->addInterface(lConfig);
            }
        }
    }
    return true;
}

void handleDataClient(std::unique_ptr <std::vector<uint8_t>> &content, SRT_MSGCTRL &msgCtrl, std::shared_ptr<NetworkConnection> &ctx, SRTSOCKET serverHandle) {
    std::cout << "FYI. Got data ->" << content->size() << std::endl;
}

void fillVector(uint8_t* pData, size_t lLength) {
    uint8_t lVector = 0;
    for (size_t x=0; x < lLength; x++) {
        pData[x] = lVector++;
    }
}

bool checkVector(uint8_t* pData, size_t lLength) {
    uint8_t lVector = 0;
    for (size_t x=0; x < lLength; x++) {
        if (pData[x] != lVector++) {
            return false;
        }
    }
    return true;
}

void tsGenerator() {
    bool running = true;
    uint64_t packetCounter8 = 0;
    uint64_t packetCounter11 = 0;
    uint64_t packetCounter14 = 0;
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        std::vector<uint8_t> tsPacket(189*7);
        tsPacket.data()[0*189] = 8;
        tsPacket.data()[0*189+1] = 0x47;
        *(uint64_t*)(tsPacket.data()+(0*189)+2) = packetCounter8++;
        fillVector(tsPacket.data()+(0*189)+10, 179);

        tsPacket.data()[1*189] = 11;
        tsPacket.data()[1*189+1] = 0x47;
        *(uint64_t*)(tsPacket.data()+(1*189)+2) = packetCounter11++;
        fillVector(tsPacket.data()+(1*189)+10, 179);

        tsPacket.data()[2*189] = 14;
        tsPacket.data()[2*189+1] = 0x47;
        *(uint64_t*)(tsPacket.data()+(2*189)+2) = packetCounter14++;
        fillVector(tsPacket.data()+(2*189)+10, 179);

        tsPacket.data()[3*189] = 8;
        tsPacket.data()[3*189+1] = 0x47;
        *(uint64_t*)(tsPacket.data()+(3*189)+2) = packetCounter8++;
        fillVector(tsPacket.data()+(3*189)+10, 179);

        tsPacket.data()[4*189] = 11;
        tsPacket.data()[4*189+1] = 0x47;
        *(uint64_t*)(tsPacket.data()+(4*189)+2) = packetCounter11++;
        fillVector(tsPacket.data()+(4*189)+10, 179);

        tsPacket.data()[5*189] = 14;
        tsPacket.data()[5*189+1] = 0x47;
        *(uint64_t*)(tsPacket.data()+(5*189)+2) = packetCounter14++;
        fillVector(tsPacket.data()+(5*189)+10, 179);

        tsPacket.data()[6*189] = 8;
        tsPacket.data()[6*189+1] = 0x47;
        *(uint64_t*)(tsPacket.data()+(6*189)+2) = packetCounter8++;
        fillVector(tsPacket.data()+(6*189)+10, 179);

        SRT_MSGCTRL thisMSGCTRL = srt_msgctrl_default;
        gSRTNetClient.sendData(tsPacket.data(),tsPacket.size(),&thisMSGCTRL);

    }
}

void getDataConfig1() {
    kissnet::udp_socket serverSocket(kissnet::endpoint("127.0.0.1", 8100));
    serverSocket.bind();
    kissnet::buffer<4096> receiveBuffer;
    uint64_t lPacketCounter = 0;
    while (true) {
        auto[received_bytes, status] = serverSocket.recv(receiveBuffer);
        if (!received_bytes || status != kissnet::socket_status::valid) {
            break;
        }

        double lPackets = (double) received_bytes / 188.0;
        if (lPackets != (int) lPackets) {
            std::cout << "Payload not X * 188 " << std::endl;
            continue;  //Drop connection?
        }

        for (int x = 0; x < lPackets; x++) {
            uint64_t lCnt = *(uint64_t*)(receiveBuffer.data()+(188*x)+1);
            if (lCnt != lPacketCounter++) {
                std::cout << "Packet counter not as expected " << std::endl;
            }
            if (!checkVector((uint8_t*)receiveBuffer.data()+(188*x)+9, 179)) {
                std::cout << "Packet data not as expected " << std::endl;
            }
        }

        {
            std::lock_guard<std::mutex> lock(gPrintMtx);
            std::cout << "Config1: " << unsigned(received_bytes)  << std::endl;
        }
        //in.append((uint8_t*)receiveBuffer.data(), received_bytes);
    }
    std::cout << "Config1 error" << std::endl;
}

void getDataFlow1() {
    kissnet::udp_socket serverSocket(kissnet::endpoint("127.0.0.1", 8102));
    serverSocket.bind();
    kissnet::buffer<4096> receiveBuffer;
    uint64_t lPacketCounter = 0;
    while (true) {
        auto[received_bytes, status] = serverSocket.recv(receiveBuffer);
        if (!received_bytes || status != kissnet::socket_status::valid) {
            break;
        }

        double lPackets = (double) received_bytes / 188.0;
        if (lPackets != (int) lPackets) {
            std::cout << "Payload not X * 188 " << std::endl;
            continue;  //Drop connection?
        }

        for (int x = 0; x < lPackets; x++) {
            uint64_t lCnt = *(uint64_t*)(receiveBuffer.data()+(188*x)+1);
            if (lCnt != lPacketCounter++) {
                std::cout << "Packet counter not as expected " << std::endl;
            }
            if (!checkVector((uint8_t*)receiveBuffer.data()+(188*x)+9, 179)) {
                std::cout << "Packet data not as expected " << std::endl;
            }
        }

        {
            std::lock_guard<std::mutex> lock(gPrintMtx);
            std::cout << "Flow1: " << unsigned(received_bytes) << std::endl;
        }
        //in.append((uint8_t*)receiveBuffer.data(), received_bytes);
    }
    std::cout << "Flow1 error" << std::endl;
}

void getDataFlow2() {
    kissnet::udp_socket serverSocket(kissnet::endpoint("127.0.0.1", 8103));
    serverSocket.bind();
    kissnet::buffer<4096> receiveBuffer;
    uint64_t lPacketCounter = 0;
    while (true) {
        auto[received_bytes, status] = serverSocket.recv(receiveBuffer);
        if (!received_bytes || status != kissnet::socket_status::valid) {
            break;
        }
        double lPackets = (double) received_bytes / 188.0;
        if (lPackets != (int) lPackets) {
            std::cout << "Payload not X * 188 " << std::endl;
            continue;  //Drop connection?
        }

        for (int x = 0; x < lPackets; x++) {
            uint64_t lCnt = *(uint64_t*)(receiveBuffer.data()+(188*x)+1);
            if (lCnt != lPacketCounter++) {
                std::cout << "Packet counter not as expected " << std::endl;
            }
            if (!checkVector((uint8_t*)receiveBuffer.data()+(188*x)+9, 179)) {
                std::cout << "Packet data not as expected " << std::endl;
            }
        }
        {
            std::lock_guard<std::mutex> lock(gPrintMtx);
            std::cout << "Flow2: " << unsigned(received_bytes) << std::endl;
        }
        //in.append((uint8_t*)receiveBuffer.data(), received_bytes);
    }
    std::cout << "Flow2 error" << std::endl;
}

int main() {
    std::cout << "MPSRTTS Test" << std::endl;
    INI ini("../config.ini", true, INI::PARSE_COMMENTS_ALL | INI::PARSE_COMMENTS_SLASH | INI::PARSE_COMMENTS_HASH);

    if (ini.sections.size() < 2) {
        std::cout << "Failed parsing configuration." << std::endl;
        return EXIT_FAILURE;
    }

    if (!startSystem(ini)) {
        std::cout << "Failed parsing configuration." <<  std::endl;
        return EXIT_FAILURE;
    }

    gSRTNetClient.receivedData=std::bind(&handleDataClient, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    auto clientConnection=std::make_shared<NetworkConnection>();
    if (!gSRTNetClient.startClient("127.0.0.1", 8000, 16, 1000, 100,clientConnection, 1456,"th15i$4k3y")) {
        std::cout << "SRT client1 failed starting." << std::endl;
        return EXIT_FAILURE;
    }

    std::thread(std::bind(&tsGenerator)).detach();
    std::thread(std::bind(&getDataConfig1)).detach();
    std::thread(std::bind(&getDataFlow1)).detach();
    std::thread(std::bind(&getDataFlow2)).detach();

    bool running = true;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        uint64_t lConnectionCounter = 1;
        std::cout << std::endl;
        for (auto &rBridge: gBridges) {
            {
                std::lock_guard<std::mutex> lock(gPrintMtx);
                NetBridge::Stats lStats = rBridge.second->getStats();
                std::cout << "Connection " << unsigned(lConnectionCounter);
                std::cout << " Port: " << unsigned(rBridge.second->mCurrentConfig.mListenPort);
                std::cout << " Clients: " << unsigned(lStats.mConnections);
                std::cout << " packetCounter: " << unsigned(lStats.mPacketCounter) << std::endl;
                lConnectionCounter++;
            }
        }
    }

    return 0;
}