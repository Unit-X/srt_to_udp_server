//
//  RESTInterface.hpp
//  restTest
//
//  Created by Anders Cedronius on 2019-03-28.
//  Copyright © 2019 Anders Cedronius. All rights reserved.
//

#ifndef RESTInterface_hpp
#define RESTInterface_hpp

#include <stdio.h>
#include <iostream>
#include <atomic>
#include <memory>

#include "httplib.h"
#include "json.hpp"

//Allow a JSON post of max MAX_POST_SIZE bytes body-payload
#define MAX_POST_SIZE 1024

using namespace httplib;
using json = nlohmann::json;

//Global template for getting value for key
template <typename T>
T getContentForKey(std::string g,json& j,json& jError, bool& jsonOK) {
    T data;
    try {
        data=j[g.c_str()];
    } catch (const std::exception& e) {
        jError.push_back(json{g.c_str(),e.what()});
        jsonOK=false;
    }
    return data;
}

class RESTInterface {
public:
    RESTInterface();
    RESTInterface(const RESTInterface& orig);
    virtual ~RESTInterface();
    bool startServer(const char* host, int port, const char* path, std::string token);
    bool stopServer();
    
    //Callback method used by example REST interface
    std::function<json(std::string command)> getStatsCallback;
    
    std::atomic<bool> serverIsRunnning;

private:
    void restWorker();
    bool qualifyJSON(const Headers& headers);
    void handlePost(const Request& req, Response& res);
    std::string readJSON(std::string data);

    const std::string contentType="Content-Type";
    const std::string applicationType="application/json";
    
    Server myServer;
    std::shared_ptr<std::string> currentHost;
    int currentPort;
    std::shared_ptr<std::string> currentPath;
    std::string mToken;
};

#endif /* RESTInterface_hpp */
