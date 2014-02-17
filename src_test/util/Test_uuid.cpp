#include <gtest/gtest.h>
#include <util/uuid.h>

namespace {

class Test_uuid : public ::testing::Test {};

TEST_F(Test_uuid, constant_seed_zero_explicit)
{
	const std::string expected = "1474ba99-9b09-4787-b317-cd6c55176bb5";

	::srand(0);
	std::string s = util::uuid();

	EXPECT_EQ(expected, s);
}

TEST_F(Test_uuid, constant_seed_nonzero_explicit)
{
	const std::string expected = "6370ddf1-c87f-4d6c-816e-86b54335deb4";

	::srand(1234);
	std::string s = util::uuid();

	EXPECT_EQ(expected, s);
}

TEST_F(Test_uuid, constant_seed_zero)
{
	::srand(0);
	std::string s0 = util::uuid();

	::srand(0);
	std::string s1 = util::uuid();

	EXPECT_EQ(s0, s1);
}

TEST_F(Test_uuid, constant_seed_nonzero)
{
	::srand(1234);
	std::string s0 = util::uuid();

	::srand(1234);
	std::string s1 = util::uuid();

	EXPECT_EQ(s0, s1);
}

TEST_F(Test_uuid, consecutive_creation)
{
	::srand(0);
	std::string s0 = util::uuid();
	std::string s1 = util::uuid();

	EXPECT_NE(s0, s1);
}

}

