#include <gtest/gtest.h>
#include <gui/XMLStyleReader.h>
#include <gui/Style.h>

namespace {

using gui::Style;

class Test_XMLStyleReader : public ::testing::Test
{
public:
	void destroy(gui::StyleReader::Styles& styles)
	{
		for (gui::StyleReader::Styles::iterator i = styles.begin(); i != styles.end(); ++i)
			delete *i;
	}
};

// FIXME: unit test for gui/Icon
// FIXME: unit test for gui/Tool
// FIXME: unit test for gui/Style
// FIXME: more unit test for gui/XMLStyleReader

TEST_F(Test_XMLStyleReader, empty_data)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	bool rc = reader.read_data("", styles);

	EXPECT_FALSE(rc);
	EXPECT_TRUE(styles.empty());

	destroy(styles);
}

TEST_F(Test_XMLStyleReader, empty_styles)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	bool rc = reader.read_data("<styles></styles>", styles);

	EXPECT_TRUE(rc);
	EXPECT_TRUE(styles.empty());

	destroy(styles);
}

TEST_F(Test_XMLStyleReader, empty_styles_short)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	bool rc = reader.read_data("<styles />", styles);

	EXPECT_TRUE(rc);
	EXPECT_TRUE(styles.empty());

	destroy(styles);
}

TEST_F(Test_XMLStyleReader, empty_invalid_styles_tag)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	bool rc = reader.read_data("<stylse></stylse>", styles);

	EXPECT_TRUE(rc);
	EXPECT_TRUE(styles.empty());

	destroy(styles);
}

TEST_F(Test_XMLStyleReader, single_style_unnamed)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	bool rc = reader.read_data("<styles><style></style></styles>", styles);

	EXPECT_TRUE(rc);
	EXPECT_TRUE(styles.empty());

	destroy(styles);
}

TEST_F(Test_XMLStyleReader, single_style_named)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	bool rc = reader.read_data("<styles><style name=\"DemoStyle\"></style></styles>", styles);

	EXPECT_TRUE(rc);
	EXPECT_TRUE(styles.empty());

	destroy(styles);
}

TEST_F(Test_XMLStyleReader, single_style_graphics_file_noname)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	const char* data =
		"<styles>"
		" <style name=\"DemoStyle\">"
		"  <graphics-file />"
		" </style>"
		"</styles>";

	bool rc = reader.read_data(data, styles);

	EXPECT_TRUE(rc);
	EXPECT_EQ(1u, styles.size());

	EXPECT_EQ(_T("DemoStyle"), styles[0]->getName());
	EXPECT_EQ(_T(""), styles[0]->graphics_filename());

	destroy(styles);
}

TEST_F(Test_XMLStyleReader, single_style_graphics_file)
{
	gui::XMLStyleReader reader;
	gui::StyleReader::Styles styles;

	const char* data =
		"<styles>"
		" <style name=\"DemoStyle\">"
		"  <graphics-file name=\"icons.png\" />"
		" </style>"
		"</styles>";

	bool rc = reader.read_data(data, styles);

	EXPECT_TRUE(rc);
	EXPECT_EQ(1u, styles.size());

	EXPECT_EQ(_T("DemoStyle"), styles[0]->getName());
	EXPECT_EQ(_T("icons.png"), styles[0]->graphics_filename());

	destroy(styles);
}

}

