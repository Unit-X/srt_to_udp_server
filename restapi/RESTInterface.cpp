//
//  RESTInterface.cpp
//  restTest
//
//  Created by Anders Cedronius on 2019-03-28.
//  Copyright © 2019 Anders Cedronius. All rights reserved.
//
//
//curl --header "Content-Type: application/json" --request POST --data '{"token":"secret","command":"dumpall"}' http://127.0.0.1:8080/restapi/version1

#include "RESTInterface.hpp"

RESTInterface::RESTInterface() {
    serverIsRunnning = false; //Since atomics can't be initialized when defined.
    std::cout << __FILE__ << " " << __LINE__ << ": RESTInterface constructed" << std::endl;
}

RESTInterface::RESTInterface(const RESTInterface& orig) {
    std::cout << __FILE__ << " " << __LINE__ << ": RESTInterface copy constructor called" << std::endl;
}

RESTInterface::~RESTInterface() {
    std::cout << __FILE__ << " " << __LINE__ << ": RESTInterface destruct" << std::endl;
}

//Is the application JSON?
bool RESTInterface::qualifyJSON(const Headers& headers) {
    for (const auto& x: headers) {
        if (!x.first.compare(contentType) & !x.second.compare(applicationType)) {
            return true;
        }
    }
    return false;
}

//This is where you need to implement your interface.
//This is an example of a simple string concatenation interface
//reading the 'key1' and 'key2' data responding with the
//data from 'key1' and 'key2' concatenated.
std::string RESTInterface::readJSON(std::string data) {
    bool jsonOK = true;
    json j,jError,jResponce;
    
    //Is the data JSON?
    try {
        j = json::parse(data.c_str());
    } catch (const std::exception& e) {
        jResponce["responce"] = "nok";
        jError.push_back(json{e.what()});
        jResponce["error"] = jError;
        return jResponce.dump();
    }
    
    //Is the data containing anything?
    if (!j.size()) {
        jResponce["responce"] = "nok";
        jError.push_back(json{"JSON data empty"});
        jResponce["error"] = jError;
        return jResponce.dump();
    }
    
    //Speculative extraction of expected data
    auto token = getContentForKey<std::string>("token", j, jError, jsonOK);
    if (token.compare(mToken)) {
        jsonOK = false;
    }
    auto command = getContentForKey<std::string>("command", j, jError, jsonOK);
    
    //Did we find data as expected or is the interface method registerd?
    if (!jsonOK || !getStatsCallback) {
        //NO
        jResponce["responce"] = "nok";
        jResponce["error"] = jError;
        return jResponce.dump();
    }
    //YES
    jResponce["responce"] = "ok";
    jResponce["info"] = getStatsCallback(command);
    return jResponce.dump();
}

//http(s) post handler.
void RESTInterface::handlePost(const Request& req, Response& res) {
    json jResponce;
    //Is it application/json and is the post less than allowed size?
    if (qualifyJSON(req.headers) && req.body.length()<MAX_POST_SIZE) {
        //Yes
        res.set_content(readJSON(req.body).c_str(), applicationType.c_str());
    } else {
        //No
        jResponce["responce"] = "nok";
        res.set_content(jResponce.dump().c_str(), applicationType.c_str());
    }
}

//Set-up the server and register callbacks
void RESTInterface::restWorker() {
    //Register handlers
    //In this example we just care about the post method
    myServer.Post(currentPath->c_str(), std::bind(&RESTInterface::handlePost, this, std::placeholders::_1, std::placeholders::_2));
    //This is a blocking call.
    if (myServer.listen(currentHost->c_str(), currentPort) == false) {
        std::cout << "Failed starting server." << std::endl;
    }
    serverIsRunnning = false;
}

//The public method for starting the service
bool RESTInterface::startServer(const char* host, int port, const char* path, std::string token) {
    if (!(currentHost = std::make_shared<std::string>(host))->length()) return false;
    currentPort = port;
    if (!(currentPath = std::make_shared<std::string>(path))->length()) return false;
    serverIsRunnning = true;
    mToken = token;
    std::thread(std::bind(&RESTInterface::restWorker, this)).detach();
    return true;
}

//The public method for stoping the service
bool RESTInterface::stopServer() {
    myServer.stop();
    return true;
}
