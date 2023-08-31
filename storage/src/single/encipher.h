#pragma once

#include <iostream>
#include <iomanip>
#include <cstring>
#include <openssl/md5.h>

namespace encipher{
    
std::string getMD5(const std::string &str, int MD5_Length);

std::string getCookieValue();

} // namespace 