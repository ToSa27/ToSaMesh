#ifndef __ToSaMeshTransceiverBase_h__
#define __ToSaMeshTransceiverBase_h__

#include "ToSaMeshNodeConfig.h"

class ToSaMeshTransceiverBase {
	protected:
		uint16_t addr;
		int index;
	public:
		void setIndex(int i);
		virtual bool Init(uint16_t addr) = 0;
        virtual void Send(meshMessage rm) = 0;
		virtual bool HasData() = 0;
		virtual meshMessage Receive() = 0;
};

#endif
