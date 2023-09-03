#include "../pool/threadpool.h"
#include "node.h"
#include "../http/httpconn.h"

std::atomic<int> ThreadPool::free_;
std::unordered_map<int, StorageNode*> StorageNode::group;

char HttpMessage::CR[] = "\r";
char HttpMessage::CRLF[] = "\r\n";