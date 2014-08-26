#include "ToSaMeshThreadSocket.h"

//LoggerPtr Thread::logger(Logger::getLogger("ToSa.Mesh"));

bool ToSaMeshThreadSocket::setup() {
	fdClient = -1;
	struct sockaddr_un address;	
	if ((fdSocket = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {
		LOG4CXX_ERROR(logger, "ToSaMeshThreadSocket::setup: error in socket()\n");
		return false;
	}
	fcntl(fdSocket, F_SETFL, O_NONBLOCK);
	unlink(TOSA_MESH_SOCKET_FILE);
	address.sun_family = AF_LOCAL;
	strcpy(address.sun_path, TOSA_MESH_SOCKET_FILE);
	if (bind(fdSocket, (struct sockaddr *) &address, sizeof (address)) == -1) {
		LOG4CXX_ERROR(logger, "ToSaMeshThreadSocket::setup: error in bind()\n");
		return false;
	}
	if (listen(fdSocket, TOSA_MESH_SOCKET_BACKLOG) == -1) {
		LOG4CXX_ERROR(logger, "ToSaMeshThreadSocket::setup: error in listen()\n");
		return false;
	}
	char mode[] = "0777";
	chmod(TOSA_MESH_SOCKET_FILE, strtol(mode, 0, 8));
	return true;
}

bool ToSaMeshThreadSocket::loop() {
	TickSocket();
	TickToUi();
	TickFromUi();
//	TickTx();
//	TickRx();
	return true;
}

bool ToSaMeshThreadSocket::TickSocket() {
	if (fdClient == -1) {
		struct sockaddr_in client;
		unsigned int sin_size = sizeof(struct sockaddr_in); 
		fdClient = accept(fdSocket, (struct sockaddr *)&client, &sin_size);
		if (fdClient != -1)
			LOG4CXX_INFO(logger, "ToSaMeshThreadSocket: TickSocket: client connected: " << fdClient);
	}
	return (fdClient != -1);
}

void ToSaMeshThreadSocket::TickToUi() {
	if (fdClient == -1)
		return;
	while (!disp->mqToUi->empty()) {
		ToSaMeshThreadMessagePtr mp = (ToSaMeshThreadMessagePtr)disp->mqToUi->dequeue(*this);
		std::string msgType = mp->getMsgType();
		std::string msgData = mp->getMsgData();
//		LOG4CXX_INFO(logger, "ToSaMeshThreadSocket: TickToUi: new message: " << msgType << " / " << msgData);
		delete mp;
		if (msgType.find("Log:") == 0) {
			Json::Value root;
			root["type"] = msgType;
			Json::Value data; // = root["data"];
			Json::Reader reader;
			if (!reader.parse(msgData, data)) {
				printf("error parsing json\n");
				return;
			}
			root["data"] = data;
			Json::FastWriter writer;
			std::string json = writer.write(root);
//			LOG4CXX_INFO(logger, "ToSaMeshThreadSocket: TickToUi: sending through unix socket: " << json);
			send(fdClient, json.c_str(), json.length(), 0);
		}
	}
}

void ToSaMeshThreadSocket::TickFromUi() {
	if (fdClient == -1)
		return;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fdClient, &fds);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1;
	select(sizeof(fds)*8, &fds, NULL, NULL, &timeout);
	if (FD_ISSET(fdClient, &fds)) {
		char sBuf[TOSA_MESH_SOCKET_BUF];
		memset(sBuf, '\0', TOSA_MESH_SOCKET_BUF);
		int msglen = recv(fdClient, sBuf, TOSA_MESH_SOCKET_BUF, 0);
		std::string sCmd(sBuf);
		LOG4CXX_INFO(logger, "ToSaMeshThreadSocket: command received: " << sCmd);
		disp->mqFromUi->enqueue(*this, new ToSaMeshThreadMessageJson(sCmd));
	}
}
