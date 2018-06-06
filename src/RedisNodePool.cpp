#include "RedisNodePool.h"
#include <sstream>
#include <iostream>

RedisNodePool::RedisNodePool()
{

}

RedisNodePool::~RedisNodePool()
{

}

void RedisNodePool::initialize(const char *assitNodeHost)
{
    mAssistNode = createNewNode(assitNodeHost);
}

RedisNode *RedisNodePool::getNodeByAddress(std::string host)
{
    auto iter = mNodes.find(host);
    if (iter == mNodes.end())
    {
        return createNewNode(host);
    }
    return iter->second.get();
}

RedisNode *RedisNodePool::createNewNode(std::string host)
{
    std::stringstream ss(host);
    std::string item;
    std::getline(ss, item, ':');
    std::string ip = item;
    std::getline(ss, item, '@');
    uint64_t port = std::atoi(item.c_str());
    RedisNode *node = new RedisNode(ip, port);
    mNodes.emplace(host, std::shared_ptr<RedisNode>(node));
    return node;
}

RedisNode *RedisNodePool::getAssistNode()
{
    return mAssistNode;
}

