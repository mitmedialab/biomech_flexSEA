#ifdef __cplusplus
extern "C" {
#endif

#include "flexsea-system_test-all.h"

//****************************************************************************
// Unit test(s)
//****************************************************************************

/*
void test_tx_cmd_tools_comm_test_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_tools_comm_test_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
						   &test_len);

	TEST_ASSERT_EQUAL(CMD_BATT, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_tools_comm_test_w(void)
{
	uint16_t expectedLen = 0;
	prepTxCmdTest();

	tx_cmd_exp_batt_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
						   &test_len);

	TEST_ASSERT_EQUAL(CMD_BATT, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);

	expectedLen = 8;

	TEST_ASSERT_EQUAL_MESSAGE(expectedLen, test_len, \
							  "Data length (manual input)");
}
*/

//****************************************************************************
// Public interface:
//****************************************************************************

void test_flexsea_cmd_tools(void)
{
	UNITY_BEGIN();

	//Add all your unit tests to this list:
	//RUN_TEST(test_tx_cmd_tools_comm_test_r);
	//RUN_TEST(test_tx_cmd_tools_comm_test_w);
	//...

	UNITY_END();
}

#ifdef __cplusplus
}
#endif
