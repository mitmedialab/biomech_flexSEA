#ifdef __cplusplus
extern "C" {
#endif

#include <flexsea_comm.h>
#include <flexsea_system.h>
#include "flexsea-system_test-all.h"

//Definitions and variables used by some/all tests:

#define TEST_PL_LEN		4

//Function under test: tx_cmd()
//Test goal: make sure that it adds the proper fields to the data it receives
//and that it returns the right number of bytes
void test_tx_cmd(void)
{
	uint8_t cmdCode = CMD_TEST, cmdType = CMD_WRITE;
	uint16_t len = TEST_PL_LEN, numb = 0;
	uint8_t myPayloadData[TEST_PL_LEN] = {0xAA, 0xBB, 0xCC, 0xDD};
	uint8_t outputBuf[48];

	numb = tx_cmd(myPayloadData, cmdCode, cmdType, len, FLEXSEA_EXECUTE_1, outputBuf);

	TEST_ASSERT_EQUAL(1, outputBuf[P_CMDS]);
	TEST_ASSERT_EQUAL(CMD_W(cmdCode), outputBuf[P_CMD1]);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(myPayloadData, &outputBuf[P_DATA1], TEST_PL_LEN);
	TEST_ASSERT_EQUAL((P_DATA1 + TEST_PL_LEN), numb);
}

void test_flexsea_system(void)
{
	RUN_TEST(test_tx_cmd);

	fflush(stdout);
}

#ifdef __cplusplus
}
#endif
