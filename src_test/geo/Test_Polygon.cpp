#include <gtest/gtest.h>
#include <geo/Polygon.h>

namespace {

class Test_Polygon : public ::testing::Test {};

TEST_F(Test_Polygon, static_inside)
{
	static const geo::PointD polygon[] =
	{
		{ 0.0, 0.0 },
		{ 1.0, 0.0 },
		{ 1.0, 1.0 },
		{ 0.0, 1.0 },
	};

	const geo::PointD point = { 0.5, 0.5 };

	bool rc = geo::Polygon::inside(polygon, sizeof(polygon) / sizeof(polygon[0]), point);

	EXPECT_TRUE(rc);
}

TEST_F(Test_Polygon, static_outside)
{
	static const geo::PointD polygon[] =
	{
		{ 0.0, 0.0 },
		{ 1.0, 0.0 },
		{ 1.0, 1.0 },
		{ 0.0, 1.0 },
	};

	const geo::PointD point0 = {  0.5, -0.5 }; // underneath
	const geo::PointD point1 = {  0.5, +1.5 }; // above
	const geo::PointD point2 = { -0.5,  0.5 }; // left, with intersections
	const geo::PointD point3 = { -0.5,  1.5 }; // left, without intersections
	const geo::PointD point4 = {  1.5,  0.5 }; // right

	EXPECT_FALSE(geo::Polygon::inside(polygon, sizeof(polygon) / sizeof(polygon[0]), point0));
	EXPECT_FALSE(geo::Polygon::inside(polygon, sizeof(polygon) / sizeof(polygon[0]), point1));
	EXPECT_FALSE(geo::Polygon::inside(polygon, sizeof(polygon) / sizeof(polygon[0]), point2));
	EXPECT_FALSE(geo::Polygon::inside(polygon, sizeof(polygon) / sizeof(polygon[0]), point3));
	EXPECT_FALSE(geo::Polygon::inside(polygon, sizeof(polygon) / sizeof(polygon[0]), point4));
}

}

