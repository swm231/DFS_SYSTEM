#include "encipher.h"


std::string encipher::getMD5(const std::string &str, int MD5_Length){
    unsigned char digest[MD5_Length];
    MD5(reinterpret_cast<const unsigned char*>(str.c_str()), str.length(), digest);

    std::stringstream  s;
    for(int i = 0; i < MD5_Length; i ++)
        s << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);

    return s.str();
}

std::string encipher::getCookieValue(){
    int rd = rand();
    return getMD5(std::to_string(rd), 8);
}