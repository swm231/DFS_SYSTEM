#pragma once

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
