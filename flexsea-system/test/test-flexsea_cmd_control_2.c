#ifdef __cplusplus
extern "C" {
#endif

#include "../../flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include "flexsea-system_test-all.h"

//****************************************************************************
// Unit test(s)
//****************************************************************************

//Current Gain:
//=============

void test_tx_cmd_ctrl_i_g_w(void)
{
	int i = 0;
	int16_t g1 = -31234, g2 = 100, g3 = 24567;
	int16_t r_g1 = 0, r_g2 = 0, r_g3 = 0;

	uint16_t index = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_i_g_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					&test_len, g1, g2, g3);

	TEST_ASSERT_EQUAL(CMD_CTRL_I_G, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(6, test_len);

	r_g1 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);
	r_g2 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);
	r_g3 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);

	TEST_ASSERT_EQUAL(g1, r_g1);
	TEST_ASSERT_EQUAL(g2, r_g2);
	TEST_ASSERT_EQUAL(g3, r_g3);

	for(i = 6; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_ctrl_i_g_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_i_g_r(test_tmpPayload, &test_cmdCode, &test_cmdType, &test_len);

	TEST_ASSERT_EQUAL(CMD_CTRL_I_G, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

//ToDo: RX functions

//Position Gain:
//=============

void test_tx_cmd_ctrl_p_g_w(void)
{
	int i = 0;
	int16_t g1 = -31234, g2 = 100, g3 = 24567;
	int16_t r_g1 = 0, r_g2 = 0, r_g3 = 0;

	uint16_t index = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_p_g_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					&test_len, g1, g2, g3);

	TEST_ASSERT_EQUAL(CMD_CTRL_P_G, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(6, test_len);

	r_g1 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);
	r_g2 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);
	r_g3 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);

	TEST_ASSERT_EQUAL(g1, r_g1);
	TEST_ASSERT_EQUAL(g2, r_g2);
	TEST_ASSERT_EQUAL(g3, r_g3);

	for(i = 6; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_ctrl_p_g_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_p_g_r(test_tmpPayload, &test_cmdCode, &test_cmdType, &test_len);

	TEST_ASSERT_EQUAL(CMD_CTRL_P_G, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_READ, test_cmdType);
	TEST_ASSERT_EQUAL(0, test_len);

	for(i = 0; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

//ToDo: RX functions

//Impedance Gain:
//===============

void test_tx_cmd_ctrl_z_g_w(void)
{
	int i = 0;
	int16_t g1 = -31234, g2 = 100, g3 = 24567;
	int16_t r_g1 = 0, r_g2 = 0, r_g3 = 0;

	uint16_t index = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_z_g_w(test_tmpPayload, &test_cmdCode, &test_cmdType, \
					&test_len, g1, g2, g3);

	TEST_ASSERT_EQUAL(CMD_CTRL_Z_G, test_cmdCode);
	TEST_ASSERT_EQUAL(CMD_WRITE, test_cmdType);
	TEST_ASSERT_EQUAL(6, test_len);

	r_g1 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);
	r_g2 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);
	r_g3 = (int16_t) REBUILD_UINT16(test_tmpPayload, &index);

	TEST_ASSERT_EQUAL(g1, r_g1);
	TEST_ASSERT_EQUAL(g2, r_g2);
	TEST_ASSERT_EQUAL(g3, r_g3);

	for(i = 6; i < PAYLOAD_BUF_LEN; i++)
	{
		TEST_ASSERT_EQUAL(FILLER, test_tmpPayload[i]);
	}
}

void test_tx_cmd_ctrl_z_g_r(void)
{
	int i = 0;
	prepTxCmdTest();

	tx_cmd_ctrl_z_g_r(test_tmpPayload, &test_cmdCode, &test_cmdType, &test_len);

	TEST_ASSERT_EQUAL(CMD_CTRL_Z_G, test_cmdCode);
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

void test_flexsea_cmd_control_2(void)
{
	//Add all your unit tests to this list:

	RUN_TEST(test_tx_cmd_ctrl_i_g_w);
	RUN_TEST(test_tx_cmd_ctrl_i_g_r);
	//RUN_TEST(test_rx_cmd_ctrl_i_w);
	//RUN_TEST(test_rx_cmd_ctrl_i_rw);
	//RUN_TEST(test_rx_cmd_ctrl_i_rr);

	RUN_TEST(test_tx_cmd_ctrl_p_g_w);
	RUN_TEST(test_tx_cmd_ctrl_p_g_r);
	//RUN_TEST(test_rx_cmd_ctrl_i_w);
	//RUN_TEST(test_rx_cmd_ctrl_i_rw);
	//RUN_TEST(test_rx_cmd_ctrl_i_rr);

	RUN_TEST(test_tx_cmd_ctrl_z_g_w);
	RUN_TEST(test_tx_cmd_ctrl_z_g_r);
	//RUN_TEST(test_rx_cmd_ctrl_i_w);
	//RUN_TEST(test_rx_cmd_ctrl_i_rw);
	//RUN_TEST(test_rx_cmd_ctrl_i_rr);

	//...

	fflush(stdout);
}

#ifdef __cplusplus
}
#endif
