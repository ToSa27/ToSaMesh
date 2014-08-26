#include "ToSaMeshRoutingTable.h"

ToSaMeshRoutingTable::ToSaMeshRoutingTable(int capacity) {
	this->capacity = capacity;
	routes = new routingTableEntry[capacity];
	for (uint16_t i = 0; i < capacity; i++)
		routes[i].dest = TOSA_MESH_NOADDRESS;
	pos = 0;
}

ToSaMeshRoutingTable::~ToSaMeshRoutingTable() {
	delete [] routes;
}

void ToSaMeshRoutingTable::is(uint16_t addr) {
	addr = addr;
}

void ToSaMeshRoutingTable::add(uint16_t dest, uint16_t hop, uint8_t tc, uint8_t cost) {
	if ((dest == addr) || (dest == TOSA_MESH_BROADCAST) || (dest == TOSA_MESH_NOADDRESS))
		return;
	for (uint16_t i = 0; i < capacity; i++) {
		if (routes[i].dest == dest) {
			if (routes[i].cost >= cost) {
				routes[i].hop = hop;
				routes[i].tc = tc;
				routes[i].cost = cost;
			}
			return;
		}
	}
	routes[pos].dest = dest;
	routes[pos].hop = hop;
	routes[pos].tc = tc;
	routes[pos].cost = cost;
	pos++;
	if (pos >= capacity)
		pos = 0;
}

bool ToSaMeshRoutingTable::has(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return true;
	return false;
}

uint16_t ToSaMeshRoutingTable::getHop(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return routes[i].hop;
	return TOSA_MESH_NOADDRESS;
}

uint8_t ToSaMeshRoutingTable::getTc(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return routes[i].tc;
	return 255;
}

uint8_t ToSaMeshRoutingTable::getCost(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return routes[i].cost;
	return 0;
}
