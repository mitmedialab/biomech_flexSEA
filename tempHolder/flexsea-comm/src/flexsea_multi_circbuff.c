#include "flexsea_multi_circbuff.h"
#include "flexsea_multi_frame_packet_def.h"
#include "flexsea_comm_multi.h"
#include "flexsea_circular_buffer.h"

typedef struct MultiInfoByte_struct {
	uint8_t packetId;
	uint8_t frameId;
	uint8_t lastFrameInPacket;
} MultiInfoByte;

// --------------------------------
// Private Function Prototypes
// --------------------------------
int circ_buff_checkFrame(circularBuffer_t *cb, int headerPos);
int circ_buff_copyToWrapper(circularBuffer_t* cb, int headerPos, MultiWrapper* p);
static inline MultiInfoByte decodeMultiInfo(circularBuffer_t* cb, int headerPos);
unsigned copyEscapedString(uint8_t *dst, uint8_t *src, unsigned nb);
void circ_buff_copyToUnpacked(circularBuffer_t* cb, int headerPos, int bytes, MultiWrapper* p);

// --------------------------------
// Public Function Implementations
// --------------------------------

uint16_t unpack_multi_payload_cb(circularBuffer_t *cb, MultiWrapper* p)
{
    int foundString = 0;
    int lastPossibleHeaderIndex = circ_buff_get_size(cb) - MULTI_NUM_OVERHEAD_BYTES_FRAME;
    int headerPos = -1;

    // search for a frame
    while(!foundString && headerPos < lastPossibleHeaderIndex)
    {
        headerPos = circ_buff_search(cb, MULTI_SOF, headerPos+1);

        //if we can't find a header, we quit searching for strings
        if(headerPos == -1)
        	break;

        foundString = circ_buff_checkFrame(cb, headerPos);
    }

    if(foundString)
    	return circ_buff_copyToWrapper(cb, headerPos, p);

    return 0;
}

uint16_t unpack_multi_payload_cb_cached(circularBuffer_t *cb, MultiWrapper* p, int *cacheStart)
{
    int bufSize = circ_buff_get_size(cb);

    if(*cacheStart > bufSize)
        *cacheStart = 0;

    if(*cacheStart == bufSize)
        return 0;

    int foundString = 0;
    int lastPossibleHeaderIndex = bufSize - MULTI_NUM_OVERHEAD_BYTES_FRAME;
    int headerPos = (*cacheStart)-1;
    int lastHeaderPos = headerPos;

    // search for a frame
    while(!foundString && headerPos < lastPossibleHeaderIndex)
    {
        headerPos = circ_buff_search(cb, MULTI_SOF, headerPos+1);

        //if we can't find a header, we quit searching for strings
        if(headerPos == -1)
        	break;

        foundString = circ_buff_checkFrame(cb, headerPos);
        lastHeaderPos = headerPos;
    }

    int numBytesInPackedString = 0;
    if(foundString)
    {
    	numBytesInPackedString = circ_buff_copyToWrapper(cb, headerPos, p);

        // update the cached header value
        *cacheStart = numBytesInPackedString;
        *cacheStart = circ_buff_search_not(cb, 0, *cacheStart);
    }
    else
    {
        // update the cached header value
        if(lastHeaderPos >= 0)
            *cacheStart = lastHeaderPos;
        else
            *cacheStart = bufSize;
    }

    return numBytesInPackedString;
}

// --------------------------------
// Private Function Implementations
// --------------------------------

int circ_buff_checkFrame(circularBuffer_t *cb, int headerPos)
{
    int foundFrame = 0, numBytes, footerPos, checksum;
    if(headerPos <= cb->size - MULTI_NUM_OVERHEAD_BYTES_FRAME)
    {
    	numBytes = circ_buff_peak(cb, headerPos + 1);
    	footerPos = MULTI_EOF_POS_FROM_SOF(headerPos, numBytes);
        foundFrame = (footerPos < cb->size && circ_buff_peak(cb, footerPos) == MULTI_EOF);
    }

    if(foundFrame)
    {
        //checksum only adds actual data, not any of the frame stuff
        checksum = circ_buff_checksum(cb, MULTI_DATA_POS_FROM_SOF(headerPos) , footerPos-1);

        //if checksum is valid than we found a valid string
        return (checksum == circ_buff_peak(cb, footerPos-1));
    }

    return 0;
}

int circ_buff_copyToWrapper(circularBuffer_t* cb, int headerPos, MultiWrapper* p)
{
	int bytes = circ_buff_peak(cb, headerPos + 1);
	int numBytesInPackedString = headerPos + bytes + MULTI_NUM_OVERHEAD_BYTES_FRAME;

	MultiInfoByte mInfo = decodeMultiInfo(cb, headerPos);

	//if we just received the first frame of a new packet, we can throw out all the old info,
	//the previous multi packet was either completed and parsed, or is incomplete and useless anyways
	if (mInfo.frameId == 0)
		resetToPacketId(p, mInfo.packetId);

	//this should always be true except in some strange case with out of order frames
	if (mInfo.packetId == p->currentMultiPacket)
	{
		// Note that in this implementation we parse each frame as we receive it and we require them to be received in order
		circ_buff_copyToUnpacked(cb, headerPos, bytes, p);

		//set the multi's map to record we received this frame
		p->frameMap |= (1 << mInfo.frameId);
		if (mInfo.frameId == mInfo.lastFrameInPacket)
			p->isMultiComplete = 1;

	}

	return numBytesInPackedString;

}

void circ_buff_copyToUnpacked(circularBuffer_t* cb, int headerPos, int bytes, MultiWrapper* p)
{
	int start = (cb->head + headerPos + MULTI_DATA_OFFSET) % CB_BUF_LEN;

	if(start + bytes > CB_BUF_LEN)
	{
		uint16_t bytesUntilEnd = CB_BUF_LEN - start;
		p->unpackedIdx += copyEscapedString(p->unpacked, cb->bytes + start, bytesUntilEnd);
		p->unpackedIdx += copyEscapedString(p->unpacked + p->unpackedIdx, cb->bytes, bytes - bytesUntilEnd);
	}
	else
	{
		p->unpackedIdx += copyEscapedString(p->unpacked, cb->bytes + start, bytes);
	}
}

static inline MultiInfoByte decodeMultiInfo(circularBuffer_t* cb, int headerPos)
{
	uint8_t multiInfo = circ_buff_peak(cb, headerPos + 2);

	MultiInfoByte result = {
			.packetId = MULTI_PACKETID(multiInfo),
			.frameId = MULTI_THIS_FRAMEID(multiInfo),
			.lastFrameInPacket = MULTI_LAST_FRAMEID(multiInfo)
		};

	return result;

}

unsigned copyEscapedString(uint8_t *dst, uint8_t *src, unsigned nb)
{
	unsigned i = 0, k, lastWasEscape = 0;
	for(k = 0; k < nb; k++)
	{
		if(src[k] == MULTI_ESC && (!lastWasEscape))
		{
			lastWasEscape = 1;
		}
		else
		{
			lastWasEscape = 0;
			dst[i++] = src[k];
		}
	}
	return i;
}
