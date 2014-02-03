#include <gtest/gtest.h>
#include <navigation/AnchorDist.h>

namespace {

class Test_AnchorDist : public ::testing::Test {};

TEST_F(Test_AnchorDist, anchor_dist_fix_in_range)
{
	const double MIN = 20.0;
	const double MAX = 80.0;
	const double D   = 50.0;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(D, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, anchor_dist_fix_on_min)
{
	const double MIN = 20.0;
	const double MAX = 80.0;
	const double D   = MIN;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(MIN, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, anchor_dist_fix_on_max)
{
	const double MIN = 20.0;
	const double MAX = 80.0;
	const double D   = MAX;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(MAX, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, anchor_dist_fix_below_min)
{
	const double MIN = 20.0;
	const double MAX = 80.0;
	const double D   = MIN - 5.0;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(MIN, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, anchor_dist_fix_above_max)
{
	const double MIN = 20.0;
	const double MAX = 80.0;
	const double D   = MAX + 5.0;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(MAX, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, neg_anchor_dist_fix_in_range)
{
	const double MIN =  20.0;
	const double MAX =  80.0;
	const double D   = -50.0;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(D, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, neg_anchor_dist_fix_on_min)
{
	const double MIN =  20.0;
	const double MAX =  80.0;
	const double D   = -MIN;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(-MIN, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, neg_anchor_dist_fix_on_max)
{
	const double MIN =  20.0;
	const double MAX =  80.0;
	const double D   = -MAX;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(-MAX, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, neg_anchor_dist_fix_below_min)
{
	const double MIN =  20.0;
	const double MAX =  80.0;
	const double D   = -MIN - 5.0;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(D, result, 1.0e-4);
}

TEST_F(Test_AnchorDist, neg_anchor_dist_fix_above_max)
{
	const double MIN =  20.0;
	const double MAX =  80.0;
	const double D   = -MAX + 5.0;

	double result = navigation::AnchorDistFix(D, MIN, MAX);

	EXPECT_NEAR(D, result, 1.0e-4);
}

}

