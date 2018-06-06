#include "CRedisClient.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <iostream>

CRedisClient::CRedisClient(const std::string & _ip, int _port, const std::string& _chunk, const std::string & _user, const std::string & _passwd, int _timeout, FunError _cb_error)
{
	mAssistNodeHost = _ip;
	mAssistNodeHost.append(":").append(std::to_string(_port));
	m_cb_error = _cb_error;
}

CRedisClient::~CRedisClient()
{

}

bool CRedisClient::init()
{
	mNodePool.initialize(mAssistNodeHost.c_str());
	mSlotKeeper.initialize(&mNodePool);
	lastAppendCommandNode = nullptr;
	return mNodePool.getAssistNode()->keepAlive();
}

ReplySptr CRedisClient::RedisCommand(RedisNode *redisNode, const char *format, ...)
{
	//char command[512];
	va_list args;
	va_start(args, format);
	//vsnprintf(command, 512, format, args);
	ReplySptr reply = redisNode->RedisCommand(format, args);
	
	RedisNode *redictionNode = rediction(reply);
	if (redictionNode != nullptr)
	{
		return redictionNode->RedisCommand(format, args);
	}
	va_end(args);
	return reply;
}

void CRedisClient::setNodeToAppendCommand(RedisNode *redisNode)
{
	lastAppendCommandNode = redisNode;
}

int CRedisClient::RedisAppendCommand(PipelineCommandCallback callback, const char *format, ...)
{
	if (lastAppendCommandNode == nullptr)
	{
		std::cout << "error: haven't set node to append node" << std::endl;
		return REDIS_ERR;
	}

	va_list args;
	va_start(args, format);
	int ret = lastAppendCommandNode->RedisAppendCommand(callback, format, args);
	va_end(args);
	return ret;
}

void CRedisClient::processPipeline()
{
	lastAppendCommandNode->processPipeline();
	lastAppendCommandNode = nullptr;
}

RedisNode *CRedisClient::rediction(ReplySptr reply)
{
    if (!reply || reply->str == nullptr || strcmp (reply->str, ""))
    {
        return nullptr;
    }

    std::string firstValue;
    std::string replyStr(reply->str);
    std::stringstream ss(replyStr);
    std::getline(ss, firstValue, ' ');
    if (firstValue == "MOVE")
    {
        mSlotKeeper.updateSlotInfo();
		size_t pos = replyStr.find_last_of(' ');
		RedisNode * node = mNodePool.getNodeByAddress(replyStr.substr(pos + 1));
		return node;
    }
    else if (firstValue == "ASK")
    {
        size_t pos = replyStr.find_last_of(' ');
        RedisNode * node = mNodePool.getNodeByAddress(replyStr.substr(pos + 1));
        ReplySptr askReply = node->RedisCommand("ASKING");
        if (strcmp(askReply->str, "OK") == 0)
        {
			return node;
        }
    }
    return nullptr;
}

RedisNode *CRedisClient::getNodeByHashTag(const char *format, ...)
{
	char tag[512];
	va_list args;
	va_start(args, format);
	vsnprintf(tag, 512, format, args);
	va_end(args);
	std::string nodeAddress = mSlotKeeper.getNodeAddressByKey(tag);
	return mNodePool.getNodeByAddress(nodeAddress);
}

RedisNode *CRedisClient::getNodeByKey(const char *key)
{
	return getNodeByHashTag(key);
}

RedisNodePool *CRedisClient::getNodePool()
{ 
	return &mNodePool; 
}
