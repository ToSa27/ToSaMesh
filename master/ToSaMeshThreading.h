#ifndef __ToSaMeshThreading_h__
#define __ToSaMeshThreading_h__

#include "ToSaMeshNodeConfig.h"

using namespace std;

class Thread {
	private:
		pthread_t _id;
		std::string _name;
		Thread(const Thread& arg);
		Thread& operator=(const Thread& rhs);
	protected:
		static log4cxx::LoggerPtr logger;
		bool started;
		void *arg;
		static void *exec(void *thr);
	public:
		Thread();
		void setName(std::string name);
		virtual ~Thread();
		unsigned int tid() const;
		std::string tname() const;
		void start(void *arg = NULL);
		void join();
		virtual void run() = 0;
};

class Lock {
	protected:
		pthread_mutex_t mutex;
		Lock(const Lock& arg);
		Lock& operator=(const Lock& rhs);
	public:
		Lock();
		virtual ~Lock();
		void lock();
		void unlock();
};

class Condition : public Lock {
	protected:
		pthread_cond_t cond;
		Condition(const Condition& arg);
		Condition& operator=(const Condition& rhs);
	public:
		Condition();
		virtual ~Condition();
		void wait();
		void notify();
};

class Message {
	protected:
		std::string _msgType;
	public:
		void setMsgType(std::string value) {
			_msgType = value;
		}
		std::string getMsgType() {
			return _msgType;
		}
};

typedef Message * MessagePtr;

class MessageQueue {
	private:
		list<MessagePtr> _queue;
		Condition *_cnd;
	protected:
		static log4cxx::LoggerPtr logger;
	public:
		MessageQueue();
		virtual ~MessageQueue();
		bool empty();
		void enqueue(const Thread& arg, MessagePtr mp);
		MessagePtr dequeue(const Thread& arg);
};

class ToSaMeshThreadMessage : public Message {
	private:
		std::string _msgData;
	public:
		ToSaMeshThreadMessage(std::string msgType, std::string msgData) {
			_msgType = msgType;
			_msgData = msgData;
		}
		void setMsgData(std::string value) {
			_msgData = value;
		}
		std::string getMsgData() {
			return _msgData;
		}
};

typedef ToSaMeshThreadMessage * ToSaMeshThreadMessagePtr;

class ToSaMeshThreadMessageApp : public Message {
	public:
		ToSaMeshThreadMessageApp(appMessage msg);
		appMessage getMessage();
	private:
		appMessage _msg;
};

typedef ToSaMeshThreadMessageApp * ToSaMeshThreadMessageAppPtr;

class ToSaMeshThreadMessageMesh : public Message {
	public:
		ToSaMeshThreadMessageMesh(meshMessage msg);
		meshMessage getMessage();
	private:
		meshMessage _msg;
};

typedef ToSaMeshThreadMessageMesh * ToSaMeshThreadMessageMeshPtr;

class ToSaMeshThreadMessageSubscribe : public Message {
	public:
		ToSaMeshThreadMessageSubscribe(MessageQueue *mq, uint8_t ptype, uint16_t node);
		MessageQueue * getMessageQueue();
		uint8_t getPType();
		uint16_t getNode();
	private:
		MessageQueue *_mq;
		uint8_t _ptype;
		uint16_t _node;
};

typedef ToSaMeshThreadMessageSubscribe * ToSaMeshThreadMessageSubscribePtr;

class ToSaMeshThreadMessageJson : public Message {
	public:
		ToSaMeshThreadMessageJson(std::string json);
		std::string getJson();
	private:
		std::string _json;
};

typedef ToSaMeshThreadMessageJson * ToSaMeshThreadMessageJsonPtr;

class ToSaMeshMessageDispatcher {
	public:
		MessageQueue *mqSubscribe;
		MessageQueue *mqTx;
		MessageQueue *mqRx;	
		MessageQueue *mqToUi;
		MessageQueue *mqFromUi;

		ToSaMeshMessageDispatcher(MessageQueue *mqSubscribe, MessageQueue *mqTx, MessageQueue *mqRx, MessageQueue *mqToUi, MessageQueue *mqFromUi) {
			this->mqSubscribe = mqSubscribe;
			this->mqTx = mqTx;
			this->mqRx = mqRx;
			this->mqToUi = mqToUi;
			this->mqFromUi = mqFromUi;
		}
};

class ToSaMeshThreadBase : public Thread {
	public:
		void run();
		virtual bool setup() = 0;
		virtual bool loop() = 0;
	protected:
		ToSaMeshMessageDispatcher *disp;
};

#endif