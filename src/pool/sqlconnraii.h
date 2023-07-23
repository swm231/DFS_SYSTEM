#pragma once

#include "sqlconnpool.h"

extern SqlConnPool &globalSqlConnPool();
class SqlConnRAII{
public:
    SqlConnRAII(MYSQL **sql){
        *sql = globalSqlConnPool().GetConn();
        sql_ = *sql;
    }
    ~SqlConnRAII(){
        if(sql_)
            globalSqlConnPool().FreeConn(sql_);
    }
private:
    MYSQL *sql_;

};