#include <iostream>
#include "SRTNet.h"
#include "INI.h"

#define MTU 1456 //SRT-max

SRTNet mySRTNetServer; //SRT

// Set global parsing/saving options
//INI::PARSE_FLAGS = INI::PARSE_COMMENTS_ALL | INI::PARSE_COMMENTS_SLASH | INI::PARSE_COMMENTS_HASH;
//INI::SAVE_FLAGS = INI::SAVE_PRUNE | INI::SAVE_PADDING_SECTIONS | INI::SAVE_SPACE_SECTIONS | INI::SAVE_SPACE_KEYS | INI::SAVE_TAB_KEYS | INI::SAVE_SEMICOLON_KEYS;


int main() {
    INI ini("file.ini", true);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
