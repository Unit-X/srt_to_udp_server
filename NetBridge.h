//
// Created by Anders Cedronius on 2020-07-07.
//

#ifndef SRT_TO_UDP_SERVER_NETBRIDGE_H
#define SRT_TO_UDP_SERVER_NETBRIDGE_H

#include "SRTNet.h"
#include "kissnet.hpp"
#define MTU 1456 //SRT-max

class NetBridge {
public:

    struct Stats {
        uint64_t mPacketCounter = 0;
        uint64_t mConnections = 0;
    };

    struct Config {
        std::string mListenIp = "";
        uint16_t mListenPort = 0;
        int mReorder = 0;
        std::string mPsk = "";
        std::string mOutIp = "";
        uint16_t mOutPort = 0;
    };

    bool startBridge(Config &rConfig);
    void stopBridge();
    std::shared_ptr<NetworkConnection> validateConnection(struct sockaddr &sin, SRTSOCKET newSocket);
    bool handleData(std::unique_ptr <std::vector<uint8_t>> &content, SRT_MSGCTRL &msgCtrl, std::shared_ptr<NetworkConnection> ctx, SRTSOCKET clientHandle);
    Stats getStats();


    Config mCurrentConfig;
    std::atomic<uint64_t> mPacketsSinceLastTime;

private:
    SRTNet mSRTServer;
    std::shared_ptr<kissnet::udp_socket> mNetOut = nullptr;
};

#endif //SRT_TO_UDP_SERVER_NETBRIDGE_H
