#include "ToSaMeshMeshMessageBacklog.h"

#ifdef TOSA_MESH_NODETYPE_MASTER
LoggerPtr ToSaMeshMeshMessageBacklog::logger(Logger::getLogger("ToSa.Mesh"));
#endif

ToSaMeshMeshMessageBacklog::ToSaMeshMeshMessageBacklog(int capacity) {
	this->capacity = capacity;
	backlog = new meshMessageBacklogEntry[capacity];
	for (uint16_t i = 0; i < capacity; i++)
		backlog[i].redo = 0;
}

ToSaMeshMeshMessageBacklog::~ToSaMeshMeshMessageBacklog() {
	delete [] backlog;
}

void ToSaMeshMeshMessageBacklog::add(meshMessage mm) {
	add(mm, TOSA_MESH_HOP_RETRY);
}

void ToSaMeshMeshMessageBacklog::add(meshMessage mm, uint8_t redo) {
	for (uint16_t i = 0; i < capacity; i++)
		if (backlog[i].redo == 0) {
			backlog[i].redo = redo;
			backlog[i].next = millis(); // direct first time submission
//			copy(mm, backlog[i].message);
			backlog[i].tc = mm.tc;
			backlog[i].to = mm.to;
			backlog[i].from = mm.from;
			backlog[i].source = mm.source;
			backlog[i].dest = mm.dest;
			backlog[i].pid = mm.pid;
			backlog[i].ptype = mm.ptype;
			backlog[i].cost = mm.cost;
			backlog[i].len = mm.len;
			for (uint8_t j = 0; j < backlog[i].len; j++)
				backlog[i].data[j] = mm.data[j];
#ifdef TOSA_MESH_NODETYPE_MASTER
//			LOG4CXX_INFO(logger, "ToSaMeshMeshMessageBacklog entry added at position " << ToSaMeshUtils::u16tos(i) << " with redo " << ToSaMeshUtils::u8tos(backlog[i].redo));		
#endif
			return;
		}
	// ToDo - handle backlog overflow
}

bool ToSaMeshMeshMessageBacklog::pending(uint16_t i) {
	return ((backlog[i].redo > 0) && (backlog[i].next < millis()));
}

void ToSaMeshMeshMessageBacklog::reset(uint16_t i, bool submitted) {
	if (backlog[i].redo > 0) {
		unsigned long pause = 1000 / (2 * backlog[i].redo);
		if (submitted)
			backlog[i].redo = backlog[i].redo - 1;
		// ToDo : random delay
		backlog[i].next = millis() + pause;
#ifdef TOSA_MESH_NODETYPE_MASTER
//		LOG4CXX_INFO(logger, "ToSaMeshMeshMessageBacklog - reset: " << ToSaMeshUtils::u16tos(i) << " now has redo: " << ToSaMeshUtils::u8tos(backlog[i].redo));		
#endif
	}
}

meshMessage ToSaMeshMeshMessageBacklog::get(uint16_t i) {
	meshMessage mm;
//	copy(backlog[i].message, mm);
	mm.tc = backlog[i].tc;
	mm.to = backlog[i].to;
	mm.from = backlog[i].from;
	mm.source = backlog[i].source;
	mm.dest = backlog[i].dest;
	mm.pid = backlog[i].pid;
	mm.ptype = backlog[i].ptype;
	mm.cost = backlog[i].cost;
	mm.len = backlog[i].len;
	for (uint8_t j = 0; j < mm.len; j++)
		mm.data[j] = backlog[i].data[j];
	return mm;
}

void ToSaMeshMeshMessageBacklog::setTo(uint16_t i, uint8_t to) {
	backlog[i].to = to;
}

void ToSaMeshMeshMessageBacklog::done(uint8_t pid) {
	for (uint16_t i = 0; i < capacity; i++)
		if ((backlog[i].redo > 0) && (backlog[i].pid == pid)) {
			backlog[i].redo = 0;
#ifdef TOSA_MESH_NODETYPE_MASTER
//			LOG4CXX_INFO(logger, "ToSaMeshMeshMessageBacklog - done: " << ToSaMeshUtils::u16tos(i));		
#endif
		}
}

bool ToSaMeshMeshMessageBacklog::has(uint8_t pid) {
	for (uint16_t i = 0; i < capacity; i++)
		if ((backlog[i].redo > 0) && (backlog[i].pid == pid))
			return true;
	return false;
}
