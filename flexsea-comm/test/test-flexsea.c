#ifdef __cplusplus
extern "C" {
#endif

#include "flexsea-comm_test-all.h"

//Definitions and variables used by some/all tests:
//...

//Functions under test: SPLIT_16() & REBUILD_UINT16()
void test_SPLIT_REBUILD_16(void)
{
	uint16_t testVar1[4] = {0, 1234, 32768, 65535};
	int16_t testVar2[4] = {-32768, 0, 1234, 32767};
	uint8_t myBuf[2] = {0,0};
	int i = 0;
	uint16_t index = 0;

	//Test with uint16:
	for(i = 0; i < 4; i++)
	{
		myBuf[0] = 0;
		myBuf[1] = 0;
		index = 0;
		SPLIT_16(testVar1[i], myBuf, &index);
		index = 0;

		TEST_ASSERT_EQUAL(testVar1[i], REBUILD_UINT16(myBuf, &index));
	}

	//Test with int16:
	for(i = 0; i < 4; i++)
	{
		myBuf[0] = 0;
		myBuf[1] = 0;
		index = 0;
		SPLIT_16((uint16_t)testVar2[i], myBuf, &index);
		index = 0;

		TEST_ASSERT_EQUAL(testVar2[i], (int16_t) REBUILD_UINT16(myBuf, &index));
	}
}

//Functions under test: SPLIT_32() & REBUILD_UINT32()
void test_SPLIT_REBUILD_32(void)
{
	uint32_t testVar1[4] = {0, 234567, 2147483647, 4294967295};
	int32_t testVar2[4] = {-2147483648, 0, 123456, 2147483647};
	uint8_t myBuf[4] = {0,0,0,0};
	int i = 0;
	uint16_t index = 0;

	//Test with uint32:
	for(i = 0; i < 4; i++)
	{
		myBuf[0] = 0;
		myBuf[1] = 0;
		myBuf[2] = 0;
		myBuf[3] = 0;
		index = 0;
		SPLIT_32(testVar1[i], myBuf, &index);
		index = 0;

		TEST_ASSERT_EQUAL(testVar1[i], REBUILD_UINT32(myBuf, &index));
	}

	//Test with int32:
	for(i = 0; i < 4; i++)
	{
		myBuf[0] = 0;
		myBuf[1] = 0;
		myBuf[2] = 0;
		myBuf[3] = 0;
		index = 0;
		SPLIT_32((uint32_t)testVar2[i], myBuf, &index);
		index = 0;

		TEST_ASSERT_EQUAL(testVar2[i], (int32_t) REBUILD_UINT32(myBuf, &index));
	}
}

void test_flexsea(void)
{
	RUN_TEST(test_SPLIT_REBUILD_16);
	RUN_TEST(test_SPLIT_REBUILD_32);

	fflush(stdout);
}

#ifdef __cplusplus
}
#endif
