#ifndef __ToSaMeshAppMessageQueue_h__
#define __ToSaMeshAppMessageQueue_h__

#include "ToSaMeshProtocol.h"

class ToSaMeshAppMessageQueue {
	private:
		uint16_t top;
		uint16_t len;
		int capacity;
		appMessage * queue;
    public:
		ToSaMeshAppMessageQueue(int capacity);
		~ToSaMeshAppMessageQueue();
		bool empty();
		void enqueue(appMessage am);
		appMessage dequeue();
};

#endif
