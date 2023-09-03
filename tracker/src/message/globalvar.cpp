#include "storagemess.h"

const char Message::CR[] = "\r";
const char Message::CRLF[] = "\r\n";
std::unordered_map<int, StorageNode*> Message::storageNode;
std::unordered_map<std::string, std::list<int> > Message::group;
uint64_t Message::timeUnitl;
bool Message::flag;
RWlock Message::lock;
std::unique_ptr<std::thread> Message::changeFlagThread;
std::atomic<bool> Message::threadIsRun(false);


uint32_t Conf::bind_addr, Conf::cookieOut, Conf::timeOut;
uint16_t Conf::http_port, Conf::task_port;
std::string Conf::mysql_host, Conf::mysql_user, Conf::mysql_pwd, Conf::mysql_database;