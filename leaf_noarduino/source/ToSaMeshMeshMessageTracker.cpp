#include "ToSaMeshMeshMessageTracker.h"

ToSaMeshMeshMessageTracker::ToSaMeshMeshMessageTracker(int capacity) {
	this->capacity = capacity;
	tracker = new meshMessageTrackerEntry[capacity];
	for (uint16_t i = 0; i < capacity; i++)
		tracker[i].addr = TOSA_MESH_NOADDRESS;
}

ToSaMeshMeshMessageTracker::~ToSaMeshMeshMessageTracker() {
	delete [] tracker;
}

uint8_t ToSaMeshMeshMessageTracker::get(uint16_t addr) {
	for (uint16_t i = 0; i < capacity; i++)
		if (tracker[i].addr == addr)
			return tracker[i].pid;
	return 0xFF;
}

void ToSaMeshMeshMessageTracker::set(uint16_t addr, uint8_t pid) {
	for (uint16_t i = 0; i < capacity; i++)
		if (tracker[i].addr == addr) {
			tracker[i].pid = pid;
			return;
		}
	for (uint16_t i = 0; i < capacity; i++)
		if (tracker[i].addr == TOSA_MESH_NOADDRESS) {
			tracker[i].addr = addr;
			tracker[i].pid = pid;
			return;
		}
	// ToDo : handle tracker overflow
}
