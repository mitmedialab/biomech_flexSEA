/*
 * flexsea_comm_multi.h
 *
 *  Created on: Apr 10, 2018
 *      Author: Dephy Inc
 */

#ifndef FLEXSEA_COMM_INC_FLEXSEA_COMM_MULTI_H_
#define FLEXSEA_COMM_INC_FLEXSEA_COMM_MULTI_H_

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include "flexsea.h"
#include "flexsea_multi_frame_packet_def.h"
//****************************************************************************
// Structure(s)
//****************************************************************************

// ---------- Multi Packet versions of Comm Periph structs

#define UNPACKED_BUFF_SIZE (MAX_FRAMES_PER_MULTI_PACKET*PACKET_WRAPPER_LEN)
typedef struct MultiWrapper_struct
{
	Port sourcePort;
	Port destinationPort;
	TravelDirection travelDir;

	uint8_t cmd;
	uint8_t numb;

	uint8_t currentMultiPacket;
	uint8_t lastFrameInMultiPacket;

	// bytes as received on the wire
	uint8_t packed[MAX_FRAMES_PER_MULTI_PACKET][PACKET_WRAPPER_LEN];
	uint8_t frameMap;
	uint8_t isMultiComplete;

	//Unpacked packet ready to be parsed.
	uint8_t unpacked[UNPACKED_BUFF_SIZE];
	uint16_t unpackedIdx;

} MultiWrapper;

typedef struct MultiCommPeriph_struct
{
	//Peripheral state and info:
	Port port;
	PortType portType;
	TransceiverSate transState;

	uint8_t bytesReadyFlag;
	uint8_t unpackedPacketsAvailable;
	uint8_t packetReady;
	uint8_t timeStamp;
	int parsingCachedIndex;

	//Data:
	circularBuffer_t circularBuff;

	//Attach PacketWrappers:
	MultiWrapper in;
	MultiWrapper out;


} MultiCommPeriph;

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void initMultiPeriph(MultiCommPeriph *cp, Port port, PortType pt);
uint8_t tryParse(MultiCommPeriph *cp);
uint8_t parseReadyMultiString(MultiCommPeriph* cp);

uint8_t receiveAndFillResponse(uint8_t cmd_7bits, uint8_t pType, MultiPacketInfo *info, MultiCommPeriph* cp);
void setMsgInfo(uint8_t* outbuf, uint8_t xid, uint8_t rid, uint8_t cmdcode, uint8_t cmdtype, uint32_t timestamp);
uint8_t packMultiPacket(MultiWrapper* p);
void resetToPacketId(MultiWrapper* p, uint8_t id);
int16_t copyIntoMultiPacket(MultiCommPeriph* p, uint8_t *src, uint16_t nb);
void advanceMultiInput(MultiCommPeriph *p, int16_t nb);

//****************************************************************************
// Definition(s):
//****************************************************************************
//Conditional printf() statement:
#ifdef DEBUG_COMM_PRINTF_
	#define DEBUG_COMM_PRINTF(...) printf(__VA_ARGS__)
#else
	#define DEBUG_COMM_PRINTF(...) do {} while (0)
#endif	//DEBUG_COMM_PRINTF_

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern MultiCommPeriph comm_multi_periph[NUMBER_OF_PORTS];

#ifdef __cplusplus
}
#endif



#endif /* FLEXSEA_COMM_INC_FLEXSEA_COMM_MULTI_H_ */
