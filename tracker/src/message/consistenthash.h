#pragma once

#include <map>
#include <iostream>

class ConsistentHash{
public:
    static ConsistentHash &Instance(){
        static ConsistentHash instance;
        return instance;
    }

    void AddNode(const std::string &node){
        for(int i = 0; i < replices_; i++){
            std::string virtualNode = node + "---" + std::to_string(i);
            uint32_t hash = Hash_(virtualNode);
            ring_[hash] = node;
        }
    }

    std::string GetGroup(const std::string &data){
        if(ring_.empty())
            return "";
        
        uint32_t hash = Hash_(data);
        auto it = ring_.lower_bound(hash);
        if(it == ring_.end())
            it = ring_.begin();
        return it->second;
    }

private:
    uint32_t Hash_(const std::string &key){
        std::hash<std::string> hasher;
        return hasher(key);
    }

    std::map<uint32_t, std::string> ring_;
    int replices_ = 5;
};
