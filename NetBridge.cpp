//
// Created by Anders Cedronius on 2020-07-07.
//

#include "NetBridge.h"

//Return a connection object. (Return nullptr if you don't want to connect to that client)
std::shared_ptr<NetworkConnection> NetBridge::validateConnection(struct sockaddr &sin, SRTSOCKET newSocket) {

    char addrIPv6[INET6_ADDRSTRLEN];

    if (sin.sa_family == AF_INET) {
        struct sockaddr_in* inConnectionV4 = (struct sockaddr_in*) &sin;
        auto *ip = (unsigned char *) &inConnectionV4->sin_addr.s_addr;
        std::cout << "Connecting IPv4: " << unsigned(ip[0]) << "." << unsigned(ip[1]) << "." << unsigned(ip[2]) << "."
                  << unsigned(ip[3]) << std::endl;

        //Do we want to accept this connection?
        //return nullptr;


    } else if (sin.sa_family == AF_INET6) {
        struct sockaddr_in6* inConnectionV6 = (struct sockaddr_in6*) &sin;
        inet_ntop(AF_INET6, &inConnectionV6->sin6_addr, addrIPv6, INET6_ADDRSTRLEN);
        printf("Connecting IPv6: %s\n", addrIPv6);

        //Do we want to accept this connection?
        //return nullptr;

    } else {
        //Not IPv4 and not IPv6. That's weird. don't connect.
        return nullptr;
    }

    auto a1 = std::make_shared<NetworkConnection>();
   // a1->object = std::make_shared<MyClass>();
    return a1;
}

//Data callback.
bool NetBridge::handleData(std::unique_ptr <std::vector<uint8_t>> &content, SRT_MSGCTRL &msgCtrl, std::shared_ptr<NetworkConnection> ctx, SRTSOCKET clientHandle) {
    mPacketsSinceLastTime++;
    mNetOut->send((const std::byte *)content->data(), content->size());
    //We should test if sending UDP works..
    return true;
}

bool NetBridge::startBridge(Config &rConfig) {
    mSRTServer.clientConnected=std::bind(&NetBridge::validateConnection, this, std::placeholders::_1, std::placeholders::_2);
    mSRTServer.receivedData=std::bind(&NetBridge::handleData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    if (!mSRTServer.startServer(rConfig.mListenIp, rConfig.mListenPort, rConfig.mReorder, 1000, 100, MTU, rConfig.mPsk)) {
        std::cout << "SRT Server failed to start." << std::endl;
        return false;
    }

    mNetOut = std::make_shared<kissnet::udp_socket>(kissnet::endpoint(rConfig.mOutIp, rConfig.mOutPort));

    mCurrentConfig = rConfig;
    mPacketsSinceLastTime = 0;

    return true;
}

void NetBridge::stopBridge() {
    mSRTServer.stop();
}

NetBridge::Stats NetBridge::getStats() {
    NetBridge::Stats lStats;
    lStats.mPacketCounter = mPacketsSinceLastTime;
    mPacketsSinceLastTime = 0;
    mSRTServer.getActiveClients([&](std::map<SRTSOCKET, std::shared_ptr<NetworkConnection>> &clientList)
                                    {
                                        lStats.mConnections = clientList.size();
                                    }
    );
    return lStats;
}
