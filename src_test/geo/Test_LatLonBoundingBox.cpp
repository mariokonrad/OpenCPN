#include <gtest/gtest.h>
#include <geo/LatLonBoundingBox.h>

namespace {

class Test_LatLonBoundingBox : public ::testing::Test {};

TEST_F(Test_LatLonBoundingBox, not_crossing_idl_no_margin)
{
	geo::LatLonBoundingBox b;

	// 10deg arround Greeenwich
	b.SetMin(-10.0, -10.0);
	b.SetMax( 10.0,  10.0);

	EXPECT_TRUE(b.PointInBox(0.0,  0.0, 0.0)); // inside
	EXPECT_FALSE(b.PointInBox( 20.0,   0.0, 0.0)); // east
	EXPECT_FALSE(b.PointInBox(  0.0,  20.0, 0.0)); // north
	EXPECT_FALSE(b.PointInBox(-20.0,   0.0, 0.0)); // west
	EXPECT_FALSE(b.PointInBox(  0.0, -20.0, 0.0)); // south
}

TEST_F(Test_LatLonBoundingBox, not_crossing_idl_with_margin)
{
	geo::LatLonBoundingBox b;

	// 10deg arround Greeenwich
	b.SetMin(-10.0, -10.0);
	b.SetMax( 10.0,  10.0);

	EXPECT_TRUE(b.PointInBox(0.0,  0.0, -1.0)); // inside
	EXPECT_TRUE(b.PointInBox(0.0,  0.0,  0.0)); // inside
	EXPECT_TRUE(b.PointInBox(0.0,  0.0,  2.0)); // inside

	EXPECT_TRUE(b.PointInBox( 11.0,   0.0, 2.0)); // east
	EXPECT_TRUE(b.PointInBox(  0.0,  11.0, 2.0)); // north
	EXPECT_TRUE(b.PointInBox(-11.0,   0.0, 2.0)); // west
	EXPECT_TRUE(b.PointInBox(  0.0, -11.0, 2.0)); // south

	EXPECT_FALSE(b.PointInBox( 11.0,   0.0, -2.0)); // east
	EXPECT_FALSE(b.PointInBox(  0.0,  11.0, -2.0)); // north
	EXPECT_FALSE(b.PointInBox(-11.0,   0.0, -2.0)); // west
	EXPECT_FALSE(b.PointInBox(  0.0, -11.0, -2.0)); // south

	EXPECT_TRUE(b.PointInBox( 12.0,   0.0, 2.0)); // east
	EXPECT_TRUE(b.PointInBox(  0.0,  12.0, 2.0)); // north
	EXPECT_TRUE(b.PointInBox(-12.0,   0.0, 2.0)); // west
	EXPECT_TRUE(b.PointInBox(  0.0, -12.0, 2.0)); // south

	EXPECT_FALSE(b.PointInBox( 12.0,   0.0, -2.0)); // east
	EXPECT_FALSE(b.PointInBox(  0.0,  12.0, -2.0)); // north
	EXPECT_FALSE(b.PointInBox(-12.0,   0.0, -2.0)); // west
	EXPECT_FALSE(b.PointInBox(  0.0, -12.0, -2.0)); // south

	EXPECT_FALSE(b.PointInBox( 13.0,   0.0, 2.0)); // east
	EXPECT_FALSE(b.PointInBox(  0.0,  13.0, 2.0)); // north
	EXPECT_FALSE(b.PointInBox(-13.0,   0.0, 2.0)); // west
	EXPECT_FALSE(b.PointInBox(  0.0, -13.0, 2.0)); // south

	EXPECT_FALSE(b.PointInBox( 13.0,   0.0, -2.0)); // east
	EXPECT_FALSE(b.PointInBox(  0.0,  13.0, -2.0)); // north
	EXPECT_FALSE(b.PointInBox(-13.0,   0.0, -2.0)); // west
	EXPECT_FALSE(b.PointInBox(  0.0, -13.0, -2.0)); // south
}

TEST_F(Test_LatLonBoundingBox, not_crossing_idl_minmax_reversed)
{
	// EXPECT_TRUE(false); // FIXME: write the unit test
}

TEST_F(Test_LatLonBoundingBox, centered_east_lon_crossing_idl)
{
	// EXPECT_TRUE(false); // FIXME: write the unit test
}

TEST_F(Test_LatLonBoundingBox, centered_west_lon_crossing_idl)
{
	// EXPECT_TRUE(false); // FIXME: write the unit test
}

}

