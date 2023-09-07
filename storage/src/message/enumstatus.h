#pragma once

typedef std::pair<uint64_t, uint64_t> Stortask;

// 表示信息处理状态
enum MSGSTATUS{
    HANDLE_INIT,
    HANDLE_HEAD,
    HANDLE_BODY,
    HANDLE_COMPLATE,
    HANDLE_ERROR,
};

// 表示文件处理过程
enum FILEMSGESTATUS{
    FILE_BEGIN,
    FILE_HEAD,
    FILE_CONTENT,
    FILE_COMPLATE
};

enum METHOD{
    GET,
    POST,
    METHOD_OTHER,
};

enum PATH{
    PUBLIC,
    PRIVATE,
    PATH_OTHER,
};

enum BEHAVIOR{
    UPLOAD,
    DOWNLOAD,
    DELETE,
    BEHAVIOR_OTHER,
};

enum VERSION{
    _0_9,
    _1_0,
    _1_1,
    _2_0,
    VERSION_OTHER,
};

enum HTML_ENUM{
    _404_, _403_, _400_, _HEAD, _HEAD_, _INDEX, _LOGIN, _LOGOUT, _NAMERR,
     _PWDERR, _PUBLIC, _PRIVATE, _REGISTER, _TITLE, _WELCOME, _LISTEND 
};

enum STORTASKTYPE{
    NEW,
    SYN
};

struct synPack{
    synPack(uint64_t _ringLogId, uint64_t _synLogId, const std::string &_username,
        const std::string &_filename, bool _isUpload):ringLogId(_ringLogId),
        synLogId(_synLogId), username(_username), filename(_filename), isUpload(_isUpload){}
    uint64_t ringLogId, synLogId;
    std::string username, filename;
    bool isUpload;
};

struct storTaskPack{
    storTaskPack(){}
    storTaskPack(STORTASKTYPE _type, uint16_t _port):type(_type), port(_port){}
    storTaskPack(STORTASKTYPE _type, synPack _synpack):type(_type), synpack(_synpack){}
    storTaskPack(const storTaskPack& pack): type(pack.type){
        switch (type)
        {
        case STORTASKTYPE::NEW:
            port = pack.port;
            break;
        case STORTASKTYPE::SYN:
            synpack = pack.synpack;
            break;
        default:
            break;
        }
    }
    storTaskPack operator=(const storTaskPack &pack){
        type = pack.type;
        switch (type)
        {
        case STORTASKTYPE::NEW:
            port = pack.port;
            break;
        case STORTASKTYPE::SYN:
            synpack = pack.synpack;
            break;
        default:
            break;
        }
        return *this;
    }
    ~storTaskPack(){}

    STORTASKTYPE type;
    union{
        struct synPack synpack;
        uint16_t port;
    };
};