#pragma once

#include <iostream>
#include <cstring>

#include "../../pool/sqlconnraii.h"

class RollbackLog{
public:
    RollbackLog();
    ~RollbackLog();

    void MakeOrder(bool isUpload, const std::string &username, const std::string &filename);
    void ClearOrder();

    void RollbackOrder();

    bool isUpload;
    std::string username, filename;

private:
    bool hasorder;
};