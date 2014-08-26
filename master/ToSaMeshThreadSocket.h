#ifndef __ToSaMeshThreadSocket_h__
#define __ToSaMeshThreadSocket_h__

#include "ToSaMeshThreading.h"

#define TOSA_MESH_SOCKET_BUF		1023
#define TOSA_MESH_SOCKET_FILE		"/tmp/ToSaMesh.sock"
#define TOSA_MESH_SOCKET_BACKLOG	5

class ToSaMeshThreadSocket : public ToSaMeshThreadBase {
	public:
		bool setup();
		bool loop();
	private:
		bool TickSocket();
		void TickToUi();
		void TickFromUi();
//		void TickRx();
//		int TickTx();
		int fdSocket;
		int fdClient;
};

#endif