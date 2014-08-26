#define TOSA_MESH_HARDWARE_ATMEGA328
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <string.h>
#include <util/crc16.h>

#define TOSA_MESH_NOADDRESS						0x0000
#define TOSA_MESH_BROADCAST						0xFFFF
#define TOSA_MESH_HOP_RETRY						5

#define TOSA_MESH_RAW_MESSAGE_LEN_MAX			32
#define TOSA_MESH_MESH_MESSAGE_LEN_HEADER		9
#define TOSA_MESH_MESH_MESSAGE_LEN_MAX			TOSA_MESH_RAW_MESSAGE_LEN_MAX - TOSA_MESH_MESH_MESSAGE_LEN_HEADER
#define TOSA_MESH_APP_MESSAGE_LEN_MAX			TOSA_MESH_MESH_MESSAGE_LEN_MAX

/* package types with high bit 7 set (& 0x80 > 0) require a hopack */
/* package types with high bit 6 set (& 0x40 > 0) require an ack */
#define TOSA_MESH_PTYPE_UNKNOWN					0x00	/* ignore */
#define TOSA_MESH_PTYPE_HOPACK					0x01	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_WHOAMI_REQUEST			0x02	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_ARP_REQUEST				0x03	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_DHCP_REQUEST			0x04	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_TIME_REQUEST			0x05	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_TIME_BROADCAST			0x06	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_HOLIDAY_REQUEST			0x07	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_HOLIDAY_BROADCAST		0x08	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_HEARTBEAT				0x09	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_DISCOVER_REQUEST		0x0A	/* no hopack / no ack */
#define TOSA_MESH_PTYPE_ACK						0x80	/* hopack / no ack */
#define TOSA_MESH_PTYPE_NACK					0x81	/* hopack / no ack */
#define TOSA_MESH_PTYPE_WHOAMI_RESPONSE			0x82	/* hopack / no ack */
#define TOSA_MESH_PTYPE_ARP_RESPONSE			0x83	/* hopack / no ack */
#define TOSA_MESH_PTYPE_ARP_FAILURE				0x84	/* hopack / no ack */
#define TOSA_MESH_PTYPE_DHCP_RESPONSE			0x85	/* hopack / no ack */
#define TOSA_MESH_PTYPE_ANNOUNCE_REQUEST		0x86	/* hopack / no ack */
#define TOSA_MESH_PTYPE_ANNOUNCE_RESPONSE		0x87	/* hopack / no ack */
#define TOSA_MESH_PTYPE_FIRMWARE_REQUEST		0x88	/* hopack / no ack */
#define TOSA_MESH_PTYPE_FIRMWARE_RESPONSE		0x89	/* hopack / no ack */
#define TOSA_MESH_PTYPE_CONFIG_REQUEST			0x8A	/* hopack / no ack */
#define TOSA_MESH_PTYPE_CONFIG_RESPONSE			0x8B	/* hopack / no ack */
#define TOSA_MESH_PTYPE_DEBUG					0x8C	/* hopack / no ack */
#define TOSA_MESH_PTYPE_PING					0x8D	/* hopack / no ack */
#define TOSA_MESH_PTYPE_PONG					0x8E	/* hopack / no ack */
#define TOSA_MESH_PTYPE_DISCOVER_RESPONSE		0x8F	/* hopack / no ack */
#define TOSA_MESH_PTYPE_RESET					0x90	/* hopack / no ack */

#define TOSA_MESH_PTYPE_DATASYNC				0xF0	/* hopack / ack */
#define TOSA_MESH_PTYPE_DATA_SET				0xF1	/* hopack / ack */
#define TOSA_MESH_PTYPE_DATA_GET_REQUEST		0xB2	/* hopack / no ack */
#define TOSA_MESH_PTYPE_DATA_GET_RESPONSE		0xF2	/* hopack / ack */

#define TOSA_MESH_CONFIG_FETCHALL			0x80
#define TOSA_MESH_CONFIG_TYPE_HEADER		0x00
#define TOSA_MESH_CONFIG_TYPE_IO			0x01
#define TOSA_MESH_CONFIG_TYPE_NETWORK		0x02
#define TOSA_MESH_CONFIG_TYPE_CONTROL		0x03
#define TOSA_MESH_CONFIG_TYPE_PARAMETER		0x04
#define TOSA_MESH_CONFIG_TYPE_TIMER			0x05
#define TOSA_MESH_CONFIG_TYPE_LAST			TOSA_MESH_CONFIG_TYPE_TIMER

#define FIRMWARE_BLOCK_SIZE		0x10

typedef struct {
	uint8_t tc;
    uint16_t to;
    uint16_t from;
    uint16_t source;
    uint16_t dest;
    uint8_t pid;
    uint8_t ptype;
    uint8_t cost;
    uint8_t len;
	uint8_t data[TOSA_MESH_MESH_MESSAGE_LEN_MAX];
} meshMessage;

typedef struct {
    uint16_t source;
    uint16_t dest;
    uint8_t pid;
    uint8_t ptype;
    uint8_t len;
	uint8_t data[TOSA_MESH_APP_MESSAGE_LEN_MAX];
} appMessage;

typedef struct {
	uint8_t macAddress[6];
} DhcpRequest;

typedef struct {
	uint8_t macAddress[6];
	uint16_t newAddress;
} DhcpResponse;

typedef struct {
	uint16_t hwType;
	uint16_t hwVersion;
	uint16_t fwVersion;
	uint16_t fwCrc;
	uint16_t configCrc;
} AnnounceRequest;

typedef struct {
	uint16_t fwVersion;
	uint16_t fwBlockCount;
	uint16_t fwCrc;
	uint16_t configCrc;
} AnnounceResponse;

typedef struct {
	uint16_t hwType;
	uint16_t hwVersion;
	uint16_t fwVersion;
	uint8_t fwBlockSize;
	uint16_t fwBlockIndex;
} FirmwareRequest;

typedef struct {
	uint16_t fwBlockIndex;
	uint8_t fwBlock[FIRMWARE_BLOCK_SIZE];
} FirmwareResponse;


typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
} Datestamp;

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t weekday;
} Timestamp;

struct HardwareConfig {
	uint16_t hwType;
	uint16_t hwVersion;
	uint8_t macAddress[6];
	uint16_t crc;
};

struct NodeConfig {
	uint16_t address;
	uint16_t rootAddress;
	uint16_t fwVersion;
	uint16_t fwBlockCount;
	uint16_t fwCrc;
	uint16_t configCrc;
	uint16_t crc;
};

#define HARDWARE_CONFIG_ADDR	((uint8_t*) (1024 - 4 - 12)) // 1024 - 4 - sizeof HardwareConfig
#define NODE_CONFIG_ADDR		((uint8_t*) (1024 - 4 - 12 - 14)) // 1024 - 4 - sizeof HardwareConfig - sizeof NodeConfig

#define	RFM70_PIN_SCLK		13
#define	RFM70_PIN_MOSI		11
#define	RFM70_PIN_MISO		12
#define	RFM70_PIN_CSN		10
#define	RFM70_PIN_CE		8
#define	LED_PIN_RED			7
#define	LED_PIN_GREEN		6
#define RFM70_DDR			DDRB
#define RFM70_PORT			PORTB
#define RFM70_PIN			PINB
#define RFM70_BIT_OFFSET	8
#define	RFM70_BIT_SCLK		(RFM70_PIN_SCLK - RFM70_BIT_OFFSET)
#define	RFM70_BIT_MOSI		(RFM70_PIN_MOSI - RFM70_BIT_OFFSET)
#define	RFM70_BIT_MISO		(RFM70_PIN_MISO - RFM70_BIT_OFFSET)
#define	RFM70_BIT_CSN		(RFM70_PIN_CSN - RFM70_BIT_OFFSET)
#define	RFM70_BIT_CE		(RFM70_PIN_CE - RFM70_BIT_OFFSET)
#define LED_DDR				DDRD
#define LED_PORT			PORTD
#define LED_PIN				PIND
#define LED_BIT_OFFSET		0
#define LED_BIT_RED			(LED_PIN_RED - LED_BIT_OFFSET)
#define LED_BIT_GREEN		(LED_PIN_GREEN - LED_BIT_OFFSET)
#define LED_BITMASK_NONE	0
#define LED_BITMASK_RED		(1 << LED_BIT_RED)
#define LED_BITMASK_GREEN	(1 << LED_BIT_GREEN)
#define LED_BITMASK_BOTH	(LED_BITMASK_RED | LED_BITMASK_GREEN)

#define RFM70_MAX_PACKET_LEN  32
#define RFM70_CMD_R_RX_PAYLOAD         0x61
#define RFM70_CMD_W_TX_PAYLOAD         0xA0
#define RFM70_CMD_FLUSH_TX             0xE1
#define RFM70_CMD_FLUSH_RX             0xE2
#define RFM70_CMD_REUSE_TX_PL          0xE3
#define RFM70_CMD_W_TX_PAYLOAD_NOACK   0xB0
#define RFM70_CMD_W_ACK_PAYLOAD        0xA8
#define RFM70_CMD_ACTIVATE             0x50
#define RFM70_CMD_R_RX_PL_WID          0x60
#define RFM70_CMD_NOP                  0xFF
#define RFM70_REG_CONFIG               0x00
#define RFM70_REG_EN_AA                0x01
#define RFM70_REG_EN_RXADDR            0x02
#define RFM70_REG_SETUP_AW             0x03
#define RFM70_REG_SETUP_RETR           0x04
#define RFM70_REG_RF_CH                0x05
#define RFM70_REG_RF_SETUP             0x06
#define RFM70_REG_STATUS               0x07
#define RFM70_REG_OBSERVE_TX           0x08
#define RFM70_REG_CD                   0x09
#define RFM70_REG_RX_ADDR_P0           0x0A
#define RFM70_REG_RX_ADDR_P1           0x0B
#define RFM70_REG_RX_ADDR_P2           0x0C
#define RFM70_REG_RX_ADDR_P3           0x0D
#define RFM70_REG_RX_ADDR_P4           0x0E
#define RFM70_REG_RX_ADDR_P5           0x0F
#define RFM70_REG_TX_ADDR              0x10
#define RFM70_REG_RX_PW_P0             0x11
#define RFM70_REG_RX_PW_P1             0x12
#define RFM70_REG_RX_PW_P2             0x13
#define RFM70_REG_RX_PW_P3             0x14
#define RFM70_REG_RX_PW_P4             0x15
#define RFM70_REG_RX_PW_P5             0x16
#define RFM70_REG_FIFO_STATUS          0x17
#define RFM70_REG_DYNPD                0x1C
#define RFM70_REG_FEATURE              0x1D
#define RFM70_CMD_READ_REG            0x00
#define RFM70_CMD_WRITE_REG           0x20
#define STATUS_RX_DR    0x40
#define STATUS_TX_DS    0x20
#define STATUS_MAX_RT   0x10
#define STATUS_TX_FULL  0x01
#define FIFO_STATUS_TX_REUSE  0x40
#define FIFO_STATUS_TX_FULL   0x20
#define FIFO_STATUS_TX_EMPTY  0x10
#define FIFO_STATUS_RX_FULL   0x02
#define FIFO_STATUS_RX_EMPTY  0x01
#define TOSA_MESH_RFM70_NET1				0x55
#define TOSA_MESH_RFM70_NET2				0x00
#define TOSA_MESH_RFM70_NET3				0x00
#define TOSA_MESH_RFM70_CHANNEL				10

typedef struct {
    uint16_t to;
    uint8_t len;
	uint8_t data[TOSA_MESH_RAW_MESSAGE_LEN_MAX];
} rawMessageRfm70;

// private functions for rfm70 start
static void delaymicro() {
	asm("nop"); 
}

static void delaymilli(uint16_t t) {
	uint16_t d, a;
	for (d = 0; d < t; d++)
		for (a = 0; a < 500; a++)
			asm("nop"); 
}

static void led(uint8_t mask, uint16_t t) {
	LED_PORT = (LED_PORT & ~(LED_BITMASK_BOTH)) | mask;
	if (t > 0) {
		delaymilli(t);
		LED_PORT = (LED_PORT & ~(LED_BITMASK_BOTH));
		delaymilli(t);
	}
}

static unsigned char rfm70_SPI_RW(unsigned char value) {
	unsigned char i;
	for(i = 0 ;i < 8;i++) {
		delaymicro();
		if ((value & 0x80) == 0x00)
			RFM70_PORT &= ~(1 << RFM70_BIT_MOSI);
		else
			RFM70_PORT |= (1 << RFM70_BIT_MOSI);
		value = (value << 1);    // shift next bit into MSB..
		delaymicro();
		RFM70_PORT |= (1 << RFM70_BIT_SCLK);
		value |= ((RFM70_PIN >> RFM70_BIT_MISO) & 0x01);     // capture current MISO bit
		delaymicro();
		RFM70_PORT &= ~(1 << RFM70_BIT_SCLK);
		delaymicro();
	}
	return value;
}

static void rfm70_register_write(unsigned char reg, unsigned char value) {
	if(reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_WRITE_REG;
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	rfm70_SPI_RW(value);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
}

static unsigned char rfm70_register_read(unsigned char reg) {
	unsigned char value;
	if(reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_READ_REG;       
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	value = rfm70_SPI_RW(0);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
	return value;
}

static void rfm70_bank(unsigned char b) {
	unsigned char st = 0x80 & rfm70_register_read(RFM70_REG_STATUS);
	if((st && (b == 0)) || ((st == 0) && b)) {
		rfm70_register_write(RFM70_CMD_ACTIVATE, 0x53);
	}
}

static void rfm70_mode_receive() {
	unsigned char value;
	rfm70_register_write(RFM70_CMD_FLUSH_RX, 0);
	value = rfm70_register_read(RFM70_REG_STATUS);
	rfm70_register_write(RFM70_REG_STATUS, value);
	RFM70_PORT &= ~(1 << RFM70_BIT_CE);
	value = rfm70_register_read(RFM70_REG_CONFIG);
	value |= 0x01;
	value |= 0x02;
	rfm70_register_write(RFM70_REG_CONFIG, value);
	RFM70_PORT |= (1 << RFM70_BIT_CE);
}

static void rfm70_mode_transmit() {
	unsigned char value;
	rfm70_register_write(RFM70_CMD_FLUSH_TX, 0);
	value = rfm70_register_read(RFM70_REG_STATUS);
	rfm70_register_write(RFM70_REG_STATUS, value);
	RFM70_PORT &= ~(1 << RFM70_BIT_CE);
	value = rfm70_register_read(RFM70_REG_CONFIG);
	value &= 0xFE;
	value |= 0x02;
	rfm70_register_write(RFM70_REG_CONFIG, value);
	RFM70_PORT |= (1 << RFM70_BIT_CE);
}

static void rfm70_buffer_write(char reg, const unsigned char *pBuf, unsigned char length) {
	unsigned char i;
	if (reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_WRITE_REG;      
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	for(i = 0; i < length; i++)
		rfm70_SPI_RW(*pBuf++);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
}

static void rfm70_buffer_read(unsigned char reg, unsigned char *pBuf, unsigned char length) {
	unsigned char i;
	if(reg < RFM70_CMD_WRITE_REG)
		reg |= RFM70_CMD_READ_REG;       
	RFM70_PORT &= ~(1 << RFM70_BIT_CSN);
	rfm70_SPI_RW(reg);
	for(i = 0; i < length; i++)
		*pBuf++ = rfm70_SPI_RW(0);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
}

static meshMessage RawToMesh(rawMessageRfm70 rm) {
	meshMessage mm;
	if (rm.len >= TOSA_MESH_MESH_MESSAGE_LEN_HEADER) {
		mm.to = rm.to;
		mm.from = (rm.data[1] << 8) + rm.data[0];
		mm.source = (rm.data[3] << 8) + rm.data[2];
		mm.dest = (rm.data[5] << 8) + rm.data[4];
		mm.pid = rm.data[6];
		mm.ptype = rm.data[7];
		mm.cost = rm.data[8];
		mm.len = rm.len - TOSA_MESH_MESH_MESSAGE_LEN_HEADER;
		for (uint8_t i = TOSA_MESH_MESH_MESSAGE_LEN_HEADER; i < rm.len; i++)
			mm.data[i - TOSA_MESH_MESH_MESSAGE_LEN_HEADER] = rm.data[i];
	}
	else
		mm.from = TOSA_MESH_BROADCAST;
	return mm;
}

static rawMessageRfm70 MeshToRaw(meshMessage mm) {
	rawMessageRfm70 rm;
	rm.to = mm.to;
	rm.len = mm.len + TOSA_MESH_MESH_MESSAGE_LEN_HEADER;
	rm.data[0] = (uint8_t)(mm.from & 0x00FF);
	rm.data[1] = (uint8_t)((mm.from & 0xFF00) >> 8);
	rm.data[2] = (uint8_t)(mm.source & 0x00FF);
	rm.data[3] = (uint8_t)((mm.source & 0xFF00) >> 8);
	rm.data[4] = (uint8_t)(mm.dest & 0x00FF);
	rm.data[5] = (uint8_t)((mm.dest & 0xFF00) >> 8);
	rm.data[6] = mm.pid;
	rm.data[7] = mm.ptype;
	rm.data[8] = mm.cost;
	for (uint8_t i = 0; i < mm.len; i++)
		rm.data[i + TOSA_MESH_MESH_MESSAGE_LEN_HEADER] = mm.data[i];
	return rm;
}
// private functions for rfm70 end

static uint16_t TransceiverAddress = TOSA_MESH_NOADDRESS;

const unsigned long Bank1_Reg0_13[] = {
   0xE2014B40,
   0x00004BC0,
   0x028CFCD0,
   0x41390099,
   0x0B869Ef9, 
   0xA67F0624,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00127300,
   0x36B48000 };   
   
const unsigned char Bank1_Reg14[] = {
   0x41, 0x20, 0x08, 0x04, 0x81, 0x20, 0xCF, 0xF7, 0xFE, 0xFF, 0xFF };   

static void TransceiverInit() {
	// init pins
	RFM70_DDR |= (1 << RFM70_BIT_MOSI) + (1 << RFM70_BIT_SCLK) + (1 << RFM70_BIT_CE) + (1 << RFM70_BIT_CSN);
	RFM70_DDR &= ~(1 << RFM70_BIT_MISO);
	RFM70_PORT |= (1 << RFM70_BIT_CSN);
	RFM70_PORT &= ~((1 << RFM70_BIT_MOSI) + (1 << RFM70_BIT_SCLK) + (1 << RFM70_BIT_CE));
	delaymilli(50);
	// init register bank 0
	rfm70_bank(0);
	rfm70_register_write( 0, 0x0F ); 
	rfm70_register_write( 1, 0x00 ); 
	rfm70_register_write( 5, TOSA_MESH_RFM70_CHANNEL );
	uint8_t addrarr[5];
	addrarr[4] = TOSA_MESH_RFM70_NET3;
	addrarr[3] = TOSA_MESH_RFM70_NET2;
	addrarr[2] = TOSA_MESH_RFM70_NET1;
    addrarr[1] = (uint8_t)((TOSA_MESH_BROADCAST & 0xFF00) >> 8);
    addrarr[0] = (uint8_t)(TOSA_MESH_BROADCAST & 0x00FF);
	rfm70_buffer_write( RFM70_REG_RX_ADDR_P1, addrarr, 5 );  
    addrarr[1] = (uint8_t)((TOSA_MESH_NOADDRESS & 0xFF00) >> 8);
    addrarr[0] = (uint8_t)(TOSA_MESH_NOADDRESS & 0x00FF);
	rfm70_buffer_write( RFM70_REG_RX_ADDR_P0, addrarr, 5 );  
	rfm70_buffer_write( RFM70_REG_TX_ADDR, addrarr, 5 );   
	if (rfm70_register_read(29) == 0)
		rfm70_register_write( RFM70_CMD_ACTIVATE, 0x73 );
	rfm70_register_write( 28, 0x3F ); 
	rfm70_register_write( 29, 0x07 );  
	// init register bank 1
	unsigned char i, j;
	unsigned char WriteArr[ 12 ];
	rfm70_bank(1);
	for( i = 0; i <= 8; i++ ){ //reverse!
		for( j = 0; j < 4; j++ ){
			WriteArr[ j ]=( Bank1_Reg0_13[ i ]>>(8*(j) ) )&0xff;
		}  
		rfm70_buffer_write( i,WriteArr, 4 );
	}
	for( i = 9; i <= 13; i++ ){
		for( j = 0; j < 4; j++ ){
			WriteArr[ j ]=( Bank1_Reg0_13[ i ]>>(8*(3-j) ) )&0xff;
		}
		rfm70_buffer_write( i, WriteArr, 4 );
	}
	rfm70_buffer_write( 14, Bank1_Reg14, 11 );
	//toggle REG4<25,26>
	for(j = 0; j < 4; j++){
		WriteArr[ j ]=( Bank1_Reg0_13[ 4 ]>>(8*(j) ) )&0xff;
	} 
	WriteArr[ 0 ] = WriteArr[ 0 ] | 0x06;
	rfm70_buffer_write( 4, WriteArr, 4);
	WriteArr[ 0 ] = WriteArr[ 0 ] & 0xf9;
	rfm70_buffer_write( 4, WriteArr,4);
	// finish init
	rfm70_bank( 0 );
	delaymilli(50); 
	rfm70_mode_receive();
}

static void TransceiverSetAddress(uint16_t addr) {
	uint8_t addrarr[5];
	addrarr[4] = TOSA_MESH_RFM70_NET3;
	addrarr[3] = TOSA_MESH_RFM70_NET2;
	addrarr[2] = TOSA_MESH_RFM70_NET1;
	addrarr[1] = (uint8_t)((addr & 0xFF00) >> 8);
	addrarr[0] = (uint8_t)(addr & 0x00FF);
	rfm70_buffer_write(RFM70_REG_RX_ADDR_P0, addrarr, 5);   
	TransceiverAddress = addr;
}

static void TransceiverSend(meshMessage mm) {
	rawMessageRfm70 rm = MeshToRaw(mm);
	uint8_t toarr[5];
	toarr[4] = TOSA_MESH_RFM70_NET3;
	toarr[3] = TOSA_MESH_RFM70_NET2;
	toarr[2] = TOSA_MESH_RFM70_NET1;
	toarr[1] = (uint8_t)((rm.to & 0xFF00) >> 8);
	toarr[0] = (uint8_t)(rm.to & 0x00FF);
	rfm70_buffer_write(RFM70_REG_TX_ADDR, toarr, 5);   
	rfm70_mode_transmit();
	rfm70_buffer_write(RFM70_CMD_W_TX_PAYLOAD_NOACK, rm.data, rm.len);
	delaymilli(1);
	rfm70_mode_receive();
}

static uint8_t TransceiverHasData() {
	return ((rfm70_register_read( RFM70_REG_FIFO_STATUS ) & FIFO_STATUS_RX_EMPTY ) == 0);
}

static meshMessage TransceiverReceive() {
	rawMessageRfm70 rm;
	if (TransceiverHasData()) {
		if (((rfm70_register_read( RFM70_REG_STATUS ) >> 1 ) & 0x07) == 0)
			rm.to = TransceiverAddress;
		else
			rm.to = TOSA_MESH_BROADCAST;
		rm.len = rfm70_register_read(RFM70_CMD_R_RX_PL_WID);
		rfm70_buffer_read(RFM70_CMD_R_RX_PAYLOAD, rm.data, rm.len);
		rfm70_register_write(RFM70_CMD_FLUSH_RX, 0);
	}
	meshMessage mm = RawToMesh(rm);
	return mm;
}

struct HardwareConfig hardwareConfig;
struct NodeConfig nodeConfig;

uint8_t progBuf[SPM_PAGESIZE];
uint8_t nextpid;

meshMessage mmRequest;
meshMessage mmResponse;
meshMessage mmHopAck;

int main(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));

volatile char dummy;

EMPTY_INTERRUPT(WDT_vect);

static uint16_t calcCRC (const void* ptr, uint16_t len) {
  uint16_t crc = ~0;
  for (uint16_t i = 0; i < len; ++i)
    crc = _crc16_update(crc, ((const char*) ptr)[i]);
  return crc;
}

static uint16_t calcCRCrom (const void* ptr, uint16_t len) {
	uint16_t crc = ~0;
	for (uint16_t i = 0; i < len; ++i)
		crc = _crc16_update(crc, pgm_read_byte((uint16_t) ptr + i));
	return crc;
}

static uint8_t validFirmware () {
	return calcCRCrom(0, nodeConfig.fwBlockCount * FIRMWARE_BLOCK_SIZE) == nodeConfig.fwCrc;
}

static void reboot() {
  wdt_enable(WDTO_15MS);
  for (;;);
}

static void startup() {
	if (validFirmware()) {
		led(LED_BITMASK_GREEN, 200);
		clock_prescale_set(clock_div_1);
		((void(*)()) 0)();
	} else
		reboot();
}

static void boot_program_page (uint32_t page, uint8_t *buf) {
  uint8_t sreg = SREG;
  cli();
  eeprom_busy_wait();
  boot_page_erase(page);
  boot_spm_busy_wait();
  for (uint16_t i = 0; i < SPM_PAGESIZE; i += 2) {
    uint16_t w = *buf++;
    w += (*buf++) << 8;
    boot_page_fill(page + i, w);
  }
  boot_page_write(page);
  boot_spm_busy_wait();
  boot_rww_enable();
  SREG = sreg;
}

static void SendHopAck() {
	mmHopAck.source = mmResponse.dest;
	mmHopAck.from = TransceiverAddress;
	mmHopAck.dest = mmResponse.source;
	mmHopAck.to = mmResponse.from;
	mmHopAck.pid = nextpid++;
	mmHopAck.ptype = TOSA_MESH_PTYPE_HOPACK;
	mmHopAck.cost = 1;
	mmHopAck.len = 1;
	mmHopAck.data[0] = mmResponse.pid;
	TransceiverSend(mmHopAck);
}

static uint8_t SendAndWait(uint8_t reqType, uint8_t resType) {
	mmRequest.pid = nextpid++;
	mmRequest.ptype = reqType;
	mmRequest.cost = 1;
	for (uint8_t i = 0; i < 10; i++) {
		TransceiverSend(mmRequest);
		for (uint16_t j = 0; j < 20; j++) {
			wdt_reset();
			for (uint16_t j = 0; j < 1000; j++) {
				if (TransceiverHasData()) {
					mmResponse = TransceiverReceive();
					if ((mmResponse.ptype & 0x80) > 0)
						SendHopAck();
					if (mmResponse.ptype == resType) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

static void run () {
	wdt_reset();
	nextpid = 0;
	TransceiverInit();
	mmRequest.pid = 0;
	wdt_reset();
	eeprom_read_block(&hardwareConfig, HARDWARE_CONFIG_ADDR, sizeof hardwareConfig);
	if (calcCRC(&hardwareConfig, sizeof hardwareConfig) != 0) {
		led(LED_BITMASK_BOTH, 0);
		mmRequest.source = TOSA_MESH_NOADDRESS;
		mmRequest.from = TOSA_MESH_NOADDRESS;
		mmRequest.dest = TOSA_MESH_BROADCAST;
		mmRequest.to = TOSA_MESH_BROADCAST;
		mmRequest.len = 0;
		for (;;) {
			if (SendAndWait(TOSA_MESH_PTYPE_WHOAMI_REQUEST, TOSA_MESH_PTYPE_WHOAMI_RESPONSE)) {
				hardwareConfig.hwType = mmResponse.data[0];
				hardwareConfig.hwVersion = mmResponse.data[1];
				for (uint8_t i = 0; i < 6; i++)
					hardwareConfig.macAddress[i] =  mmResponse.data[i + 2];
				hardwareConfig.crc = calcCRC(&hardwareConfig, sizeof hardwareConfig - 2);
				eeprom_write_block(&hardwareConfig, HARDWARE_CONFIG_ADDR, sizeof hardwareConfig);
				reboot();
			}
		}
	}
	wdt_reset();
	eeprom_read_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
	uint16_t rootNextHop = TOSA_MESH_NOADDRESS;
	if (calcCRC(&nodeConfig, sizeof nodeConfig) != 0) {
		nodeConfig.address = TOSA_MESH_NOADDRESS;
		TransceiverSetAddress(nodeConfig.address);
		mmRequest.source = TOSA_MESH_NOADDRESS;
		mmRequest.from = TOSA_MESH_NOADDRESS;
		mmRequest.dest = TOSA_MESH_BROADCAST;
		mmRequest.to = TOSA_MESH_BROADCAST;
		mmRequest.len = 6;
		for (uint8_t i = 0; i < 6; i++)
			mmRequest.data[i] = hardwareConfig.macAddress[i];
		while (nodeConfig.address == TOSA_MESH_NOADDRESS)
			if (SendAndWait(TOSA_MESH_PTYPE_DHCP_REQUEST, TOSA_MESH_PTYPE_DHCP_RESPONSE)) {
				uint8_t macCheck = 1;
				for (uint8_t m = 0; m < 6; m++)
					if (mmResponse.data[m] != hardwareConfig.macAddress[m]) {
						macCheck = 0;
						break;
					}
				if (macCheck > 0) {
					nodeConfig.address = (mmResponse.data[6 + 1] << 8) + mmResponse.data[6];
					nodeConfig.rootAddress = mmResponse.source;
					nodeConfig.fwVersion = 0;
					nodeConfig.fwBlockCount = 0;
					nodeConfig.fwCrc = 0;
					nodeConfig.configCrc = 0;
					nodeConfig.crc = calcCRC(&nodeConfig, sizeof nodeConfig - 2);
					eeprom_write_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
					rootNextHop = mmResponse.from;
				}
			}
	}
	wdt_reset();
	TransceiverSetAddress(nodeConfig.address);
	mmRequest.source = nodeConfig.address;
	mmRequest.from = nodeConfig.address;
	mmRequest.dest = nodeConfig.rootAddress;
	mmRequest.to = TOSA_MESH_BROADCAST;
	mmRequest.len = 0;
	for (uint8_t i = 0; i < 10; i++) {
		if (rootNextHop != TOSA_MESH_NOADDRESS)
			break;
		if (SendAndWait(TOSA_MESH_PTYPE_ARP_REQUEST, TOSA_MESH_PTYPE_ARP_RESPONSE))
			rootNextHop = mmResponse.from;
	}
	if (rootNextHop == TOSA_MESH_NOADDRESS) { // no response from root - start without announce
		startup();
	}
	wdt_reset();
	mmRequest.to = rootNextHop;
	mmRequest.len = 10; // ToDo : sizeof AnnounceRequest;
	AnnounceRequest *announceRequest = (AnnounceRequest *)mmRequest.data;
	announceRequest->hwType = hardwareConfig.hwType;
	announceRequest->hwVersion = hardwareConfig.hwVersion;
	announceRequest->fwVersion = nodeConfig.fwVersion;
	announceRequest->fwCrc = nodeConfig.fwCrc;
	announceRequest->configCrc = nodeConfig.configCrc;
	uint8_t announced = 0;
	for (uint8_t i = 0; i < 10; i++) {
		if (SendAndWait(TOSA_MESH_PTYPE_ANNOUNCE_REQUEST, TOSA_MESH_PTYPE_ANNOUNCE_RESPONSE)) {
			announced = 1;
			break;
		}
	}
	if (announced == 0) { // no response from root - start without announce
		startup();
	}
	AnnounceResponse *announceResponse = (AnnounceResponse *)mmResponse.data;
	if ((nodeConfig.fwVersion == announceResponse->fwVersion) && (nodeConfig.fwCrc == announceResponse->fwCrc) && validFirmware()) {
		if (nodeConfig.configCrc != announceResponse->configCrc) {
			nodeConfig.configCrc = 0;
			nodeConfig.crc = calcCRC(&nodeConfig, sizeof nodeConfig - 2);
			eeprom_write_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
		}
		startup();
	}
	led(LED_BITMASK_RED, 200);
	nodeConfig.fwVersion = announceResponse->fwVersion;
	nodeConfig.fwBlockCount = announceResponse->fwBlockCount;
	nodeConfig.fwCrc = announceResponse->fwCrc;
	nodeConfig.crc = calcCRC(&nodeConfig, sizeof nodeConfig - 2);
	mmRequest.len = 9; // ToDo : sizeof FirmwareRequest;
	FirmwareRequest *firmwareRequest = (FirmwareRequest *)mmRequest.data;
	FirmwareResponse *firmwareResponse = (FirmwareResponse *)mmResponse.data;
	firmwareRequest->hwType = hardwareConfig.hwType;
	firmwareRequest->hwVersion = hardwareConfig.hwVersion;
	firmwareRequest->fwVersion = nodeConfig.fwVersion;
	firmwareRequest->fwBlockSize = FIRMWARE_BLOCK_SIZE;
	wdt_reset();
	for (uint16_t block = 0; block < nodeConfig.fwBlockCount; block++) {
		firmwareRequest->fwBlockIndex = block;
		for (uint8_t i = 0; i < 101; i++) {
			if (SendAndWait(TOSA_MESH_PTYPE_FIRMWARE_REQUEST, TOSA_MESH_PTYPE_FIRMWARE_RESPONSE)) {
				if (block == firmwareResponse->fwBlockIndex) {
					uint8_t offset = (block * FIRMWARE_BLOCK_SIZE) % SPM_PAGESIZE;
					memcpy(progBuf + offset, firmwareResponse->fwBlock, FIRMWARE_BLOCK_SIZE);
					if (offset == SPM_PAGESIZE - FIRMWARE_BLOCK_SIZE)
						boot_program_page((block * FIRMWARE_BLOCK_SIZE) - offset, progBuf);
					break;
				} else if (i == 100) {
					reboot();
				}
			}
		}
	}
	wdt_reset();
	if (validFirmware())
		eeprom_write_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
}	

int main () {
	asm volatile ("clr __zero_reg__");
	// switch to 4 MHz
	clock_prescale_set(clock_div_4);
	MCUSR = 0;
//	wdt_disable();
	wdt_enable(WDTO_8S);
	LED_DDR |= LED_BITMASK_BOTH;
	led(LED_BITMASK_BOTH, 200);
/*
	uint8_t backoff = 0;
	while (run() > 100) {
		if (++backoff > 10)
			backoff = 0;
		clock_prescale_set(clock_div_256);
		for (long i = 0; i < 10000L << backoff && !dummy; ++i);
		clock_prescale_set(clock_div_4);
	}
	reboot();
*/
	run();
	reboot();
}
