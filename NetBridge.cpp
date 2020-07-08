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

//Data callback in MPEGTS mode.
bool NetBridge::handleDataMPEGTS(std::unique_ptr <std::vector<uint8_t>> &content, SRT_MSGCTRL &msgCtrl, std::shared_ptr<NetworkConnection> ctx, SRTSOCKET clientHandle) {
    mPacketCounter++;


    mConnections[0].mNetOut->send((const std::byte *)content->data(), content->size());

    //for (auto &rOut: mNetOut) {
    //    rOut
    //}

    //We should test if sending UDP works..
    return true;
}

//Data callback in MPSRTTS mode.
bool NetBridge::handleDataMPSRTTS(std::unique_ptr <std::vector<uint8_t>> &content, SRT_MSGCTRL &msgCtrl, std::shared_ptr<NetworkConnection> ctx, SRTSOCKET clientHandle) {
    mPacketCounter++;

    double packets = (double) content->size() / 189.0;
    if (packets != (int) packets) {
        std::cout << "Payload not X * 189 " << std::endl;
        return true;  //Drop connection?
    }

    //Place the TS packets in respective tags queue
    for (int x = 0; x < (int)packets ; x++) {
        uint8_t tag = content->data()[x*189];
        std::vector<uint8_t> lPacket(content->data()+(x*189)+1,content->data()+(x*189)+189);
        mTSPackets[tag].push_back(lPacket);
    }

    //Check what queue we should empty
    std::vector<uint8_t> lSendData(188*7);
    for (auto &rPackets: mTSPackets) {
        if (rPackets.second.size() >= 7) {
            //We should send the data now
            for (int x = 0; x < 7 ; x++) {
               memmove(lSendData.data()+(188*x),rPackets.second.data()[0].data(),188);
                rPackets.second.erase(rPackets.second.begin());
            }
            uint8_t tag = rPackets.first;
            for (auto &rConnection: mConnections) {
                if (rConnection.tag == tag) {
                    rConnection.mNetOut->send((const std::byte *)lSendData.data(), lSendData.size());
                }
            }
        }
    }
    return true;
}

bool NetBridge::startBridge(Config &rConfig) {

    //Set the mode, save the config and zero counters
    mCurrentMode = rConfig.mMode;
    mCurrentConfig = rConfig;
    mPacketCounter = 0;

    //Start the SRT server
    mSRTServer.clientConnected=std::bind(&NetBridge::validateConnection, this, std::placeholders::_1, std::placeholders::_2);
    if (rConfig.mMode == Mode::MPEGTS) {
        mSRTServer.receivedData = std::bind(&NetBridge::handleDataMPEGTS, this, std::placeholders::_1,
                                            std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    } else {
        mSRTServer.receivedData = std::bind(&NetBridge::handleDataMPSRTTS, this, std::placeholders::_1,
                                            std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    }
    if (!mSRTServer.startServer(rConfig.mListenIp, rConfig.mListenPort, rConfig.mReorder, 1000, 100, MTU, rConfig.mPsk)) {
        std::cout << "SRT Server failed to start." << std::endl;
        return false;
    }

    //Add the out put connection
    Connection lConnection;
    lConnection.mNetOut = std::make_shared<kissnet::udp_socket>(kissnet::endpoint(rConfig.mOutIp, rConfig.mOutPort));
    if (rConfig.mMode == Mode::MPSRTTS) {
        lConnection.tag = rConfig.mTag;
    }
    mConnections.push_back(lConnection);
    return true;
}

void NetBridge::stopBridge() {
    mSRTServer.stop();
}
bool NetBridge::addInterface(Config &rConfig) {
    if (mCurrentMode != Mode::MPSRTTS) {
        return false;
    }
    //Add the out put connection
    Connection lConnection;
    lConnection.mNetOut = std::make_shared<kissnet::udp_socket>(kissnet::endpoint(rConfig.mOutIp, rConfig.mOutPort));
    lConnection.tag = rConfig.mTag;
    mConnections.push_back(lConnection);
    return true;
}

NetBridge::Stats NetBridge::getStats() {
    NetBridge::Stats lStats;
    lStats.mPacketCounter = mPacketCounter;
    mSRTServer.getActiveClients([&](std::map<SRTSOCKET, std::shared_ptr<NetworkConnection>> &clientList)
                                    {
                                        lStats.mConnections = clientList.size();
                                    }
    );
    return lStats;
}
