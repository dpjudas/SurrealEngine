/*
 * xmp-openmpt.cpp
 * ---------------
 * Purpose: libopenmpt xmplay input plugin implementation
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#if defined(__MINGW32__) && !defined(__MINGW64__)
#include <sys/types.h>
#endif

#if defined(MPT_WITH_MFC)
#include <afxwin.h>
#include <afxcmn.h>
#endif // MPT_WITH_MFC
#include <windows.h>
#include <windowsx.h>

#include <cctype>
#include <cstring>
#include <numeric>

#include <tchar.h>

#include "../libopenmpt.hpp"
#include "../libopenmpt_ext.hpp"

#include "../plugin-common/libopenmpt_plugin_settings.hpp"

#include "../plugin-common/libopenmpt_plugin_gui.hpp"

#if __has_include("svn_version.h")
#include "svn_version.h"
#else
#include "../../build/svn_version/svn_version.h"
#endif
#if defined(OPENMPT_VERSION_REVISION)
static const char * xmp_openmpt_string = "OpenMPT (" OPENMPT_API_VERSION_STRING "." OPENMPT_API_VERSION_STRINGIZE(OPENMPT_VERSION_REVISION) ")";
#else
static const char * xmp_openmpt_string = "OpenMPT (" OPENMPT_API_VERSION_STRING ")";
#endif

// XMPLAY expects a WINAPI (which is __stdcall) function using an undecorated symbol name which conflicts with the provided declaration.
#if defined(__GNUC__)
#define XMPIN_GetInterface XMPIN_GetInterface_Dummy
#endif
#include "xmplay/xmpin.h"
#if defined(__GNUC__)
#undef XMPIN_GetInterface
#endif

// Shortcut block assigned to the OpenMPT plugin by un4seen.
enum {
	openmpt_shortcut_first = 0x21000,
	openmpt_shortcut_tempo_decrease = openmpt_shortcut_first,
	openmpt_shortcut_tempo_increase,
	openmpt_shortcut_pitch_decrease,
	openmpt_shortcut_pitch_increase,
	openmpt_shortcut_switch_interpolation,
	openmpt_shortcut_last = 0x21fff,

	openmpt_shortcut_ex = 0x80000000,	// Use extended version of the shortcut callback
};

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <sstream>
#include <string>

#include <cmath>

#include <pugixml.hpp>

#define SHORT_TITLE "xmp-openmpt"
#define SHORTER_TITLE "openmpt"

static CRITICAL_SECTION xmpopenmpt_mutex;
class xmpopenmpt_lock {
public:
	xmpopenmpt_lock() {
		EnterCriticalSection( &xmpopenmpt_mutex );
	}
	~xmpopenmpt_lock() {
		LeaveCriticalSection( &xmpopenmpt_mutex );
	}
};

static XMPFUNC_IN * xmpfin = nullptr;
static XMPFUNC_MISC * xmpfmisc = nullptr;
static XMPFUNC_REGISTRY * xmpfregistry = nullptr;
static XMPFUNC_FILE * xmpffile = nullptr;
static XMPFUNC_TEXT * xmpftext = nullptr;
static XMPFUNC_STATUS * xmpfstatus = nullptr;

struct self_xmplay_t;

static self_xmplay_t * self = 0;

static void save_options();

static void apply_and_save_options();


static std::string convert_to_native( const std::string & str );

#if !defined(UNICODE)
static std::string StringEncode( const std::wstring &src, UINT codepage );
#endif

static std::wstring StringDecode( const std::string & src, UINT codepage );

#if defined(UNICODE)
static std::wstring StringToWINAPI( const std::wstring & src );
#else
static std::string StringToWINAPI( const std::wstring & src );
#endif

class xmp_openmpt_settings
 : public libopenmpt::plugin::settings
{
protected:
	void read_setting( const std::string & key, const std::basic_string<TCHAR> & keyW, int & val ) override {
		libopenmpt::plugin::settings::read_setting( key, keyW, val );
		int storedVal = 0;
		if ( xmpfregistry->GetInt( "OpenMPT", key.c_str(), &storedVal ) ) {
			val = storedVal;
		}
	}
	void write_setting( const std::string & key, const std::basic_string<TCHAR> & /* keyW */ , int val ) override {
		if ( !xmpfregistry->SetInt( "OpenMPT", key.c_str(), &val ) ) {
			// error
		}
		// ok
	}
public:
	xmp_openmpt_settings()
		: libopenmpt::plugin::settings(TEXT(SHORT_TITLE), false, TEXT("XMPlay output format"))
	{
		return;
	}
	virtual ~xmp_openmpt_settings() = default;
};

struct self_xmplay_t {
	std::vector<double> subsong_lengths;
	std::vector<std::string> subsong_names;
	std::size_t samplerate = 48000;
	std::size_t num_channels = 2;
	xmp_openmpt_settings settings;
	openmpt::module_ext * mod = nullptr;
	bool set_format_called = false;
	openmpt::ext::pattern_vis * pattern_vis = nullptr;
	std::int32_t tempo_factor = 0, pitch_factor = 0;
	bool single_subsong_mode = false;
	self_xmplay_t() {
		settings.changed = apply_and_save_options;
	}
	void on_new_mod() {
		set_format_called = false;
		self->pattern_vis = static_cast<openmpt::ext::pattern_vis *>( self->mod->get_interface( openmpt::ext::pattern_vis_id ) );
	}
	void delete_mod() {
		if ( mod ) {
			pattern_vis = 0;
			set_format_called = false;
			delete mod;
			mod = 0;
		}
	}
};

static std::string convert_to_native( const std::string & str ) {
	char * native_string = xmpftext->Utf8( str.c_str(), -1 );
	std::string result = native_string ? native_string : "";
	if ( native_string ) {
		xmpfmisc->Free( native_string );
		native_string = 0;
	}
	return result;
}

#if !defined(UNICODE)
static std::string StringEncode( const std::wstring &src, UINT codepage )
{
	int required_size = WideCharToMultiByte( codepage, 0, src.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if(required_size <= 0)
	{
		return std::string();
	}
	std::vector<CHAR> encoded_string( required_size );
	WideCharToMultiByte( codepage, 0, src.c_str(), -1, encoded_string.data(), encoded_string.size(), nullptr, nullptr);
	return encoded_string.data();
}
#endif

static std::wstring StringDecode( const std::string & src, UINT codepage )
{
	int required_size = MultiByteToWideChar( codepage, 0, src.c_str(), -1, nullptr, 0 );
	if(required_size <= 0)
	{
		return std::wstring();
	}
	std::vector<WCHAR> decoded_string( required_size );
	MultiByteToWideChar( codepage, 0, src.c_str(), -1, decoded_string.data(), decoded_string.size() );
	return decoded_string.data();
}

#if defined(UNICODE)

static std::wstring StringToWINAPI( const std::wstring & src )
{
	return src;
}

#else

static std::string StringToWINAPI( const std::wstring & src )
{
	return StringEncode( src, CP_ACP );
}

#endif

template <typename Tstring, typename Tstring2, typename Tstring3>
static inline Tstring StringReplace( Tstring str, const Tstring2 & oldStr_, const Tstring3 & newStr_ ) {
	std::size_t pos = 0;
	const Tstring oldStr = oldStr_;
	const Tstring newStr = newStr_;
	while ( ( pos = str.find( oldStr, pos ) ) != Tstring::npos ) {
		str.replace( pos, oldStr.length(), newStr );
		pos += newStr.length();
	}
	return str;
}

static std::string StringUpperCase( std::string str ) {
	std::transform( str.begin(), str.end(), str.begin(), []( char c ) { return static_cast<char>( std::toupper( c ) ); } );
	return str;
}

static std::string seconds_to_string( double time ) {
	if ( !std::isnormal( time ) ) {
		return "?";
	}
	std::int64_t time_ms = static_cast<std::int64_t>( time * 1000 );
	std::int64_t seconds = ( time_ms / 1000 ) % 60;
	std::int64_t minutes = ( time_ms / ( 1000 * 60 ) ) % 60;
	std::int64_t hours = ( time_ms / ( 1000 * 60 * 60 ) );
	std::ostringstream str;
	if ( hours > 0 ) {
		str << hours << ":";
	}
	str << std::setfill('0') << std::setw(2) << minutes;
	str << ":";
	str << std::setfill('0') << std::setw(2) << seconds;
	return str.str();
}

static void save_settings_to_map( std::map<std::string,int> & result, const libopenmpt::plugin::settings & s ) {
	result.clear();
	result[ "Samplerate_Hz" ] = s.samplerate;
	result[ "Channels" ] = s.channels;
	result[ "MasterGain_milliBel" ] = s.mastergain_millibel;
	result[ "StereoSeparation_Percent" ] = s.stereoseparation;
	result[ "RepeatCount" ] = s.repeatcount;
	result[ "InterpolationFilterLength" ] = s.interpolationfilterlength;
	result[ "UseAmigaResampler" ] = s.use_amiga_resampler;
	result[ "AmigaFilterType" ] = s.amiga_filter_type;
	result[ "VolumeRampingStrength" ] = s.ramping;
}

static inline void load_map_setting( const std::map<std::string,int> & map, const std::string & key, int & val ) {
	auto it = map.find( key );
	if ( it != map.end() ) {
		val = it->second;
	}
}

static void load_settings_from_map( libopenmpt::plugin::settings & s, const std::map<std::string,int> & map ) {
	load_map_setting( map, "Samplerate_Hz", s.samplerate );
	load_map_setting( map, "Channels", s.channels );
	load_map_setting( map, "MasterGain_milliBel", s.mastergain_millibel );
	load_map_setting( map, "StereoSeparation_Percent", s.stereoseparation );
	load_map_setting( map, "RepeatCount", s.repeatcount );
	load_map_setting( map, "InterpolationFilterLength", s.interpolationfilterlength );
	load_map_setting( map, "UseAmigaResampler", s.use_amiga_resampler );
	load_map_setting( map, "AmigaFilterType", s.amiga_filter_type );
	load_map_setting( map, "VolumeRampingStrength", s.ramping );
}

static void load_settings_from_xml( libopenmpt::plugin::settings & s, const std::string & xml ) {
	pugi::xml_document doc;
	doc.load_string( xml.c_str() );
	pugi::xml_node settings_node = doc.child( "settings" );
	std::map<std::string,int> map;
	for ( const auto & attr : settings_node.attributes() ) {
		map[ attr.name() ] = attr.as_int();
	}
	load_settings_from_map( s, map );
}

static void save_settings_to_xml( std::string & xml, const libopenmpt::plugin::settings & s ) {
	std::map<std::string,int> map;
	save_settings_to_map( map, s );
	pugi::xml_document doc;
	pugi::xml_node settings_node = doc.append_child( "settings" );
	for ( const auto & setting : map ) {
		settings_node.append_attribute( setting.first.c_str() ).set_value( setting.second );
	}
	std::ostringstream buf;
	doc.save( buf );
	xml = buf.str();
}

static void apply_options() {
	if ( self->mod ) {
		if ( !self->set_format_called ) {
			// SetFormat will only be called once after loading a file.
			// We cannot apply samplerate or numchannels changes afterwards during playback.
			self->samplerate = self->settings.samplerate;
			self->num_channels = self->settings.channels;
		}
		self->mod->set_repeat_count( self->settings.repeatcount );
		self->mod->set_render_param( openmpt::module::RENDER_MASTERGAIN_MILLIBEL, self->settings.mastergain_millibel );
		self->mod->set_render_param( openmpt::module::RENDER_STEREOSEPARATION_PERCENT, self->settings.stereoseparation );
		self->mod->set_render_param( openmpt::module::RENDER_INTERPOLATIONFILTER_LENGTH, self->settings.interpolationfilterlength );
		self->mod->set_render_param( openmpt::module::RENDER_VOLUMERAMPING_STRENGTH, self->settings.ramping );
		self->mod->ctl_set_boolean( "render.resampler.emulate_amiga", self->settings.use_amiga_resampler ? true : false );
		switch ( self->settings.amiga_filter_type ) {
			case 0:
				self->mod->ctl_set_text( "render.resampler.emulate_amiga_type", "auto" );
				break;
			case 1:
				self->mod->ctl_set_text( "render.resampler.emulate_amiga_type", "unfiltered" );
				break;
			case 0xA500:
				self->mod->ctl_set_text( "render.resampler.emulate_amiga_type", "a500" );
				break;
			case 0xA1200:
				self->mod->ctl_set_text( "render.resampler.emulate_amiga_type", "a1200" );
				break;
		}
	}
}

static void save_options() {
	self->settings.save();
}

static void apply_and_save_options() {
	apply_options();
	save_options();
}

static void reset_options() {
	self->settings = xmp_openmpt_settings();
	self->settings.changed = apply_and_save_options;
	self->settings.load();
}

// get config (return size of config data) (OPTIONAL)
static DWORD WINAPI openmpt_GetConfig( void * config ) {
	std::string xml;
	save_settings_to_xml( xml, self->settings );
	if ( config ) {
		std::memcpy( config, xml.c_str(), xml.length() + 1 );
	}
	return xml.length() + 1;
}

// apply config (OPTIONAL)
static void WINAPI openmpt_SetConfig( void * config, DWORD size ) {
	reset_options();
	if ( config ) {
		load_settings_from_xml( self->settings, std::string( (char*)config, (char*)config + size ) );
		apply_options();
	}
}

static void WINAPI ShortcutHandler( DWORD id ) {
	if ( !self->mod ) {
		return;
	}

	bool tempo_changed = false, pitch_changed = false;
	switch ( id ) {
	case openmpt_shortcut_tempo_decrease: self->tempo_factor--; tempo_changed = true; break;
	case openmpt_shortcut_tempo_increase: self->tempo_factor++; tempo_changed = true; break;
	case openmpt_shortcut_pitch_decrease: self->pitch_factor--; pitch_changed = true; break;
	case openmpt_shortcut_pitch_increase: self->pitch_factor++; pitch_changed = true; break;
	case openmpt_shortcut_switch_interpolation:
		self->settings.interpolationfilterlength *= 2;
		if ( self->settings.interpolationfilterlength > 8 ) {
			self->settings.interpolationfilterlength = 1;
		}
		apply_and_save_options();
		const char *s = nullptr;
		switch ( self->settings.interpolationfilterlength )
		{
		case 1: s = "Interpolation: Off"; break;
		case 2: s = "Interpolation: Linear"; break;
		case 4: s = "Interpolation: Cubic"; break;
		case 8: s = "Interpolation: Polyphase"; break;
		}
		if ( s ) {
			xmpfmisc->ShowBubble( s, 0 );
		}
		break;
	}
	
	self->tempo_factor = std::min( 48, std::max( -48, self->tempo_factor ) );
	self->pitch_factor = std::min( 48, std::max( -48, self->pitch_factor ) );
	const double tempo_factor = std::pow( 2.0, self->tempo_factor / 24.0 );
	const double pitch_factor = std::pow( 2.0, self->pitch_factor / 24.0 );

	if ( tempo_changed ) {
		std::ostringstream s;
		s << "Tempo: " << static_cast<std::int32_t>( 100.0 * tempo_factor ) << "%";
		xmpfmisc->ShowBubble( s.str().c_str(), 0 );
	} else if ( pitch_changed) {
		std::ostringstream s;
		s << "Pitch: ";
		if ( self->pitch_factor > 0 )
			s << "+";
		else if ( self->pitch_factor == 0 )
			s << "+/-";
		s << (self->pitch_factor * 0.5) << " semitones";
		xmpfmisc->ShowBubble( s.str().c_str(), 0 );
	}

	openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>( self->mod->get_interface( openmpt::ext::interactive_id ) );
	interactive->set_tempo_factor( tempo_factor );
	interactive->set_pitch_factor( pitch_factor );
	xmpfin->SetLength( static_cast<float>( self->mod->get_duration_seconds() / tempo_factor ), TRUE );
}


static double timeinfo_position = 0.0;
struct timeinfo {
	bool valid;
	double seconds;
	std::int32_t pattern;
	std::int32_t row;
};
static std::queue<timeinfo> timeinfos;
static void reset_timeinfos( double position = 0.0 ) {
	while ( !timeinfos.empty() ) {
		timeinfos.pop();
	}
	timeinfo_position = position;
}
static void update_timeinfos( std::int32_t samplerate, std::int32_t count ) {
	timeinfo_position += (double)count / (double)samplerate;
	timeinfo info;
	info.valid = true;
	info.seconds = timeinfo_position;
	info.pattern = self->mod->get_current_pattern();
	info.row = self->mod->get_current_row();
	timeinfos.push( info );
}

static timeinfo current_timeinfo;

static timeinfo lookup_timeinfo( double seconds ) {
	timeinfo info = current_timeinfo;
#if 0
	info.seconds = timeinfo_position;
	info.pattern = self->mod->get_current_pattern();
	info.row = self->mod->get_current_row();
#endif
	while ( timeinfos.size() > 0 && timeinfos.front().seconds <= seconds ) {
		info = timeinfos.front();
		timeinfos.pop();
	}
	current_timeinfo = info;
	return current_timeinfo;
}

static void clear_current_timeinfo() {
	current_timeinfo = timeinfo();
}

static void WINAPI openmpt_About( HWND win ) {
	std::ostringstream about;
	about << SHORT_TITLE << " version " << openmpt::string::get( "library_version" ) << " " << "(built " << openmpt::string::get( "build" ) << ")" << std::endl;
	about << " Copyright (c) 2013-2025 OpenMPT Project Developers and Contributors (https://lib.openmpt.org/)" << std::endl;
	about << " OpenMPT version " << openmpt::string::get( "core_version" ) << std::endl;
	about << std::endl;
	about << openmpt::string::get( "contact" ) << std::endl;
	std::ostringstream credits;
	credits << openmpt::string::get( "credits" );
	credits << "Additional thanks to:" << std::endl;
	credits << std::endl;
	credits << "Arseny Kapoulkine for pugixml" << std::endl;
	credits << "https://pugixml.org/" << std::endl;
	libopenmpt::plugin::gui_show_about( win, TEXT(SHORT_TITLE), StringReplace( StringToWINAPI( StringDecode( about.str(), CP_UTF8 ) ), TEXT("\n"), TEXT("\r\n") ), StringReplace( StringToWINAPI( StringDecode( credits.str(), CP_UTF8 ) ), TEXT("\n"), TEXT("\r\n") ) );
}

static void WINAPI openmpt_Config( HWND win ) {
	libopenmpt::plugin::gui_edit_settings( &self->settings, win, TEXT(SHORT_TITLE) );
	apply_and_save_options();
}

class xmplay_streambuf : public std::streambuf {
public:
	explicit xmplay_streambuf( XMPFILE & file );
private:
	xmplay_streambuf( const xmplay_streambuf & ) = delete;
	xmplay_streambuf & operator = ( const xmplay_streambuf & ) = delete;
protected:
	int_type underflow() override;
protected:
	XMPFILE & file;
	static inline constexpr std::size_t put_back = 4096;
	static inline constexpr std::size_t buf_size = 65536;
	std::vector<char> buffer;
}; // class xmplay_streambuf

xmplay_streambuf::xmplay_streambuf( XMPFILE & file_ ) : file(file_), buffer(buf_size) {
	setg( buffer.data(), buffer.data() + buffer.size(), buffer.data() + buffer.size() );
}

std::streambuf::int_type xmplay_streambuf::underflow() {
	if ( gptr() < egptr() ) {
		return traits_type::to_int_type( *gptr() );
	}
	std::size_t put_back_count = std::min( put_back, static_cast<std::size_t>( egptr() - buffer.data() ) );
	std::memmove( buffer.data(), egptr() - put_back_count, put_back_count );
	std::size_t readcount = xmpffile->Read( file, buffer.data() + put_back_count, buffer.size() - put_back_count );
	setg( buffer.data(), buffer.data() + put_back_count, buffer.data() + put_back_count + readcount );
	if ( readcount == 0 ) {
		return traits_type::eof();
	}
	return traits_type::to_int_type( *gptr() );
}

#include <fstream>
class xmplay_streambuf_seekable : public xmplay_streambuf {
public:
	explicit xmplay_streambuf_seekable( XMPFILE & file );
private:
	xmplay_streambuf_seekable( const xmplay_streambuf_seekable & ) = delete;
	xmplay_streambuf_seekable & operator = ( const xmplay_streambuf_seekable & ) = delete;
private:
	pos_type seekoff( off_type off, std::ios::seekdir dir, std::ios::openmode which ) override;
	pos_type seekpos( pos_type pos, std::ios::openmode which ) override;
}; // class xmplay_streambuf_seekable

xmplay_streambuf_seekable::xmplay_streambuf_seekable( XMPFILE & file )
	: xmplay_streambuf(file)
{
	return;
}

std::streambuf::pos_type xmplay_streambuf_seekable::seekoff( off_type off, std::ios::seekdir dir, std::ios::openmode which ) {
	if ( !(which & std::ios::in) ) {
		return pos_type(off_type(-1));
	}
	if ( (dir == std::ios::cur) && (off == 0) ) {
		// shortcut without invalidating buffer
		return xmpffile->Tell64( file ) - (egptr() - gptr());
	}
	if ( (dir == std::ios::beg) && (off == static_cast<std::int64_t>( xmpffile->Tell64( file ) ) - (egptr() - gptr())) ) {
		// shortcut without invalidating buffer
		return off;
	}
	switch ( dir ) {
		case std::ios::beg:
			if ( !xmpffile->Seek64( file, off ) ) {
				return pos_type(off_type(-1));
			}
			break;
		case std::ios::cur:
			if ( !xmpffile->Seek64( file, xmpffile->Tell64( file ) - (egptr() - gptr()) + off ) ) {
				return pos_type(off_type(-1));
			}
			break;
		case std::ios::end:
			if ( !xmpffile->Seek64( file, xmpffile->GetSize64( file ) + off ) ) {
				return pos_type(off_type(-1));
			}
			break;
		default:
			return pos_type(off_type(-1));
			break;
	}
	setg( buffer.data(), buffer.data() + buffer.size(), buffer.data() + buffer.size() );
	return xmpffile->Tell64( file );
}

std::streambuf::pos_type xmplay_streambuf_seekable::seekpos( pos_type pos, std::ios::openmode which ) {
	if ( !(which & std::ios::in) ) {
		return pos_type(off_type(-1));
	}
	if ( pos == static_cast<std::int64_t>( xmpffile->Tell64( file ) - (egptr() - gptr()) ) ) {
		// shortcut without invalidating buffer
		return pos;
	}
	if ( !xmpffile->Seek64( file, pos ) ) {
		return pos_type(off_type(-1));
	}
	setg( buffer.data(), buffer.data() + buffer.size(), buffer.data() + buffer.size() );
	return xmpffile->Tell64( file );
}

class xmplay_istream : public std::istream {
private:
	xmplay_streambuf buf;
private:
	xmplay_istream( const xmplay_istream & ) = delete;
	xmplay_istream & operator = ( const xmplay_istream & ) = delete;
public:
	xmplay_istream( XMPFILE & file ) : std::istream(&buf), buf(file) {
		return;
	}
}; // class xmplay_istream

class xmplay_istream_seekable : public std::istream {
private:
	xmplay_streambuf_seekable buf;
private:
	xmplay_istream_seekable( const xmplay_istream_seekable & ) = delete;
	xmplay_istream_seekable & operator = ( const xmplay_istream_seekable & ) = delete;
public:
	xmplay_istream_seekable( XMPFILE & file ) : std::istream(&buf), buf(file) {
		return;
	}
}; // class xmplay_istream_seekable


// Stream for memory-based files (required for could_open_probability)
struct xmplay_membuf : std::streambuf {
	xmplay_membuf( const char * base, size_t size ) {
		char* p( const_cast<char *>( base ) );
		setg(p, p, p + size);
	}
};

struct xmplay_imemstream : virtual xmplay_membuf, std::istream {
	xmplay_imemstream( const char * base, size_t size )
		: xmplay_membuf( base, size )
		, std::istream( static_cast<std::streambuf *>(this)) {
			return;
	}
};

static std::string string_replace( std::string str, const std::string & oldStr, const std::string & newStr ) {
	std::size_t pos = 0;
	while((pos = str.find(oldStr, pos)) != std::string::npos)
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
	return str;
}

static void write_xmplay_string( char * dst, std::string src ) {
	// xmplay buffers are ~40kB, be conservative and truncate at 32kB-2
	if ( !dst ) {
		return;
	}
	src = src.substr( 0, (1<<15) - 2 );
	std::strcpy( dst, src.c_str() );
}

static std::string extract_date( const openmpt::module & mod ) {
	std::string result = mod.get_metadata("date");
	if ( result.empty() ) {
		// Search the sample, instrument and message texts for possible release years.
		// We'll look for things that may vaguely resemble a release year, such as 4-digit numbers
		// or 2-digit numbers with a leading apostrophe. Generally, only years between
		// 1988 (release of Ultimate SoundTracker) and the current year + 1 (safety margin) will
		// be considered.
		std::string s = " " + mod.get_metadata("message");
		auto names = mod.get_sample_names();
		for ( const auto & name : names ) {
			s += " " + name;
		}
		names = mod.get_instrument_names();
		for ( const auto & name : names ) {
			s += " " + name;
		}
		s += " ";

		int32_t best_year = 0;

		SYSTEMTIME time;
		GetSystemTime( &time );
		const int32_t current_year = time.wYear + 1;

#define MPT_NUMERIC( x ) ( ( x >= '0' ) && ( x <= '9' ) )
		for ( auto i = s.cbegin(); i != s.cend(); ++i ) {
			std::size_t len = s.length();
			std::size_t idx = i - s.begin();
			std::size_t remaining = len - idx;
			if ( ( remaining >= 6 ) && !MPT_NUMERIC( i[0] ) && MPT_NUMERIC( i[1] ) && MPT_NUMERIC( i[2] ) && MPT_NUMERIC( i[3] ) && MPT_NUMERIC( i[4] ) && !MPT_NUMERIC( i[5] ) ) {
				// Four-digit year
				const int32_t year = ( i[1] - '0' ) * 1000 + ( i[2] - '0' ) * 100 + ( i[3] - '0' ) * 10 + ( i[4] - '0' );
				if ( year >= 1988 && year <= current_year ) {
					best_year = std::max( year, best_year );
				}
			} else if ( ( remaining >= 4 ) && ( i[0] == '\'' ) && MPT_NUMERIC( i[1] ) && MPT_NUMERIC( i[2] ) && !MPT_NUMERIC( i[3] ) ) {
				// Apostrophe + two-digit year
				const int32_t year = ( i[1] - '0' ) * 10 + ( i[2] - '0' );
				if ( year >= 88 && year <= 99 ) {
					best_year = std::max( 1900 + year, best_year );
				} else if ( year >= 00 && ( 2000 + year ) <= current_year ) {
					best_year = std::max( 2000 + year, best_year );
				}
			}
		}
#undef MPT_NUMERIC

		if ( best_year != 0 ) {
			std::ostringstream os;
			os << best_year;
			result = os.str();
		}
	}

	return result;
}

static void append_xmplay_tag( std::string & tags, const std::string & tag, const std::string & val ) {
	if ( tag.empty() ) {
		return;
	}
	if ( val.empty() ) {
		return;
	}
	tags.append( tag );
	tags.append( 1, '\0' );
	tags.append( val );
	tags.append( 1, '\0' );
}

static char * build_xmplay_tags( const openmpt::module & mod, int32_t subsong = -1 ) {
	std::string tags;
	const std::string title = mod.get_metadata("title");

	const auto subsong_names = mod.get_subsong_names();
	auto first_subsong = subsong_names.cbegin(), last_subsong = subsong_names.cend();
	if ( subsong >= 0 && static_cast<size_t>( subsong ) < subsong_names.size() ) {
		first_subsong += subsong;
		last_subsong = first_subsong + 1;
	}

	for ( auto subsong_name = first_subsong; subsong_name != last_subsong; subsong_name++ ) {
		append_xmplay_tag( tags, "filetype", convert_to_native( StringUpperCase( mod.get_metadata( "type" ) ) ) );
		append_xmplay_tag( tags, "title", convert_to_native( ( subsong_name->empty() || subsong == -1 || subsong_names.size() == 1 ) ? title : *subsong_name ) );
		append_xmplay_tag( tags, "artist", convert_to_native( mod.get_metadata( "artist" ) ) );
		append_xmplay_tag( tags, "album", convert_to_native( mod.get_metadata( "xmplay-album" ) ) );  // todo, libopenmpt does not support that
		append_xmplay_tag( tags, "date", convert_to_native( extract_date( mod ) ) );
		append_xmplay_tag( tags, "track", convert_to_native( mod.get_metadata( "xmplay-tracknumber" ) ) );  // todo, libopenmpt does not support that
		append_xmplay_tag( tags, "genre", convert_to_native( mod.get_metadata( "xmplay-genre" ) ) );        // todo, libopenmpt does not support that
		append_xmplay_tag( tags, "comment", convert_to_native( mod.get_metadata( "message" ) ) );
		tags.append( 1, '\0' );
	}
	char * result = static_cast<char*>( xmpfmisc->Alloc( tags.size() ) );
	if ( !result ) {
		return nullptr;
	}
	std::copy( tags.data(), tags.data() + tags.size(), result );
	return result;
}

static std::vector<double> build_subsong_lengths( openmpt::module & mod ) {
	std::int32_t num_subsongs = mod.get_num_subsongs();
	std::vector<double> subsong_lengths( num_subsongs );
	for ( std::int32_t i = 0; i < num_subsongs; ++i ) {
		mod.select_subsong( i );
		subsong_lengths[i] = mod.get_duration_seconds();
	}
	return subsong_lengths;
}

static float * build_xmplay_length( openmpt::module & mod ) {
	const auto subsong_lengths = build_subsong_lengths( mod );
	float * result = static_cast<float*>( xmpfmisc->Alloc( sizeof( float ) * subsong_lengths.size() ) );
	if ( !result ) {
		return nullptr;
	}
	for ( std::size_t i = 0; i < subsong_lengths.size(); ++i ) {
		result[i] = static_cast<float>( subsong_lengths[i] );
	}
	return result;
}

static DWORD build_xmplay_file_info( openmpt::module & mod, float ** length, char ** tags ) {
	if ( length ) {
		*length = build_xmplay_length( mod );
	}
	if ( tags ) {
		*tags = build_xmplay_tags( mod );
	}
	return static_cast<DWORD>( mod.get_num_subsongs() );
}

static void clear_xmplay_string( char * str ) {
	if ( !str ) {
		return;
	}
	str[0] = '\0';
}

static std::string sanitize_xmplay_info_string( const std::string & str ) {
	std::string result;
	result.reserve(str.size());
	for ( auto c : str ) {
		switch ( c ) {
			case '\0':
			case '\t':
			case '\r':
			case '\n':
				break;
			default:
				result.push_back( c );
				break;
		}
	}
	return result;
}

static std::string sanitize_xmplay_multiline_info_string( const std::string & str ) {
	std::string result;
	result.reserve(str.size());
	for ( auto c : str ) {
		switch ( c ) {
			case '\0':
			case '\t':
			case '\r':
				break;
			case '\n':
				result.push_back( '\r' );
				result.push_back( '\t' );
				break;
			default:
				result.push_back( c );
				break;
		}
	}
	return result;
}

static std::string sanitize_xmplay_multiline_string( const std::string & str ) {
	std::string result;
	result.reserve(str.size());
	for ( auto c : str ) {
		switch ( c ) {
			case '\0':
			case '\t':
				break;
			default:
				result.push_back( c );
				break;
		}
	}
	return result;
}

// check if a file is playable by this plugin
// more thorough checks can be saved for the GetFileInfo and Open functions
static BOOL WINAPI openmpt_CheckFile( const char * filename, XMPFILE file ) {
	static_cast<void>( filename );
	try {
		switch ( xmpffile->GetType( file ) ) {
			case XMPFILE_TYPE_MEMORY:
				{
					xmplay_imemstream s( reinterpret_cast<const char *>( xmpffile->GetMemory( file ) ), xmpffile->GetSize( file ) );
					return ( openmpt::probe_file_header( openmpt::probe_file_header_flags_default2, s ) == openmpt::probe_file_header_result_success ) ? TRUE : FALSE;
				}
				break;
			case XMPFILE_TYPE_FILE:
			case XMPFILE_TYPE_NETFILE:
				{
					if ( xmpfmisc->GetVersion() >= 0x03080200 ) {
						xmplay_istream_seekable s( file );
						return ( openmpt::probe_file_header( openmpt::probe_file_header_flags_default2, s ) == openmpt::probe_file_header_result_success ) ? TRUE : FALSE;
					} else {
						xmplay_istream s( file );
						return ( openmpt::probe_file_header( openmpt::probe_file_header_flags_default2, s ) == openmpt::probe_file_header_result_success ) ? TRUE : FALSE;
					}
				}
				break;
			case XMPFILE_TYPE_NETSTREAM:
			default:
				{
					xmplay_istream s( file );
					return ( openmpt::probe_file_header( openmpt::probe_file_header_flags_default2, s ) == openmpt::probe_file_header_result_success ) ? TRUE : FALSE;
				}
				break;
		}
	} catch ( ... ) {
		return FALSE;
	}
	return FALSE;
}

static DWORD WINAPI openmpt_GetFileInfo( const char * filename, XMPFILE file, float * * length, char * * tags ) {
	static_cast<void>( filename );
	DWORD subsongs = 0;
	try {
		std::map< std::string, std::string > ctls
		{
			{ "load.skip_plugins", "1" },
			{ "load.skip_samples", "1" },
		};
		switch ( xmpffile->GetType( file ) ) {
			case XMPFILE_TYPE_MEMORY:
				{
					openmpt::module mod( xmpffile->GetMemory( file ), xmpffile->GetSize( file ), std::clog, ctls );
					subsongs = build_xmplay_file_info( mod, length, tags );
				}
				break;
			case XMPFILE_TYPE_FILE:
			case XMPFILE_TYPE_NETFILE:
				{
					if ( xmpfmisc->GetVersion() >= 0x03080200 ) {
						xmplay_istream_seekable s( file );
						openmpt::module mod( s, std::clog, ctls );
						subsongs = build_xmplay_file_info( mod, length, tags );
					} else {
						xmplay_istream s( file );
						openmpt::module mod( s, std::clog, ctls );
						subsongs = build_xmplay_file_info( mod, length, tags );
					}
				}
				break;
			case XMPFILE_TYPE_NETSTREAM:
			default:
				{
					xmplay_istream s( file );
					openmpt::module mod( s, std::clog, ctls );
					subsongs = build_xmplay_file_info( mod, length, tags );
				}
				break;
		}
	} catch ( ... ) {
		if ( length ) *length = nullptr;
		if ( tags ) *tags = nullptr;
		return 0;
	}
	return subsongs;
}

// open a file for playback
// return:  0=failed, 1=success, 2=success and XMPlay can close the file
static DWORD WINAPI openmpt_Open( const char * filename, XMPFILE file ) {
	static_cast<void>( filename );
	xmpopenmpt_lock guard;
	reset_options();
	try {
		std::map< std::string, std::string > ctls
		{
			{ "play.at_end", "continue" },
		};
		self->delete_mod();
		switch ( xmpffile->GetType( file ) ) {
			case XMPFILE_TYPE_MEMORY:
				self->mod = new openmpt::module_ext( xmpffile->GetMemory( file ), xmpffile->GetSize( file ), std::clog, ctls );
				break;
			case XMPFILE_TYPE_FILE:
			case XMPFILE_TYPE_NETFILE:
				{
					if ( xmpfmisc->GetVersion() >= 0x03080200 ) {
						xmplay_istream_seekable s( file );
						self->mod = new openmpt::module_ext( s, std::clog, ctls );
					} else {
						xmplay_istream s( file );
						self->mod = new openmpt::module_ext( s, std::clog, ctls );
					}
				}
				break;
			case XMPFILE_TYPE_NETSTREAM:
			default:
				{
					xmplay_istream s( file );
					self->mod = new openmpt::module_ext( s, std::clog, ctls );
				}
				break;
		}
		self->on_new_mod();
		clear_current_timeinfo();
		reset_timeinfos();
		apply_options();

		self->subsong_lengths = build_subsong_lengths( *self->mod );
		self->subsong_names = self->mod->get_subsong_names();
		self->mod->select_subsong( 0 );
		self->tempo_factor = 0;
		self->pitch_factor = 0;

		xmpfin->SetLength( static_cast<float>( self->subsong_lengths[0] ), TRUE );
		return 2;
	} catch ( ... ) {
		self->delete_mod();
		return 0;
	}
	return 0;
}

// close the file
static void WINAPI openmpt_Close() {
	xmpopenmpt_lock guard;
	self->delete_mod();
}

// set the sample format (in=user chosen format, out=file format if different)
static void WINAPI openmpt_SetFormat( XMPFORMAT * form ) {
	if ( !form ) {
		return;
	}
	// SetFormat will only be called once after loading a file.
	// We cannot apply samplerate or numchannels changes afterwards during playback.
	self->set_format_called = true;
	if ( !self->mod ) {
		form->rate = 0;
		form->chan = 0;
		form->res = 0;
		form->chanmask = 0;
		return;
	}
	if ( self->settings.samplerate != 0 ) {
		form->rate = self->samplerate;
	} else {
		if ( form->rate > 0 ) {
			self->samplerate = form->rate;
		} else {
			form->rate = 48000;
			self->samplerate = 48000;
		}
	}
	if ( self->settings.channels != 0 ) {
		form->chan = self->num_channels;
	} else {
		if ( form->chan > 0 ) {
			if ( form->chan > 2 ) {
				form->chan = 4;
				self->num_channels = 4;
			} else {
				self->num_channels = form->chan;
			}
		} else {
			form->chan = 2;
			self->num_channels = 2;
		}
	}
	form->res = 4; // float
	form->chanmask = 0;
}

// get the tags
static char * WINAPI openmpt_GetTags() {
	if ( !self->mod ) {
		char * tags = static_cast<char*>( xmpfmisc->Alloc( 1 ) );
		tags[0] = '\0';
		return tags;
	}
	return build_xmplay_tags( *self->mod, std::max( 0, self->mod->get_selected_subsong() ) );
}

// get the main panel info text
static void WINAPI openmpt_GetInfoText( char * format, char * length ) {
	if ( !self->mod ) {
		clear_xmplay_string( format );
		clear_xmplay_string( length );
		return;
	}
	if ( format ) {
		std::ostringstream str;
		str
			<< StringUpperCase( self->mod->get_metadata("type") )
			<< " - "
			<< self->mod->get_num_channels() << " ch"
			<< " - "
			<< "(via " << SHORTER_TITLE << ")"
			;
		write_xmplay_string( format, sanitize_xmplay_info_string( str.str() ) );
	}
	if ( length ) {
		std::ostringstream str;
		str
			<< length
			<< " - "
			<< self->mod->get_num_orders() << " orders"
			;
		write_xmplay_string( length, sanitize_xmplay_info_string( str.str() ) );
	}
}

// get text for "General" info window
// separate headings and values with a tab (\t), end each line with a carriage-return (\r)
static void WINAPI openmpt_GetGeneralInfo( char * buf ) {
	if ( !self->mod ) {
		clear_xmplay_string( buf );
		return;
	}
	std::ostringstream str;
	str << "\r";
	bool metadatainfo = false;
	if ( !self->mod->get_metadata("artist").empty() ) {
		metadatainfo = true;
		str << "Artist" << "\t"  << sanitize_xmplay_info_string( self->mod->get_metadata("artist") ) << "\r";
	}
	const std::string date = extract_date( *self->mod );
	if ( !date.empty() ) {
		metadatainfo = true;
		str << "Date" << "\t"  << sanitize_xmplay_info_string( date ) << "\r";
	}
	if ( metadatainfo ) {
		str << "\r";
	}
	str << "Format" << "\t" << sanitize_xmplay_info_string( self->mod->get_metadata("type") ) << " (" << sanitize_xmplay_info_string( self->mod->get_metadata("type_long") ) << ")" << "\r";
	if ( !self->mod->get_metadata("originaltype").empty() ) {
		str << "Original Type" << "\t"  << sanitize_xmplay_info_string( self->mod->get_metadata("originaltype") ) << " (" << sanitize_xmplay_info_string( self->mod->get_metadata("originaltype_long") ) << ")" << "\r";
	}
	if ( !self->mod->get_metadata("container").empty() ) {
		str << "Container" << "\t"  << sanitize_xmplay_info_string( self->mod->get_metadata("container") ) << " (" << sanitize_xmplay_info_string( self->mod->get_metadata("container_long") ) << ")" << "\r";
	}
	str
		<< "Channels" << "\t" << self->mod->get_num_channels() << "\r"
		<< "Orders" << "\t" << self->mod->get_num_orders() << "\r"
		<< "Patterns" << "\t" << self->mod->get_num_patterns() << "\r";
	if ( self->mod->get_num_instruments() != 0 ) {
		str << "Instruments" << "\t" << self->mod->get_num_instruments() << "\r";
	}
	str << "Samples" << "\t" << self->mod->get_num_samples() << "\r";

	if( !self->single_subsong_mode && self->subsong_lengths.size() > 1 ) {
		for ( std::size_t i = 0; i < self->subsong_lengths.size(); ++i ) {
			str << ( i == 0 ? "Subsongs\t" : "\t" ) << (i + 1) << ". " << seconds_to_string( self->subsong_lengths[i] ) << " " << self->subsong_names[i] << "\r";
		}
	}

	str
		<< "\r"
		<< "Tracker" << "\t" << sanitize_xmplay_info_string( self->mod->get_metadata("tracker") ) << "\r"
		<< "Player" << "\t" << "xmp-openmpt" << " version " << openmpt::string::get( "library_version" ) << "\r"
		;
	std::string warnings = self->mod->get_metadata("warnings");
	if ( !warnings.empty() ) {
		str << "Warnings" << "\t" << sanitize_xmplay_multiline_info_string( warnings ) << "\r";
	}
	str << "\r";
	write_xmplay_string( buf, str.str() );
}

// get text for "Message" info window
// separate tag names and values with a tab (\t), and end each line with a carriage-return (\r)
static void WINAPI openmpt_GetMessage( char * buf ) {
	if ( !self->mod ) {
		clear_xmplay_string( buf );
		return;
	}
	write_xmplay_string( buf, convert_to_native( sanitize_xmplay_multiline_string( string_replace( self->mod->get_metadata("message"), "\n", "\r" ) ) ) );
}

// Seek to a position (in granularity units)
// return the new position in seconds (-1 = failed)
static double WINAPI openmpt_SetPosition( DWORD pos ) {
	if ( !self->mod ) {
		return -1.0;
	}
	if ( pos == static_cast<DWORD>(static_cast<LONG>(XMPIN_POS_LOOP)) ) {
		// If the time of the loop start position is known, that should be returned, otherwise -2 can be returned to let the time run on.
		// There is currently no way to easily figure out at which time the loop restarts.
		return -2;
	} else if ( pos == static_cast<DWORD>(static_cast<LONG>(XMPIN_POS_AUTOLOOP)) ) {
		// In the auto-looping case, the function should only loop when a loop has been detected, and otherwise return -1
		// If the time of the loop start position is known, that should be returned, otherwise -2 can be returned to let the time run on.
		// There is currently no way to easily figure out at which time the loop restarts.
		return -2;
	}
	if ( pos & XMPIN_POS_SUBSONG ) {
		self->single_subsong_mode = ( pos & XMPIN_POS_SUBSONG1 ) != 0;
		const int32_t subsong = pos & 0xffff;
		try {
			self->mod->select_subsong( subsong );
		} catch ( ... ) {
			return 0.0;
		}
		openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>( self->mod->get_interface( openmpt::ext::interactive_id ) );
		xmpfin->SetLength( static_cast<float>( self->subsong_lengths[ subsong ] / interactive->get_tempo_factor() ), TRUE );
		xmpfin->UpdateTitle( nullptr );
		reset_timeinfos( 0 );
		return 0.0;
	}
	double new_position = self->mod->set_position_seconds( static_cast<double>( pos ) * 0.001 );
	reset_timeinfos( new_position );
	return new_position;
}

// Get the seeking granularity in seconds
static double WINAPI openmpt_GetGranularity() {
	return 0.001;
}

// get some sample data, always floating-point
// count=number of floats to write (not bytes or samples)
// return number of floats written. if it's less than requested, playback is ended...
// so wait for more if there is more to come (use CheckCancel function to check if user wants to cancel)
static DWORD WINAPI openmpt_Process( float * dstbuf, DWORD count ) {
	xmpopenmpt_lock guard;
	if ( !self->mod || self->num_channels == 0 ) {
		return 0;
	}
	update_timeinfos( self->samplerate, 0 );
	std::size_t frames = count / self->num_channels;
	std::size_t frames_to_render = frames;
	std::size_t frames_rendered = 0;
	while ( frames_to_render > 0 ) {
		std::size_t frames_chunk = std::min( frames_to_render, static_cast<std::size_t>( ( self->samplerate + 99 ) / 100 ) ); // 100 Hz timing info update interval
		switch ( self->num_channels ) {
		case 1:
			{
				frames_chunk = self->mod->read( self->samplerate, frames_chunk, dstbuf );
			}
			break;
		case 2:
			{
				frames_chunk = self->mod->read_interleaved_stereo( self->samplerate, frames_chunk, dstbuf );
			}
			break;
		case 4:
			{
				frames_chunk = self->mod->read_interleaved_quad( self->samplerate, frames_chunk, dstbuf );
			}
			break;
		}
		dstbuf += frames_chunk * self->num_channels;
		if ( frames_chunk == 0 ) {
			break;
		}
		update_timeinfos( self->samplerate, frames_chunk );
		frames_to_render -= frames_chunk;
		frames_rendered += frames_chunk;
	}
	if ( frames_rendered == 0 ) {
		return 0;
	}
	return frames_rendered * self->num_channels;
}

static void add_names( std::ostream & str, const std::string & title, const std::vector<std::string> & names, int display_offset ) {
	if ( names.size() > 0 ) {
		bool valid = false;
		for ( std::size_t i = 0; i < names.size(); i++ ) {
			if ( names[i] != "" ) {
				valid = true;
			}
		}
		if ( !valid ) {
			return;
		}
		str << title << " Names:" << "\r";
		for ( std::size_t i = 0; i < names.size(); i++ ) {
			str << std::setfill( '0' ) << std::setw( 2 ) << ( display_offset + i ) << std::setw( 0 ) << "\t" << convert_to_native( sanitize_xmplay_info_string( names[i] ) ) << "\r";
		}
		str << "\r";
	}
}

static void WINAPI openmpt_GetSamples( char * buf ) {
	if ( !self->mod ) {
		clear_xmplay_string( buf );
		return;
	}
	std::ostringstream str;
	add_names( str, "Instrument", self->mod->get_instrument_names(), 1 );
	add_names( str, "Sample", self->mod->get_sample_names(), 1 );
	add_names( str, "Channel", self->mod->get_channel_names(), 1 );
	add_names( str, "Order", self->mod->get_order_names(), 0 );
	add_names( str, "Pattern", self->mod->get_pattern_names(), 0 );
	write_xmplay_string( buf, str.str() );
}

static DWORD WINAPI openmpt_GetSubSongs( float * length ) {
	*length = static_cast<float>( std::accumulate( self->subsong_lengths.cbegin(), self->subsong_lengths.cend(), 0.0 ) );
	return static_cast<DWORD>( self->subsong_lengths.size() );
}

enum ColorIndex
{
	col_background = 0,
	col_unknown,
	col_text,
	col_empty,
	col_instr,
	col_vol,
	col_pitch,
	col_global,

	col_max
};

static ColorIndex effect_type_to_color_index( openmpt::ext::pattern_vis::effect_type effect_type ) {
	switch ( effect_type ) {
		case openmpt::ext::pattern_vis::effect_unknown: return col_unknown; break;
		case openmpt::ext::pattern_vis::effect_general: return col_text   ; break;
		case openmpt::ext::pattern_vis::effect_global : return col_global ; break;
		case openmpt::ext::pattern_vis::effect_volume : return col_vol    ; break;
		case openmpt::ext::pattern_vis::effect_panning: return col_instr  ; break;
		case openmpt::ext::pattern_vis::effect_pitch  : return col_pitch  ; break;
		default: return col_unknown; break;
	}
}

const struct Columns
{
	int num_chars;
	int color;
} pattern_columns[] = {
	{ 3, col_text },	// C-5
	{ 2, col_instr },	// 01
	{ 3, col_vol },		// v64
	{ 3, col_pitch },	// EFF
};

static const int max_cols = 4;

static void assure_width( std::string & str, std::size_t width ) {
	if ( str.length() == width ) {
		return;
	} else if ( str.length() < width ) {
		str += std::string( width - str.length(), ' ' );
	} else if ( str.length() > width ) {
		str = str.substr( 0, width );
	}
}

struct ColorRGBA
{
	uint8_t r, g, b, a;
};

union Color
{
	ColorRGBA rgba;
	COLORREF dw;
};

static_assert(sizeof(Color) == 4);

HDC visDC;
HGDIOBJ visbitmap;

Color viscolors[col_max];
HPEN vispens[col_max];
HBRUSH visbrushs[col_max];
HFONT visfont;
static int last_pattern = -1;

static Color invert_color( Color c ) {
	Color res;
	res.rgba.a = c.rgba.a;
	res.rgba.r = 255 - c.rgba.r;
	res.rgba.g = 255 - c.rgba.g;
	res.rgba.b = 255 - c.rgba.b;
	return res;
}

static BOOL WINAPI VisOpen(DWORD colors[3]) {
	xmpopenmpt_lock guard;
	visDC = 0;
	visbitmap = 0;
	visfont = 0;

	viscolors[col_background].dw = colors[0];
	viscolors[col_unknown].dw = colors[1];
	viscolors[col_text].dw = colors[2];

	viscolors[col_global] = invert_color( viscolors[col_background] );

	const int r = viscolors[col_text].rgba.r, g = viscolors[col_text].rgba.g, b = viscolors[col_text].rgba.b;
	viscolors[col_empty].rgba.r = static_cast<std::uint8_t>( (r + viscolors[col_background].rgba.r) / 2 );
	viscolors[col_empty].rgba.g = static_cast<std::uint8_t>( (g + viscolors[col_background].rgba.g) / 2 );
	viscolors[col_empty].rgba.b = static_cast<std::uint8_t>( (b + viscolors[col_background].rgba.b) / 2 );
	viscolors[col_empty].rgba.a = 0;

#define MIXCOLOR(col, c1, c2, c3) { \
	viscolors[col] = viscolors[col_text]; \
	int mix = viscolors[col].rgba.c1 + 0xA0; \
	viscolors[col].rgba.c1 = static_cast<std::uint8_t>( mix ); \
	if ( mix > 0xFF ) { \
		viscolors[col].rgba.c2 = std::max( static_cast<std::uint8_t>( c2 - viscolors[col].rgba.c1 / 2 ), std::uint8_t(0) ); \
		viscolors[col].rgba.c3 = std::max( static_cast<std::uint8_t>( c3 - viscolors[col].rgba.c1 / 2 ), std::uint8_t(0) ); \
		viscolors[col].rgba.c1 = 0xFF; \
	} }

	MIXCOLOR(col_instr, g, r, b);
	MIXCOLOR(col_vol, b, r, g);
	MIXCOLOR(col_pitch, r, g, b);
#undef MIXCOLOR

	for( int i = 0; i < col_max; ++i ) {
		vispens[i] = CreatePen( PS_SOLID, 1, viscolors[i].dw );
		visbrushs[i] = CreateSolidBrush( viscolors[i].dw );
	}

	clear_current_timeinfo();

	if ( !self->mod ) {
		return FALSE;
	}
	return TRUE;
}
static void WINAPI VisClose() {
	xmpopenmpt_lock guard;

	for( int i = 0; i < col_max; ++i ) {
		DeletePen( vispens[i] );
		DeleteBrush( visbrushs[i] );
	}

	DeleteFont( visfont );
	DeleteBitmap( visbitmap );
	if ( visDC ) {
		DeleteDC( visDC );
	}
}
static void WINAPI VisSize( HDC /* dc */ , SIZE * /* size */ ) {
	xmpopenmpt_lock guard;
	last_pattern = -1;	// Force redraw
}

#if 0
static BOOL WINAPI VisRender( DWORD * /* buf */ , SIZE /* size */ , DWORD /* flags */ ) {
	xmpopenmpt_lock guard;
	return FALSE;
}
#endif

static int get_pattern_width( int chars_per_channel, int spaces_per_channel, int num_cols, int text_size, int channels )
{
	int pattern_width = ((chars_per_channel * channels + 4) * text_size) + (spaces_per_channel * channels + channels - (num_cols == 1 ? 1 : 2)) * (text_size / 2);
	return pattern_width;
}

static BOOL WINAPI VisRenderDC( HDC dc, SIZE size, DWORD flags ) {
	xmpopenmpt_lock guard;
	RECT rect;

	if ( !visfont ) {
		// Force usage of a nice monospace font
		LOGFONT logfont;
		GetObject ( GetCurrentObject( dc, OBJ_FONT ), sizeof(logfont), &logfont );
		_tcscpy( logfont.lfFaceName, TEXT("Lucida Console") );
		visfont = CreateFontIndirect( &logfont );
	}
	SIZE text_size;
	SelectFont( dc, visfont );
	if ( GetTextExtentPoint32( dc, TEXT("W"), 1, &text_size ) == FALSE ) {
		return FALSE;
	}

	if ( flags & XMPIN_VIS_INIT ) {
		last_pattern = -1;
	}

	timeinfo info = lookup_timeinfo( xmpfstatus->GetTime() );

	if ( !info.valid ) {
		RECT bgrect;
		bgrect.top = 0;
		bgrect.left = 0;
		bgrect.right = size.cx;
		bgrect.bottom = size.cy;
		FillRect(dc, &bgrect, visbrushs[col_background]);
		return TRUE;
	}

	int pattern = info.pattern;
	int current_row = info.row;

	const std::size_t channels = self->mod->get_num_channels();
	const std::size_t rows = self->mod->get_pattern_num_rows( pattern );

	const std::size_t num_half_chars = std::max( static_cast<std::size_t>( 2 * size.cx / text_size.cx ), std::size_t(8) ) - 8;
	//const std::size_t num_rows = size.cy / text_size.cy;

	// Spaces between pattern components are half width, full space at channel end
	const std::size_t half_chars_per_channel = num_half_chars / channels;
	std::size_t chars_per_channel, spaces_per_channel;
	std::size_t num_cols;
	std::size_t col0_width = pattern_columns[0].num_chars;
	for ( num_cols = sizeof ( pattern_columns ) / sizeof ( pattern_columns[0] ); num_cols >= 1; num_cols-- ) {
		chars_per_channel = 0;
		spaces_per_channel = num_cols > 1 ? num_cols : 0;	// No extra space if we only display notes
		for ( std::size_t i = 0; i < num_cols; i++ ) {
			chars_per_channel += pattern_columns[i].num_chars;
		}

		if ( half_chars_per_channel >= chars_per_channel * 2 + spaces_per_channel + 1 || num_cols == 1 ) {
			break;
		}
	}

	if ( !self->settings.vis_allow_scroll ) {
		if ( num_cols == 1 ) {
			spaces_per_channel = 0;
			while ( get_pattern_width( chars_per_channel, spaces_per_channel, num_cols, text_size.cx, channels ) > size.cx && chars_per_channel > 1 ) {
				chars_per_channel -= 1;
			}
			col0_width = chars_per_channel;
			chars_per_channel = col0_width;
		}
	}

	int pattern_width = get_pattern_width( chars_per_channel, spaces_per_channel, num_cols, text_size.cx, channels );
	int pattern_height = rows * text_size.cy;

	if ( !visDC || last_pattern != pattern ) {
		DeleteBitmap( visbitmap );
		if ( visDC ) {
			DeleteDC( visDC );
		}

		visDC = CreateCompatibleDC( dc );
		visbitmap = CreateCompatibleBitmap( dc, pattern_width, pattern_height );
		SelectBitmap( visDC, visbitmap );

		SelectBrush( visDC, vispens[col_unknown] );
		SelectBrush( visDC, visbrushs[col_background] );

		SelectFont( visDC, visfont );

		rect.top = 0;
		rect.left = 0;
		rect.right = pattern_width;
		rect.bottom = pattern_height;
		FillRect( visDC, &rect, visbrushs[col_background] );

		SetBkColor( visDC, viscolors[col_background].dw );

		POINT pos;
		pos.y = 0;

		for ( std::size_t row = 0; row < rows; row++ ) {
			pos.x = 0;

			std::ostringstream s;
			s.imbue(std::locale::classic());
			s << std::setfill('0') << std::setw(3) << row;
			const std::string rowstr = s.str();

			SetTextColor( visDC, viscolors[1].dw );
			TextOutA( visDC, pos.x, pos.y, rowstr.c_str(), rowstr.length() );
			pos.x += 4 * text_size.cx;

			for ( std::size_t channel = 0; channel < channels; ++channel ) {

				struct coldata {
					std::string text;
					bool is_empty;
					ColorIndex color;
					coldata()
						: is_empty(false)
						, color(col_unknown)
					{
						return;
					}
				};

				coldata cols[max_cols];

				for ( std::size_t col = 0; col < max_cols; ++col ) {
					switch ( col ) {
						case 0:
							cols[col].text = self->mod->format_pattern_row_channel_command( pattern, row, channel, openmpt::module::command_note );
							break;
						case 1:
							cols[col].text = self->mod->format_pattern_row_channel_command( pattern, row, channel, openmpt::module::command_instrument );
							break;
						case 2:
							cols[col].text = self->mod->format_pattern_row_channel_command( pattern, row, channel, openmpt::module::command_volumeffect )
								+ self->mod->format_pattern_row_channel_command( pattern, row, channel, openmpt::module::command_volume );
							break;
						case 3:
							cols[col].text = self->mod->format_pattern_row_channel_command( pattern, row, channel, openmpt::module::command_effect )
								+ self->mod->format_pattern_row_channel_command( pattern, row, channel, openmpt::module::command_parameter );
							break;
					}
					int color = pattern_columns[col].color;
					if ( self->pattern_vis && ( col == 2 || col == 3 ) ) {
						if ( col == 2 ) {
							color = effect_type_to_color_index( self->pattern_vis->get_pattern_row_channel_volume_effect_type( pattern, row, channel ) );
						}
						if ( col == 3 ) {
							color = effect_type_to_color_index( self->pattern_vis->get_pattern_row_channel_effect_type( pattern, row, channel ) );
						}
					}
					switch ( cols[col].text[0] ) {
						case ' ':
							[[fallthrough]];
						case '.':
							cols[col].is_empty = true;
							[[fallthrough]];
						case '^':
							[[fallthrough]];
						case '=':
							[[fallthrough]];
						case '~':
							color = col_empty;
							break;
					}
					cols[col].color = (ColorIndex)color;

				}

				if ( num_cols <= 3 && !cols[3].is_empty ) {
					if ( cols[2].is_empty ) {
						cols[2] = cols[3];
					} else if ( cols[0].is_empty ) {
						cols[0] = cols[3];
					}
				}

				if ( num_cols <= 2 && !cols[2].is_empty ) {
					if ( cols[0].is_empty ) {
						cols[0] = cols[2];
					}
				}

				for ( std::size_t col = 0; col < num_cols; ++col ) {

					std::size_t col_width = ( num_cols > 1 ) ? pattern_columns[col].num_chars : col0_width;

					assure_width( cols[col].text, col_width );

					SetTextColor( visDC, viscolors[cols[col].color].dw );
					TextOutA( visDC, pos.x, pos.y, cols[col].text.c_str(), cols[col].text.length() );
					pos.x += col_width * text_size.cx + text_size.cx / 2;
				}
				// Extra padding
				if ( num_cols > 1 ) {
					pos.x += text_size.cx / 2;
				}
			}

			pos.y += text_size.cy;
		}
	}

	rect.top = 0;
	rect.left = 0;
	rect.right = size.cx;
	rect.bottom = size.cy;
	FillRect( dc, &rect, visbrushs[col_background] );

	int offset_x = (size.cx - pattern_width) / 2;
	int offset_y = (size.cy - text_size.cy) / 2 - current_row * text_size.cy;
	int src_offset_x = 0;
	int src_offset_y = 0;

	if ( offset_x < 0 ) {
		src_offset_x -= offset_x;
		pattern_width = std::min( static_cast<int>( pattern_width + offset_x ), static_cast<int>( size.cx ) );
		offset_x = 0;
	}

	if ( offset_y < 0 ) {
		src_offset_y -= offset_y;
		pattern_height = std::min( static_cast<int>( pattern_height + offset_y ), static_cast<int>( size.cy ) );
		offset_y = 0;
	}

	BitBlt( dc, offset_x, offset_y, pattern_width, pattern_height, visDC, src_offset_x, src_offset_y , SRCCOPY );

	// Highlight current row
	rect.left = offset_x;
	rect.top = (size.cy - text_size.cy) / 2;
	rect.right = rect.left + pattern_width;
	rect.bottom = rect.top + text_size.cy;
	InvertRect( dc, &rect );
	
	last_pattern = pattern;

	return TRUE;
}

#if 0
static void WINAPI VisButton( DWORD /* x */ , DWORD /* y */ ) {
	//xmpopenmpt_lock guard;
}
#endif

static XMPIN xmpin = {
	XMPIN_FLAG_CONFIG | XMPIN_FLAG_LOOP,
	xmp_openmpt_string,
	nullptr, // "libopenmpt\0mptm/mptmz",
	openmpt_About,
	openmpt_Config,
	openmpt_CheckFile,
	openmpt_GetFileInfo,
	openmpt_Open,
	openmpt_Close,
	nullptr, // reserved
	openmpt_SetFormat,
	openmpt_GetTags,
	openmpt_GetInfoText,
	openmpt_GetGeneralInfo,
	openmpt_GetMessage,
	openmpt_SetPosition,
	openmpt_GetGranularity,
	nullptr, // GetBuffering
	openmpt_Process,
	nullptr, // WriteFile
	openmpt_GetSamples,
	openmpt_GetSubSongs, // GetSubSongs
	nullptr, // GetCues
	nullptr, // GetDownloaded

	"OpenMPT Pattern Display",
	VisOpen,
	VisClose,
	VisSize,
	/*VisRender,*/nullptr,
	VisRenderDC,
	/*VisButton,*/nullptr,

	nullptr, // reserved2
	openmpt_GetConfig,
	openmpt_SetConfig,

	nullptr
};

static const char * xmp_openmpt_default_exts = "OpenMPT\0mptm/mptmz";

static char * file_formats;

static void xmp_openmpt_on_dll_load() {
	ZeroMemory( &xmpopenmpt_mutex, sizeof( xmpopenmpt_mutex ) );
	#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable:28125) // The function 'InitializeCriticalSection' must be called from within a try/except block:  The requirement might be conditional.
	#endif // _MSC_VER
		InitializeCriticalSection( &xmpopenmpt_mutex );
	#if defined(_MSC_VER)
	#pragma warning(pop)
	#endif // _MSC_VER
	std::vector<std::string> extensions = openmpt::get_supported_extensions();
	std::string filetypes_string = "OpenMPT";
	filetypes_string.push_back('\0');
	bool first = true;
	for ( const auto & ext : extensions ) {
		if ( first ) {
			first = false;
		} else {
			filetypes_string.push_back('/');
		}
		filetypes_string += ext;
	}
	filetypes_string.push_back('\0');
	file_formats = (char*)HeapAlloc( GetProcessHeap(), 0, filetypes_string.size() );
	if ( file_formats ) {
		std::copy( filetypes_string.begin(), filetypes_string.end(), file_formats );
		xmpin.exts = file_formats;
	} else {
		xmpin.exts = xmp_openmpt_default_exts;
	}
	self = new self_xmplay_t();
}

static void xmp_openmpt_on_dll_unload() {
	delete self;
	self = nullptr;
	if ( xmpin.exts != xmp_openmpt_default_exts ) {
		HeapFree(GetProcessHeap(), 0, (LPVOID)const_cast<char*>(xmpin.exts));
	}
	xmpin.exts = nullptr;
	DeleteCriticalSection( &xmpopenmpt_mutex );
}

static XMPIN * XMPIN_GetInterface_cxx( DWORD face, InterfaceProc faceproc ) {
	if ( face != XMPIN_FACE ) return nullptr;
	xmpfin=(XMPFUNC_IN*)faceproc(XMPFUNC_IN_FACE);
	xmpfmisc=(XMPFUNC_MISC*)faceproc(XMPFUNC_MISC_FACE);
	xmpfregistry=(XMPFUNC_REGISTRY*)faceproc(XMPFUNC_REGISTRY_FACE);
	xmpffile=(XMPFUNC_FILE*)faceproc(XMPFUNC_FILE_FACE);
	xmpftext=(XMPFUNC_TEXT*)faceproc(XMPFUNC_TEXT_FACE);
	xmpfstatus=(XMPFUNC_STATUS*)faceproc(XMPFUNC_STATUS_FACE);

	// Register keyboard shortcuts
	static constexpr std::pair<DWORD, const char *> shortcuts[] = {
		{ openmpt_shortcut_ex | openmpt_shortcut_tempo_decrease, "OpenMPT - Decrease Tempo" },
		{ openmpt_shortcut_ex | openmpt_shortcut_tempo_increase, "OpenMPT - Increase Tempo" },
		{ openmpt_shortcut_ex | openmpt_shortcut_pitch_decrease, "OpenMPT - Decrease Pitch" },
		{ openmpt_shortcut_ex | openmpt_shortcut_pitch_increase, "OpenMPT - Increase Pitch" },
		{ openmpt_shortcut_ex | openmpt_shortcut_switch_interpolation, "OpenMPT - Switch Interpolation" },
	};
	XMPSHORTCUT cut;
	cut.procex = &ShortcutHandler;
	for ( const auto & shortcut : shortcuts ) {
		cut.id = shortcut.first;
		cut.text = shortcut.second;
		xmpfmisc->RegisterShortcut( &cut );
	}

	self->settings.load();

	return &xmpin;
}

extern "C" {

// XMPLAY expects a WINAPI (which is __stdcall) function using an undecorated symbol name.
#if defined(__GNUC__)
XMPIN * WINAPI XMPIN_GetInterface_( DWORD face, InterfaceProc faceproc );
XMPIN * WINAPI XMPIN_GetInterface_( DWORD face, InterfaceProc faceproc ) {
	return XMPIN_GetInterface_cxx( face, faceproc );
}
#if (__GNUC__ >= 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattribute-alias"
#endif
// clang-format off
__declspec(dllexport) void XMPIN_GetInterface() __attribute__((alias("XMPIN_GetInterface_@8")));
// clang-format on
#if (__GNUC__ >= 8)
#pragma GCC diagnostic pop
#endif
#else
XMPIN * WINAPI XMPIN_GetInterface( DWORD face, InterfaceProc faceproc ) {
	return XMPIN_GetInterface_cxx( face, faceproc );
}
#pragma comment(linker, "/EXPORT:XMPIN_GetInterface=_XMPIN_GetInterface@8")
#endif

} // extern "C"


#if defined(MPT_WITH_MFC) && defined(_MFC_VER)

namespace libopenmpt {
namespace plugin {

void DllMainAttach() {
	xmp_openmpt_on_dll_load();
}

void DllMainDetach() {
	xmp_openmpt_on_dll_unload();
}

} // namespace plugin
} // namespace libopenmpt

#else

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved );
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {
	static_cast<void>(hinstDLL);
	static_cast<void>(lpvReserved);
	switch ( fdwReason ) {
	case DLL_PROCESS_ATTACH:
		xmp_openmpt_on_dll_load();
		break;
	case DLL_PROCESS_DETACH:
		xmp_openmpt_on_dll_unload();
		break;
	}
	return TRUE;
}

#endif
