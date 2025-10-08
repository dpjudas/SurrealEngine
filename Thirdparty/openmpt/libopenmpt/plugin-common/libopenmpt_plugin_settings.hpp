/*
 * libopenmpt_plugin_settings.hpp
 * ------------------------------
 * Purpose: libopenmpt plugin settings
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_PLUGIN_SETTINGS_HPP
#define LIBOPENMPT_PLUGIN_SETTINGS_HPP

#include <windows.h>

#include <optional>
#include <string>


namespace libopenmpt {
namespace plugin {


typedef void (*changed_func)();

struct libopenmpt_settings {
	bool no_default_format = true;
	int samplerate = 48000;
	int channels = 2;
	int mastergain_millibel = 0;
	int stereoseparation = 100;
	int use_amiga_resampler = 1;
	int amiga_filter_type = 0;
	int repeatcount = 0;
	int interpolationfilterlength = 8;
	int ramping = -1;
	int vis_allow_scroll = 1;
	std::optional<std::basic_string<TCHAR>> player_setting_name = std::nullopt;
	changed_func changed = nullptr;
};


class settings : public libopenmpt_settings {
private:
	std::basic_string<TCHAR> subkey;
protected:
	virtual void read_setting( const std::string & /* key */ , const std::basic_string<TCHAR> & key, int & val ) {
		HKEY regkey = HKEY();
		if ( RegOpenKeyEx( HKEY_CURRENT_USER, ( TEXT("Software\\libopenmpt\\") + subkey ).c_str(), 0, KEY_READ, &regkey ) == ERROR_SUCCESS ) {
			DWORD v = val;
			DWORD type = REG_DWORD;
			DWORD typesize = sizeof(v);
			if ( RegQueryValueEx( regkey, key.c_str(), NULL, &type, (BYTE *)&v, &typesize ) == ERROR_SUCCESS )
			{
				val = v;
			}
			RegCloseKey( regkey );
			regkey = HKEY();
		}
	}
	virtual void write_setting( const std::string & /* key */, const std::basic_string<TCHAR> & key, int val ) {
		HKEY regkey = HKEY();
		if ( RegCreateKeyEx( HKEY_CURRENT_USER, ( TEXT("Software\\libopenmpt\\") + subkey ).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regkey, NULL ) == ERROR_SUCCESS ) {
			DWORD v = val;
			DWORD type = REG_DWORD;
			DWORD typesize = sizeof(v);
			if ( RegSetValueEx( regkey, key.c_str(), 0, type, (const BYTE *)&v, typesize ) == ERROR_SUCCESS )
			{
				// ok
			}
			RegCloseKey( regkey );
			regkey = HKEY();
		}
	}
public:
	settings( const std::basic_string<TCHAR> & subkey_, bool no_default_format_, const std::optional<std::basic_string<TCHAR>> & player_setting_name_ = std::nullopt)
		: subkey(subkey_)
	{
		no_default_format = no_default_format_;
		player_setting_name = player_setting_name_;
	}
	void load()
	{
		#define read_setting(a,b,c) read_setting( b , TEXT(b) , c)
			read_setting( subkey, "Samplerate_Hz", samplerate );
			read_setting( subkey, "Channels", channels );
			read_setting( subkey, "MasterGain_milliBel", mastergain_millibel );
			read_setting( subkey, "StereoSeparation_Percent", stereoseparation );
			read_setting( subkey, "RepeatCount", repeatcount );
			read_setting( subkey, "InterpolationFilterLength", interpolationfilterlength );
			read_setting( subkey, "UseAmigaResampler", use_amiga_resampler );
			read_setting( subkey, "AmigaFilterType", amiga_filter_type );
			read_setting( subkey, "VolumeRampingStrength", ramping );
			read_setting( subkey, "VisAllowScroll", vis_allow_scroll );
		#undef read_setting
	}
	void save()
	{
		#define write_setting(a,b,c) write_setting( b , TEXT(b) , c)
			write_setting( subkey, "Samplerate_Hz", samplerate );
			write_setting( subkey, "Channels", channels );
			write_setting( subkey, "MasterGain_milliBel", mastergain_millibel );
			write_setting( subkey, "StereoSeparation_Percent", stereoseparation );
			write_setting( subkey, "RepeatCount", repeatcount );
			write_setting( subkey, "InterpolationFilterLength", interpolationfilterlength );
			write_setting( subkey, "UseAmigaResampler", use_amiga_resampler );
			write_setting( subkey, "AmigaFilterType", amiga_filter_type );
			write_setting( subkey, "VolumeRampingStrength", ramping );
			write_setting( subkey, "VisAllowScroll", vis_allow_scroll );
		#undef write_setting
	}
	virtual ~settings()
	{
		return;
	}
};


} // namespace plugin
} // namespace libopenmpt


#endif // LIBOPENMPT_PLUGIN_SETTINGS_HPP
