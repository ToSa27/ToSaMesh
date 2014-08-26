#include "ToSaMeshLeafBase.h"

uint16_t ToSaMeshLeafBase::calcCRC (const void* ptr, uint16_t len, uint16_t carryover) {
	uint16_t crc = carryover;
	for (uint16_t i = 0; i < len; ++i)
		crc = _crc16_update(crc, ((const char*) ptr)[i]);
	return crc;
}

uint16_t ToSaMeshLeafBase::calcCRC (const void* ptr, uint16_t len) {
	return calcCRC(ptr, len, (uint16_t)~0);
}

void ToSaMeshLeafBase::reboot() {
	wdt_enable(WDTO_15MS);
	for (;;);
}

void ToSaMeshLeafBase::init() {
	node = new ToSaMeshNode(1, 10, 5, 5, 10);
	ToSaMeshTransceiverRfm70 *tc;
	tc = new ToSaMeshTransceiverRfm70(RFM70_PIN_SCLK, RFM70_PIN_MOSI, RFM70_PIN_MISO, RFM70_PIN_CSN, RFM70_PIN_CE);
	node->addTransceiver(tc);
	eeprom_read_block(&hardwareConfig, HARDWARE_CONFIG_ADDR, sizeof hardwareConfig);
	if (calcCRC(&hardwareConfig, sizeof hardwareConfig) != 0)
		reboot();
	eeprom_read_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
	if (calcCRC(&nodeConfig, sizeof nodeConfig) != 0)
		reboot();
	if (!node->Init(nodeConfig.address))
		reboot();
	node->SendHeartbeat(nodeConfig.rootAddress);
}
