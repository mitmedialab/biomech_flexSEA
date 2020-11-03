/*
 * flexsea_multi_frame_packet_def.h
 *
 *  Created on: May 18, 2018
 *      Author: Dephy Inc
 */

#ifndef FLEXSEA_COMM_INC_FLEXSEA_MULTI_FRAME_PACKET_DEF_H_
#define FLEXSEA_COMM_INC_FLEXSEA_MULTI_FRAME_PACKET_DEF_H_


#define MAX_FRAMES_PER_MULTI_PACKET 4

/* A packet can be composed of 1-4 frames
 * Each frame has the following format
 *
 * SOF (1 byte), NB (1 byte), MultiInfo(1 byte), PacketData(NB bytes), checksum (1 byte), EOF(1 byte)
 *
 * The MultiInfo is a byte containing information about a frames place within the packet.
 * bits 0-2 : id of last frame in packet
 * bits 3-6 : id of this frame
 * bits 7-8 : packet id
 * (lowest bit is 0, highest is 8)
 * */

#define MULTI_SOF 0xED
#define MULTI_EOF 0xEE
#define MULTI_ESC 0xE9

#define MULTI_PACKETID(multiInfo) ( ( (multiInfo) >> 6) & 0x03 )
#define MULTI_THIS_FRAMEID(multiInfo) ( ( (multiInfo) >> 3) & 0x07 )
#define MULTI_LAST_FRAMEID(multiInfo) ( (multiInfo) & 0x07 )
#define MULTI_GENINFO(pid, thisframe, lastframe) ( ((pid) << 6) | ((thisframe)&0x07) << 3 | ((lastframe)&0x03)  )

#define MULTI_NUM_OVERHEAD_BYTES_FRAME 5
#define MULTI_DATA_OFFSET 3

#define MULTI_EOF_POS_FROM_SOF(sof, nb) ( (sof) + (nb) + MULTI_NUM_OVERHEAD_BYTES_FRAME - 1 )
#define MULTI_DATA_POS_FROM_SOF(sof) ( (sof) + MULTI_DATA_OFFSET )
#define MULTI_CHECKSUM_POS_FROM_SOF(sof, nb) ( (sof) + (nb) + MULTI_NUM_OVERHEAD_BYTES_FRAME - 2 )
#define MULTI_INFO_POS_FROM_SOF(sof) ( (sof)+2 )

#define SIZE_OF_MULTIFRAME(frame) ( frame[1] + 5 )

/*
 *	The PacketData has the following format
 *
 *	XID - emitter ID (1 byte)
 *	RID - receiver ID (1 byte)
 *	TSTP - timestamp (4 bytes)
 *	CMDS - number of commands (1 byte)
 *	CMD1 - command byte of first command
 *	DATA1 - first data byte of first command
 *
 * */

#define MP_XID		0
#define MP_RID		1
#define MP_TSTP		2
#define MP_CMDS		6
#define MP_CMD1		7
#define MP_DATA1	8

#define MULTI_PACKET_OVERHEAD MP_DATA1
#define MULTI_GET_CMD7(packet) ( (packet)[MP_CMD1] >> 1 )

#endif /* FLEXSEA_COMM_INC_FLEXSEA_MULTI_FRAME_PACKET_DEF_H_ */
