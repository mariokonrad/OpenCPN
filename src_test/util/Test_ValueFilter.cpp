#include <gtest/gtest.h>
#include <util/ValueFilter.h>

namespace {

class Test_ValueFilter : public ::testing::Test {};

TEST_F(Test_ValueFilter, construction_size)
{
	util::ValueFilter f0(0);
	util::ValueFilter f1(1);
	util::ValueFilter f2(2);
	util::ValueFilter f4(4);

	EXPECT_EQ(0, f0.size());
	EXPECT_EQ(1, f1.size());
	EXPECT_EQ(2, f2.size());
	EXPECT_EQ(4, f4.size());
}

TEST_F(Test_ValueFilter, initialized_get)
{
	util::ValueFilter f(4);

	EXPECT_EQ(0.0, f.get());
}

TEST_F(Test_ValueFilter, repeated_push_const_size)
{
	util::ValueFilter f(4);

	EXPECT_EQ(4, f.size());
	f.push(1.0);
	EXPECT_EQ(4, f.size());
	f.push(1.0);
	EXPECT_EQ(4, f.size());
	f.push(1.0);
	EXPECT_EQ(4, f.size());
	f.push(1.0);
	EXPECT_EQ(4, f.size());
}

TEST_F(Test_ValueFilter, push_same_value_get)
{
	util::ValueFilter f(4);

	f.push(1.0);
	f.push(1.0);
	f.push(1.0);
	f.push(1.0);
	EXPECT_EQ(1.0, f.get());
}

TEST_F(Test_ValueFilter, push_same_value_repeated_get)
{
	util::ValueFilter f(4);

	f.push(4.0);
	EXPECT_EQ(1.0, f.get());
	EXPECT_EQ(1.0, f.get());
	EXPECT_EQ(1.0, f.get());
	EXPECT_EQ(1.0, f.get());
}

TEST_F(Test_ValueFilter, push_different_value_get)
{
	util::ValueFilter f(4);

	f.push(1.0);
	f.push(2.0);
	f.push(4.0);
	f.push(8.0);
	EXPECT_EQ(15.0 / 4.0, f.get());
}

TEST_F(Test_ValueFilter, push_different_intermediate_values)
{
	util::ValueFilter f(4);

	EXPECT_EQ((0.0 + 0.0 + 0.0 + 0.0), f.get());
	f.push(1.0);
	EXPECT_EQ((1.0 + 0.0 + 0.0 + 0.0) / 4.0, f.get());
	f.push(2.0);
	EXPECT_EQ((1.0 + 2.0 + 0.0 + 0.0) / 4.0, f.get());
	f.push(4.0);
	EXPECT_EQ((1.0 + 2.0 + 4.0 + 0.0) / 4.0, f.get());
	f.push(8.0);
	EXPECT_EQ((1.0 + 2.0 + 4.0 + 8.0) / 4.0, f.get());
	f.push(16.0);
	EXPECT_EQ((2.0 + 4.0 + 8.0 + 16.0) / 4.0, f.get());
	f.push(32.0);
	EXPECT_EQ((4.0 + 8.0 + 16.0 + 32.0) / 4.0, f.get());
}

TEST_F(Test_ValueFilter, push_different_intermediate_values_repeated_get)
{
	util::ValueFilter f(4);

	EXPECT_EQ(0.0, f.get());
	EXPECT_EQ(0.0, f.get());
	EXPECT_EQ(0.0, f.get());
	f.push(1.0);
	EXPECT_EQ(1.0 / 4.0, f.get());
	EXPECT_EQ(1.0 / 4.0, f.get());
	EXPECT_EQ(1.0 / 4.0, f.get());
	f.push(2.0);
	EXPECT_EQ(3.0 / 4.0, f.get());
	EXPECT_EQ(3.0 / 4.0, f.get());
	EXPECT_EQ(3.0 / 4.0, f.get());
	f.push(4.0);
	EXPECT_EQ(7.0 / 4.0, f.get());
	EXPECT_EQ(7.0 / 4.0, f.get());
	EXPECT_EQ(7.0 / 4.0, f.get());
	f.push(8.0);
	EXPECT_EQ(15.0 / 4.0, f.get());
	EXPECT_EQ(15.0 / 4.0, f.get());
	EXPECT_EQ(15.0 / 4.0, f.get());
}

TEST_F(Test_ValueFilter, resize_downsize)
{
	util::ValueFilter f(4);

	f.push(1.0);
	f.push(2.0);
	f.push(4.0);
	f.push(8.0);
	EXPECT_EQ((1.0 + 2.0 + 4.0 + 8.0) / 4.0, f.get());

	f.resize(2);
	EXPECT_EQ((1.0 + 2.0) / 2.0, f.get());
}

TEST_F(Test_ValueFilter, resize_upsize)
{
	util::ValueFilter f(2);

	f.push(1.0);
	f.push(2.0);
	EXPECT_EQ((1.0 + 2.0) / 2.0, f.get());

	f.resize(4);
	EXPECT_EQ((1.0 + 2.0 + 0.0 + 0.0) / 4.0, f.get());

	f.push(4.0);
	EXPECT_EQ((2.0 + 0.0 + 0.0 + 4.0) / 4.0, f.get());

	f.push(8.0);
	EXPECT_EQ((0.0 + 0.0 + 4.0 + 8.0) / 4.0, f.get());
}

TEST_F(Test_ValueFilter, resize_same_size)
{
	util::ValueFilter f(4);

	f.push(1.0);
	f.push(2.0);
	f.push(4.0);
	f.push(8.0);
	EXPECT_EQ((1.0 + 2.0 + 4.0 + 8.0) / 4.0, f.get());

	f.resize(4);
	EXPECT_EQ((1.0 + 2.0 + 4.0 + 8.0) / 4.0, f.get());
}

TEST_F(Test_ValueFilter, fill)
{
	util::ValueFilter f(4);

	EXPECT_EQ(0.0, f.get());

	f.fill(1.0);
	EXPECT_EQ(1.0, f.get());

	f.fill(2.0);
	EXPECT_EQ(2.0, f.get());
}

TEST_F(Test_ValueFilter, fill_after_resize)
{
	util::ValueFilter f(2);

	EXPECT_EQ(0.0, f.get());

	f.push(1.0);
	f.push(2.0);
	EXPECT_EQ(3.0 / 2.0, f.get());

	f.resize(4);
	EXPECT_EQ(3.0 / 4.0, f.get());

	f.fill(2.0);
	EXPECT_EQ(2.0, f.get());
}

}

