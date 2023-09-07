#pragma once

typedef std::pair<void*, uint64_t> Page;

#define RINGLOG_BASE_SIZE       64
#define RINGLOG_MEST_SIZE       48

#define RINGLOG_HEAD_STATUS     63
#define RINGLOG_HEAD_ROLLBACK   62
#define RINGLOG_HEAD_SYN        61
#define RINGLOG_HEAD_SYNADDR    60
#define RINGLOG_HEAD_INITIATIVE 59
#define RINGLOG_HEAD_UPLOAD     58
#define RINGLOG_HEAD_USERNAME   57


#define SYNLOG_BASE_SIZE        32

#define SYNLOG_BASE_STATUS      31
#define SYNLOG_BASE_UPLOAD      30
#define SYNLOG_BASE_USERNAME    29
