#include "ToSaMeshAppMessageQueue.h"

ToSaMeshAppMessageQueue::ToSaMeshAppMessageQueue(int capacity) {
	top = 0;
	len = 0;
	this->capacity = capacity;
	queue = new appMessage[capacity];
}

ToSaMeshAppMessageQueue::~ToSaMeshAppMessageQueue() {
	delete [] queue;
}

bool ToSaMeshAppMessageQueue::empty() {
	return (len == 0);
}

void ToSaMeshAppMessageQueue::enqueue(appMessage am) {
	queue[top] = am;
	if (top < capacity - 1)
		top++;
	else
		top = 0;
	if (len < capacity)
		len++;
}

appMessage ToSaMeshAppMessageQueue::dequeue() {
	appMessage am;
	if (len > 0) {
		uint16_t pos;
		if (len <= top)
			pos = top - len;
		else
			pos = capacity - (len - top);
		am.source = queue[pos].source;
		am.dest = queue[pos].dest;
		am.pid = queue[pos].pid;
		am.ptype = queue[pos].ptype;
		am.len = queue[pos].len;
		for (uint8_t i = 0; i < am.len; i++)
			am.data[i] = queue[pos].data[i];
		len--;
	}
	return am;
}
