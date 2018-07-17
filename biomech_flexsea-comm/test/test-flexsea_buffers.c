#ifdef __cplusplus
extern "C" {
#endif
#include <flexsea_buffers.h>
#include "flexsea-comm_test-all.h"
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <stdlib.h>

//Definitions and variables used by some/all tests:
//...

#define TEST_LEN	26
void test_update_rx_buf_byte_1(void)
{
	int i;
	uint8_t inputStr[TEST_LEN];
	for(i = 0; i < TEST_LEN; i++)
	{
		inputStr[i] = 'a'+i;
	}
	//printf("Test val: %s.\n", inputStr);
	//fflush(stdout);

	//Start with all 0
	memset(rx_buf_1, 0, RX_BUF_LEN);

	//Fill buffer:
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}

	TEST_ASSERT_EQUAL_STRING_LEN(inputStr, rx_buf_1, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	//Now we keep filling it:

	//Fill buffer:
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}
	for(i = 0; i < TEST_LEN; i++)
	{
		update_rx_buf_byte_1(inputStr[i]);
	}

	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	//Final test:
	for(i = 0; i < RX_BUF_LEN - 1; i++)
	{
		if(rx_buf_1[i] != 'z')
			TEST_ASSERT_EQUAL_UINT8(rx_buf_1[i], rx_buf_1[i+1]-1);
		else
			TEST_ASSERT_EQUAL_UINT8('a', rx_buf_1[i+1]);
	}
}

void test_update_rx_buf_array_1(void)
{
	int i;
	uint8_t inputStr[TEST_LEN];
	for(i = 0; i < TEST_LEN; i++)
	{
		inputStr[i] = 'a'+i;
	}
	//printf("Test val: %s.\n", inputStr);
	//fflush(stdout);

	//Start with all 0
	memset(rx_buf_1, 0, RX_BUF_LEN);

	//Fill buffer:
	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(inputStr, rx_buf_1, TEST_LEN, "First update");


	//Now we keep filling it:

	//Fill buffer:
	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	update_rx_buf_array_1(inputStr, TEST_LEN);
	//printf("rx_buf_1: %s.\n", rx_buf_1);
	//fflush(stdout);

	//Final test:
	for(i = 0; i < RX_BUF_LEN - 1; i++)
	{
		if(rx_buf_1[i] != 'z')
			TEST_ASSERT_EQUAL_UINT8(rx_buf_1[i], rx_buf_1[i+1]-1);
		else
			TEST_ASSERT_EQUAL_UINT8('a', rx_buf_1[i+1]);
	}
}

void test_buffer_stack(void)
{
	int i;

	//Start with all 0
	memset(rx_buf_1, 0, RX_BUF_LEN);

	for(i = 0; i < 100; i++)
	{
		update_rx_buf_byte_1(i);
	}

	for(i = 0; i < 50; i++)
	{
		update_rx_buf_byte_1(i);
	}

	TEST_ASSERT_EQUAL(50, rx_buf_1[0]);
	TEST_ASSERT_EQUAL(49, rx_buf_1[99]);
}

void test_buffer_circular(void)
{
	int i;

	circularBuffer_t cb;
	circ_buff_init(&cb);
	uint8_t buf[CB_BUF_LEN];

	uint8_t v;
	for(i = 0; i < CB_BUF_LEN; i++)
	{
		v = i;
		circ_buff_write(&cb, &v, 1);
	}

	circ_buff_read(&cb, buf, CB_BUF_LEN);

    int fail = 0;
	for(i = 0; i < CB_BUF_LEN; i++)
    {
		v = i;
		fail = (v != buf[i]);
		if(fail)
			break;
    }
    TEST_ASSERT_FALSE_MESSAGE(fail, "READ/WRITE fail");

	for(i = 0; i < CB_BUF_LEN/2; i++)
	{
		v = i;
		circ_buff_write(&cb, &v, 1);
	}

	int expectedSize = CB_BUF_LEN;
	int actualSize = circ_buff_get_size(&cb);
	TEST_ASSERT_EQUAL(expectedSize, actualSize);


	TEST_ASSERT_EQUAL(0, circ_buff_read(&cb, buf, CB_BUF_LEN));
	int firstVal = CB_BUF_LEN/2;
    int lastVal = i-1;
    TEST_ASSERT_EQUAL(firstVal, buf[0]);
	TEST_ASSERT_EQUAL(lastVal, buf[CB_BUF_LEN-1]);

	circ_buff_init(&cb);
	//random tests
	srand(time(NULL));
	int lastSize = 0, lastHead = -1, lastTail = 0, firstTime = 1;
	expectedSize = 0;
	int expectedHead = -1, expectedTail = 0;
	for(i = 0; i < 5000; i++)
	{
		int shouldWrite = rand() % 2;
		int l = rand() % (CB_BUF_LEN + 10);
		int result;
		if(shouldWrite)
		{
			result = circ_buff_write(&cb, buf, l);
			if(l > CB_BUF_LEN)
			{ //expect
				TEST_ASSERT_EQUAL_MESSAGE(1, result, "Expected to fail on write of size larger than buffer");
			}
			else
			{
				expectedSize += l;
				expectedTail = (expectedTail + l) % CB_BUF_LEN;
				if(expectedSize > CB_BUF_LEN)
				{
					expectedHead = expectedTail;
					expectedSize = CB_BUF_LEN;
					TEST_ASSERT_EQUAL_MESSAGE(2, result, "Expected overwrite");
				}
				else
				{
					expectedHead = expectedHead < 0 ? 0 : expectedHead;
					if(result != 0)
					{
						printf("ASdf");
					}
					TEST_ASSERT_EQUAL_MESSAGE(0, result, "Expected write success");
				}
			}
		}
		else
		{
			result = circ_buff_move_head(&cb, l);
			if(l > CB_BUF_LEN)
			{ //expect to fail

				TEST_ASSERT_EQUAL_MESSAGE(2, result, "Expected move more than max error");
				expectedHead = -1;
				expectedTail = 0;
				expectedSize = 0;
			}
			else
			{
				if(lastSize - l >= 0)
				{
					if(0!=result)
						printf("Asdf");
					TEST_ASSERT_EQUAL_MESSAGE(0, result, "Expected move success");
				}
				else
					TEST_ASSERT_EQUAL_MESSAGE(1, result, "Expected move more than buffered error");

				expectedSize -= l;
				if(expectedSize < 1)
				{
					expectedHead = -1;
					expectedTail = 0;
					expectedSize = 0;

					if(expectedHead != cb.head || expectedTail != cb.tail || expectedSize != cb.size)
					{
						printf("AsdfSD");
					}
				}
				else
				{
					expectedHead = (expectedHead + l) % CB_BUF_LEN;
				}
			}
		}



		if(expectedHead != cb.head || expectedTail != cb.tail || expectedSize != cb.size)
		{
			printf("AsdfSD");
		}


		TEST_ASSERT_EQUAL_MESSAGE(expectedHead, cb.head, "Heads did not match");
		TEST_ASSERT_EQUAL_MESSAGE(expectedTail, cb.tail, "Tails did not match");
		TEST_ASSERT_EQUAL_MESSAGE(expectedSize, cb.size, "Sizes did not match");
		lastSize = expectedSize;
		lastHead = expectedHead;
		lastTail = expectedTail;
		firstTime = 0;
	}
}

void test_buffer_circular_alphabet(void)
{
	circularBuffer_t buf;
	circularBuffer_t* cb = &buf;
	circ_buff_init(cb);

	srand(time(NULL));
	const int ALPHABET_LEN = 31;

	int i;
	uint8_t alphabet[ALPHABET_LEN];
	for(i = 0; i < ALPHABET_LEN; i++)
	{
		alphabet[i] = rand() % 256;
	}

	for(i = 0; i < 1000; i++)
	{
		circ_buff_write(cb, alphabet, ALPHABET_LEN);
	}

	uint8_t outputBuf[CB_BUF_LEN];
	circ_buff_read(cb, outputBuf, CB_BUF_LEN);
	int j = ALPHABET_LEN-1;
	for(i = CB_BUF_LEN - 1; i >= 0; i--)
	{
		TEST_ASSERT_EQUAL(alphabet[j], outputBuf[i]);
		j--;
		if(j < 0)
			j+=ALPHABET_LEN;
	}
}

void test_buffer_circular_write_erase(void)
{
	circularBuffer_t buf;
	circularBuffer_t* cb = &buf;
	circ_buff_init(cb);

	srand(time(NULL));
	const int ALPHABET_LEN = 31;

	int i;
	uint8_t alphabet[ALPHABET_LEN];
	for(i = 0; i < ALPHABET_LEN; i++)
	{
		alphabet[i] = rand() % 256;
	}

	int shouldWrite, lengthToErase;
	int bufSize = circ_buff_get_size(cb);
	for(i = 0; i < 5000 || bufSize < (CB_BUF_LEN / 2); i++)
	{
		bufSize = circ_buff_get_size(cb);
		shouldWrite = rand() % 2;
		if(shouldWrite || bufSize < 2)
		{
			circ_buff_write(cb, alphabet, ALPHABET_LEN);
		}
		else
		{
			lengthToErase = rand() % (circ_buff_get_size(cb)/2);
			circ_buff_move_head(cb, lengthToErase);
		}
	}

	bufSize = circ_buff_get_size(cb);
	uint8_t outputBuf[CB_BUF_LEN];
	circ_buff_read(cb, outputBuf, bufSize);
	int j = ALPHABET_LEN-1;
	for(i = bufSize - 1; i >= 0; i--)
	{
		TEST_ASSERT_EQUAL(alphabet[j], outputBuf[i]);
		j--;
		if(j < 0)
			j+=ALPHABET_LEN;
	}
}

int getIndexOf(uint8_t value, uint8_t* buf, uint16_t start, uint16_t length)
{
	int i;
	for(i = start; i < length; i++)
	{
		if(buf[i] == value) return i;
	}
	return -1;
}

void test_buffer_circular_search(void)
{
	circularBuffer_t circBuf;
	circularBuffer_t* cb = &circBuf;
	circ_buff_init(cb);
	srand(time(NULL));

	uint8_t buf[CB_BUF_LEN];
	int i;
	for(i = 0; i < CB_BUF_LEN; i++)
		buf[i] = rand();

	//Perform a bunch of random writes to mess up the tail/head positions
	int length;
	for(i = 0; i < 100; i++)
	{
		length = rand() % CB_BUF_LEN;
		circ_buff_write(cb, buf, length);
	}

	//Write the actual values we will compare to
	circ_buff_write(cb, buf, CB_BUF_LEN);

	int expectedIndex, actualIndex, lastIndex;
	uint8_t value = 0, lastValue = 0;
	do {
		expectedIndex = 1;
		actualIndex = 1;
		lastIndex = -1;
		while(expectedIndex > 0)
		{
			expectedIndex = getIndexOf(value, buf, lastIndex+1, CB_BUF_LEN);
			actualIndex = circ_buff_search(cb, value, lastIndex+1);
			TEST_ASSERT_EQUAL(expectedIndex, actualIndex);
			lastIndex = expectedIndex;
		}

		lastValue = value;
		value++;
	} while(value > lastValue);
}

void test_buffer_circular_checksum(void)
{
	circularBuffer_t circBuf;
	circularBuffer_t* cb = &circBuf;
	circ_buff_init(cb);
	srand(time(NULL));

	uint8_t buf[CB_BUF_LEN];
	int i;
	for(i = 0; i < CB_BUF_LEN; i++)
		buf[i] = rand();


	int testI;
	for(testI = 0; testI < 100; testI++)
	{
		//Perform a bunch of random writes to mess up the tail/head positions
		int start, length;
		for(i = 0; i < 100; i++)
		{
			start = rand() % CB_BUF_LEN;
			length = rand() % (CB_BUF_LEN - start);
			circ_buff_write(cb, buf, length);
		}

		uint8_t *d = cb->bytes;
		uint8_t expectedSum = 0;
		for(i = 0; i < CB_BUF_LEN; i++)
		{
			expectedSum += d[i];
		}
		uint8_t actualSum = circ_buff_checksum(cb, 0, CB_BUF_LEN);
		TEST_ASSERT_EQUAL(expectedSum, actualSum);
	}
}

void test_flexsea_buffers(void)
{
	//RUN_TEST(test_update_rx_buf_byte_1);
	RUN_TEST(test_update_rx_buf_array_1);
	RUN_TEST(test_buffer_stack);
	RUN_TEST(test_buffer_circular);
	RUN_TEST(test_buffer_circular_alphabet);
	RUN_TEST(test_buffer_circular_write_erase);
	RUN_TEST(test_buffer_circular_search);
	RUN_TEST(test_buffer_circular_checksum);

	fflush(stdout);
}


#ifdef __cplusplus
}
#endif
