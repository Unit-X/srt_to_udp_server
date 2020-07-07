# srt\_to\_udp\_server

Acts as a SRT server and bridges incomming SRT data to UDP

The benefit of this program compared to the built in SRT conversion is that you can process multiple flows from one instance. This solution also got a rest interface making it simpler to integrate and monitor in cloud environments. 

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


Edit the configuration file

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


Then start the server:

```sh

#Run the program
./Executable configuration.file

```

Talk to the REST API like this:

```sh
curl --header "Content-Type: application/json" --request POST --data '{"token":"superSecret","command":"dumpall"}' http://127.0.0.1:8080/restapi/version1
```


## License

*MIT*

Read *LICENCE.md* for details