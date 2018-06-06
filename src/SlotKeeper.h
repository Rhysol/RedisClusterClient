#ifndef __SLOTKEEPER_H__
#define __SLOTKEEPER_H__

#include <string>
#include <vector>
#define SLOT_NUM 16384
#define HOST_COLUMN 2
#define SLOT_RANGE_COLUMN 9

class RedisNodePool;
class RedisNode;

class SlotKeeper
{
public:
	SlotKeeper();
	~SlotKeeper();

	void initialize(RedisNodePool *nodePool);
	std::string getNodeAddressByKey(const char *key);
	void updateSlotInfo();

private:
	uint64_t calculateSlot(char *key, int keylen);
	uint16_t crc16(const char *buf, int len);
	void analyzeNodesInfo(std::string &nodesInfo);
	void doUpdateSlotInfo(std::string &slotRange, std::string &host);

private:
	std::vector<std::string> mNodesAddress;
	uint64_t mNode2AddressIndex[SLOT_NUM];
	static const uint16_t crc16tab[256];
	RedisNodePool *mNodePool;
};
#endif //__SLOTKEEPER_H__