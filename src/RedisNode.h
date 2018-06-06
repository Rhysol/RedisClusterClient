#ifndef __REDISNODE_H__
#define __REDISNODE_H__
#include <string>
#include "hiredis.h"
#include <memory>
#include <functional>
#include <list>
#include <chrono>

typedef std::shared_ptr<redisReply> ReplySptr;
typedef std::function<void(redisReply *, const char *command)> PipelineCommandCallback;
struct PipelineNode
{
	PipelineNode()
	{
		callback = nullptr;
		command.clear();
	}

	PipelineNode(PipelineNode && src) :
		callback(src.callback),
		command(src.command)
	{
		src.callback = nullptr;
		src.command.clear();
	}

	PipelineCommandCallback callback;
	std::string command;
};

class RedisNode
{
	friend class RedisNodePool;
public:
	RedisNode(std::string host, uint64_t port);
	~RedisNode();

	bool initialize();
	bool connect();
	bool keepAlive();

	ReplySptr RedisCommand(const char *command, va_list &ap);
	ReplySptr RedisCommand(const char *command);

	void freeReply(redisReply * reply);

	int  RedisAppendCommand(PipelineCommandCallback callback, const char *command, va_list& ap);
	void processPipeline();

private:
	redisContext *mContext;
	std::string mHost;
	uint64_t mPort;
	std::list<PipelineNode> mPipelineNodes;
	std::chrono::system_clock::time_point mLastOperationTimestamp;
};
#endif //__REDISNODE_H__