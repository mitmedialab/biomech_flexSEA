#ifdef __cplusplus
extern "C" {
#endif

#include "unity.h"
#include "../inc/flexsea_comm.h"
#include "flexsea-comm_test-all.h"

//****************************************************************************
// Variables used for these sets of tests:
//****************************************************************************

//****************************************************************************
// Helper function(s):
//****************************************************************************


//****************************************************************************
// Main test function:
//****************************************************************************

//Call this function to test the 'flexsea-comm' stack:
int flexsea_comm_test(void)
{
	UNITY_BEGIN();

	//One call per file here:
	test_flexsea();
	test_flexsea_comm();
	test_flexsea_payload();
	test_flexsea_buffers();

	return UNITY_END();
}

#ifdef __cplusplus
}
#endif
