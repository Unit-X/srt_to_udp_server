# srt\_to\_udp\_server

Acts as a SRT server and bridges incomming SRT data to UDP

The benefit of this program compared to the built in SRT conversion is that you can process multiple flows from one instance. This solution also implements a REST interface making it simpler to integrate and monitor in cloud environments. 

This implementation also introduce a concept of MPSRTTS (Multi Program SRT Transport Streams) instead of regular MPEG style MPTS. The new concept multiplex multiple SPTS (Or MPTS ) to a single SRT flow creating a multiple program single SRT stream. This to avoid multiple SRT network flows unaware of each other fighting over the same resources at the aggregation points where bandwidth is scarce. This aproach can also simplify the firewall configuration in cases where mutliple input flows (meaning incomming MPEG-TS flows to the server) is fed multiple destinations internally.

**MPEG-TS mode** (no tag see below)

```
MPEG-TS -> SRT -> UDP

MPEG-TS packets
tsPacket[188]
```

**MPSRTTS mode** (Aggregating MPEG-TS to a single SRT flow)

```
MPEG-TS \      /  MPEG-TS -> UDP
MPEG-TS -> SRT -> MPEG-TS -> UDP
MPEG-TS /      \  MPEG-TS -> UDP

MPSRTTS packets are 189 bytes

uint8_t tag
tsPacket[188]

```

**Current auto build status:**

![Ubuntu 18.04](https://github.com/Unit-X/srt_to_udp_server/workflows/Ubuntu%2018.04/badge.svg)

![MacOS](https://github.com/Unit-X/srt_to_udp_server/workflows/MacOS/badge.svg)

![Windows x64](https://github.com/Unit-X/srt_to_udp_server/workflows/Windows%20x64/badge.svg)

![CentOS7](https://github.com/Unit-X/srt_to_udp_server/workflows/CentOS7/badge.svg)

Get the latest binary by->

1. Click the 'Actions' tab above
2. Select your platform (to the left)
3. Select the latest build (If you don't know what you're looking for, it's the top most item in the list of builds)
4. Downlod the binary under the Artifacts text.


## Building

Requires cmake version >= **3.10** and **C++17**

*Linux, MacOS and Windows*

(Read below for how to prepare the different systems)

**Release:**

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

***Debug:***

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
```

**Output (Linux and MacOS):**
 
*./srt\_to\_udp\_server*

**Output (Windows):** 
 
*./Release/srt\_to\_udp\_server.exe* (Release version)
 
*./Debug/srt\_to\_udp\_server.exe* (Debug version)


-

# Preparing Linux

```sh
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install tclsh pkg-config cmake libssl-dev build-essential
```

# Preparing MacOS

Prepare your system by installing ->

* [Xcode](https://itunes.apple.com/us/app/xcode/id497799835)
. Then start Xcode and let xcode install Command Line Tools or run *xcode-select --install* from the terminal.

* **Homebrew** -> [[https://brew.sh](https://brew.sh))

* Then Install dependencies

```sh
brew install cmake
brew install openssl
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)
export OPENSSL_LIB_DIR=$(brew --prefix openssl)"/lib"
export OPENSSL_INCLUDE_DIR=$(brew --prefix openssl)"/include"
```

# Preparing Windows


Prepare your system by installing->

* [Visual Studio](https://visualstudio.microsoft.com/downloads/)
(Also add the CMake build support if you plan to develop applications)

*  **chocolatey** -> [https://chocolatey.org](https://chocolatey.org)

* Then Install dependencies

```sh
choco install openssl
choco install cmake
choco install git
```


## Using the SRT -> UDP bridge


Create a configuration file

Example of MPEG-TS mode transparent mapping where 1 SRT connection in and 1 UDP stream out.

This configuration is creating a server listening on all interfaces and port 8000 for incomming SRT clients. The key 'th15i$4k3y' is used for AES-128 encryption.

The data comming in is sent out on interface 127.0.0.1 port 8100. The content can be whatever since the mapping from SRT to UDP is transparent.

There is also a rest interface configured listening at 127.0.0.1:8080


```
//One part must contain the REST server configuration
[restif]
rest_ip = 127.0.0.1 		//Listen interface for the REST server
rest_port = 8080			//Listen port for the REST server
rest_secret = superSecret	//The secret token to be used for the command API

[config1] 			//Unique name must contain the word 'config'
listen_port=8000 		//SRT listening port
listen_ip=0.0.0.0 		//SRT listening IP
out_port=8100			//UDP send target port
out_ip=127.0.0.1		//UDP send target port
key=th15i$4k3y 			//PSK used
reorder_distance=4 		//Server SRT reorder tollerance
[new server]..... and so on.
```

Example of a MPSRTTS configuration

REST interface as above

The server is configured as above but there is a 'tag' key configured, this means that the server [config1] is put into MPSRTTS-mode and the incomming data needs to be 189*X bytes where the extra byte is a preamble (tag) infront of the TS 188 byte packet.

flow(x) attached outputs to the server. Tags can be reused if for example a MPEG-TS flow has more than one destination.

```
//This is the configuration for the REST interface
[restif]
rest_ip = 127.0.0.1
rest_port = 8080
rest_secret = superSecret
//This is a configuration for server worker 1
[config1]
listen_port=8000
listen_ip=0.0.0.0
out_port=8100
out_ip=127.0.0.1
key=th15i$4k3y
reorder_distance=4
//The tag key is optional and puts the config into MPSRTTS mode
tag = 8
//a MPSRTTS configuration can be attached a flow. The flow must be entered after the configuration block
//The flow configures what to bind to. What tag should be served and the output if and port
[flow1]
bind_to=config1
out_port=8102
out_ip=127.0.0.1
tag = 11
[flow2]
bind_to=config1
out_port=8103
out_ip=127.0.0.1
tag = 14
```



Then start the server:

```sh
./Executable configuration.file

```

Talk to the REST API like this:

```sh
curl --header "Content-Type: application/json" --request POST --data '{"token":"superSecret","command":"dumpall"}' http://127.0.0.1:8080/restapi/version1
```


## License

*MIT*

Read *LICENCE.md* for details