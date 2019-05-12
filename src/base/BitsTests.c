#include <string.h>
#include <stdbool.h>

#include <zrlib/base/Bits.h>

#include "../main.h"

// ============================================================================

static ZRBits MEM_REF[] =
	{ ZRBITS_MASK_1L | ZRBITS_MASK_1R, 0x000f, 0x8000, 0x003f };

#define NBBYTES sizeof(MEM_REF)
#define NBITEMS (NBBYTES / sizeof(ZRBits))

static ZRBits MEM[NBITEMS];

// ============================================================================

#define GUARD_BITS  (ZRBITS_MASK_1R | ZRBITS_MASK_1L)

static void testSetup()
{
	mainTestSetup();
	memcpy(MEM, MEM_REF, NBBYTES);
}

static int FUN_CMP_NAME(ZRBits *result, ZRBits *expected)
{
	int ret;

	while (*result != GUARD_BITS)
	{
		ret = memcmp(result, expected, sizeof(ZRBits));

		if (ret != 0)
			return ret;

		result++;
		expected++;
	}
	return 0;
}

static void FUN_PRINT_NAME(char * out, ZRBits *bits)
{
	char buff[150];
	strcat(out, "[");

	while (*bits != GUARD_BITS)
	{
		sprintf(buff, "%lX:", *bits);
		strcat(out, buff);
		bits++;
	}
	strcat(out, "]\n");
}

// ============================================================================
// SETBITS
// ============================================================================

MU_TEST(testSetBits)
{
	ZRBits source;
	//Zero local
	{
		ZRBits expected[] =
			{ (ZRBits)0x5 << ZRBITS_NBOF - 4, GUARD_BITS };
		ZRBits local[] =
			{ 0, GUARD_BITS };

		ZRBits const localRef = *local;

		ZRTEST_BEGIN();
		ZRBits_setBitsFromTheRight(local, 0, 4, (ZRBits)5);
		ZRTEST_END(MESSAGE_BUFF, local, expected);

		*local = localRef;
		ZRTEST_BEGIN();
		ZRBits_setBitsFromTheRight(local, 1, 3, (ZRBits)5);
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	//Full one local
	{
		ZRBits expected[] =
			{ ZRBITS_MASK_FULL & ~ ((ZRBits)0x5 << ZRBITS_NBOF - 4), GUARD_BITS };
		ZRBits local[] =
			{ ZRBITS_MASK_FULL, GUARD_BITS };

		ZRBits const localRef = *local;

		ZRTEST_BEGIN();
		ZRBits_setBitsFromTheRight(local, 0, 4, (ZRBits)0xa);
		ZRTEST_END(MESSAGE_BUFF, local, expected);

		*local = localRef;
		ZRTEST_BEGIN();
		ZRBits_setBitsFromTheRight(local, 1, 3, (ZRBits)0xa);
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	//Overlap
	{
		ZRBits expected[] =
			{ 5, (ZRBits)7 << ZRBITS_NBOF - 3, GUARD_BITS };
		ZRBits local[] =
			{ 0, 0, GUARD_BITS };

		ZRTEST_BEGIN();
		ZRBits_setBitsFromTheRight(local, ZRBITS_NBOF - 3, 6, (ZRBits)0x2f);
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
}

// ============================================================================
// GETMASK
// ============================================================================

MU_TEST(testGetlMask)
{
	ZRBits expected[2] =
		{ 0, GUARD_BITS };
	ZRBits local[2] =
		{ 0, GUARD_BITS };

	for (int i = 0; i <= ZRBITS_NBOF; i++)
	{
		*local = ZRBits_getLMask(i);
		ZRTEST_BEGIN();
		ZRTEST_END(MESSAGE_BUFF, local, expected);
		*expected >>= 1;
		*expected |= ZRBITS_MASK_1L;
	}
}

MU_TEST(testGetrMask)
{
	ZRBits expected[2] =
		{ 0, GUARD_BITS };
	ZRBits local[2] =
		{ 0, GUARD_BITS };

	for (int i = 0; i <= ZRBITS_NBOF; i++)
	{
		*local = ZRBits_getRMask(i);
		ZRTEST_BEGIN();
		ZRTEST_END(MESSAGE_BUFF, local, expected);
		*expected <<= 1;
		*expected |= ZRBITS_MASK_1R;
	}
}

// ============================================================================
// PACK
// ============================================================================

MU_TEST(testPack)
{
	ZRBits expected[2] =
		{ ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 2) | (ZRBITS_MASK_1L >> 3), GUARD_BITS };
	ZRBits local[2] =
		{ 0, GUARD_BITS };
	{
		unsigned char packet[] =
			{ 1, 0, 1, 1 };
		ZRBits_cpack(local, 1, packet, sizeof (packet) / sizeof (*packet));
		ZRTEST_BEGIN_MSG("1x4");
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	{
		unsigned char packet[] =
			{ 2, 3 };
		ZRBits_cpack(local, 2, packet, sizeof (packet) / sizeof (*packet));
		ZRTEST_BEGIN_MSG("2x2");
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	{
		unsigned char packet[] =
			{ 11 };
		ZRBits_cpack(local, 4, packet, sizeof (packet) / sizeof (*packet));
		ZRTEST_BEGIN_MSG("4x1");
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	{
		ZRBits const part = (ZRBits)5 << ZRBITS_NBOF - 3;
		ZRBits expected[2] =
			{ part | part >> 3 | part >> 6, GUARD_BITS };
		unsigned packet[] =
			{ 5, 5, 5 };
		ZRBITS_PACK_ARRAY(local, 3, packet);
		ZRTEST_BEGIN_MSG("array 3x3");
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
}
// ============================================================================
// COPY
// ============================================================================

MU_TEST(testCopyOneInsideOne)
{
	ZRBits local[] =
		{ ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 2) | (ZRBITS_MASK_1L >> 3) | ZRBITS_MASK_1R | ZRBITS_MASK_1R << 1,
		GUARD_BITS };
	ZRBits result[] =
		{ 0, GUARD_BITS };
	ZRBits expected[] =
		{ 0, GUARD_BITS };
	{
		expected[0] = *local & ZRBits_getLMask(3);
		ZRTEST_BEGIN_MSG("pos=0, nbBits=3, outPos=0");
		ZRBits_copy(local, 0, 3, result, 0);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = (*local & ZRBits_getRMask(3)) << (ZRBITS_NBOF - 3);
		ZRTEST_BEGIN_MSG("pos=*1 - 3, nbBits=3, outPos=0");
		ZRBits_copy(local, ZRBITS_NBOF - 3, 3, result, 0);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = (*local & ZRBits_getLMask(3)) >> 1;
		ZRTEST_BEGIN_MSG("pos=0, nbBits=3, outPos=1");
		ZRBits_copy(local, 0, 3, result, 1);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = (*local & ZRBits_getRMask(3)) << (ZRBITS_NBOF - 4);
		ZRTEST_BEGIN_MSG("pos=*1 - 3, nbBits=3, outPos=1");
		ZRBits_copy(local, ZRBITS_NBOF - 3, 3, result, 1);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
}

MU_TEST(testCopyOneOverTwo)
{
	ZRBits local[] =
		{ ZRBITS_MASK_1R | ZRBITS_MASK_1R << 2, ZRBITS_MASK_1L | ZRBITS_MASK_1L >> 1 | ZRBITS_MASK_1R << 3,
		GUARD_BITS };
	ZRBits result[] =
		{ 0, 0, GUARD_BITS };
	ZRBits expected[] =
		{ 0, 0, GUARD_BITS };
	{
		expected[0] = ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 2) | (ZRBITS_MASK_1L >> 3) | (ZRBITS_MASK_1L >> 4);
		ZRTEST_BEGIN_MSG("pos=*1-3, nbBits=5, outPos=0");
		ZRBits_copy(local, ZRBITS_NBOF - 3, 5, result, 0);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] >>= 1;
		ZRTEST_BEGIN_MSG("pos=*1-3, nbBits=5, outPos=1");
		ZRBits_copy(local, ZRBITS_NBOF - 3, 5, result, 1);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = 0x17;
		ZRTEST_BEGIN_MSG("pos=*1-3, nbBits=5, outPos=-5");
		ZRBits_copy(local, ZRBITS_NBOF - 3, 5, result, ZRBITS_NBOF - 5);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = (ZRBits)0x17 << (ZRBITS_NBOF - 5) | ZRBITS_MASK_1R;
		ZRTEST_BEGIN_MSG("pos=*1-3, nbBits=*1, outPos=0");
		ZRBits_copy(local, ZRBITS_NBOF - 3, ZRBITS_NBOF, result, 0);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
}

MU_TEST(testCopyMore)
{
	ZRBits local[] =
		{ 0, 0, 0, 0,
		GUARD_BITS };
	ZRBits result[] =
		{ 0, 0, 0, 0, GUARD_BITS };
	ZRBits expected[] =
		{ 0, 0, 0, 0, GUARD_BITS };

	local[0] = ZRBITS_MASK_1L | ZRBITS_MASK_1L >> 2 | ZRBITS_MASK_1R << 1 | ZRBITS_MASK_1R;
	local[1] = *local;
	local[2] = *local;
	local[3] = *local;
	{
		memcpy(expected, local, sizeof(ZRBits) * 4);
		ZRTEST_BEGIN_MSG("pos=0, nbBits=*4, outPos=0");
		ZRBits_copy(local, 0, ZRBITS_NBOF * 4, result, 0);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		memset(result, 0, sizeof(ZRBits) * 4);
		expected[0] = *local << (ZRBITS_NBOF - 2) | *local >> 2;
		expected[1] = *local << (ZRBITS_NBOF - 2);
		expected[2] = 0;
		expected[3] = 0;
		ZRTEST_BEGIN_MSG("pos=*1 - 2, nbBits=*1 + 4, outPos=0");
		ZRBits_copy(local, ZRBITS_NBOF - 2, ZRBITS_NBOF + 4, result, 0);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		memset(result, 0, sizeof(ZRBits) * 4);
		expected[0] >>= 2;
		expected[1] >>= 2;
		ZRTEST_BEGIN_MSG("pos=*1 - 2, nbBits=*1 + 4, outPos=2");
		ZRBits_copy(local, ZRBITS_NBOF - 2, ZRBITS_NBOF + 4, result, 2);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		memset(result, 0, sizeof(ZRBits) * 4);
		expected[0] = *local << (ZRBITS_NBOF - 2) | *local >> 2;
		expected[1] = *local << (ZRBITS_NBOF - 2) | *local >> 2;
		expected[2] = *local << (ZRBITS_NBOF - 2);
		expected[3] = 0;
		ZRTEST_BEGIN_MSG("pos=*1 - 2, nbBits=*2 + 4, outPos=0");
		ZRBits_copy(local, ZRBITS_NBOF - 2, ZRBITS_NBOF * 2 + 4, result, 0);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}

	{
		memset(result, 0, sizeof(ZRBits) * 4);
		expected[0] = *local >> 2;
		expected[1] = (*local & 3) << (ZRBITS_NBOF - 2) | (*local >> 2);
		ZRTEST_BEGIN_MSG("pos=0, nbBits=*2, outPos=2");
		ZRBits_copy(local, 0, ZRBITS_NBOF * 2, result, 2);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		memset(result, 0, sizeof(ZRBits) * 4);
		ZRTEST_BEGIN_MSG("pos=0, nbBits=*2 + 2, outPos=2");
		expected[2] = (*local & 3) << (ZRBITS_NBOF - 2);
		ZRBits_copy(local, 0, ZRBITS_NBOF * 2 + 2, result, 2);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}

	{
		memset(result, 0, sizeof(ZRBits) * 4);
		memcpy(expected, local, sizeof(ZRBits) * 4);
		expected[0] = *local & ZRBits_getRMask(ZRBITS_NBOF - 2);
		expected[2] = *local & ZRBits_getLMask(2);
		expected[3] = 0;
		ZRTEST_BEGIN_MSG("pos=2, nbBits=*2, outPos=2");
		ZRBits_copy(local, 2, ZRBITS_NBOF * 2, result, 2);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		memset(result, 0, sizeof(ZRBits) * 4);
		memcpy(expected, local, sizeof(ZRBits) * 4);
		expected[0] = *local & ZRBits_getRMask(ZRBITS_NBOF - 2);
		expected[2] = *local & ZRBits_getLMask(4);
		expected[3] = 0;
		ZRTEST_BEGIN_MSG("pos=2, nbBits=*2 + 2, outPos=2");
		ZRBits_copy(local, 2, ZRBITS_NBOF * 2 + 2, result, 2);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
}

// ============================================================================
// GETBIT
// ============================================================================
MU_TEST(testGetBit)
{
	mu_check(ZRBits_getBit(MEM + 0, 0) == true);
	mu_check(ZRBits_getBit(MEM + 0, 1) == false);
	mu_check(ZRBits_getBit(MEM+0, ZRBITS_NBOF - 1) == true);
	mu_check(ZRBits_getBit(MEM+0, ZRBITS_NBOF - 2) == false);
	mu_check(ZRBits_getBit(MEM + 0, 0) == true);
	mu_check(ZRBits_getBit(MEM + 0, 1) == false);
	mu_check(ZRBits_getBit(MEM+0, ZRBITS_NBOF - 1) == true);
	mu_check(ZRBits_getBit(MEM+0, ZRBITS_NBOF - 2) == false);

	mu_check(ZRBits_getBit(MEM+1, ZRBITS_NBOF - 4) == true);
	mu_check(ZRBits_getBit(MEM+0, 2*ZRBITS_NBOF - 4) == true);
	mu_check(ZRBits_getBit(MEM+1, ZRBITS_NBOF - 5) == false);
	mu_check(ZRBits_getBit(MEM+0, 2*ZRBITS_NBOF - 5) == false);
}

MU_TEST(testGetOneBits)
{
	ZRBits local[] =
		{ ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 2) | (ZRBITS_MASK_1L >> 3) | ZRBITS_MASK_1R, ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 2),
		ZRBITS_MASK_1L, GUARD_BITS };
	ZRBits result[] =
		{ 0, GUARD_BITS };
	ZRBits expected[] =
		{ 0, GUARD_BITS };

	{
		expected[0] = ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 2);
		ZRTEST_BEGIN();
		ZRBits_getBits(local, 0, 3, result);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = (ZRBITS_MASK_1L >> 2);
		ZRTEST_BEGIN();
		ZRBits_getBits(local, ZRBITS_NBOF - 3, 3, result);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 1);
		ZRTEST_BEGIN();
		ZRBits_getBits(local, ZRBITS_NBOF - 1, 2, result);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
	{
		expected[0] = (ZRBITS_MASK_1L | (ZRBITS_MASK_1L >> 1) | (ZRBITS_MASK_1L >> 3)) >> 1;
		ZRTEST_BEGIN();
		ZRBits_getBits(local, ZRBITS_NBOF - 2, 5, result);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
}

MU_TEST(testGetBits)
{
	ZRBits rpart = 0x5;
	ZRBits lpart = rpart << (ZRBITS_NBOF - 3);
	ZRBits local[] =
		{ rpart | lpart, ZRBITS_MASK_1L | ZRBITS_MASK_1R, ZRBITS_MASK_1L | rpart, GUARD_BITS };
	ZRBits result[] =
		{ 0, 0, 0, GUARD_BITS };
	ZRBits expected[] =
		{ 0, 0, 0, GUARD_BITS };
	{
		expected[0] = ( (rpart | lpart) << 1) | ZRBITS_MASK_1R;
		expected[1] = 0x3;
		expected[2] = rpart << 1;
		ZRTEST_BEGIN();
		ZRBits_getBits(local, 1, (ZRBITS_NBOF * 3) - 1, result);
		ZRTEST_END(MESSAGE_BUFF, result, expected);
	}
}

// ============================================================================
// SHIFT
// ============================================================================

MU_TEST(testInArrayShift)
{
	{
		ZRBits expected[] =
			{ 0x2345, 0, GUARD_BITS };
		ZRBits local[] =
			{ 0x1234, 0x2345, GUARD_BITS };

		ZRTEST_BEGIN();
		ZRBits_inArrayShift(local, 2, sizeof (*local) * CHAR_BIT, false);
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	{
		ZRBits expected[] =
			{ 0, 0x1234, GUARD_BITS };
		ZRBits local[] =
			{ 0x1234, 0x2345, GUARD_BITS };

		ZRTEST_BEGIN();
		ZRBits_inArrayShift(local, 2, sizeof (*local) * CHAR_BIT, true);
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	{
		ZRBits expected[] =
			{ ZRBITS_MASK_1L >> 1, ZRBITS_MASK_1R << 2, GUARD_BITS };
		ZRBits local[] =
			{ ZRBITS_MASK_1L >> 2, ZRBITS_MASK_1R << 1, GUARD_BITS };

		ZRTEST_BEGIN();
		ZRBits_inArrayShift(local, 2, 1, false);
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
	{
		ZRBits expected[] =
			{ ZRBITS_MASK_1L >> 3, ZRBITS_MASK_1R, GUARD_BITS };
		ZRBits local[] =
			{ ZRBITS_MASK_1L >> 2, ZRBITS_MASK_1R << 1, GUARD_BITS };

		ZRTEST_BEGIN();
		ZRBits_inArrayShift(local, 2, 1, true);
		ZRTEST_END(MESSAGE_BUFF, local, expected);
	}
}

// ============================================================================
// TESTS
// ============================================================================

MU_TEST_SUITE(AllTests)
{
	MU_RUN_TEST(testGetlMask);
	MU_RUN_TEST(testGetrMask);
	MU_RUN_TEST(testSetBits);
	MU_RUN_TEST(testPack);
	MU_RUN_TEST(testGetBit);
	MU_RUN_TEST(testCopyOneInsideOne);
	MU_RUN_TEST(testCopyOneOverTwo);
	MU_RUN_TEST(testCopyMore);
	MU_RUN_TEST(testGetOneBits);
	MU_RUN_TEST(testGetBits);
	MU_RUN_TEST(testInArrayShift);
}

int BitsTests(void)
{
	puts(__FUNCTION__);
	MU_SUITE_CONFIGURE(testSetup, NULL);
	MU_RUN_SUITE(AllTests);
	MU_REPORT()
	;
	return minunit_status;
}
