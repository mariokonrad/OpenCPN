#include <gtest/gtest.h>
#include <PositionConvert.h>

namespace {

class Test_PositionConvert : public ::testing::Test {};

/*
TEST_F(Test_PositionConvert, latitude_to_string_low_prec)
{
	EXPECT_TRUE(false); // TODO
}

TEST_F(Test_PositionConvert, latitude_to_string_high_prec)
{
	EXPECT_TRUE(false); // TODO
}

TEST_F(Test_PositionConvert, longitude_to_string_low_prec)
{
	EXPECT_TRUE(false); // TODO
}

TEST_F(Test_PositionConvert, longitude_to_string_high_prec)
{
	EXPECT_TRUE(false); // TODO
}
*/

TEST_F(Test_PositionConvert, string_to_latitude)
{
	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("37°54.204' N")), 1.0e-4);
	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("N37 54 12")), 1.0e-4);
	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("37°54'12")), 1.0e-4);
	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("37.9034")), 1.0e-4);
}

TEST_F(Test_PositionConvert, string_to_longitude)
{
	EXPECT_NEAR(  37.9033,  PositionConvert::lon(_T("37°54'12")), 1.0e-4);
	EXPECT_NEAR(  37.9033,  PositionConvert::lon(_T("37.9034")), 1.0e-4);
	EXPECT_NEAR(-122.31035, PositionConvert::lon(_T("122°18.621' W")), 1.0e-4);
	EXPECT_NEAR(-122.31035, PositionConvert::lon(_T("122w 18 37")), 1.0e-4);
	EXPECT_NEAR(-122.31035, PositionConvert::lon(_T("-122.31035")), 1.0e-4);
}

TEST_F(Test_PositionConvert, strings_to_position)
{
	geo::Position pos;

	pos = PositionConvert::pos(_T("37°54.204' N"), _T("37°54'12"));
	EXPECT_NEAR(37.9034, pos.lat(), 1.0e-4);
	EXPECT_NEAR(37.9034, pos.lon(), 1.0e-4);

	pos = PositionConvert::pos(_T("37°54.204' N"), _T("122°18.621' W"));
	EXPECT_NEAR(  37.9034,  pos.lat(), 1.0e-4);
	EXPECT_NEAR(-122.31035, pos.lon(), 1.0e-4);
}

}

