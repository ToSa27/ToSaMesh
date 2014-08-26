#ifndef __ToSaMeshThreadNode_h__
#define __ToSaMeshThreadNode_h__

#include "ToSaMeshThreading.h"

#include "ToSaMeshObserver.h"
#include "ToSaMeshNode.h"
#include "ToSaMeshDatabase.h"
#include "ToSaMeshTransceiverRfm70.h"

class ToSaMeshAppSubscriber : public IAppMessageObserver {
	private:
		MessageQueue* queue;
		Thread* thread;
	public:
		ToSaMeshAppSubscriber(uint8_t ptype, uint16_t node, MessageQueue* queue, Thread* thread) {
			if (ptype != TOSA_MESH_PTYPE_UNKNOWN)
				this->appObservePTypes.push_back(ptype);
			if (node != TOSA_MESH_NOADDRESS)
				this->appObserveNodes.push_back(node);
			this->queue = queue;
			this->thread = thread;
		}
		bool newAppMessage(appMessage am) {
			queue->enqueue(*thread, new ToSaMeshThreadMessageApp(am));
		}
};

class ToSaMeshThreadNode : public ToSaMeshThreadBase, public ILogMessageObserver, public IMeshMessageObserver, public IAppMessageObserver {
//class ToSaMeshThreadNode : public ToSaMeshThreadBase {
	public:
		bool setup();
		bool loop();
		virtual void newLogMessage(std::string msgType, void *msgData);
		virtual bool newMeshMessage(meshMessage mm);
		virtual bool newAppMessage(appMessage am);
	private:
//		ToSaMeshDatabase *db;
		ToSaMeshTransceiverBase *tc;
		ToSaMeshNode *node;
		vector<ToSaMeshAppSubscriber*> subscribers;
		void handleUiRequest(std::string json);
		void SendWhoAmIResponse(meshMessage mm);
		void SendDhcpResponse(meshMessage mm);
		void SendAnnounceResponse(uint16_t address);
		void SendAnnounceResponse(meshMessage mm);
		void SendFirmwareResponse(meshMessage mm);
		void SendConfigResponse(meshMessage mm);
};

#endif