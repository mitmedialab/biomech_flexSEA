#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "../inc/flexsea.h"
#include "flexsea-comm_test-all.h"
#include <flexsea_comm.h>
#include <flexsea_sys_def.h>

#include <time.h>
#include <stdlib.h>

//Definitions and variables used by some/all tests:
uint8_t fakePayload[PAYLOAD_BUF_LEN];
uint8_t fakeCommStr[COMM_STR_BUF_LEN];
uint8_t fakeCommStrArray0[COMM_STR_BUF_LEN];
uint8_t fakeCommStrArray1[COMM_STR_BUF_LEN];
uint8_t fakeCommStrArray2[COMM_STR_BUF_LEN];
uint8_t packed[COMM_STR_BUF_LEN];
uint8_t rx_cmd_test[PACKAGED_PAYLOAD_LEN];
uint8_t retVal = 0;
int8_t retVal2 = 0;

void resetCommStats(void)
{
	//All stats to 0:
	commSpy1.bytes = 0;
	commSpy1.total_bytes = 0;
	commSpy1.checksum = 0;
	commSpy1.retVal = 0;
	commSpy1.escapes = 0;
}

void test_comm_gen_str_simple(void)
{
	int i = 0;
	uint8_t checksum = 0;

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	resetCommStats();

	//Gen comm str:
	retVal = comm_gen_str(fakePayload, fakeCommStr, 4);

	//Recalculate the checksum here:
	for(i = 0; i < commSpy1.total_bytes; i++)
	{
		checksum += fakeCommStr[2+i];
	}

	//Tests:

	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.retVal, retVal, "retval");
	TEST_ASSERT_EQUAL(7, retVal);
	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.checksum, checksum, "checksum");

	for(i = retVal+1; i < COMM_STR_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL_MESSAGE(0xAA, fakeCommStr[i], "filler");
	}
}

//Too long because I'm feeding it 48 bytes:
void test_comm_gen_str_tooLong1(void)
{
	int i = 0;
	uint8_t checksum = 0;

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	resetCommStats();

	//Gen comm str, with 48 bytes:
	retVal = comm_gen_str(fakePayload, fakeCommStr, 48);

	//Recalculate the checksum here:
	for(i = 0; i < commSpy1.total_bytes; i++)
	{
		checksum += fakeCommStr[2+i];
	}

	//Tests:

	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.retVal, retVal, "retval");
	TEST_ASSERT_EQUAL(0, retVal);

	for(i = 0; i < COMM_STR_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL_MESSAGE(0, fakeCommStr[i], "filler");
	}
}

//Too long because I'm feeding it chars that need escaping:
void test_comm_gen_str_tooLong2(void)
{
	int i = 0;
	uint8_t checksum = 0;

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	//24 bytes that will need escaping:
	for(i = 0; i < 24; i++)
	{
		fakePayload[P_DATA1+i] = HEADER;
	}

	resetCommStats();

	//Gen comm str, with 48 bytes:
	retVal = comm_gen_str(fakePayload, fakeCommStr, 28);

	//Recalculate the checksum here:
	for(i = 0; i < commSpy1.total_bytes; i++)
	{
		checksum += fakeCommStr[2+i];
	}

	//Tests:

	TEST_ASSERT_EQUAL_MESSAGE(commSpy1.retVal, retVal, "retval");
	TEST_ASSERT_EQUAL(0, retVal);
	TEST_ASSERT_EQUAL_MESSAGE(24, commSpy1.escapes, "Escape characters");

	for(i = 0; i < COMM_STR_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL_MESSAGE(0, fakeCommStr[i], "filler");
	}
}

//
void test_unpack_payload_1(void)
{
	//First, we generate a comm_str:
	//==============================

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	resetCommStats();
	retVal = comm_gen_str(fakePayload, fakeCommStr, 4);

	//Second, we parse it:
	//====================

	retVal2 = unpack_payload_test(fakeCommStr, packed, rx_cmd_test);

	//Tests:
	//======

	TEST_ASSERT_EQUAL_MESSAGE(1, retVal2, "Unpack payload: payloads found?");
}

void test_unpack_payload_2(void)
{
	//First, we generate a comm_str:
	//==============================

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	//We build a fake "Read All" command:
	fakePayload[P_XID] = FLEXSEA_PLAN_1;
	fakePayload[P_RID] = FLEXSEA_MANAGE_1;
	fakePayload[P_CMDS] = 1;
	fakePayload[P_DATA1] = CMD_R(CMD_READ_ALL);

	resetCommStats();
	retVal = comm_gen_str(fakePayload, fakeCommStr, 4);

	//We make copies for different tests:
	memcpy(fakeCommStrArray0, fakeCommStr, COMM_STR_BUF_LEN);
	memcpy(fakeCommStrArray1, fakeCommStr, COMM_STR_BUF_LEN);
	memcpy(fakeCommStrArray2, fakeCommStr, COMM_STR_BUF_LEN);

	//We keed 0 intact, but we modify the others:
	fakeCommStrArray1[0] = 0;		//No header
	fakeCommStrArray2[1] = 123;		//Invalid # of bytes

	//Tests:
	//======

	retVal2 = unpack_payload_test(fakeCommStrArray0, packed, rx_cmd_test);
	TEST_ASSERT_EQUAL_INT8_MESSAGE(1, retVal2, "Unpack payload: payloads found?");

	retVal2 = unpack_payload_test(fakeCommStrArray1, packed, rx_cmd_test);
	TEST_ASSERT_EQUAL_INT8_MESSAGE(UNPACK_ERR_HEADER, retVal2, "Missing header");

	retVal2 = unpack_payload_test(fakeCommStrArray2, packed, rx_cmd_test);
	TEST_ASSERT_EQUAL_INT8_MESSAGE(UNPACK_ERR_LEN, retVal2, "Wrong length / too long");
}

//Helper function for test_circ_unpack
int testCircPackUnpack(circularBuffer_t* cb, uint8_t* comm_str, uint8_t* packed, uint8_t* unpacked,
						uint8_t preoffset, uint8_t postOffset)
{
	int i;
	uint8_t value;
	for(i=0; i < preoffset; i++)
	{
		value = rand();
		circ_buff_write(cb, &value, 1);
	}

	circ_buff_write(cb, comm_str, COMM_STR_BUF_LEN);

	for(i=0; i < postOffset; i++)
	{
		value = rand();
		circ_buff_write(cb, &value, 1);
	}

	return unpack_payload_cb(cb, packed, unpacked);
}

int fillFakeReadAll(uint8_t* fakePayload)
{
	int index = 0;
	fakePayload[index++] = FLEXSEA_PLAN_1;
	fakePayload[index++] = FLEXSEA_MANAGE_1;
	fakePayload[index++] = 1;
	fakePayload[index++] = CMD_R(CMD_READ_ALL);

	uint8_t value;
	while(index < 44)
	{
		do { value = rand(); }
		while(value == HEADER || value == FOOTER || value == ESCAPE);

		fakePayload[index++] = value;
	}

	return index;
}

void test_circ_unpack(void)
{
	//First, we generate a comm_str:
	//==============================
	srand(time(NULL));

	//Empty strings:
	memset(fakePayload, 0, PAYLOAD_BUF_LEN);
	memset(fakeCommStr, 0, COMM_STR_BUF_LEN);

	circularBuffer_t cb;
	circ_buff_init(&cb);

	//Positive tests, it should find a comm str
	int preOffsetMax;
	uint8_t tPacked[RX_BUF_LEN];
	uint8_t tUnpacked[RX_BUF_LEN];
	int i, j, preoffset, postoffset, expected, result, lengthUnpacked, comm_str_last_ind;
	for(i = 0; i < 100; i++)
	{
		lengthUnpacked = fillFakeReadAll(fakePayload);
		resetCommStats();
		comm_str_last_ind = comm_gen_str(fakePayload, fakeCommStr, lengthUnpacked);
		preOffsetMax = CB_BUF_LEN - comm_str_last_ind - 1;
		preoffset = rand() % preOffsetMax;
		postoffset = CB_BUF_LEN - COMM_STR_BUF_LEN - preoffset;
		result = testCircPackUnpack(&cb, fakeCommStr, tPacked, tUnpacked, preoffset, postoffset);
		expected = preoffset + comm_str_last_ind + 1;
		TEST_ASSERT_EQUAL(expected, result);
		for(j = 0; j < lengthUnpacked; j++)
		{
			TEST_ASSERT_EQUAL_MESSAGE(fakePayload[j], tUnpacked[j], "Positive case, unpacked strings mismatch");
		}
		for(j = 0; j <= comm_str_last_ind; j++)
		{
			TEST_ASSERT_EQUAL_MESSAGE(fakeCommStr[j], tPacked[j], "Positive case, packed strings mismatch");
		}
	}

	//Negative tests, invalid comm str
	//numbytes is wrong
	fakeCommStr[1] = fakeCommStr[1] * 2;
	expected = 0;
	for(i = 0; i < 10; i++)
	{
		preoffset = rand() % preOffsetMax;
		postoffset = CB_BUF_LEN - COMM_STR_BUF_LEN - preoffset;
		result = testCircPackUnpack(&cb, fakeCommStr, tPacked, tUnpacked, preoffset, postoffset);
		TEST_ASSERT_EQUAL(expected, result);
	}
	fakeCommStr[1] = fakeCommStr[1] / 2;
	//footer is wrong
	fakeCommStr[comm_str_last_ind] = 0;
	expected = 0;
	for(i = 0; i < 10; i++)
	{
		preoffset = rand() % preOffsetMax;
		postoffset = CB_BUF_LEN - COMM_STR_BUF_LEN - preoffset;
		result = testCircPackUnpack(&cb, fakeCommStr, tPacked, tUnpacked, preoffset, postoffset);
		TEST_ASSERT_EQUAL(expected, result);
	}
	//checksum is wrong
	fakeCommStr[comm_str_last_ind] = FOOTER;
	fakeCommStr[comm_str_last_ind-1] *= 2;
	expected = 0;
	for(i = 0; i < 10; i++)
	{
		preoffset = rand() % preOffsetMax;
		postoffset = CB_BUF_LEN - COMM_STR_BUF_LEN - preoffset;
		result = testCircPackUnpack(&cb, fakeCommStr, tPacked, tUnpacked, preoffset, postoffset);
		TEST_ASSERT_EQUAL(expected, result);
	}
}

void test_flexsea_comm(void)
{
	RUN_TEST(test_comm_gen_str_simple);
	RUN_TEST(test_comm_gen_str_tooLong1);
	RUN_TEST(test_comm_gen_str_tooLong2);
	RUN_TEST(test_unpack_payload_1);
	RUN_TEST(test_unpack_payload_2);
	RUN_TEST(test_circ_unpack);

	fflush(stdout);
}

#ifdef __cplusplus
}
#endif
