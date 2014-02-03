#include <gtest/gtest.h>
#include <global/Mock_ColorProvider.h>
#include <UserColors.h>

using ::testing::_;
using ::testing::Return;

namespace {

class Test_UserColors : public ::testing::Test {};

TEST_F(Test_UserColors, construction_default_values)
{
	UserColors c;

	const wxColour blue3 = c.get_color(_T("BLUE3"));

	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DAY, c.get_current());
	EXPECT_EQ(  0, blue3.Red());
	EXPECT_EQ(  0, blue3.Green());
	EXPECT_EQ(255, blue3.Blue());
}

TEST_F(Test_UserColors, construction_valid_schemes)
{
	UserColors c_day;
	UserColors c_dusk;
	UserColors c_night;

	c_day.set_current(global::GLOBAL_COLOR_SCHEME_DAY);
	c_dusk.set_current(global::GLOBAL_COLOR_SCHEME_DUSK);
	c_night.set_current(global::GLOBAL_COLOR_SCHEME_NIGHT);

	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DAY, c_day.get_current());
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DUSK, c_dusk.get_current());
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_NIGHT, c_night.get_current());
}

TEST_F(Test_UserColors, construction_scheme_invalid)
{
	UserColors c;

	c.set_current(global::GLOBAL_COLOR_SCHEME_NIGHT);
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_NIGHT, c.get_current());

	c.set_current(global::GLOBAL_COLOR_SCHEME_INVALID);
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DAY, c.get_current());
}

TEST_F(Test_UserColors, construction_scheme_rgb)
{
	UserColors c;

	c.set_current(global::GLOBAL_COLOR_SCHEME_NIGHT);
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_NIGHT, c.get_current());

	c.set_current(global::GLOBAL_COLOR_SCHEME_RGB);
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DAY, c.get_current());
}

TEST_F(Test_UserColors, construction_scheme_max)
{
	UserColors c;

	c.set_current(global::GLOBAL_COLOR_SCHEME_NIGHT);
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_NIGHT, c.get_current());

	c.set_current(global::GLOBAL_COLOR_SCHEME_MAX);
	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DAY, c.get_current());
}

TEST_F(Test_UserColors, construction_null_provider_injection)
{
	UserColors c;

	c.inject_chart_color_provider(NULL);

	const wxColour blue3 = c.get_color(_T("BLUE3"));

	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DAY, c.get_current());
	EXPECT_EQ(  0, blue3.Red());
	EXPECT_EQ(  0, blue3.Green());
	EXPECT_EQ(255, blue3.Blue());
}

TEST_F(Test_UserColors, construction_provider_injection)
{
	test::global::Mock_ColorProvider provider;
	UserColors c;

	EXPECT_CALL(provider, get_color(_))
		.Times(1)
		.WillOnce(Return(wxColour(12, 34, 56)));

	c.inject_chart_color_provider(&provider);
	wxColour blue3 = c.get_color(_T("BLUE3"));

	EXPECT_EQ(12, blue3.Red());
	EXPECT_EQ(34, blue3.Green());
	EXPECT_EQ(56, blue3.Blue());
}

TEST_F(Test_UserColors, construction_provider_injection_returning_invalid)
{
	test::global::Mock_ColorProvider provider;
	UserColors c;

	EXPECT_CALL(provider, get_color(_))
		.Times(1)
		.WillOnce(Return(wxColour()));

	c.inject_chart_color_provider(&provider);
	wxColour blue3 = c.get_color(_T("BLUE3"));

	EXPECT_EQ(  0, blue3.Red());
	EXPECT_EQ(  0, blue3.Green());
	EXPECT_EQ(255, blue3.Blue());
}

TEST_F(Test_UserColors, get_unknown_name)
{
	UserColors c;

	const wxColour blue3 = c.get_color(_T("foobar-abc"));

	EXPECT_EQ(global::GLOBAL_COLOR_SCHEME_DAY, c.get_current());
	EXPECT_EQ(128, blue3.Red());
	EXPECT_EQ(128, blue3.Green());
	EXPECT_EQ(128, blue3.Blue());
}

}

