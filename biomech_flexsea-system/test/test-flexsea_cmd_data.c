#ifdef __cplusplus
extern "C" {
#endif

#include "flexsea-system_test-all.h"

//****************************************************************************
// Unit test(s)
//****************************************************************************

void test_tx_cmd_data_read_all_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_data_read_all_r(test_tmpPayload, &test_cmdCode, &test_cmdType, \
						   &test_len);

	TEST_ASSERT_EQUAL(CMD_READ_ALL, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_data_read_all_w(void)
{
	uint16_t expectedLen = 0;
	prepTxCmdTest();

	tx_cmd_data_read_all_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
						   &test_len);

	TEST_ASSERT_EQUAL(CMD_READ_ALL, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);

	//Length depends on board type:

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
	expectedLen = 29;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

	#ifdef BOARD_TYPE_FLEXSEA_MANAGE
	expectedLen = 12;
	#endif  //BOARD_TYPE_FLEXSEA_MANAGE

	#ifdef BOARD_TYPE_FLEXSEA_GOSSIP
	expectedLen = 18;
	#endif  //BOARD_TYPE_FLEXSEA_GOSSIP

	#ifdef BOARD_TYPE_FLEXSEA_STRAIN_AMP
	expectedLen = 9;
	#endif 	//BOARD_TYPE_FLEXSEA_STRAIN_AMP

	TEST_ASSERT_EQUAL_MESSAGE(expectedLen, test_len, \
							  "Data length (manual input)");
}

void test_tx_cmd_data_user_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_data_user_r(test_tmpPayload, &test_cmdCode, &test_cmdType, \
						   &test_len);

	TEST_ASSERT_EQUAL(CMD_USER_DATA, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_data_user_w(void)
{
	uint16_t expectedLen = 0;
	uint8_t selectW = 2;
	prepTxCmdTest();

	tx_cmd_data_user_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
						   &test_len, selectW);

	TEST_ASSERT_EQUAL(CMD_USER_DATA, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(selectW, test_tmpPayload[0]);

	//Length depends on board type:

	#ifdef BOARD_TYPE_FLEXSEA_PLAN
		expectedLen = 5;
	#else
		expectedLen = 16;
	#endif

	TEST_ASSERT_EQUAL_MESSAGE(expectedLen, test_len, \
							  "Data length (manual input)");
}

//****************************************************************************
// Public interface:
//****************************************************************************

void test_flexsea_cmd_data(void)
{
	//Add all your unit tests to this list:
	RUN_TEST(test_tx_cmd_data_read_all_r);
	RUN_TEST(test_tx_cmd_data_read_all_w);
	RUN_TEST(test_tx_cmd_data_user_r);
	RUN_TEST(test_tx_cmd_data_user_w);
	//...

	fflush(stdout);
}

#ifdef __cplusplus
}
#endif
