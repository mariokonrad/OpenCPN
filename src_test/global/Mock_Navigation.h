#include <gmock/gmock.h>
#include <global/Navigation.h>

namespace test {
namespace global {

class Mock_Navigation : public ::global::Navigation
{
public: // data
	MOCK_CONST_METHOD0(get_data, const ::global::Navigation::Data&());
	MOCK_METHOD1(set_view_point, void(const geo::Position&));
	MOCK_METHOD1(set_position, void(const geo::Position&));
	MOCK_METHOD1(set_latitude, void(double));
	MOCK_METHOD1(set_longitude, void(double));
	MOCK_METHOD1(set_magn_var, void(double));
	MOCK_METHOD1(set_heading_true, void(double));
	MOCK_METHOD1(set_heading_magn, void(double));
	MOCK_METHOD1(set_speed_over_ground, void(double));
	MOCK_METHOD1(set_course_over_ground, void(double));
	MOCK_METHOD1(set_user_var, void(double));
	MOCK_METHOD1(set_CourseUp, void(bool));
	MOCK_METHOD1(set_COGAvgSec, void(int));
	MOCK_METHOD1(set_MagneticAPB, void(bool));

public: // route
	MOCK_CONST_METHOD0(route, const ::global::Navigation::Route&());
	MOCK_METHOD1(set_route_arrival_circle_radius, void(double));

public: // track
	MOCK_CONST_METHOD0(get_track, const ::global::Navigation::Track&());
	MOCK_METHOD1(set_TrackPrecision, void(long));
	MOCK_METHOD1(set_HighliteTracks, void(bool));
	MOCK_METHOD1(set_TrackDaily, void(bool));
	MOCK_METHOD1(set_TrackDeltaDistance, void(double));
	MOCK_METHOD1(set_PlanSpeed, void(double));

public: // anchor
	MOCK_CONST_METHOD0(anchor, const ::global::Navigation::Anchor&());
	MOCK_METHOD1(set_anchor_PointMinDist, void(double));
	MOCK_METHOD1(set_anchor_AlertOn1, void(bool));
	MOCK_METHOD1(set_anchor_AlertOn2, void(bool));
	MOCK_METHOD1(set_anchor_AWDefault, void(long));
	MOCK_METHOD1(set_anchor_AWMax, void(long));
	MOCK_METHOD1(set_anchor_AW1GUID, void(const wxString&));
	MOCK_METHOD1(set_anchor_AW2GUID, void(const wxString&));

public: // gps
	MOCK_CONST_METHOD0(gps, const ::global::Navigation::GPS&());
	MOCK_METHOD1(set_gps_valid, void(bool));
	MOCK_METHOD1(set_gps_SatsInView, void(int));
	MOCK_METHOD1(set_gps_SatValid, void(bool));
};

}}

