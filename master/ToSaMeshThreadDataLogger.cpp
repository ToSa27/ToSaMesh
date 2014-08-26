#include "ToSaMeshThreadDataLogger.h"

//#include <iostream>
//#include <fstream>
//using namespace std;

bool ToSaMeshThreadDataLogger::setup() {
	disp->mqSubscribe->enqueue(*this, new ToSaMeshThreadMessageSubscribe(&mqDataLogger, TOSA_MESH_PTYPE_DATASYNC, TOSA_MESH_NOADDRESS));
//	std::string fn = "DataLogger_F0.log";
//	ofs.open(fn.c_str(), std::ofstream::out);
//	ofs.open("DataLogger_F0.log");
	return true;
}

bool ToSaMeshThreadDataLogger::loop() {
	if (!mqDataLogger.empty()) {
		ToSaMeshThreadMessageAppPtr mp = (ToSaMeshThreadMessageAppPtr)mqDataLogger.dequeue(*this);
		appMessage am = mp->getMessage();
		delete mp;
//		if (mFiles.find(am.data[1]) == mFiles.end()) {
//			ofstream f;
//			std::string fn = "DataLogger_" + ToSaMeshUtils::u8tohex(am.data[1]) + ".log";
//			f.open(fn.c_str(), std::ofstream::out);
//			mFiles[am.data[1]] = f;
//		}
//		uint16_t val = am.data[2] + (am.data[3] << 8);
//		(ofstream)(mFiles[am.data[1]]) << ToSaMeshUtils::now() << " : " << ToSaMeshUtils::u16tohex(val) << std::endl << std::flush;		
//		ofs << ToSaMeshUtils::now() << " : " << ToSaMeshUtils::u16tohex(val) << std::endl << std::flush;		
		uint16_t raw = (am.data[3] << 8) + am.data[2];
		ToSaMeshDatabase::addDataLogging(am.data[1], raw);
	}
}
