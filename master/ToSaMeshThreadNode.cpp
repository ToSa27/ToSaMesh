#include "ToSaMeshThreadNode.h"

bool ToSaMeshThreadNode::setup() {
//	db = new ToSaMeshDatabase();
	node = new ToSaMeshNode(1, 1024, 50, 50, 30);
	tc = new ToSaMeshTransceiverRfm70(RFM70_PIN_SCLK, RFM70_PIN_MOSI, RFM70_PIN_MISO, RFM70_PIN_RX_CSN, RFM70_PIN_RX_CE);
	node->addTransceiver(tc);
//	tc = new ToSaMeshTransceiverRfm70(RFM70_PIN_SCLK, RFM70_PIN_MOSI, RFM70_PIN_MISO, RFM70_PIN_TX_CSN, RFM70_PIN_TX_CE);
//	node->addTransceiver(tc);
	meshObservePTypes.push_back(TOSA_MESH_PTYPE_WHOAMI_REQUEST);
	meshObservePTypes.push_back(TOSA_MESH_PTYPE_DHCP_REQUEST);
	meshObservePTypes.push_back(TOSA_MESH_PTYPE_ANNOUNCE_REQUEST);
	meshObservePTypes.push_back(TOSA_MESH_PTYPE_FIRMWARE_REQUEST);
	meshObservePTypes.push_back(TOSA_MESH_PTYPE_CONFIG_REQUEST);
	node->addLogObserver(this);
	node->addMeshObserver(this);
	node->addAppObserver(this);
	if (!node->Init(TOSA_MESH_MASTER_ADDRESS)) {
		LOG4CXX_ERROR(logger, "...ERROR: node init failed\n");
		return false;
	}
	vector<uint16_t> nodes = ToSaMeshDatabase::getNodes();
	for (int i = 0; i < nodes.size(); i++) {
		if (ToSaMeshDatabase::needsUpdate(nodes[i])) {
			LOG4CXX_INFO(logger, "Node " + ToSaMeshUtils::u16tohex(nodes[i]) + " needs update.");
			if (ToSaMeshDatabase::nodeAutoUpdate(nodes[i]))
				SendAnnounceResponse(nodes[i]);
		} else {
			LOG4CXX_INFO(logger, "Node " + ToSaMeshUtils::u16tohex(nodes[i]) + " is up-to-date.");
		}
	}
	return true;
}

bool ToSaMeshThreadNode::loop() {
	node->Tick();
	if (!disp->mqSubscribe->empty()) {
		ToSaMeshThreadMessageSubscribePtr sp = (ToSaMeshThreadMessageSubscribePtr)disp->mqSubscribe->dequeue(*this);
		ToSaMeshAppSubscriber* subscriber = new ToSaMeshAppSubscriber(sp->getPType(), sp->getNode(), sp->getMessageQueue(), this);
		subscribers.push_back(subscriber);
		delete sp;
	}
	if (node->HasData()) {
//		std::cout << "hasData" << std::endl << std::flush;
		appMessage Message = node->Receive();
//		LOG4CXX_INFO(logger, "ToSaMeshThreadNode: push new message to mqRx");
		disp->mqRx->enqueue(*this, new ToSaMeshThreadMessageApp(Message));
		newAppMessage(Message);
	}
	if (!disp->mqTx->empty()) {
//		LOG4CXX_INFO(logger, "ToSaMeshThreadNode: found message in mqTx");
		ToSaMeshThreadMessageAppPtr mp = (ToSaMeshThreadMessageAppPtr)disp->mqTx->dequeue(*this);
		appMessage am = mp->getMessage();
		node->Send(am);
		delete mp;
	}
	if (!disp->mqFromUi->empty()) {
//		LOG4CXX_INFO(logger, "ToSaMeshThreadNode: found message in mqFromUi");
		ToSaMeshThreadMessageJsonPtr mp = (ToSaMeshThreadMessageJsonPtr)disp->mqFromUi->dequeue(*this);
		std::string json = mp->getJson();
		handleUiRequest(json);
		delete mp;
	}
	return true;
}

void ToSaMeshThreadNode::handleUiRequest(std::string json) {
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(json, root)) {
		printf("error parsing json\n");
		return;
	}
	std::string msgType = root["type"].asString();
	Json::Value data = root["data"];
	bool send = true;
	if (msgType == "dataLogging") {
		uint8_t logAddr = (uint8_t)atoi(data["dataLoggingAddress"].asCString());
		vector<string> data = ToSaMeshDatabase::getDataLogging(logAddr);
		for (int i = 0; i < data.size(); i++)
			disp->mqToUi->enqueue(*this, new ToSaMeshThreadMessage("Log:DataLogging", data[i]));
		return;
	}
	if (msgType == "update") {
		uint16_t na = (uint16_t)atoi(data["nodeAddress"].asCString());
		if (ToSaMeshDatabase::needsUpdate(na))
			SendAnnounceResponse(na);
		return;
	}
	appMessage am;
	am.source = TOSA_MESH_NOADDRESS;
	am.dest = (uint16_t)atoi(data["nodeAddress"].asCString());
	am.pid = 0;
	if (msgType == "getConfig") {
		am.ptype = TOSA_MESH_PTYPE_CONFIG_REQUEST;
		am.len = 2;
		am.data[0] = (uint8_t)atoi(data["configType"].asCString());
		am.data[1] = (uint8_t)atoi(data["configIndex"].asCString());
	} else if (msgType == "pushConfig") {
		SendAnnounceResponse(am.dest);
		send = false;
	} else if (msgType == "getMem") {
		am.ptype = TOSA_MESH_PTYPE_DATA_GET_REQUEST;
		am.len = 1;
		am.data[0] = (uint8_t)atoi(data["memAddress"].asCString());
	} else if (msgType == "setMem") {
		am.ptype = TOSA_MESH_PTYPE_DATA_SET;
		am.len = 3;
		am.data[0] = (uint8_t)atoi(data["memAddress"].asCString());
		uint16_t val = (uint16_t)atoi(data["memValue"].asCString());
		am.data[1] = (uint8_t)(val & 0x00FF);
		am.data[2] = (uint8_t)((val & 0xFF00) >> 8);
	} else if (msgType == "discover") {
		am.ptype = TOSA_MESH_PTYPE_DISCOVER_REQUEST;
		am.len = 0;
	} else if (msgType == "reset") {
		am.ptype = TOSA_MESH_PTYPE_RESET;
		am.len = 0;
	} else if (msgType == "checkForNewFirmware") {
		ToSaMeshDatabase::checkFirmwareDirectory();
	} else
		send = false;
	if (send)
		node->Send(am);
}

bool ToSaMeshThreadNode::newMeshMessage(meshMessage mm) {
//	LOG4CXX_INFO(logger, "newMeshMessage");
	switch (mm.ptype) {
		case TOSA_MESH_PTYPE_WHOAMI_REQUEST:
			SendWhoAmIResponse(mm);
			return true;
		case TOSA_MESH_PTYPE_DHCP_REQUEST:
			SendDhcpResponse(mm);
			return true;
		case TOSA_MESH_PTYPE_ANNOUNCE_REQUEST:
			SendAnnounceResponse(mm);
			return true;
		case TOSA_MESH_PTYPE_FIRMWARE_REQUEST:
			SendFirmwareResponse(mm);
			return true;
		case TOSA_MESH_PTYPE_CONFIG_REQUEST:
			SendConfigResponse(mm);
			return true;
	}
	return false;
}

// ToDo : get from somewhere else
#define DEFAULT_NODE_HW_TYPE	2
#define DEFAULT_NODE_HW_VERSION	1

void ToSaMeshThreadNode::SendWhoAmIResponse(meshMessage mm) {
	meshMessage mmr;
	mmr.tc = 255;
	mmr.to = TOSA_MESH_BROADCAST;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = TOSA_MESH_BROADCAST;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = TOSA_MESH_PTYPE_WHOAMI_RESPONSE;
	mmr.cost = 1;
	mmr.len = 2 + 6;
	mmr.data[0] = DEFAULT_NODE_HW_TYPE;
	mmr.data[1] = DEFAULT_NODE_HW_VERSION;
	uint8_t * mac = ToSaMeshDatabase::getNewMac();
	for (uint8_t i = 0; i < 6; i++)
		mmr.data[2 + i] = mac[i];
	node->Send(mmr);
}

void ToSaMeshThreadNode::SendDhcpResponse(meshMessage mm) {
	meshMessage mmr;
	DhcpRequest *req = (DhcpRequest *)mm.data;
	DhcpResponse *res = (DhcpResponse *)mmr.data;
	mmr.tc = 255;
	mmr.to = TOSA_MESH_BROADCAST;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = TOSA_MESH_BROADCAST;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = TOSA_MESH_PTYPE_DHCP_RESPONSE;
	mmr.cost = 1;
	mmr.len = 6 + 2; // ToDo : sizeof DhcpResponse;
	for (uint8_t i = 0; i < 6; i++) // ToDo : sizeof DhcpRequest.macAddress
		res->macAddress[i] = req->macAddress[i];
	uint16_t newAddress = ToSaMeshDatabase::getAddress(req->macAddress);
	res->newAddress = newAddress;
	node->Send(mmr);
}

void ToSaMeshThreadNode::SendAnnounceResponse(uint16_t address) {
	appMessage am;
	AnnounceResponse *res = (AnnounceResponse *)am.data;
	Firmware fw = ToSaMeshDatabase::getLatestFirmware(address);
	am.source = node->addr;
	am.dest = address;
	am.pid = 0;
	am.ptype = TOSA_MESH_PTYPE_ANNOUNCE_RESPONSE;
	am.len = 8; // ToDo : sizeof AnnounceResponse;
	res->fwVersion = fw.fwVersion;
	res->fwBlockCount = fw.fwBlockCount;
	res->fwCrc = fw.fwCrc;
	res->configCrc = ToSaMeshDatabase::getConfigCrc(address);
	node->Send(am);
}

void ToSaMeshThreadNode::SendAnnounceResponse(meshMessage mm) {
	meshMessage mmr;
	AnnounceRequest *req = (AnnounceRequest *)mm.data;
	AnnounceResponse *res = (AnnounceResponse *)mmr.data;
	ToSaMeshDatabase::editNodeDetails(mm.source, req->hwType, req->hwVersion, req->fwVersion, req->fwCrc);
	mmr.tc = mm.tc;
	mmr.to = mm.from;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = mm.source;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = TOSA_MESH_PTYPE_ANNOUNCE_RESPONSE;
	mmr.cost = 1;
	mmr.len = 8; // ToDo : sizeof AnnounceResponse;
	if (ToSaMeshDatabase::nodeAutoUpdate(mm.source) || (req->fwVersion == 0)) {
		Firmware fw = ToSaMeshDatabase::getLatestFirmware(req->hwType, req->hwVersion);
		res->fwVersion = fw.fwVersion;
		res->fwBlockCount = fw.fwBlockCount;
		res->fwCrc = fw.fwCrc;
	} else {
		res->fwVersion = req->fwVersion;
		res->fwBlockCount = 0;
		res->fwCrc = req->fwCrc;
	}
	res->configCrc = ToSaMeshDatabase::getConfigCrc(mm.source);
	node->Send(mmr);
}

void ToSaMeshThreadNode::SendFirmwareResponse(meshMessage mm) {
	meshMessage mmr;
	FirmwareRequest *req = (FirmwareRequest *)mm.data;
	FirmwareResponse *res = (FirmwareResponse *)mmr.data;
	mmr.tc = mm.tc;
	mmr.to = mm.from;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = mm.source;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = TOSA_MESH_PTYPE_FIRMWARE_RESPONSE;
	mmr.cost = 1;
	mmr.len = 18; // ToDo : sizeof FirmwareResponse;
	res->fwBlockIndex = req->fwBlockIndex;
	ToSaMeshDatabase::getFirmwareBlock(req->hwType, req->hwVersion, req->fwVersion, req->fwBlockIndex, res->fwBlock);
	node->Send(mmr);
}

void ToSaMeshThreadNode::SendConfigResponse(meshMessage mm) {
	meshMessage mmr;
	mmr.tc = mm.tc;
	mmr.to = mm.from;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = mm.source;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = TOSA_MESH_PTYPE_CONFIG_RESPONSE;
	mmr.cost = 1;
	mmr.len = ToSaMeshDatabase::getNodeConfig(mm.source, mm.data[0], mm.data[1], mmr.data);
	node->Send(mmr);
}

bool ToSaMeshThreadNode::newAppMessage(appMessage am) {
	bool ret = false;
	for (int i = 0; i < subscribers.size(); i++) {
		if (subscribers[i]->inAppScope(am)) {
			subscribers[i]->newAppMessage(am);
			ret = true;
		}
	}
	return ret;
}

void ToSaMeshThreadNode::newLogMessage(std::string msgType, void *msgData) {
	if (msgType == "meshMessageRx") {
		meshMessage mm = *(meshMessage *)msgData;
//		if (mm.ptype == TOSA_MESH_PTYPE_HOPACK)
//			return;
		std::string log = "NodeRx: tc:" + ToSaMeshUtils::u8tohex(mm.tc) + " source:" + ToSaMeshUtils::u16tohex(mm.source) + " from:" + ToSaMeshUtils::u16tohex(mm.from) + " to:" + ToSaMeshUtils::u16tohex(mm.to) + " dest:" + ToSaMeshUtils::u16tohex(mm.dest) + " pid:" + ToSaMeshUtils::u8tohex(mm.pid) + " ptype:" + ToSaMeshUtils::u8tohex(mm.ptype) + " cost:" + ToSaMeshUtils::u8tohex(mm.cost);
		LOG4CXX_INFO(logger, log);
		log = "        data[" + ToSaMeshUtils::u8tohex(mm.len) + "]:(";
		for (int j = 0; j < mm.len; j++) {
			if (j > 0)
				log += " ";
			log += ToSaMeshUtils::u8tohex(mm.data[j]);
		}
		log += ")";
		LOG4CXX_INFO(logger, log);
		disp->mqToUi->enqueue(*this, new ToSaMeshThreadMessage("Log:MeshMessageReceived", ToSaMeshUtils::MeshMessageToJson(mm)));
	} else if (msgType == "meshMessageTx") {
		meshMessage mm = *(meshMessage *)msgData;
//		if (mm.ptype == TOSA_MESH_PTYPE_HOPACK)
//			return;
		std::string log = "NodeTx: tc:" + ToSaMeshUtils::u8tohex(mm.tc) + " source:" + ToSaMeshUtils::u16tohex(mm.source) + " from:" + ToSaMeshUtils::u16tohex(mm.from) + " to:" + ToSaMeshUtils::u16tohex(mm.to) + " dest:" + ToSaMeshUtils::u16tohex(mm.dest) + " pid:" + ToSaMeshUtils::u8tohex(mm.pid) + " ptype:" + ToSaMeshUtils::u8tohex(mm.ptype) + " cost:" + ToSaMeshUtils::u8tohex(mm.cost);
		LOG4CXX_INFO(logger, log);
		log = "        data[" + ToSaMeshUtils::u8tohex(mm.len) + "]:(";
		for (int j = 0; j < mm.len; j++) {
			if (j > 0)
				log += " ";
			log += ToSaMeshUtils::u8tohex(mm.data[j]);
		}
		log += ")";
		LOG4CXX_INFO(logger, log);
		disp->mqToUi->enqueue(*this, new ToSaMeshThreadMessage("Log:MeshMessageTransmitted", ToSaMeshUtils::MeshMessageToJson(mm)));
	}
}
