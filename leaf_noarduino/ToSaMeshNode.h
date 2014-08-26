#ifndef __ToSaMeshNode_h__
#define __ToSaMeshNode_h__

#include "ToSaMeshNodeConfig.h"
#include "ToSaMeshProtocol.h"
#include "ToSaMeshTransceiverBase.h"
#include "ToSaMeshRoutingTable.h"
#include "ToSaMeshAppMessageQueue.h"
#include "ToSaMeshMeshMessageBacklog.h"
#include "ToSaMeshMeshMessageTracker.h"

#ifdef TOSA_MESH_NODETYPE_MASTER
#include "ToSaMeshObserver.h"
#include "ToSaMeshUtils.h"
#include "ToSaMeshLogger.h"
#include <iostream>
#include <vector>
#include <algorithm>
#endif

//#ifdef TOSA_MESH_NODETYPE_LEAF
//#include "Arduino.h"
//#endif

class ToSaMeshNode {
	private:
#ifdef TOSA_MESH_NODETYPE_MASTER
		static log4cxx::LoggerPtr logger;
		bool timeTrigger;
		unsigned long timeTriggerMillis;
#endif
		ToSaMeshRoutingTable *rt;
		ToSaMeshAppMessageQueue *rxq;
		ToSaMeshMeshMessageBacklog *txb;
		ToSaMeshMeshMessageTracker *rxt;
		appMessage MeshToApp(meshMessage rm);
		meshMessage AppToMesh(appMessage mm);
		void SendArpRequest(uint16_t dest);
		void SendDhcpRequest();
		void SendPing(uint16_t dest);
		bool Forward(meshMessage mm, bool isBroadcast, bool addAddressToData);
#ifdef TOSA_MESH_NODETYPE_MASTER
		std::vector<ILogMessageObserver*> logObservers;
		std::vector<IMeshMessageObserver*> meshObservers;
		std::vector<IAppMessageObserver*> appObservers;
		void notifyLogObserver(std::string msgType, void *msgData);
		bool notifyMeshObserver(meshMessage mm);
		bool notifyAppObserver(appMessage am);
//		std::vector<uint8_t> answerBroadcastTypes;
#endif
		ToSaMeshTransceiverBase** tcs;
		uint8_t tcsc;
	public:
		ToSaMeshNode(int transceiverCount, int routingTableCapacity, int AppMessageQueueCapacity, int MeshMessageBacklogCapacity, int MeshMessageTrackerCapacity);
		~ToSaMeshNode();
		bool Init(uint16_t newaddr);
		bool Send(appMessage Message);
		void SendWaitHopAck(meshMessage mm);
		void SendHeartbeat(uint16_t dest);
		bool HasData();
		appMessage Receive();
		void Tick();
		void TickTx();
		void TickRx();
		void addTransceiver(ToSaMeshTransceiverBase *tcvr);
#ifdef TOSA_MESH_NODETYPE_MASTER
		static appMessage getTimeMessage();
		void addLogObserver(ILogMessageObserver* obj);
		void addMeshObserver(IMeshMessageObserver* obj);
		void addAppObserver(IAppMessageObserver* obj);
//		void answerBroadcastType(uint8_t ptype);
#endif
		uint16_t addr;
		bool SetAddress(uint16_t newAddr);
		uint8_t macAddress[6];
		void SetMacAddress(uint8_t *newMacAddr);
		uint8_t nextpid;
		void Send(meshMessage Message);
#ifdef TOSA_MESH_NODETYPE_LEAF
		bool tsValid;
		Timestamp tsLast;
		uint32_t tsMillis;
		void refreshTime();
		Datestamp nextBankHoliday;
		Datestamp nextVacationStart;
		Datestamp nextVacationEnd;
#endif
};

#endif
