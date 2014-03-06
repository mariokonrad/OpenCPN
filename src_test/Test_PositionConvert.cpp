#include <gtest/gtest.h>
#include <global/Mock_System.h>
#include <global/OCPN.h>
#include <PositionConvert.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

class Test_PositionConvert : public ::testing::Test
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

TEST_F(Test_PositionConvert, latitude_to_string_low_prec_format_0)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 0;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("00 00.0 N"),   0.0000 },
		{ _T("01 00.0 N"),   1.0000 },
		{ _T("12 20.7 N"),  12.3456 },
		{ _T("89 00.0 N"),  89.0000 },
		{ _T("89 59.0 N"),  89.9830 },
		{ _T("89 59.9 N"),  89.9990 },
		{ _T("90 00.0 N"),  90.0000 },
		{ _T("00 00.0 N"),  -0.0000 },
		{ _T("01 00.0 S"),  -1.0000 },
		{ _T("12 20.7 S"), -12.3456 },
		{ _T("89 00.0 S"), -89.0000 },
		{ _T("89 59.0 S"), -89.9830 },
		{ _T("89 59.9 S"), -89.9990 },
		{ _T("90 00.0 S"), -90.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lat(tests[i].coordinate, false);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, latitude_to_string_high_prec_format_0)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 0;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("00 00.0000 N"),   0.0000 },
		{ _T("01 00.0000 N"),   1.0000 },
		{ _T("12 20.7360 N"),  12.3456 },
		{ _T("89 00.0000 N"),  89.0000 },
		{ _T("89 58.9800 N"),  89.9830 },
		{ _T("89 59.9400 N"),  89.9990 },
		{ _T("90 00.0000 N"),  90.0000 },
		{ _T("00 00.0000 N"),  -0.0000 },
		{ _T("01 00.0000 S"),  -1.0000 },
		{ _T("12 20.7360 S"), -12.3456 },
		{ _T("89 00.0000 S"), -89.0000 },
		{ _T("89 58.9800 S"), -89.9830 },
		{ _T("89 59.9400 S"), -89.9990 },
		{ _T("90 00.0000 S"), -90.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lat(tests[i].coordinate, true);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, latitude_to_string_low_prec_format_1)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 1;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T(  "0.0000"),   0.0000 },
		{ _T(  "1.0000"),   1.0000 },
		{ _T( "12.3456"),  12.3456 },
		{ _T( "89.0000"),  89.0000 },
		{ _T( "89.9830"),  89.9830 },
		{ _T( "89.9990"),  89.9990 },
		{ _T( "90.0000"),  90.0000 },
		{ _T( "-0.0000"),  -0.0000 },
		{ _T( "-1.0000"),  -1.0000 },
		{ _T("-12.3456"), -12.3456 },
		{ _T("-89.0000"), -89.0000 },
		{ _T("-89.9830"), -89.9830 },
		{ _T("-89.9990"), -89.9990 },
		{ _T("-90.0000"), -90.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lat(tests[i].coordinate, false);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, latitude_to_string_high_prec_format_1)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 1;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T(  "0.000000"),   0.0000 },
		{ _T(  "1.000000"),   1.0000 },
		{ _T( "12.345600"),  12.3456 },
		{ _T( "89.000000"),  89.0000 },
		{ _T( "89.983000"),  89.9830 },
		{ _T( "89.999000"),  89.9990 },
		{ _T( "90.000000"),  90.0000 },
		{ _T( "-0.000000"),  -0.0000 },
		{ _T( "-1.000000"),  -1.0000 },
		{ _T("-12.345600"), -12.3456 },
		{ _T("-89.000000"), -89.0000 },
		{ _T("-89.983000"), -89.9830 },
		{ _T("-89.999000"), -89.9990 },
		{ _T("-90.000000"), -90.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lat(tests[i].coordinate, true);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, latitude_to_string_low_prec_format_2)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 2;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("00 00 00.0 N"),   0.0000 },
		{ _T("01 00 00.0 N"),   1.0000 },
		{ _T("12 20 44.1 N"),  12.3456 },
		{ _T("89 00 00.0 N"),  89.0000 },
		{ _T("89 58 58.8 N"),  89.9830 },
		{ _T("89 59 56.3 N"),  89.9990 },
		{ _T("90 00 00.0 N"),  90.0000 },
		{ _T("00 00 00.0 N"),  -0.0000 },
		{ _T("01 00 00.0 S"),  -1.0000 },
		{ _T("12 20 44.1 S"), -12.3456 },
		{ _T("89 00 00.0 S"), -89.0000 },
		{ _T("89 58 58.8 S"), -89.9830 },
		{ _T("89 59 56.3 S"), -89.9990 },
		{ _T("90 00 00.0 S"), -90.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lat(tests[i].coordinate, false);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, latitude_to_string_high_prec_format_2)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 2;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("00 00 00.000 N"),   0.0000 },
		{ _T("01 00 00.000 N"),   1.0000 },
		{ _T("12 20 44.159 N"),  12.3456 },
		{ _T("89 00 00.000 N"),  89.0000 },
		{ _T("89 58 58.800 N"),  89.9830 },
		{ _T("89 59 56.399 N"),  89.9990 },
		{ _T("90 00 00.000 N"),  90.0000 },
		{ _T("00 00 00.000 N"),  -0.0000 },
		{ _T("01 00 00.000 S"),  -1.0000 },
		{ _T("12 20 44.159 S"), -12.3456 },
		{ _T("89 00 00.000 S"), -89.0000 },
		{ _T("89 58 58.800 S"), -89.9830 },
		{ _T("89 59 56.399 S"), -89.9990 },
		{ _T("90 00 00.000 S"), -90.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lat(tests[i].coordinate, true);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, longitude_to_string_low_prec_format_0)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 0;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("000 00.0 E"),    0.0000 },
		{ _T("001 00.0 E"),    1.0000 },
		{ _T("012 20.7 E"),   12.3456 },
		{ _T("089 00.0 E"),   89.0000 },
		{ _T("089 59.0 E"),   89.9830 },
		{ _T("089 59.9 E"),   89.9990 },
		{ _T("090 00.0 E"),   90.0000 },
		{ _T("179 00.0 E"),  179.0000 },
		{ _T("180 00.0 E"),  180.0000 },
		{ _T("000 00.0 E"),   -0.0000 },
		{ _T("001 00.0 W"),   -1.0000 },
		{ _T("012 20.7 W"),  -12.3456 },
		{ _T("089 00.0 W"),  -89.0000 },
		{ _T("089 59.0 W"),  -89.9830 },
		{ _T("089 59.9 W"),  -89.9990 },
		{ _T("090 00.0 W"),  -90.0000 },
		{ _T("179 00.0 W"), -179.0000 },
		{ _T("180 00.0 W"), -180.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lon(tests[i].coordinate, false);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, longitude_to_string_high_prec_format_0)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 0;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("000 00.0000 E"),    0.0000 },
		{ _T("001 00.0000 E"),    1.0000 },
		{ _T("012 20.7360 E"),   12.3456 },
		{ _T("089 00.0000 E"),   89.0000 },
		{ _T("089 58.9800 E"),   89.9830 },
		{ _T("089 59.9400 E"),   89.9990 },
		{ _T("090 00.0000 E"),   90.0000 },
		{ _T("179 00.0000 E"),  179.0000 },
		{ _T("180 00.0000 E"),  180.0000 },
		{ _T("000 00.0000 E"),   -0.0000 },
		{ _T("001 00.0000 W"),   -1.0000 },
		{ _T("012 20.7360 W"),  -12.3456 },
		{ _T("089 00.0000 W"),  -89.0000 },
		{ _T("089 58.9800 W"),  -89.9830 },
		{ _T("089 59.9400 W"),  -89.9990 },
		{ _T("090 00.0000 W"),  -90.0000 },
		{ _T("179 00.0000 W"), -179.0000 },
		{ _T("180 00.0000 W"), -180.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lon(tests[i].coordinate, true);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, longitude_to_string_low_prec_format_1)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 1;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T(   "0.0000"),    0.0000 },
		{ _T(   "1.0000"),    1.0000 },
		{ _T(  "12.3456"),   12.3456 },
		{ _T(  "89.0000"),   89.0000 },
		{ _T(  "89.9830"),   89.9830 },
		{ _T(  "89.9990"),   89.9990 },
		{ _T(  "90.0000"),   90.0000 },
		{ _T( "179.0000"),  179.0000 },
		{ _T( "180.0000"),  180.0000 },
		{ _T(  "-0.0000"),   -0.0000 },
		{ _T(  "-1.0000"),   -1.0000 },
		{ _T( "-12.3456"),  -12.3456 },
		{ _T( "-89.0000"),  -89.0000 },
		{ _T( "-89.9830"),  -89.9830 },
		{ _T( "-89.9990"),  -89.9990 },
		{ _T( "-90.0000"),  -90.0000 },
		{ _T("-179.0000"), -179.0000 },
		{ _T("-180.0000"), -180.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lon(tests[i].coordinate, false);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, longitude_to_string_high_prec_format_1)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 1;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T(   "0.000000"),    0.0000 },
		{ _T(   "1.000000"),    1.0000 },
		{ _T(  "12.345600"),   12.3456 },
		{ _T(  "89.000000"),   89.0000 },
		{ _T(  "89.983000"),   89.9830 },
		{ _T(  "89.999000"),   89.9990 },
		{ _T(  "90.000000"),   90.0000 },
		{ _T( "179.000000"),  179.0000 },
		{ _T( "180.000000"),  180.0000 },
		{ _T(  "-0.000000"),   -0.0000 },
		{ _T(  "-1.000000"),   -1.0000 },
		{ _T( "-12.345600"),  -12.3456 },
		{ _T( "-89.000000"),  -89.0000 },
		{ _T( "-89.983000"),  -89.9830 },
		{ _T( "-89.999000"),  -89.9990 },
		{ _T( "-90.000000"),  -90.0000 },
		{ _T("-179.000000"), -179.0000 },
		{ _T("-180.000000"), -180.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lon(tests[i].coordinate, true);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, longitude_to_string_low_prec_format_2)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 2;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("000 00 00.0 E"),    0.0000 },
		{ _T("001 00 00.0 E"),    1.0000 },
		{ _T("012 20 44.1 E"),   12.3456 },
		{ _T("089 00 00.0 E"),   89.0000 },
		{ _T("089 58 58.8 E"),   89.9830 },
		{ _T("089 59 56.3 E"),   89.9990 },
		{ _T("090 00 00.0 E"),   90.0000 },
		{ _T("179 00 00.0 E"),  179.0000 },
		{ _T("180 00 00.0 E"),  180.0000 },
		{ _T("000 00 00.0 E"),   -0.0000 },
		{ _T("001 00 00.0 W"),   -1.0000 },
		{ _T("012 20 44.1 W"),  -12.3456 },
		{ _T("089 00 00.0 W"),  -89.0000 },
		{ _T("089 58 58.8 W"),  -89.9830 },
		{ _T("089 59 56.3 W"),  -89.9990 },
		{ _T("090 00 00.0 W"),  -90.0000 },
		{ _T("179 00 00.0 W"), -179.0000 },
		{ _T("180 00 00.0 W"), -180.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lon(tests[i].coordinate, false);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, longitude_to_string_high_prec_format_2)
{
	global::System::Config config;
	test::global::Mock_System sys;

	config.SDMMFormat = 2;

	EXPECT_CALL(sys, config())
		.WillRepeatedly(ReturnRef(config));

	global::OCPN::get().inject(&sys);

	struct Test
	{
		wxString expected;
		double coordinate;
	};

	static const Test tests[] =
	{
		{ _T("000 00 00.000 E"),    0.0000 },
		{ _T("001 00 00.000 E"),    1.0000 },
		{ _T("012 20 44.159 E"),   12.3456 },
		{ _T("089 00 00.000 E"),   89.0000 },
		{ _T("089 58 58.800 E"),   89.9830 },
		{ _T("089 59 56.399 E"),   89.9990 },
		{ _T("090 00 00.000 E"),   90.0000 },
		{ _T("179 00 00.000 E"),  179.0000 },
		{ _T("180 00 00.000 E"),  180.0000 },
		{ _T("000 00 00.000 E"),   -0.0000 },
		{ _T("001 00 00.000 W"),   -1.0000 },
		{ _T("012 20 44.159 W"),  -12.3456 },
		{ _T("089 00 00.000 W"),  -89.0000 },
		{ _T("089 58 58.800 W"),  -89.9830 },
		{ _T("089 59 56.399 W"),  -89.9990 },
		{ _T("090 00 00.000 W"),  -90.0000 },
		{ _T("179 00 00.000 W"), -179.0000 },
		{ _T("180 00 00.000 W"), -180.0000 },
	};

	for (unsigned int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		wxString result = PositionConvert::lon(tests[i].coordinate, true);
		EXPECT_EQ(tests[i].expected, result);
	}
}

TEST_F(Test_PositionConvert, string_to_latitude)
{
	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("37°54.204' N")), 1.0e-4);
	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("N37 54 12")), 1.0e-4);
//	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("37°54'12")), 1.0e-4); // FIXME: disabled
	EXPECT_NEAR(37.9034, PositionConvert::lat(_T("37.9034")), 1.0e-4);
}

TEST_F(Test_PositionConvert, string_to_longitude)
{
//	EXPECT_NEAR(  37.9033,  PositionConvert::lon(_T("37°54'12")), 1.0e-4); // FIXME: disabled
	EXPECT_NEAR(  37.9033,  PositionConvert::lon(_T("37.9034")), 1.0e-4);
	EXPECT_NEAR(-122.31035, PositionConvert::lon(_T("122°18.621' W")), 1.0e-4);
	EXPECT_NEAR(-122.31035, PositionConvert::lon(_T("122w 18 37")), 1.0e-4);
	EXPECT_NEAR(-122.31035, PositionConvert::lon(_T("-122.31035")), 1.0e-4);
}

TEST_F(Test_PositionConvert, strings_to_position)
{
	geo::Position pos;

//	pos = PositionConvert::pos(_T("37°54.204' N"), _T("37°54'12")); // FIXME: disabled
//	EXPECT_NEAR(37.9034, pos.lat(), 1.0e-4);
//	EXPECT_NEAR(37.9033, pos.lon(), 1.0e-4);

	pos = PositionConvert::pos(_T("37°54.204' N"), _T("122°18.621' W"));
	EXPECT_NEAR(  37.9034,  pos.lat(), 1.0e-4);
	EXPECT_NEAR(-122.31035, pos.lon(), 1.0e-4);
}

}

