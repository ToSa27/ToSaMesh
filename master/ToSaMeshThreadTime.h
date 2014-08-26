#ifndef __ToSaMeshThreadTime_h__
#define __ToSaMeshThreadTime_h__

#include "ToSaMeshThreading.h"

#define TOSA_MESH_TIME_SLEEP			10
#define TOSA_MESH_TIME_RESPONSE_DELAY	60
#define TOSA_MESH_TIME_INTERVAL			15*60		// alle 15 minuten zeit senden
#define TOSA_MESH_HOLIDAY_INTERVAL		12*60*60	// alle 12 stunden feiertage senden

class ToSaMeshThreadTime : public ToSaMeshThreadBase {
	public:
		bool setup();
		bool loop();
	private:
		uint16_t timeToInterval;
		uint16_t timeToResponse;
		uint16_t timeToIntervalHoliday;
		void SendTime();
		void SendHoliday();
		appMessage getTimeMessage();
		appMessage getHolidayMessage();
		MessageQueue mqTimeRequests;
		Datestamp getNextBankHoliday();
		Datestamp getNextVacationStart();
		Datestamp getNextVacationEnd();
};

#endif