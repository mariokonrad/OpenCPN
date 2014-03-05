#include <gtest/gtest.h>
#include <global/Mock_GUI.h>
#include <global/Mock_Navigation.h>
#include <global/OCPN.h>
#include <navigation/MagneticVariation.h>
#include <cmath>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

class Test_MagneticVariation : public ::testing::Test
{
public:
	virtual void SetUp()
	{
		global::OCPN::get().clear();
	}

	virtual void TearDown()
	{
		global::OCPN::get().clear();
	}
};

TEST_F(Test_MagneticVariation, invisible_mag)
{
	global::GUI::View view;
	test::global::Mock_GUI gui;

	view.ShowMag = false;

	EXPECT_CALL(gui, view())
		.Times(1)
		.WillOnce(ReturnRef(view));

	global::OCPN::get().inject(&gui);

	const double A = 123.456;

	double result = navigation::GetTrueOrMag(A);

	EXPECT_NEAR(A, result, 1.0e-4);
}

TEST_F(Test_MagneticVariation, real_var_above_360)
{
	global::GUI::View view;
	global::Navigation::Data nav_data;
	test::global::Mock_GUI gui;
	test::global::Mock_Navigation nav;

	view.ShowMag = true;
	nav_data.var = 5.0;

	EXPECT_CALL(gui, view())
		.Times(1)
		.WillOnce(ReturnRef(view));

	EXPECT_CALL(nav, get_data())
		.Times(1)
		.WillOnce(ReturnRef(nav_data));

	global::OCPN::get().inject(&gui);
	global::OCPN::get().inject(&nav);

	const double A = 373.456;

	double result = navigation::GetTrueOrMag(A);

	EXPECT_NEAR(A - 5.0 - 360.0, result, 1.0e-4);
}

TEST_F(Test_MagneticVariation, real_var_between_0_and_360)
{
	global::GUI::View view;
	global::Navigation::Data nav_data;
	test::global::Mock_GUI gui;
	test::global::Mock_Navigation nav;

	view.ShowMag = true;
	nav_data.var = 5.0;

	EXPECT_CALL(gui, view())
		.Times(1)
		.WillOnce(ReturnRef(view));

	EXPECT_CALL(nav, get_data())
		.Times(1)
		.WillOnce(ReturnRef(nav_data));

	global::OCPN::get().inject(&gui);
	global::OCPN::get().inject(&nav);

	const double A = 273.456;

	double result = navigation::GetTrueOrMag(A);

	EXPECT_NEAR(A - 5.0, result, 1.0e-4);
}

TEST_F(Test_MagneticVariation, real_var_below_0)
{
	global::GUI::View view;
	global::Navigation::Data nav_data;
	test::global::Mock_GUI gui;
	test::global::Mock_Navigation nav;

	view.ShowMag = true;
	nav_data.var = 5.0;

	EXPECT_CALL(gui, view())
		.Times(1)
		.WillOnce(ReturnRef(view));

	EXPECT_CALL(nav, get_data())
		.Times(1)
		.WillOnce(ReturnRef(nav_data));

	global::OCPN::get().inject(&gui);
	global::OCPN::get().inject(&nav);

	const double A = 3.456;

	double result = navigation::GetTrueOrMag(A);

	EXPECT_NEAR(A - 5.0 + 360.0, result, 1.0e-4);
}

TEST_F(Test_MagneticVariation, user_var_above_360)
{
	global::GUI::View view;
	global::Navigation::Data nav_data;
	test::global::Mock_GUI gui;
	test::global::Mock_Navigation nav;

	view.ShowMag = true;
	nav_data.var = NAN;
	nav_data.user_var = 8.0;

	EXPECT_CALL(gui, view())
		.Times(1)
		.WillOnce(ReturnRef(view));

	EXPECT_CALL(nav, get_data())
		.Times(1)
		.WillOnce(ReturnRef(nav_data));

	global::OCPN::get().inject(&gui);
	global::OCPN::get().inject(&nav);

	const double A = 373.456;

	double result = navigation::GetTrueOrMag(A);

	EXPECT_NEAR(A - 8.0 - 360.0, result, 1.0e-4);
}

TEST_F(Test_MagneticVariation, user_var_between_0_and_360)
{
	global::GUI::View view;
	global::Navigation::Data nav_data;
	test::global::Mock_GUI gui;
	test::global::Mock_Navigation nav;

	view.ShowMag = true;
	nav_data.var = NAN;
	nav_data.user_var = 8.0;

	EXPECT_CALL(gui, view())
		.Times(1)
		.WillOnce(ReturnRef(view));

	EXPECT_CALL(nav, get_data())
		.Times(1)
		.WillOnce(ReturnRef(nav_data));

	global::OCPN::get().inject(&gui);
	global::OCPN::get().inject(&nav);

	const double A = 273.456;

	double result = navigation::GetTrueOrMag(A);

	EXPECT_NEAR(A - 8.0, result, 1.0e-4);
}

TEST_F(Test_MagneticVariation, user_var_below_0)
{
	global::GUI::View view;
	global::Navigation::Data nav_data;
	test::global::Mock_GUI gui;
	test::global::Mock_Navigation nav;

	view.ShowMag = true;
	nav_data.var = NAN;
	nav_data.user_var = 8.0;

	EXPECT_CALL(gui, view())
		.Times(1)
		.WillOnce(ReturnRef(view));

	EXPECT_CALL(nav, get_data())
		.Times(1)
		.WillOnce(ReturnRef(nav_data));

	global::OCPN::get().inject(&gui);
	global::OCPN::get().inject(&nav);

	const double A = 3.456;

	double result = navigation::GetTrueOrMag(A);

	EXPECT_NEAR(A - 8.0 + 360.0, result, 1.0e-4);
}

}

