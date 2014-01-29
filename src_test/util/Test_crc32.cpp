#include <gtest/gtest.h>
#include <util/crc32.h>

namespace {

class Test_crc32 : public ::testing::Test {};

TEST_F(Test_crc32, empty)
{
	const char* buf = "";
	const unsigned char* ptr = reinterpret_cast<const unsigned char*>(buf);

	unsigned int result = util::crc32buf(ptr, 0);

	EXPECT_EQ(0x000000000, result);
}

TEST_F(Test_crc32, quick_brown_fox)
{
	const char* buf = "The quick brown fox jumps over the lazy dog";
	const unsigned char* ptr = reinterpret_cast<const unsigned char*>(buf);

	unsigned int result = util::crc32buf(ptr, 43);

	EXPECT_EQ(0x414fa339, result);
}

TEST_F(Test_crc32, test_vector_from_febooti_com)
{
	const char* buf = "Test vector from febooti.com";
	const unsigned char* ptr = reinterpret_cast<const unsigned char*>(buf);

	unsigned int result = util::crc32buf(ptr, 28);

	EXPECT_EQ(0x0c877f61, result);
}


}

