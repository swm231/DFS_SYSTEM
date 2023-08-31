#pragma once

#include "sqlconnpool.h"

class SqlConnRAII{
public:
    SqlConnRAII(MYSQL **sql){
        *sql = SqlConnPool::Instance().GetConn();
        sql_ = *sql;
    }
    ~SqlConnRAII(){
        if(sql_)
            SqlConnPool::Instance().FreeConn(sql_);
    }
private:
    MYSQL *sql_;

};