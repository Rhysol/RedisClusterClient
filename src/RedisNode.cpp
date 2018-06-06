#include "RedisNode.h"
#include <iostream>
#include <string.h>

RedisNode::RedisNode(std::string host, uint64_t port):
    mHost(host), mPort(port), mContext(nullptr)
{
    initialize();
}

RedisNode::~RedisNode()
{
    redisFree(mContext);
}

bool RedisNode::initialize()
{
	if (connect())
	{
		return keepAlive();
	}
	return false;
}

bool RedisNode::connect()
{
	if (mContext != nullptr)
	{
		redisFree(mContext);
		mContext = nullptr;
	}

    struct timeval timeout = { 30, 500000 }; // 1.5 seconds
    std::cout << "redis connect : host:" << mHost.c_str() << " port:" << mPort << std::endl;
    mContext = redisConnectWithTimeout(mHost.c_str(), mPort, timeout);
    if (mContext == NULL || mContext->err) 
    {
        if (mContext) {
            std::cout << "Connection Error:" << mContext->errstr << std::endl;
        }
        else
        {
            std::cout << "Connection Error: can't alocate redis context" << std::endl;
        }
        return false;
    }
    return true;
}

bool RedisNode::keepAlive()
{
	if (mContext == nullptr)
	{
		return false;
	}

    ReplySptr reply = RedisCommand("PING");
	
    if (!reply || reply->str == nullptr || strcmp(reply->str, "PONG") != 0)
    {
        return connect();
    }

    return true;
}

ReplySptr RedisNode::RedisCommand(const char *command, va_list &ap)
{
    processPipeline();

    if (mContext == nullptr)
    {
        return ReplySptr(nullptr);
    }

    redisReply *reply = nullptr;
    reply = (redisReply *)redisvCommand(mContext, command, ap);
    if (reply != nullptr)
    {
        return ReplySptr(reply, std::bind(&RedisNode::freeReply, this, std::placeholders::_1));
    }
    else
    {
        return ReplySptr(nullptr);
    }
}

ReplySptr RedisNode::RedisCommand(const char *command)
{
	va_list ap;
	return RedisNode::RedisCommand(command, ap);
}

void RedisNode::freeReply(redisReply *reply)
{
    if (reply != nullptr)
    {
        freeReplyObject(reply);
        reply = nullptr;
    }
}

int RedisNode::RedisAppendCommand(PipelineCommandCallback callback, const char *command, va_list& ap)
{
    if(mContext == nullptr || mContext->err != 0)
    {
         return REDIS_ERR;
    } 
    if (strlen(command) == 0)
    {
        std::cout << "no commands to append!" << std::endl;
        return REDIS_ERR;
    }

    int ret = redisvAppendCommand(mContext, command, ap);
    if (ret != REDIS_OK)
    {
        std::cout << "redis command error" << std::endl;
        return false;
    }
    PipelineNode node;
    node.callback = std::move(callback);
    node.command = command;
    mPipelineNodes.push_back(std::move(node));
    return ret;
}

void RedisNode::processPipeline()
{
    std::list<PipelineNode> tmp;
    tmp.swap(mPipelineNodes);

    for (auto &node : tmp)
    {
        redisReply *reply = NULL;
        if (REDIS_OK != redisGetReply(mContext, (void **)&reply))
        {
            if (reply != NULL)
            {
                freeReply(reply);
                reply = nullptr;
            }
        }
		if (REDIS_REPLY_ERROR == reply->type) {
			std::cout << "err:" << reply->str << std::endl;
			return;
		}
        if (node.callback)
        {
            node.callback(reply, node.command.c_str());
        }
        freeReply(reply);
        if (mContext != nullptr && mContext->err != 0)
        {
            std::cout << mContext->errstr << std::endl;
            connect();
        }
        else if (mContext == nullptr)
        {
            std::cout << "connection uninitialized error" << std::endl;
            connect();
        }
    }
}

