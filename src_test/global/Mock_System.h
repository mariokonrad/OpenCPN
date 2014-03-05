#include <gmock/gmock.h>
#include <global/System.h>

namespace test {
namespace global {

class Mock_System : public ::global::System
{
public: // data
	MOCK_CONST_METHOD0(data, const ::global::System::Data&());
	MOCK_METHOD1(set_home_location, void(const wxString&));
	MOCK_METHOD1(set_private_data_dir, void(const wxString&));
	MOCK_METHOD1(set_tc_data_dir, void(const wxString&));
	MOCK_METHOD1(set_config_file, void(const wxString&));
	MOCK_METHOD1(set_log_file, void(const wxString&));
	MOCK_METHOD1(set_sound_data_location, void(const wxString&));
	MOCK_METHOD1(set_world_map_location, void(const wxString&));
	MOCK_METHOD1(set_chartlist_fileame, void(const wxString&));
	MOCK_METHOD1(set_init_chart_dir, void(const wxString&));
	MOCK_METHOD1(set_plugin_dir, void(const wxString&));
	MOCK_METHOD1(set_locale, void(const wxString&));
	MOCK_METHOD1(set_csv_location, void(const wxString&));
	MOCK_METHOD1(set_SENCPrefix, void(const wxString&));
	MOCK_METHOD1(set_UserPresLibData, void(const wxString&));
	MOCK_METHOD1(set_exe_path, void(const wxString&));
	MOCK_METHOD1(set_current_tide_dataset, void(const std::vector<wxString>&));

public: // config
	MOCK_CONST_METHOD0(config, const ::global::System::Config&());
	MOCK_METHOD1(set_config_version_string, void(const wxString&));
	MOCK_METHOD1(set_config_nav_message_shown, void(bool));
	MOCK_METHOD1(set_config_memory_footprint, void(long));
	MOCK_METHOD1(set_config_autosave_interval_seconds, void(long));
	MOCK_METHOD1(set_config_CacheLimit, void(long));
	MOCK_METHOD1(set_config_memCacheLimit, void(long));
	MOCK_METHOD1(set_config_GPU_MemSize, void(long));
	MOCK_METHOD1(set_config_nmea_use_gll, void(bool));
	MOCK_METHOD1(set_config_SetSystemTime, void(bool));
	MOCK_METHOD1(set_config_PlayShipsBells, void(bool));
	MOCK_METHOD1(set_config_restore_stackindex, void(int));
	MOCK_METHOD1(set_config_restore_dbindex, void(int));
	MOCK_METHOD1(set_config_SDMMFormat, void(int));
	MOCK_METHOD1(set_config_DistanceFormat, void(int));
	MOCK_METHOD1(set_config_SpeedFormat, void(int));
	MOCK_METHOD1(set_config_COMPortCheck, void(int));
	MOCK_METHOD1(set_config_uploadConnection, void(const wxString&));
	MOCK_METHOD1(set_config_GPS_Ident, void(const wxString&));
	MOCK_METHOD1(set_config_GarminHostUpload, void(bool));
	MOCK_METHOD1(set_config_filter_cogsog, void(bool));
	MOCK_METHOD1(set_config_COGFilterSec, void(int));
	MOCK_METHOD1(set_config_SOGFilterSec, void(int));
	MOCK_METHOD1(set_config_portable, void(bool));
	MOCK_METHOD1(set_config_assume_azerty, void(bool));

public: // debug
	MOCK_CONST_METHOD0(debug, const ::global::System::Debug&());
	MOCK_METHOD1(set_debug_gdal, void(bool));
	MOCK_METHOD1(set_debug_nmea, void(long));
	MOCK_METHOD1(set_debug_ogl, void(bool));
	MOCK_METHOD1(set_debug_cm93, void(bool));
	MOCK_METHOD1(set_debug_s57, void(bool));
	MOCK_METHOD1(set_debug_total_NMEAerror_messages, void(int));
	MOCK_METHOD0(inc_debug_total_NMEAerror_messages, void());
};

}}

