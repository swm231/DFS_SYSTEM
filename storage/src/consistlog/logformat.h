#pragma once

#include <iostream>
#include <cstring>

#include "logargument.h"

/***
 * ringlog 头
 * 64字节
*/
struct ringLogHead
{
    uint64_t todoList;
    uint64_t freeList, freeListEnd;
    uint64_t todoListNum, freeListNum;
    char fillBytes[24];
};

/**
 * 一条 ringlog 
 * 64字节
*/
struct ringLogBody{
    uint64_t status;
    uint64_t fillin;
    uint64_t synLogAddr;
    char username[12];
    char filename[12];
    uint64_t pre, nxt;
};

/**
 * 一条 synlog
 * 32字节
*/
struct synLogBody{
    uint32_t controlHead;
    uint32_t ip;
    char username[12];
    char filename[12];
};