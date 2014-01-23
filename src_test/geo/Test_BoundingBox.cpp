#include <gtest/gtest.h>
#include <geo/BoundingBox.h>

namespace {

class Test_BoundingBox : public ::testing::Test {};

TEST_F(Test_BoundingBox, construction_default)
{
	geo::BoundingBox box;

	EXPECT_EQ(0.0, box.GetMinX());
	EXPECT_EQ(0.0, box.GetMinY());
	EXPECT_EQ(0.0, box.GetMaxX());
	EXPECT_EQ(0.0, box.GetMaxY());
	EXPECT_FALSE(box.GetValid());
}

TEST_F(Test_BoundingBox, construction_boundaries)
{
	geo::BoundingBox box(11.0, 22.0, 44.0, 88.0);

	EXPECT_EQ(11.0, box.GetMinX());
	EXPECT_EQ(22.0, box.GetMinY());
	EXPECT_EQ(44.0, box.GetMaxX());
	EXPECT_EQ(88.0, box.GetMaxY());
	EXPECT_TRUE(box.GetValid());
}

TEST_F(Test_BoundingBox, construction_boundaries_inverse_x)
{
	geo::BoundingBox box(44.0, 22.0, 11.0, 88.0);

	EXPECT_EQ(44.0, box.GetMinX());
	EXPECT_EQ(22.0, box.GetMinY());
	EXPECT_EQ(11.0, box.GetMaxX());
	EXPECT_EQ(88.0, box.GetMaxY());
	EXPECT_TRUE(box.GetValid());
}

TEST_F(Test_BoundingBox, construction_boundaries_inverse_y)
{
	geo::BoundingBox box(11.0, 88.0, 44.0, 22.0);

	EXPECT_EQ(11.0, box.GetMinX());
	EXPECT_EQ(88.0, box.GetMinY());
	EXPECT_EQ(44.0, box.GetMaxX());
	EXPECT_EQ(22.0, box.GetMaxY());
	EXPECT_TRUE(box.GetValid());
}

TEST_F(Test_BoundingBox, construction_copy)
{
	geo::BoundingBox box0(11.0, 22.0, 44.0, 88.0);
	geo::BoundingBox box1(box0);

	EXPECT_EQ(11.0, box1.GetMinX());
	EXPECT_EQ(22.0, box1.GetMinY());
	EXPECT_EQ(44.0, box1.GetMaxX());
	EXPECT_EQ(88.0, box1.GetMaxY());
	EXPECT_TRUE(box0.GetValid());
	EXPECT_TRUE(box1.GetValid());
}

TEST_F(Test_BoundingBox, set_min)
{
	geo::BoundingBox box;

	box.SetMin(11.0, 22.0);

	EXPECT_EQ(11.0, box.GetMinX());
	EXPECT_EQ(22.0, box.GetMinY());
	EXPECT_EQ(11.0, box.GetMaxX());
	EXPECT_EQ(22.0, box.GetMaxY());
	EXPECT_TRUE(box.GetValid());
}

TEST_F(Test_BoundingBox, set_max)
{
	geo::BoundingBox box;

	box.SetMax(11.0, 22.0);

	EXPECT_EQ(11.0, box.GetMinX());
	EXPECT_EQ(22.0, box.GetMinY());
	EXPECT_EQ(11.0, box.GetMaxX());
	EXPECT_EQ(22.0, box.GetMaxY());
	EXPECT_TRUE(box.GetValid());
}

TEST_F(Test_BoundingBox, set_valid_explicit)
{
	geo::BoundingBox box;

	box.SetValid(true);

	EXPECT_EQ(0.0, box.GetMinX());
	EXPECT_EQ(0.0, box.GetMinY());
	EXPECT_EQ(0.0, box.GetMaxX());
	EXPECT_EQ(0.0, box.GetMaxY());
	EXPECT_TRUE(box.GetValid());
}

TEST_F(Test_BoundingBox, set_valid_explicit_keep_values)
{
	geo::BoundingBox box(11.0, 22.0, 44.0, 88.0);

	EXPECT_EQ(11.0, box.GetMinX());
	EXPECT_EQ(22.0, box.GetMinY());
	EXPECT_EQ(44.0, box.GetMaxX());
	EXPECT_EQ(88.0, box.GetMaxY());
	EXPECT_TRUE(box.GetValid());

	box.SetValid(false);

	EXPECT_EQ(11.0, box.GetMinX());
	EXPECT_EQ(22.0, box.GetMinY());
	EXPECT_EQ(44.0, box.GetMaxX());
	EXPECT_EQ(88.0, box.GetMaxY());
	EXPECT_FALSE(box.GetValid());
}

TEST_F(Test_BoundingBox, get_height)
{
	geo::BoundingBox box(11.0, 22.0, 44.0, 88.0);

	EXPECT_EQ(66.0, box.GetHeight());
}

TEST_F(Test_BoundingBox, get_width)
{
	geo::BoundingBox box(11.0, 22.0, 44.0, 88.0);

	EXPECT_EQ(33.0, box.GetWidth());
}

// TODO: Reset
// TODO: And
// TODO: EnLarge
// TODO: Shrink
// TODO: Expand
// TODO: Intersect
// TODO: LineIntersect
// TODO: PointInBox
// TODO: Translate
// TODO: operator+
// TODO: operator=

}

