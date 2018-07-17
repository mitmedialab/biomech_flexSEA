#ifdef __cplusplus
extern "C" {
#endif

#include "../../biomech_flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include "flexsea-system_test-all.h"

//****************************************************************************
// Unit test(s)
//****************************************************************************

//Control Mode:
//=============

void test_tx_cmd_ctrl_mode_w(void)
{
	int i = 0;
	uint8_t controller = 5;
	prepTxCmdTest();

	tx_cmd_ctrl_mode_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					   &test_len, controller);

	TEST_ASSERT_EQUAL(CMD_CTRL_MODE, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(1, test_len);

	TEST_ASSERT_EQUAL(controller, test_tmpPayload[0]);
	for(i = 1; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_ctrl_mode_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_mode_r(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					   &test_len);

	TEST_ASSERT_EQUAL(CMD_CTRL_MODE, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

//TODO:
void test_rx_cmd_ctrl_mode_w(void)
{

}

void test_rx_cmd_ctrl_mode_rw(void)
{

}

void test_rx_cmd_ctrl_mode_rr(void)
{

}

//Current Setpoint:
//=================

void test_tx_cmd_ctrl_i_w(void)
{
	int i = 0;
	int16_t setP = 3456, res = 0;
	uint16_t index = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_i_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					&test_len, setP);

	TEST_ASSERT_EQUAL(CMD_CTRL_I, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(4, test_len);

	//Only testing the 2nd word:
	res = (int16_t) REBUILD_UINT16(&test_tmpPayload[2], &index);
	TEST_ASSERT_EQUAL(setP, res);

	for(i = 4; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_ctrl_i_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_i_r(test_tmpPayload, &test_cmdCode, &test_cmdType, &test_len);

	TEST_ASSERT_EQUAL(CMD_CTRL_I, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

//TODO:
void test_rx_cmd_ctrl_i_w(void)
{

}

void test_rx_cmd_ctrl_i_rw(void)
{

}

void test_rx_cmd_ctrl_i_rr(void)
{

}

//Control Open Setpoint:
//======================

void test_tx_cmd_ctrl_o_w(void)
{
	int i = 0;
	int16_t setP = 3456, res = 0;
	uint16_t index = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_o_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					&test_len, setP);

	TEST_ASSERT_EQUAL(CMD_CTRL_O, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(2, test_len);

	res = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);
	TEST_ASSERT_EQUAL(setP, res);

	for(i = 2; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_ctrl_o_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_o_r(test_tmpPayload, &test_cmdCode, &test_cmdType, &test_len);

	TEST_ASSERT_EQUAL(CMD_CTRL_O, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

//ToDo: RX functions

//Control Position Setpoint:
//==========================

void test_tx_cmd_ctrl_p_w(void)
{
	int i = 0;
	int32_t pos = 3456, posi = -1234567, posf = 2345678, spdm = 70000, acc = 1000000;
	int32_t r_pos = 0, r_posi = 0, r_posf = 0, r_spdm = 0, r_acc = 0;

	uint16_t index = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_p_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					&test_len, pos, posi, posf, spdm, acc);

	TEST_ASSERT_EQUAL(CMD_CTRL_P, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(20, test_len);

	r_pos = (int32_t) REBUILD_UINT32(test_tmpPayload, &index);
	r_posi = (int32_t) REBUILD_UINT32(test_tmpPayload, &index);
	r_posf = (int32_t) REBUILD_UINT32(test_tmpPayload, &index);
	r_spdm = (int32_t) REBUILD_UINT32(test_tmpPayload, &index);
	r_acc = (int32_t) REBUILD_UINT32(test_tmpPayload, &index);

	TEST_ASSERT_EQUAL(pos, r_pos);
	TEST_ASSERT_EQUAL(posi, r_posi);
	TEST_ASSERT_EQUAL(posf, r_posf);
	TEST_ASSERT_EQUAL(spdm, r_spdm);
	TEST_ASSERT_EQUAL(acc, r_acc);

	for(i = 20; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_ctrl_p_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_p_r(test_tmpPayload, &test_cmdCode, &test_cmdType, &test_len);

	TEST_ASSERT_EQUAL(CMD_CTRL_P, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

//ToDo: RX functions

//****************************************************************************
// Public interface:
//****************************************************************************

void test_flexsea_cmd_control_1(void)
{
	//Add all your unit tests to this list:

	RUN_TEST(test_tx_cmd_ctrl_mode_w);
	RUN_TEST(test_tx_cmd_ctrl_mode_r);
	//RUN_TEST(test_rx_cmd_ctrl_mode_w);
	//RUN_TEST(test_rx_cmd_ctrl_mode_rw);
	//RUN_TEST(test_rx_cmd_ctrl_mode_rr);

	RUN_TEST(test_tx_cmd_ctrl_i_w);
	RUN_TEST(test_tx_cmd_ctrl_i_r);
	//RUN_TEST(test_rx_cmd_ctrl_i_w);
	//RUN_TEST(test_rx_cmd_ctrl_i_rw);
	//RUN_TEST(test_rx_cmd_ctrl_i_rr);

	RUN_TEST(test_tx_cmd_ctrl_o_w);
	RUN_TEST(test_tx_cmd_ctrl_o_r);
	//RUN_TEST(test_rx_cmd_ctrl_i_w);
	//RUN_TEST(test_rx_cmd_ctrl_i_rw);
	//RUN_TEST(test_rx_cmd_ctrl_i_rr);

	RUN_TEST(test_tx_cmd_ctrl_p_w);
	RUN_TEST(test_tx_cmd_ctrl_p_r);
	//RUN_TEST(test_rx_cmd_ctrl_i_w);
	//RUN_TEST(test_rx_cmd_ctrl_i_rw);
	//RUN_TEST(test_rx_cmd_ctrl_i_rr);

	//...

	fflush(stdout);
}

#ifdef __cplusplus
}
#endif
