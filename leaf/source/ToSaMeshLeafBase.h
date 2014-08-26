#ifndef __ToSaMeshLeafBase_h__
#define __ToSaMeshLeafBase_h__

#include "ToSaMeshNodeConfig.h"

#include "ToSaMeshNode.h"
#include "ToSaMeshTransceiverRfm70.h"

class ToSaMeshLeafBase {
	protected:
		ToSaMeshNode *node;
		HardwareConfig hardwareConfig;
		NodeConfig nodeConfig;
		uint16_t calcCRC (const void* ptr, uint16_t len, uint16_t carryover);
		uint16_t calcCRC (const void* ptr, uint16_t len);
		void reboot();
	public:
		void init();
};

#endif
