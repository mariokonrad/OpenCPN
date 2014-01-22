#include <gtest/gtest.h>
#include <geo/Position.h>

namespace {

class Test_Position : public ::testing::Test {};

TEST_F(Test_Position, default_construction)
{
	geo::Position p;

	EXPECT_EQ(0.0, p.lat());
	EXPECT_EQ(0.0, p.lon());
}

TEST_F(Test_Position, construction)
{
	geo::Position p(12.34, 45.67);

	EXPECT_EQ(12.34, p.lat());
	EXPECT_EQ(45.67, p.lon());
}

TEST_F(Test_Position, compare_default_constructed)
{
	geo::Position p0;
	geo::Position p1;

	EXPECT_EQ(p0, p1);
}

TEST_F(Test_Position, compare)
{
	geo::Position p0(1.0, 2.0);
	geo::Position p1(1.0, 2.0);
	geo::Position p2(1.0, 3.0);
	geo::Position p3(3.0, 2.0);

	EXPECT_EQ(p0, p0);
	EXPECT_EQ(p0, p1);
	EXPECT_NE(p0, p2);
	EXPECT_NE(p0, p3);

	EXPECT_NE(p1, p2);
	EXPECT_NE(p1, p3);

	EXPECT_NE(p2, p3);
}

TEST_F(Test_Position, normalize_lon)
{
	geo::Position p0(0.0,   0.0);
	geo::Position p1(0.0,  90.0);
	geo::Position p2(0.0, 180.0);
	geo::Position p3(0.0, 270.0);
	geo::Position p4(0.0, 360.0);

	p0.normalize_lon();
	p1.normalize_lon();
	p2.normalize_lon();
	p3.normalize_lon();
	p4.normalize_lon();

	EXPECT_EQ(  0.0, p0.lon());
	EXPECT_EQ( 90.0, p1.lon());
	EXPECT_EQ(180.0, p2.lon());
	EXPECT_EQ(-90.0, p3.lon());
	EXPECT_EQ(  0.0, p4.lon());
}

TEST_F(Test_Position, operator_add)
{
	geo::Position p0( 1.0,  2.0);
	geo::Position p1(10.0, 20.0);

	geo::Position r0 = p0 + p1;
	EXPECT_EQ(11.0, r0.lat());
	EXPECT_EQ(22.0, r0.lon());

	geo::Position r1 = p1 + p0;
	EXPECT_EQ(11.0, r1.lat());
	EXPECT_EQ(22.0, r1.lon());
}

TEST_F(Test_Position, operator_sub)
{
	geo::Position p0( 1.0,  2.0);
	geo::Position p1(10.0, 20.0);

	geo::Position r0 = p0 - p1;
	EXPECT_EQ( -9.0, r0.lat());
	EXPECT_EQ(-18.0, r0.lon());

	geo::Position r1 = p1 - p0;
	EXPECT_EQ( 9.0, r1.lat());
	EXPECT_EQ(18.0, r1.lon());
}

}

