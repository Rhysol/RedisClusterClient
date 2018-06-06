#ifndef __REDISNODEPOOL_H__
#define __REDISNODEPOOL_H__

#include "RedisNode.h" 
#include <map>
#include <memory>

class RedisNodePool
{
public:
	RedisNodePool();
	~RedisNodePool();

	void initialize(const char *assitNodeHost);
	RedisNode *getNodeByAddress(std::string host);
	RedisNode *getAssistNode();

private:
	RedisNode *createNewNode(std::string host);

private:
	std::map<std::string, std::shared_ptr<RedisNode>> mNodes;
	RedisNode *mAssistNode; // node for get cluster info
};

#endif //__REDISNODEPOOL_H__