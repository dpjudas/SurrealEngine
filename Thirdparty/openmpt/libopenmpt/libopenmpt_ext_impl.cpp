/*
 * libopenmpt_ext_impl.cpp
 * -----------------------
 * Purpose: libopenmpt extensions - implementation
 * Notes  :
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "common/stdafx.h"

#include "libopenmpt_internal.h"
#include "libopenmpt_ext.hpp"

#include "libopenmpt_ext_impl.hpp"

#include "mpt/base/saturate_round.hpp"

#include "soundlib/Sndfile.h"

// assume OPENMPT_NAMESPACE is OpenMPT

namespace openmpt {

	module_ext_impl::module_ext_impl( callback_stream_wrapper stream, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( stream, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( std::istream & stream, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( stream, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( const std::vector<std::byte> & data, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( data, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( const std::vector<std::uint8_t> & data, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( data, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( const std::vector<char> & data, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( data, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( const std::byte * data, std::size_t size, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( data, size, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( const std::uint8_t * data, std::size_t size, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( data, size, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( const char * data, std::size_t size, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( data, size, std::move(log), ctls ) {
		ctor();
	}
	module_ext_impl::module_ext_impl( const void * data, std::size_t size, std::unique_ptr<log_interface> log, const std::map< std::string, std::string > & ctls ) : module_impl( data, size, std::move(log), ctls ) {
		ctor();
	}



	void module_ext_impl::ctor() {



		/* add stuff here */



	}



	module_ext_impl::~module_ext_impl() {



		/* add stuff here */



	}



	void * module_ext_impl::get_interface( const std::string & interface_id ) {
		if ( interface_id.empty() ) {
			return 0;
		} else if ( interface_id == ext::pattern_vis_id ) {
			return dynamic_cast< ext::pattern_vis * >( this );
		} else if ( interface_id == ext::interactive_id ) {
			return dynamic_cast< ext::interactive * >( this );
		} else if ( interface_id == ext::interactive2_id ) {
			return dynamic_cast< ext::interactive2 * >( this );
		} else if ( interface_id == ext::interactive3_id ) {
			return dynamic_cast< ext::interactive3 * >( this );



			/* add stuff here */



		} else {
			return 0;
		}
	}

	// pattern_vis

	module_ext_impl::effect_type module_ext_impl::get_pattern_row_channel_volume_effect_type( std::int32_t pattern, std::int32_t row, std::int32_t channel ) const {
		auto volcmd = static_cast<OpenMPT::VolumeCommand>( get_pattern_row_channel_command( pattern, row, channel, module::command_volumeffect ) );
		switch ( OpenMPT::ModCommand::GetVolumeEffectType( volcmd ) ) {
			case OpenMPT::EffectType::Normal : return effect_general; break;
			case OpenMPT::EffectType::Global : return effect_global ; break;
			case OpenMPT::EffectType::Volume : return effect_volume ; break;
			case OpenMPT::EffectType::Panning: return effect_panning; break;
			case OpenMPT::EffectType::Pitch  : return effect_pitch  ; break;
			default: return effect_unknown; break;
		}
	}

	module_ext_impl::effect_type module_ext_impl::get_pattern_row_channel_effect_type( std::int32_t pattern, std::int32_t row, std::int32_t channel ) const {
		auto command = static_cast<OpenMPT::EffectCommand>( get_pattern_row_channel_command( pattern, row, channel, module::command_effect ) );
		switch (OpenMPT::ModCommand::GetEffectType( command ) ) {
			case OpenMPT::EffectType::Normal : return effect_general; break;
			case OpenMPT::EffectType::Global : return effect_global ; break;
			case OpenMPT::EffectType::Volume : return effect_volume ; break;
			case OpenMPT::EffectType::Panning: return effect_panning; break;
			case OpenMPT::EffectType::Pitch  : return effect_pitch  ; break;
			default: return effect_unknown; break;
		}
	}

	// interactive

	void module_ext_impl::set_current_speed( std::int32_t speed ) {
		if ( speed < 1 || speed > 65535 ) {
			throw openmpt::exception("invalid tick count");
		}
		m_sndFile->m_PlayState.m_nMusicSpeed = speed;
	}

	void module_ext_impl::set_current_tempo( std::int32_t tempo ) {
		if ( tempo < 32 || tempo > 512 ) {
			throw openmpt::exception("invalid tempo");
		}
		m_sndFile->m_PlayState.m_nMusicTempo.Set( tempo );
	}

	void module_ext_impl::set_tempo_factor( double factor ) {
		if ( factor <= 0.0 || factor > 4.0 ) {
			throw openmpt::exception("invalid tempo factor");
		}
		m_sndFile->m_nTempoFactor = mpt::saturate_round<uint32_t>( 65536.0 / factor );
		m_sndFile->RecalculateSamplesPerTick();
	}

	double module_ext_impl::get_tempo_factor( ) const {
		return 65536.0 / m_sndFile->m_nTempoFactor;
	}

	void module_ext_impl::set_pitch_factor( double factor ) {
		if ( factor <= 0.0 || factor > 4.0 ) {
			throw openmpt::exception("invalid pitch factor");
		}
		m_sndFile->m_nFreqFactor = mpt::saturate_round<uint32_t>( 65536.0 * factor );
		m_sndFile->RecalculateSamplesPerTick();
	}

	double module_ext_impl::get_pitch_factor( ) const {
		return m_sndFile->m_nFreqFactor / 65536.0;
	}

	void module_ext_impl::set_global_volume( double volume ) {
		if ( volume < 0.0 || volume > 1.0 ) {
			throw openmpt::exception("invalid global volume");
		}
		m_sndFile->m_PlayState.m_nGlobalVolume = mpt::saturate_round<uint32_t>( volume * OpenMPT::MAX_GLOBAL_VOLUME );
	}

	double module_ext_impl::get_global_volume( ) const {
		return m_sndFile->m_PlayState.m_nGlobalVolume / static_cast<double>( OpenMPT::MAX_GLOBAL_VOLUME );
	}
	
	void module_ext_impl::set_channel_volume( std::int32_t channel, double volume ) {
		if ( channel < 0 || channel >= get_num_channels() ) {
			throw openmpt::exception("invalid channel");
		}
		if ( volume < 0.0 || volume > 1.0 ) {
			throw openmpt::exception("invalid global volume");
		}
		m_sndFile->m_PlayState.Chn[channel].nGlobalVol = mpt::saturate_round<std::uint8_t>(volume * 64.0);
	}

	double module_ext_impl::get_channel_volume( std::int32_t channel ) const {
		if ( channel < 0 || channel >= get_num_channels() ) {
			throw openmpt::exception("invalid channel");
		}
		return m_sndFile->m_PlayState.Chn[channel].nGlobalVol / 64.0;
	}

	void module_ext_impl::set_channel_mute_status( std::int32_t channel, bool mute ) {
		if ( channel < 0 || channel >= get_num_channels() ) {
			throw openmpt::exception("invalid channel");
		}
		m_sndFile->ChnSettings[channel].dwFlags.set( OpenMPT::CHN_MUTE | OpenMPT::CHN_SYNCMUTE , mute );
		m_sndFile->m_PlayState.Chn[channel].dwFlags.set( OpenMPT::CHN_MUTE | OpenMPT::CHN_SYNCMUTE , mute );

		// Also update NNA channels
		for ( OpenMPT::CHANNELINDEX i = m_sndFile->GetNumChannels(); i < OpenMPT::MAX_CHANNELS; i++)
		{
			if ( m_sndFile->m_PlayState.Chn[i].nMasterChn == channel + 1)
			{
				m_sndFile->m_PlayState.Chn[i].dwFlags.set( OpenMPT::CHN_MUTE | OpenMPT::CHN_SYNCMUTE, mute );
			}
		}
	}

	bool module_ext_impl::get_channel_mute_status( std::int32_t channel ) const {
		if ( channel < 0 || channel >= get_num_channels() ) {
			throw openmpt::exception("invalid channel");
		}
		return m_sndFile->m_PlayState.Chn[channel].dwFlags[OpenMPT::CHN_MUTE | OpenMPT::CHN_SYNCMUTE];
	}
	
	void module_ext_impl::set_instrument_mute_status( std::int32_t instrument, bool mute ) {
		const bool instrument_mode = get_num_instruments() != 0;
		const std::int32_t max_instrument = instrument_mode ? get_num_instruments() : get_num_samples();
		if ( instrument < 0 || instrument >= max_instrument ) {
			throw openmpt::exception("invalid instrument");
		}
		if ( instrument_mode ) {
			if ( m_sndFile->Instruments[instrument + 1] != nullptr ) {
				m_sndFile->Instruments[instrument + 1]->dwFlags.set( OpenMPT::INS_MUTE, mute );
			}
		} else {
			m_sndFile->GetSample( static_cast<OpenMPT::SAMPLEINDEX>( instrument + 1 ) ).uFlags.set( OpenMPT::CHN_MUTE, mute ) ;
		}
	}

	bool module_ext_impl::get_instrument_mute_status( std::int32_t instrument ) const {
		const bool instrument_mode = get_num_instruments() != 0;
		const std::int32_t max_instrument = instrument_mode ? get_num_instruments() : get_num_samples();
		if ( instrument < 0 || instrument >= max_instrument ) {
			throw openmpt::exception("invalid instrument");
		}
		if ( instrument_mode ) {
			if ( m_sndFile->Instruments[instrument + 1] != nullptr ) {
				return m_sndFile->Instruments[instrument + 1]->dwFlags[OpenMPT::INS_MUTE];
			}
			return true;
		} else {
			return m_sndFile->GetSample( static_cast<OpenMPT::SAMPLEINDEX>( instrument + 1 ) ).uFlags[OpenMPT::CHN_MUTE];
		}
	}

	std::int32_t module_ext_impl::play_note( std::int32_t instrument, std::int32_t note, double volume, double panning ) {
		const bool instrument_mode = get_num_instruments() != 0;
		const std::int32_t max_instrument = instrument_mode ? get_num_instruments() : get_num_samples();
		if ( instrument < 0 || instrument >= max_instrument ) {
			throw openmpt::exception("invalid instrument");
		}
		note += OpenMPT::NOTE_MIN;
		if ( note < OpenMPT::NOTE_MIN || note > OpenMPT::NOTE_MAX ) {
			throw openmpt::exception("invalid note");
		}

		// Find a free channel
		OpenMPT::CHANNELINDEX free_channel = m_sndFile->GetNNAChannel( OpenMPT::CHANNELINDEX_INVALID );
		if ( free_channel == OpenMPT::CHANNELINDEX_INVALID ) {
			free_channel = OpenMPT::MAX_CHANNELS - 1;
		}

		OpenMPT::ModChannel &chn = m_sndFile->m_PlayState.Chn[free_channel];
		chn.Reset( OpenMPT::ModChannel::resetTotal, *m_sndFile, OpenMPT::CHANNELINDEX_INVALID, OpenMPT::CHN_MUTE );
		chn.nMasterChn = 0;	// remove NNA association
		chn.nNewNote = chn.nLastNote = static_cast<std::uint8_t>(note);
		chn.ResetEnvelopes();
		m_sndFile->InstrumentChange(chn, instrument + 1);
		chn.nFadeOutVol = 0x10000;
		m_sndFile->NoteChange(chn, note, false, true, true);
		chn.nPan = mpt::saturate_round<std::int32_t>( OpenMPT::Clamp( panning * 128.0, -128.0, 128.0 ) + 128.0 );
		chn.nVolume = mpt::saturate_round<std::int32_t>( OpenMPT::Clamp( volume * 256.0, 0.0, 256.0 ) );

		// Remove channel from list of mixed channels to fix https://bugs.openmpt.org/view.php?id=209
		// This is required because a previous note on the same channel might have just stopped playing,
		// but the channel is still in the mix list.
		// Since the channel volume / etc is only updated every tick in CSoundFile::ReadNote, and we
		// do not want to duplicate mixmode-dependant logic here, CSoundFile::CreateStereoMix may already
		// try to mix our newly set up channel at volume 0 if we don't remove it from the list.
		auto mix_begin = std::begin( m_sndFile->m_PlayState.ChnMix );
		auto mix_end = std::remove( mix_begin, mix_begin + m_sndFile->m_nMixChannels, free_channel );
		m_sndFile->m_nMixChannels = static_cast<OpenMPT::CHANNELINDEX>( std::distance( mix_begin, mix_end ) );

		return free_channel;
	}

	void module_ext_impl::stop_note( std::int32_t channel ) {
		if ( channel < 0 || channel >= OpenMPT::MAX_CHANNELS ) {
			throw openmpt::exception("invalid channel");
		}
		auto & chn = m_sndFile->m_PlayState.Chn[channel];
		chn.nLength = 0;
		chn.pCurrentSample = nullptr;
	}

	void module_ext_impl::note_off(int32_t channel ) {
		if ( channel < 0 || channel >= OpenMPT::MAX_CHANNELS ) {
			throw openmpt::exception( "invalid channel" );
		}
		auto & chn = m_sndFile->m_PlayState.Chn[channel];
		chn.dwFlags |= OpenMPT::CHN_KEYOFF;
	}

	void module_ext_impl::note_fade(int32_t channel ) {
		if ( channel < 0 || channel >= OpenMPT::MAX_CHANNELS ) {
			throw openmpt::exception( "invalid channel" );
		}
		auto & chn = m_sndFile->m_PlayState.Chn[channel];
		chn.dwFlags |= OpenMPT::CHN_NOTEFADE;
	}

	void module_ext_impl::set_channel_panning( int32_t channel, double panning ) {
		if ( channel < 0 || channel >= OpenMPT::MAX_CHANNELS ) {
			throw openmpt::exception( "invalid channel" );
		}
		auto & chn = m_sndFile->m_PlayState.Chn[channel];
		chn.nPan = mpt::saturate_round<int32_t>( std::clamp( panning, -1.0, 1.0 ) * 128.0 + 128.0 );
	}

	double module_ext_impl::get_channel_panning( int32_t channel ) {
		if ( channel < 0 || channel >= OpenMPT::MAX_CHANNELS ) {
			throw openmpt::exception( "invalid channel" );
		}
		auto & chn = m_sndFile->m_PlayState.Chn[channel];
		return ( chn.nPan - 128 ) / 128.0;
	}

	void module_ext_impl::set_note_finetune( int32_t channel, double finetune ) {
		if ( channel < 0 || channel >= OpenMPT::MAX_CHANNELS ) {
			throw openmpt::exception( "invalid channel" );
		}
		auto & chn = m_sndFile->m_PlayState.Chn[channel];
		chn.microTuning = mpt::saturate_round<int16_t>( finetune * 32768.0 );
	}

	double module_ext_impl::get_note_finetune( int32_t channel ) {
		if ( channel < 0 || channel >= OpenMPT::MAX_CHANNELS ) {
			throw openmpt::exception( "invalid channel" );
		}
		auto & chn = m_sndFile->m_PlayState.Chn[channel];
		return chn.microTuning / 32768.0;
	}

	void module_ext_impl::set_current_tempo2( double tempo ) {
		if ( tempo < 32.0 || tempo > 512.0 ) {
			throw openmpt::exception("invalid tempo");
		}
		m_sndFile->m_PlayState.m_nMusicTempo = decltype( m_sndFile->m_PlayState.m_nMusicTempo )( tempo );
	}

	/* add stuff here */



} // namespace openmpt

