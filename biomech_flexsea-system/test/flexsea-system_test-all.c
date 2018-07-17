#ifdef __cplusplus
extern "C" {
#endif

#include "unity.h"
#include "flexsea-system_test-all.h"
#include <string.h>

//****************************************************************************
// Variables used for these sets of tests:
//****************************************************************************

uint8_t test_tmpPayload[PAYLOAD_BUF_LEN];
uint8_t test_transferBuf[COMM_STR_BUF_LEN];
uint8_t test_cmdCode = 0, test_cmdType = 0;
uint16_t test_len = 0;

//****************************************************************************
// Helper function(s):
//****************************************************************************

void prepTxCmdTest(void)
{
	memset(test_tmpPayload, FILLER, PAYLOAD_BUF_LEN);
	memset(test_transferBuf, FILLER, COMM_STR_BUF_LEN);
	test_cmdCode = 0;
	test_cmdType = 0;
	test_len = 0;
}

//****************************************************************************
// Main test function:
//****************************************************************************

//Call this function to test the 'flexsea-system' stack:
void flexsea_system_test(void)
{
	UNITY_BEGIN();

	//One call per file here:
	test_flexsea_system();
	test_flexsea_cmd_control_1();
	test_flexsea_cmd_control_2();
	test_flexsea_cmd_data();
	test_flexsea_cmd_external();
	test_flexsea_cmd_sensors();

	UNITY_END();
}

#ifdef __cplusplus
}
#endif
