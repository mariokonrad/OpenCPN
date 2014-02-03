#include <gmock/gmock.h>
#include <global/ColorProvider.h>

namespace test {
namespace global {

class Mock_ColorProvider : public ::global::ColorProvider
{
public:
	MOCK_CONST_METHOD1(get_color, wxColour(const wxString&));
};

}}

