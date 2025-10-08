/*
 * libopenmpt_plugin_gui.cpp
 * -------------------------
 * Purpose: libopenmpt plugin GUI
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#if defined(__MINGW32__) && !defined(__MINGW64__)
#include <sys/types.h>
#endif

#if !defined(MPT_WITH_MFC)
#include <fstream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>
#endif

#if defined(MPT_WITH_MFC)
#include <tchar.h>
#endif

#if !defined(MPT_WITH_MFC)
#include <windows.h>
#endif

#if defined(MPT_WITH_MFC)
#include <afxwin.h>
#include <afxcmn.h>
#endif // MPT_WITH_MFC

#if defined(MPT_WITH_MFC)
#include "resource.h"
#endif // MPT_WITH_MFC

#include "libopenmpt_plugin_gui.hpp"


namespace libopenmpt {
namespace plugin {


#if defined(MPT_WITH_MFC)


class CSettingsApp : public CWinApp {

public:

	BOOL InitInstance() override {
		if ( !CWinApp::InitInstance() )
		{
			return FALSE;
		}
		DllMainAttach();
		return TRUE;
	}

	int ExitInstance() override {
		DllMainDetach();
		return CWinApp::ExitInstance();
	}

};


CSettingsApp theApp;


class CSettingsDialog : public CDialog {

protected:

	DECLARE_MESSAGE_MAP()

	libopenmpt_settings * s;

	CString m_Title;

	CComboBox m_ComboBoxSamplerate;
	CComboBox m_ComboBoxChannels;
	CSliderCtrl m_SliderCtrlGain;
	CComboBox m_ComboBoxInterpolation;
	CButton m_CheckBoxAmigaResampler;
	CComboBox m_ComboBoxAmigaFilter;
	CComboBox m_ComboBoxRepeat;
	CSliderCtrl m_SliderCtrlStereoSeparation;
	CComboBox m_ComboBoxRamping;

public:

	CSettingsDialog( libopenmpt_settings * s_, CString title, CWnd * parent = nullptr )
		: CDialog( IDD_SETTINGS, parent )
		, s( s_ )
		, m_Title( title )
	{
		return;
	}

protected:

	void DoDataExchange( CDataExchange * pDX ) override
	{
		CDialog::DoDataExchange( pDX );
		DDX_Control( pDX, IDC_COMBO_SAMPLERATE, m_ComboBoxSamplerate );
		DDX_Control( pDX, IDC_COMBO_CHANNELS, m_ComboBoxChannels );
		DDX_Control( pDX, IDC_SLIDER_GAIN, m_SliderCtrlGain );
		DDX_Control( pDX, IDC_COMBO_INTERPOLATION, m_ComboBoxInterpolation );
		DDX_Control( pDX, IDC_CHECK_AMIGA_RESAMPLER, m_CheckBoxAmigaResampler );
		DDX_Control( pDX, IDC_COMBO_AMIGA_FILTER, m_ComboBoxAmigaFilter );
		DDX_Control( pDX, IDC_COMBO_REPEAT, m_ComboBoxRepeat );
		DDX_Control( pDX, IDC_SLIDER_STEREOSEPARATION, m_SliderCtrlStereoSeparation );
		DDX_Control( pDX, IDC_COMBO_RAMPING, m_ComboBoxRamping );
	}

	afx_msg BOOL OnInitDialog() override {

		CDialog::OnInitDialog();

		SetWindowText( m_Title );
		EnableToolTips();

		bool selected = false;

		selected = false;
		if ( !s->no_default_format ) {
			m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( s->player_setting_name ? s->player_setting_name.value().c_str() : TEXT("Default") ), 0 );
		}
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("6000") ), 6000 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("8000") ), 8000 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("11025") ), 11025 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("16000") ), 16000 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("22050") ), 22050 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("32000") ), 32000 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("44100") ), 44100 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("48000") ), 48000 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("88200") ), 88200 );
		m_ComboBoxSamplerate.SetItemData( m_ComboBoxSamplerate.AddString( TEXT("96000") ), 96000 );
		if ( !s->no_default_format && s->samplerate == 0 ) {
			m_ComboBoxSamplerate.SelectString( 0, TEXT("Default") );
		}
		for ( int index = 0; index < m_ComboBoxSamplerate.GetCount(); ++index ) {
			if ( static_cast<int>( m_ComboBoxSamplerate.GetItemData( index ) ) == s->samplerate ) {
				m_ComboBoxSamplerate.SetCurSel( index );
				selected = true;
			}
		}
		if ( !selected ) {
			m_ComboBoxSamplerate.SelectString( 0, TEXT("48000") );
		}

		selected = false;
		if ( !s->no_default_format ) {
			m_ComboBoxChannels.SetItemData( m_ComboBoxChannels.AddString( s->player_setting_name ? s->player_setting_name.value().c_str() : TEXT("Default") ), 0 );
		}
		m_ComboBoxChannels.SetItemData( m_ComboBoxChannels.AddString( TEXT("Mono") ), 1 );
		m_ComboBoxChannels.SetItemData( m_ComboBoxChannels.AddString( TEXT("Stereo") ), 2 );
		m_ComboBoxChannels.SetItemData( m_ComboBoxChannels.AddString( TEXT("Quad") ), 4 );
		if ( !s->no_default_format && s->channels == 0 ) {
			m_ComboBoxChannels.SelectString( 0, TEXT("Default") );
		}
		for ( int index = 0; index < m_ComboBoxChannels.GetCount(); ++index ) {
			if ( static_cast<int>( m_ComboBoxChannels.GetItemData( index ) ) == s->channels ) {
				m_ComboBoxChannels.SetCurSel( index );
				selected = true;
			}
		}
		if ( !selected ) {
			m_ComboBoxChannels.SelectString( 0, TEXT("Stereo") );
		}

		m_SliderCtrlGain.SetRange( -1200, 1200 );
		m_SliderCtrlGain.SetTicFreq( 100 );
		m_SliderCtrlGain.SetPageSize( 300 );
		m_SliderCtrlGain.SetLineSize( 100 );
		m_SliderCtrlGain.SetPos( s->mastergain_millibel );

		selected = false;
		m_ComboBoxInterpolation.SetItemData( m_ComboBoxInterpolation.AddString( TEXT("Off / 1 Tap (Nearest)") ), 1 );
		m_ComboBoxInterpolation.SetItemData( m_ComboBoxInterpolation.AddString( TEXT("2 Tap (Linear)") ), 2 );
		m_ComboBoxInterpolation.SetItemData( m_ComboBoxInterpolation.AddString( TEXT("4 Tap (Cubic)") ), 4 );
		m_ComboBoxInterpolation.SetItemData( m_ComboBoxInterpolation.AddString( TEXT("8 Tap (Polyphase FIR)") ), 8 );
		for ( int index = 0; index < m_ComboBoxInterpolation.GetCount(); ++index ) {
			if ( static_cast<int>( m_ComboBoxInterpolation.GetItemData( index ) ) == s->interpolationfilterlength ) {
				m_ComboBoxInterpolation.SetCurSel( index );
				selected = true;
			}
		}
		if ( !selected ) {
			m_ComboBoxInterpolation.SelectString( 0, TEXT("8 Tap (Polyphase FIR)") );
		}
		
		m_CheckBoxAmigaResampler.SetCheck( s->use_amiga_resampler ? BST_CHECKED : BST_UNCHECKED );
		selected = false;
		m_ComboBoxAmigaFilter.EnableWindow( s->use_amiga_resampler ? TRUE : FALSE );
		m_ComboBoxAmigaFilter.SetItemData( m_ComboBoxAmigaFilter.AddString( TEXT("Default") ), 0 );
		m_ComboBoxAmigaFilter.SetItemData( m_ComboBoxAmigaFilter.AddString( TEXT("A500 Filter") ), 0xA500 );
		m_ComboBoxAmigaFilter.SetItemData( m_ComboBoxAmigaFilter.AddString( TEXT("A1200 Filter") ), 0xA1200 );
		m_ComboBoxAmigaFilter.SetItemData( m_ComboBoxAmigaFilter.AddString( TEXT("Unfiltered") ), 1 );
		for ( int index = 0; index < m_ComboBoxAmigaFilter.GetCount(); ++index ) {
			if ( static_cast<int>( m_ComboBoxAmigaFilter.GetItemData( index ) ) == s->amiga_filter_type ) {
				m_ComboBoxAmigaFilter.SetCurSel( index );
				selected = true;
			}
		}
		if ( !selected ) {
			m_ComboBoxAmigaFilter.SelectString( 0, TEXT("Default") );
		}

		selected = false;
		m_ComboBoxRepeat.SetItemData( m_ComboBoxRepeat.AddString( TEXT("Forever") ), static_cast<unsigned int>( -1 ) );
		m_ComboBoxRepeat.SetItemData( m_ComboBoxRepeat.AddString( TEXT("Never") ), 0 );
		m_ComboBoxRepeat.SetItemData( m_ComboBoxRepeat.AddString( TEXT("Once") ), 1 );
		for ( int index = 0; index < m_ComboBoxRepeat.GetCount(); ++index ) {
			if ( static_cast<int>( m_ComboBoxRepeat.GetItemData( index ) ) == s->repeatcount ) {
				m_ComboBoxRepeat.SetCurSel( index );
				selected = true;
			}
		}
		if ( !selected ) {
			m_ComboBoxRepeat.SelectString( 0, TEXT("Never") );
		}

		m_SliderCtrlStereoSeparation.SetRange( 0, 200 );
		m_SliderCtrlStereoSeparation.SetTicFreq( 100 );
		m_SliderCtrlStereoSeparation.SetPageSize( 25 );
		m_SliderCtrlStereoSeparation.SetLineSize( 5 );
		m_SliderCtrlStereoSeparation.SetPos( s->stereoseparation );

		selected = false;
		m_ComboBoxRamping.SetItemData( m_ComboBoxRamping.AddString( TEXT("Default") ), static_cast<unsigned int>( -1 ) );
		m_ComboBoxRamping.SetItemData( m_ComboBoxRamping.AddString( TEXT("Off") ), 0 );
		m_ComboBoxRamping.SetItemData( m_ComboBoxRamping.AddString( TEXT("1 ms") ), 1 );
		m_ComboBoxRamping.SetItemData( m_ComboBoxRamping.AddString( TEXT("2 ms") ), 2 );
		m_ComboBoxRamping.SetItemData( m_ComboBoxRamping.AddString( TEXT("3 ms") ), 3 );
		m_ComboBoxRamping.SetItemData( m_ComboBoxRamping.AddString( TEXT("5 ms") ), 5 );
		m_ComboBoxRamping.SetItemData( m_ComboBoxRamping.AddString( TEXT("10 ms") ), 10 );
		for ( int index = 0; index < m_ComboBoxRamping.GetCount(); ++index ) {
			if ( static_cast<int>( m_ComboBoxRamping.GetItemData( index ) ) == s->ramping ) {
				m_ComboBoxRamping.SetCurSel( index );
				selected = true;
			}
		}
		if ( !selected ) {
			m_ComboBoxRamping.SelectString( 0, TEXT("Default") );
		}

		return TRUE;

	}

	void OnOK() override {

		s->samplerate = m_ComboBoxSamplerate.GetItemData( m_ComboBoxSamplerate.GetCurSel() );

		s->channels = m_ComboBoxChannels.GetItemData( m_ComboBoxChannels.GetCurSel() );

		s->mastergain_millibel = m_SliderCtrlGain.GetPos();

		s->interpolationfilterlength = m_ComboBoxInterpolation.GetItemData( m_ComboBoxInterpolation.GetCurSel() );

		s->use_amiga_resampler = ( m_CheckBoxAmigaResampler.GetCheck() != BST_UNCHECKED ) ? 1 : 0;
		s->amiga_filter_type = m_ComboBoxAmigaFilter.GetItemData( m_ComboBoxAmigaFilter.GetCurSel() );

		s->repeatcount = m_ComboBoxRepeat.GetItemData( m_ComboBoxRepeat.GetCurSel() );

		s->stereoseparation = m_SliderCtrlStereoSeparation.GetPos();

		s->ramping = m_ComboBoxRamping.GetItemData( m_ComboBoxRamping.GetCurSel() );

		s->changed();

		CDialog::OnOK();

	}

	BOOL OnToolTipText( UINT, NMHDR * pNMHDR, LRESULT * pResult ) {
		TOOLTIPTEXT * pTTT = reinterpret_cast<TOOLTIPTEXT *>( pNMHDR );

		UINT_PTR nID = pNMHDR->idFrom;
		if( pTTT->uFlags & TTF_IDISHWND )
		{
			// idFrom is actually the HWND of the tool
			nID = (UINT_PTR)::GetDlgCtrlID((HWND)nID);
		}

		switch ( nID ) {
		case IDC_SLIDER_GAIN:
			_sntprintf( pTTT->szText, _countof(pTTT->szText), TEXT("%.02f dB"), m_SliderCtrlGain.GetPos() * 0.01f );
			break;

		case IDC_SLIDER_STEREOSEPARATION:
			_sntprintf( pTTT->szText, _countof(pTTT->szText), TEXT("%d %%"), m_SliderCtrlStereoSeparation.GetPos() );
			break;

		default:
			return FALSE;
		}

		*pResult = 0;
		return TRUE;
	}

	void OnAmigaResamplerChanged() {
		m_ComboBoxAmigaFilter.EnableWindow( IsDlgButtonChecked( IDC_CHECK_AMIGA_RESAMPLER ) != BST_UNCHECKED ? TRUE : FALSE );
	}

};

BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CSettingsDialog::OnToolTipText)
	ON_COMMAND( IDC_CHECK_AMIGA_RESAMPLER, &CSettingsDialog::OnAmigaResamplerChanged )
END_MESSAGE_MAP()



class CInfoDialog : public CDialog {

protected:

	CString m_Title;
	CString m_FileInfo;
	CEdit m_EditFileInfo;

public:

	CInfoDialog( CString title, CString info, CWnd * parent = NULL )
		: CDialog( IDD_FILEINFO, parent )
		, m_Title( title )
		, m_FileInfo( info )
	{
		return;
	}

protected:

	void DoDataExchange( CDataExchange * pDX ) override
	{
		CDialog::DoDataExchange( pDX );
		DDX_Control( pDX, IDC_FILEINFO, m_EditFileInfo );
	}

	afx_msg BOOL OnInitDialog() override {

		if ( !CDialog::OnInitDialog() ) {
			return false;
		}

		SetWindowText( m_Title );

		m_EditFileInfo.SetWindowText( m_FileInfo );

		return TRUE;

	}

};


#endif // MPT_WITH_MFC


#if defined(MPT_WITH_MFC)


void gui_edit_settings( libopenmpt_settings * s, HWND parent, std::basic_string<TCHAR> title ) {
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	CSettingsDialog dlg( s, title.c_str(), parent ? CWnd::FromHandle( parent ) : nullptr );
	dlg.DoModal();
}


void gui_show_file_info( HWND parent, std::basic_string<TCHAR> title, std::basic_string<TCHAR> info ) {
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	CInfoDialog dlg( title.c_str(), info.c_str(), parent ? CWnd::FromHandle( parent ) : nullptr );
	dlg.DoModal();
}


void gui_show_about( HWND parent, std::basic_string<TCHAR> title, std::basic_string<TCHAR> about, std::basic_string<TCHAR> credits ) {
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	about += TEXT("\r\n");
	about += TEXT("Show full credits?\r\n");
	if ( parent ) {
		if ( CWnd::FromHandle( parent )->MessageBox( about.c_str(), title.c_str(), MB_ICONINFORMATION | MB_YESNOCANCEL | MB_DEFBUTTON1 ) != IDYES ) {
			return;
		}
	} else {
		if ( MessageBox( parent, about.c_str(), title.c_str(), MB_ICONINFORMATION | MB_YESNOCANCEL | MB_DEFBUTTON1 ) != IDYES ) {
			return;
		}
	}
	CInfoDialog dlg( title.c_str(), credits.c_str(), parent ? CWnd::FromHandle( parent ) : nullptr );
	dlg.DoModal();
}


#else // !MPT_WITH_MFC


static std::basic_string<TCHAR> GetTempDirectory() {
	DWORD size = GetTempPath(0, nullptr);
	if (size) {
		std::vector<TCHAR> tempPath(size + 1);
		if (GetTempPath(size + 1, tempPath.data())) {
			return tempPath.data();
		}
	}
	return {};
}

static std::basic_string<TCHAR> GetTempFilename( std::basic_string<TCHAR> prefix ) {
	std::vector<TCHAR> buf(MAX_PATH);
	if (GetTempFileName(GetTempDirectory().c_str(), prefix.c_str(), 0, buf.data()) == 0) {
		return {};
	}
	return buf.data();
}

template <typename T>
static std::basic_string<TCHAR> as_string( T x ) {
	std::basic_ostringstream<TCHAR> s;
	s.imbue(std::locale::classic());
	s << x;
	return s.str();
}


void gui_edit_settings( libopenmpt_settings * s, HWND /* parent */ , std::basic_string<TCHAR> title ) {
	std::basic_string<TCHAR> filename = GetTempFilename( title );
	WritePrivateProfileString( title.c_str(), TEXT("Samplerate_Hz"), as_string( s->samplerate ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("Channels"), as_string( s->channels ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("MasterGain_milliBel"), as_string( s->mastergain_millibel ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("StereoSeparation_Percent"), as_string( s->stereoseparation ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("RepeatCount"), as_string( s->repeatcount ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("InterpolationFilterLength"), as_string( s->interpolationfilterlength ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("UseAmigaResampler"), as_string( s->use_amiga_resampler ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("AmigaFilterType"), as_string( s->amiga_filter_type ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("VolumeRampingStrength"), as_string( s->ramping ).c_str(), filename.c_str() );
	WritePrivateProfileString( title.c_str(), TEXT("VisAllowScroll"), as_string( s->vis_allow_scroll ).c_str(), filename.c_str() );
	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInformation = {};
	std::basic_string<TCHAR> command = std::basic_string<TCHAR>(TEXT("notepad.exe")) + TEXT(" ") + filename;
	std::vector<TCHAR> commandBuf{ command.c_str(), command.c_str() + command.length() + 1 };
	if ( CreateProcess( NULL, commandBuf.data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation ) == FALSE ) {
		MessageBox(NULL, as_string(GetLastError()).c_str(), TEXT("fail"), 0);
		return;
	}
	CloseHandle( processInformation.hThread );
	WaitForSingleObject( processInformation.hProcess, INFINITE );
	CloseHandle( processInformation.hProcess );
	s->samplerate = GetPrivateProfileInt( title.c_str(), TEXT("Samplerate_Hz"), libopenmpt_settings{}.samplerate, filename.c_str() );
	s->channels = GetPrivateProfileInt( title.c_str(), TEXT("Channels"), libopenmpt_settings{}.channels, filename.c_str() );
	s->mastergain_millibel = GetPrivateProfileInt( title.c_str(), TEXT("MasterGain_milliBel"), libopenmpt_settings{}.mastergain_millibel, filename.c_str() );
	s->stereoseparation = GetPrivateProfileInt( title.c_str(), TEXT("StereoSeparation_Percent"), libopenmpt_settings{}.stereoseparation, filename.c_str() );
	s->repeatcount = GetPrivateProfileInt( title.c_str(), TEXT("RepeatCount"), libopenmpt_settings{}.repeatcount, filename.c_str() );
	s->interpolationfilterlength = GetPrivateProfileInt( title.c_str(), TEXT("InterpolationFilterLength"), libopenmpt_settings{}.interpolationfilterlength, filename.c_str() );
	s->use_amiga_resampler = GetPrivateProfileInt( title.c_str(), TEXT("UseAmigaResampler"), libopenmpt_settings{}.use_amiga_resampler, filename.c_str() );
	s->amiga_filter_type = GetPrivateProfileInt( title.c_str(), TEXT("AmigaFilterType"), libopenmpt_settings{}.amiga_filter_type, filename.c_str() );
	s->ramping = GetPrivateProfileInt( title.c_str(), TEXT("VolumeRampingStrength"), libopenmpt_settings{}.ramping, filename.c_str() );
	s->vis_allow_scroll = GetPrivateProfileInt( title.c_str(), TEXT("VisAllowScroll"), libopenmpt_settings{}.vis_allow_scroll, filename.c_str() );
	DeleteFile( filename.c_str() );
}


void gui_show_file_info( HWND /* parent */ , std::basic_string<TCHAR> title, std::basic_string<TCHAR> info ) {
	std::basic_string<TCHAR> filename = GetTempFilename( title );
	{
		std::basic_ofstream<TCHAR> f( filename.c_str(), std::ios::out );
		f << info;
	}
	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInformation = {};
	std::basic_string<TCHAR> command = std::basic_string<TCHAR>(TEXT("notepad.exe")) + TEXT(" ") + filename;
	std::vector<TCHAR> commandBuf{ command.c_str(), command.c_str() + command.length() + 1 };
	if ( CreateProcess( NULL, commandBuf.data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation ) == FALSE ) {
		return;
	}
	CloseHandle( processInformation.hThread );
	WaitForSingleObject( processInformation.hProcess, INFINITE );
	CloseHandle( processInformation.hProcess );
	DeleteFile( filename.c_str() );
}


void gui_show_about( HWND parent, std::basic_string<TCHAR> title, std::basic_string<TCHAR> about, std::basic_string<TCHAR> credits ) {
	if ( MessageBox( parent, about.c_str(), title.c_str(), MB_ICONINFORMATION | MB_YESNOCANCEL | MB_DEFBUTTON1 ) != IDYES ) {
		return;
	}
	gui_show_file_info( parent, title, credits );
}


#endif // MPT_WITH_MFC


} // namespace plugin
} // namespace libopenmpt

