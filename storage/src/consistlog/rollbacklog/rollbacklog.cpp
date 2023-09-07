#include "rollbacklog.h"

RollbackLog::RollbackLog(){}
RollbackLog::~RollbackLog(){}

void RollbackLog::MakeOrder(bool _isUpload, const std::string &_username, const std::string &_filename){
    isUpload = !_isUpload;
    username = _username;
    filename = _filename;
    hasorder = true;
}

void RollbackLog::ClearOrder(){
    hasorder = false;
}

void RollbackLog::RollbackOrder(){
    if(hasorder == false)
        return;

    char order[256];
    if(isUpload == true){
        if(username == "")
            snprintf(order, 256, "INSERT INTO public values ('%s');", filename.c_str());
        else
            snprintf(order, 256, "INSERT INTO public values ('%s', '%s');", username.c_str(), filename.c_str());
    }
    else{
        if(username == "")
            snprintf(order, 256, "DELETE FROM public WHERE filename = '%s';", filename.c_str());
        else
            snprintf(order, 256, "DELETE FROM private WHERE username = '%s' AND filename = '%s';", username.c_str(), filename.c_str());
    }

    MYSQL *sql;
    SqlConnRAII RAII(&sql);

    if(mysql_query(sql, order))
        LOG_ERROR("[sql] rollback updatesql error");
}