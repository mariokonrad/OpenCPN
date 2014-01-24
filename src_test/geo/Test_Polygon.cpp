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

TEST_F(Test_Polygon, static_inside_float)
{
	static const geo::PointF polygon[] =
	{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f },
	};

	const geo::PointF point = { 0.5f, 0.5f };

	bool rc = geo::Polygon::insidef(polygon, sizeof(polygon) / sizeof(polygon[0]), point);

	EXPECT_TRUE(rc);
}

TEST_F(Test_Polygon, static_outside_float)
{
	static const geo::PointF polygon[] =
	{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f },
	};

	const geo::PointF point0 = {  0.5f, -0.5f }; // underneath
	const geo::PointF point1 = {  0.5f, +1.5f }; // above
	const geo::PointF point2 = { -0.5f,  0.5f }; // left, with intersections
	const geo::PointF point3 = { -0.5f,  1.5f }; // left, without intersections
	const geo::PointF point4 = {  1.5f,  0.5f }; // right

	EXPECT_FALSE(geo::Polygon::insidef(polygon, sizeof(polygon) / sizeof(polygon[0]), point0));
	EXPECT_FALSE(geo::Polygon::insidef(polygon, sizeof(polygon) / sizeof(polygon[0]), point1));
	EXPECT_FALSE(geo::Polygon::insidef(polygon, sizeof(polygon) / sizeof(polygon[0]), point2));
	EXPECT_FALSE(geo::Polygon::insidef(polygon, sizeof(polygon) / sizeof(polygon[0]), point3));
	EXPECT_FALSE(geo::Polygon::insidef(polygon, sizeof(polygon) / sizeof(polygon[0]), point4));
}

}

