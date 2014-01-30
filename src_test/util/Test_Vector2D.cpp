#include <gtest/gtest.h>
#include <util/Vector2D.h>

namespace {

class Test_Vector2D : public ::testing::Test {};

TEST_F(Test_Vector2D, default_construction)
{
	util::Vector2D v;

	EXPECT_EQ(0.0, v.x);
	EXPECT_EQ(0.0, v.y);
	EXPECT_EQ(0.0, v.lat);
	EXPECT_EQ(0.0, v.lon);
}

TEST_F(Test_Vector2D, construction)
{
	util::Vector2D v(1.0, 2.0);

	EXPECT_EQ(1.0, v.x);
	EXPECT_EQ(2.0, v.y);
	EXPECT_EQ(1.0, v.lon);
	EXPECT_EQ(2.0, v.lat);
}

TEST_F(Test_Vector2D, comparison_equal)
{
	util::Vector2D v0(1.0, 2.0);
	util::Vector2D v1(1.0, 2.0);

	EXPECT_EQ(v0, v1);
	EXPECT_TRUE(v0 == v1);
}

TEST_F(Test_Vector2D, comparison_not_equal)
{
	util::Vector2D v0(1.0, 2.0);
	util::Vector2D v1(2.0, 1.0);

	EXPECT_NE(v0, v1);
	EXPECT_TRUE(v0 != v1);
}

TEST_F(Test_Vector2D, operation_minus)
{
	util::Vector2D v0(1.0, 2.0);
	util::Vector2D v1(2.0, 1.0);

	util::Vector2D v = v0 - v1;

	EXPECT_EQ(-1.0, v.x);
	EXPECT_EQ( 1.0, v.y);
	EXPECT_EQ(-1.0, v.lon);
	EXPECT_EQ( 1.0, v.lat);
}

TEST_F(Test_Vector2D, operation_plus)
{
	util::Vector2D v0(1.0, 2.0);
	util::Vector2D v1(4.0, 1.0);

	util::Vector2D v = v0 + v1;

	EXPECT_EQ(5.0, v.x);
	EXPECT_EQ(3.0, v.y);
	EXPECT_EQ(5.0, v.lon);
	EXPECT_EQ(3.0, v.lat);
}

TEST_F(Test_Vector2D, operation_scale_prefix)
{
	util::Vector2D v0(1.0, 2.0);

	util::Vector2D v = 10.0 * v0;

	EXPECT_EQ(10.0, v.x);
	EXPECT_EQ(20.0, v.y);
	EXPECT_EQ(10.0, v.lon);
	EXPECT_EQ(20.0, v.lat);
}

TEST_F(Test_Vector2D, operation_scale_postfix)
{
	util::Vector2D v0(1.0, 2.0);

	util::Vector2D v = v0 * 10.0;

	EXPECT_EQ(10.0, v.x);
	EXPECT_EQ(20.0, v.y);
	EXPECT_EQ(10.0, v.lon);
	EXPECT_EQ(20.0, v.lat);
}

TEST_F(Test_Vector2D, operation_mul)
{
	util::Vector2D v0(1.0, 2.0);
	util::Vector2D v1(4.0, 8.0);

	double s = v0 * v1;

	EXPECT_EQ(20.0, s);
}

TEST_F(Test_Vector2D, dot)
{
	util::Vector2D v0(1.0, 2.0);
	util::Vector2D v1(4.0, 8.0);

	double s = v0.dot(v1);

	EXPECT_EQ(20.0, s);
}

TEST_F(Test_Vector2D, sqr)
{
	util::Vector2D v0(2.0, 3.0);

	double s = v0.sqr();

	EXPECT_EQ(13.0, s);
}

TEST_F(Test_Vector2D, length)
{
	util::Vector2D v0(3.0, 4.0);

	double s = v0.length();

	EXPECT_EQ(5.0, s);
}

TEST_F(Test_Vector2D, lengthOfNormal)
{
	util::Vector2D v0(0.0, 5.0);
	util::Vector2D v1(3.0, 3.0);

	util::Vector2D n;
	double s = util::lengthOfNormal(v0, v1, n);

	EXPECT_NEAR(3.53553, s, 0.0001);
	EXPECT_EQ(-2.5, n.x);
	EXPECT_EQ( 2.5, n.y);
}

}

