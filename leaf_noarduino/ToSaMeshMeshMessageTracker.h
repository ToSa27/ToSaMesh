#ifndef __ToSaMeshMeshMessageTracker_h__
#define __ToSaMeshMeshMessageTracker_h__

#include "ToSaMeshProtocol.h"

typedef struct {
	uint16_t addr;
	uint8_t pid;
} meshMessageTrackerEntry;

class ToSaMeshMeshMessageTracker {
	private:
		int capacity;
		meshMessageTrackerEntry * tracker;
    public:
		ToSaMeshMeshMessageTracker(int capacity);
		~ToSaMeshMeshMessageTracker();
		uint8_t get(uint16_t addr);
		void set(uint16_t addr, uint8_t pid);
};

#endif
