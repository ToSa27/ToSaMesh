#ifndef __ToSaMeshRoutingTable_h__
#define __ToSaMeshRoutingTable_h__

#include "ToSaMeshProtocol.h"

typedef struct {
	uint16_t dest;
	uint16_t hop;
	uint8_t tc;
	uint8_t cost;
} routingTableEntry;

class ToSaMeshRoutingTable {
	private:
		uint16_t addr;
		uint16_t pos;
		int capacity;
		routingTableEntry * routes;
    public:
		ToSaMeshRoutingTable(int capacity);
		~ToSaMeshRoutingTable();
		void is(uint16_t addr);
		void add(uint16_t dest, uint16_t hop, uint8_t tc, uint8_t cost);
		bool has(uint16_t dest);
		uint16_t getHop(uint16_t dest);
		uint8_t getTc(uint16_t dest);
		uint8_t getCost(uint16_t dest);
};

#endif
