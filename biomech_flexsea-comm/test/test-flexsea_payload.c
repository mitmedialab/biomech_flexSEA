#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <flexsea_payload.h>
#include "flexsea-comm_test-all.h"
#include <flexsea_sys_def.h>

//Definitions and variables used by some/all tests:
//...

void test_payload_parse_str(void)
{
	//ToDo
}

void test_prepare_empty_payload(void)
{
	uint8_t testBuffer[48];
	uint8_t xid = FLEXSEA_PLAN_1, rid = FLEXSEA_EXECUTE_1;
	int i = 0;

	memset(testBuffer, 1, sizeof(testBuffer));
	prepare_empty_payload(xid, rid, testBuffer, 48);

	TEST_ASSERT_EQUAL(xid, testBuffer[P_XID]);
	TEST_ASSERT_EQUAL(rid, testBuffer[P_RID]);

	for(i = P_RID+1; i < 48; i++)
	{
		TEST_ASSERT_EQUAL(0, testBuffer[i]);
	}
}

void test_sent_from_a_slave(void)
{
	uint8_t testBuffer[48];
	memset(testBuffer, 0, sizeof(testBuffer));

	//Master => Master
	testBuffer[P_XID] = FLEXSEA_PLAN_1;
	testBuffer[P_RID] = FLEXSEA_PLAN_1;
	TEST_ASSERT_EQUAL(0, sent_from_a_slave(testBuffer));

	//Master => Slave
	testBuffer[P_XID] = FLEXSEA_PLAN_1;
	testBuffer[P_RID] = FLEXSEA_EXECUTE_1;
	TEST_ASSERT_EQUAL(0, sent_from_a_slave(testBuffer));

	//Slave => Master
	testBuffer[P_XID] = FLEXSEA_EXECUTE_1;
	testBuffer[P_RID] = FLEXSEA_PLAN_1;
	TEST_ASSERT_EQUAL(1, sent_from_a_slave(testBuffer));
}

void test_packetType(void)
{
	uint8_t testBuffer[48];
	uint8_t xid = 0, rid = 0;

	//Master Writing to Slave:
	xid = FLEXSEA_PLAN_1;
	rid = FLEXSEA_EXECUTE_1;
	prepare_empty_payload(xid, rid, testBuffer, 48);
	testBuffer[P_CMD1] = CMD_W(CMD_TEST);
	TEST_ASSERT_EQUAL_MESSAGE(RX_PTYPE_WRITE, packetType(testBuffer), "Master Write");
	//Requesting a Read instead:
	testBuffer[P_CMD1] = CMD_R(CMD_TEST);
	TEST_ASSERT_EQUAL_MESSAGE(RX_PTYPE_READ, packetType(testBuffer), "Master read");

	//Slave Writing to Master:
	xid = FLEXSEA_EXECUTE_1;
	rid = FLEXSEA_PLAN_1;
	prepare_empty_payload(xid, rid, testBuffer, 48);
	testBuffer[P_CMD1] = CMD_W(CMD_TEST);
	TEST_ASSERT_EQUAL_MESSAGE(RX_PTYPE_REPLY, packetType(testBuffer), "Slave Write (Reply)");
	//Requesting a Read instead:
	testBuffer[P_CMD1] = CMD_R(CMD_TEST);
	TEST_ASSERT_EQUAL_MESSAGE(RX_PTYPE_INVALID, packetType(testBuffer), "Slave read (invalid)");
}

void test_flexsea_payload(void)
{
	//RUN_TEST(test_payload_parse_str);
	RUN_TEST(test_prepare_empty_payload);
	RUN_TEST(test_sent_from_a_slave);
	RUN_TEST(test_packetType);

	fflush(stdout);
}

#ifdef __cplusplus
}
#endif
