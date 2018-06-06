#ifndef __REDISCLUSTERCLIENT_H__
#define __REDISCLUSTERCLIENT_H__
#include "SlotKeeper.h"
#include "RedisNodePool.h"
#include "hiredis.h"
#include <vector>
#include <map>
#include "common_define.h"

class CRedisClient
{
public:
    CRedisClient(const std::string & _ip, int _port, const std::string& _chunk, const std::string & _user, const std::string & _passwd, int _timeout, FunError _cb_error);
    ~CRedisClient();

    bool init();
    
	ReplySptr RedisCommand(RedisNode *redisNode, const char *format, ...);
	void setNodeToAppendCommand(RedisNode *redisNode);
	int RedisAppendCommand(PipelineCommandCallback callback, const char *format, ...);
	void processPipeline();

	RedisNodePool *getNodePool();
    RedisNode *getNodeByHashTag(const char *format, ...);
	RedisNode *getNodeByKey(const char *key);

private:
    RedisNode *rediction(ReplySptr reply);

private:
    SlotKeeper mSlotKeeper;
    RedisNodePool mNodePool;
	RedisNode *lastAppendCommandNode;
	FunError m_cb_error;
	std::string mAssistNodeHost;
};

#endif //__REDISCLUSTERCLIENT_H__
