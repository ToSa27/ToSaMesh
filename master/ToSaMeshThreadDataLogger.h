#ifndef __ToSaMeshThreadDataLogger_h__
#define __ToSaMeshThreadDataLogger_h__

#include "ToSaMeshThreadNode.h"

class ToSaMeshThreadDataLogger : public ToSaMeshThreadBase {
	public:
		bool setup();
		bool loop();
	private:
		map<uint8_t,ofstream> mFiles;
		MessageQueue mqDataLogger;
		ofstream ofs;
};

#endif
