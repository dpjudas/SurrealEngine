/*
 * libopenmpt_ext.hpp
 * ------------------
 * Purpose: libopenmpt public c++ interface for libopenmpt extensions
 * Notes  :
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_EXT_HPP
#define LIBOPENMPT_EXT_HPP

#include "libopenmpt_config.h"
#include "libopenmpt.hpp"

/*!
 * \page libopenmpt_ext_cpp_overview libopenmpt_ext C++ API
 *
 * libopenmpt_ext is now included in all builds by default.
 *
 * \section libopenmpt-ext-cpp-detailed Detailed documentation
 *
 * \ref libopenmpt_ext_cpp
 *
 */

/*! \defgroup libopenmpt_ext_cpp libopenmpt_ext C++ */

namespace openmpt {

/*! \addtogroup libopenmpt_ext_cpp
  @{
*/

class module_ext_impl;

class LIBOPENMPT_CXX_API_CLASS module_ext : public module {
	
private:
	module_ext_impl * ext_impl;
private:
	// non-copyable
	LIBOPENMPT_CXX_API_MEMBER module_ext( const module_ext & );
	LIBOPENMPT_CXX_API_MEMBER void operator = ( const module_ext & );
public:
	LIBOPENMPT_CXX_API_MEMBER module_ext( std::istream & stream, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER module_ext( const std::vector<std::byte> & data, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER module_ext( const std::vector<std::uint8_t> & data, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER module_ext( const std::vector<char> & data, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER module_ext( const std::byte * data, std::size_t size, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER module_ext( const std::uint8_t * data, std::size_t size, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER module_ext( const char * data, std::size_t size, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER module_ext( const void * data, std::size_t size, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	LIBOPENMPT_CXX_API_MEMBER virtual ~module_ext();

public:

	//! Retrieve a libopenmpt extension.
	/*! Example: Retrieving the interactive extension to change the tempo of a module:
	  \code{.cpp}
	  openmpt::module_ext *mod = new openmpt::module_ext( stream );
	  #ifdef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE
	    openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>( self->mod->get_interface( openmpt::ext::interactive_id ) );
	    if ( interactive ) {
	      interactive->set_tempo_factor( 2.0 ); // play module at double speed
	    } else {
	      // interface not available
	    }
	  #else
	    // interfae not available
	  #endif
	  \endcode
	  \param interface_id The name of the extension interface to retrieve.
	  \return The interface object. This may be a nullptr if the extension was not found.
	*/
	LIBOPENMPT_CXX_API_MEMBER void * get_interface( const std::string & interface_id );

}; // class module_ext

/*!
  @}
*/

namespace ext {

/*! \addtogroup libopenmpt_ext_cpp
  @{
*/

#define LIBOPENMPT_DECLARE_EXT_CXX_INTERFACE(name) \
	static const char name ## _id [] = # name ; \
	class name; \
/**/

#define LIBOPENMPT_EXT_CXX_INTERFACE(name) \
	protected: \
		name () {} \
		virtual ~ name () {} \
	public: \
/**/


#ifndef LIBOPENMPT_EXT_INTERFACE_PATTERN_VIS
#define LIBOPENMPT_EXT_INTERFACE_PATTERN_VIS
#endif

LIBOPENMPT_DECLARE_EXT_CXX_INTERFACE(pattern_vis)

class pattern_vis {

	LIBOPENMPT_EXT_CXX_INTERFACE(pattern_vis)

	//! Pattern command type
	enum effect_type {

		effect_unknown = 0,
		effect_general = 1,
		effect_global = 2,
		effect_volume = 3,
		effect_panning = 4,
		effect_pitch = 5

	}; // enum effect_type

	//! Get pattern command type for pattern highlighting
	/*!
	  \param pattern The pattern whose data should be retrieved.
	  \param row The row from which the data should be retrieved.
	  \param channel The channel from which the data should be retrieved.
	  \return The command type in the effect column at the given pattern position (see openmpt::ext::pattern_vis::effect_type)
	  \sa openmpt::ext::pattern_vis::get_pattern_row_channel_effect_type
	*/
	virtual effect_type get_pattern_row_channel_volume_effect_type( std::int32_t pattern, std::int32_t row, std::int32_t channel ) const = 0;

	//! Get pattern command type for pattern highlighting
	/*!
	  \param pattern The pattern whose data should be retrieved.
	  \param row The row from which the data should be retrieved.
	  \param channel The channel from which the data should be retrieved.
	  \return The command type in the volume column at the given pattern position (see openmpt::ext::pattern_vis::effect_type)
	  \sa openmpt::ext::pattern_vis::get_pattern_row_channel_volume_effect_type
	*/
	virtual effect_type get_pattern_row_channel_effect_type( std::int32_t pattern, std::int32_t row, std::int32_t channel ) const = 0;

}; // class pattern_vis


#ifndef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE
#define LIBOPENMPT_EXT_INTERFACE_INTERACTIVE
#endif

LIBOPENMPT_DECLARE_EXT_CXX_INTERFACE(interactive)

class interactive {

	LIBOPENMPT_EXT_CXX_INTERFACE(interactive)

	//! Set the current ticks per row (speed)
	/*!
	  \param speed The new tick count in range [1, 65535].
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the speed is outside the specified range.
	  \remarks The tick count may be reset by pattern commands at any time.
	  \sa openmpt::module::get_current_speed
	*/
	virtual void set_current_speed( std::int32_t speed ) = 0;

	//! Set the current module tempo
	/*!
	  \param tempo The new tempo in range [32, 512]. The exact meaning of the value depends on the tempo mode used by the module.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the tempo is outside the specified range.
	  \remarks The tempo may be reset by pattern commands at any time. Use openmpt::ext:interactive::set_tempo_factor to apply a tempo factor that is independent of pattern commands.
	  \sa openmpt::module::get_current_tempo
	  \deprecated Please use openmpt::ext::interactive3::set_current_tempo2().
	*/
	LIBOPENMPT_ATTR_DEPRECATED virtual void set_current_tempo( std::int32_t tempo ) = 0;

	//! Set the current module tempo factor without affecting playback pitch
	/*!
	  \param factor The new tempo factor in range ]0.0, 4.0] - 1.0 means unmodified tempo.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the factor is outside the specified range.
	  \remarks Modifying the tempo without applying the same pitch factor using openmpt::ext::interactive::set_pitch_factor may cause rhythmic samples (e.g. drum loops) to go out of sync.
	  \sa openmpt::ext::interactive::get_tempo_factor
	*/
	virtual void set_tempo_factor( double factor ) = 0;

	//! Gets the current module tempo factor
	/*!
	  \return The current tempo factor.
	  \sa openmpt::ext::interactive::set_tempo_factor
	*/
	virtual double get_tempo_factor( ) const = 0;
	
	//! Set the current module pitch factor without affecting playback speed
	/*!
	  \param factor The new pitch factor in range ]0.0, 4.0] - 1.0 means unmodified pitch.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the factor is outside the specified range.
	  \remarks Modifying the pitch without applying the the same tempo factor using openmpt::ext::interactive::set_tempo_factor may cause rhythmic samples (e.g. drum loops) to go out of sync.
	  \remarks To shift the pich by `n` semitones, the parameter can be calculated as follows: `pow( 2.0, n / 12.0 )`
	  \sa openmpt::ext::interactive::get_pitch_factor
	*/
	virtual void set_pitch_factor( double factor ) = 0;

	//! Gets the current module pitch factor
	/*!
	  \return The current pitch factor.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the pitch is outside the specified range.
	  \sa openmpt::ext::interactive::set_pitch_factor
	*/
	virtual double get_pitch_factor( ) const = 0;

	//! Set the current global volume
	/*!
	  \param volume The new global volume in range [0.0, 1.0]
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the volume is outside the specified range.
	  \remarks The global volume may be reset by pattern commands at any time. Use openmpt::module::set_render_param to apply a global overall volume factor that is independent of pattern commands.
	  \sa openmpt::ext::interactive::get_global_volume
	*/
	virtual void set_global_volume( double volume ) = 0;

	//! Get the current global volume
	/*!
	  \return The current global volume in range [0.0, 1.0]
	  \sa openmpt::ext::interactive::set_global_volume
	*/
	virtual double get_global_volume( ) const = 0;
	
	//! Set the current channel volume for a channel
	/*!
	  \param channel The channel whose volume should be set, in range [0, openmpt::module::get_num_channels()[
	  \param volume The new channel volume in range [0.0, 1.0]
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel or volume is outside the specified range.
	  \remarks The channel volume may be reset by pattern commands at any time.
	  \sa openmpt::ext::interactive::get_channel_volume
	*/
	virtual void set_channel_volume( std::int32_t channel, double volume ) = 0;

	//! Get the current channel volume for a channel
	/*!
	  \param channel The channel whose volume should be retrieved, in range [0, openmpt::module::get_num_channels()[
	  \return The current channel volume in range [0.0, 1.0]
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel is outside the specified range.
	  \sa openmpt::ext::interactive::set_channel_volume
	*/
	virtual double get_channel_volume( std::int32_t channel ) const = 0;

	//! Set the current mute status for a channel
	/*!
	  \param channel The channel whose mute status should be set, in range [0, openmpt::module::get_num_channels()[
	  \param mute The new mute status. true is muted, false is unmuted.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel is outside the specified range.
	  \sa openmpt::ext::interactive::get_channel_mute_status
	*/
	virtual void set_channel_mute_status( std::int32_t channel, bool mute ) = 0;

	//! Get the current mute status for a channel
	/*!
	  \param channel The channel whose mute status should be retrieved, in range [0, openmpt::module::get_num_channels()[
	  \return The current channel mute status. true is muted, false is unmuted.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel is outside the specified range.
	  \sa openmpt::ext::interactive::set_channel_mute_status
	*/
	virtual bool get_channel_mute_status( std::int32_t channel ) const = 0;
	
	//! Set the current mute status for an instrument
	/*!
	  \param instrument The instrument whose mute status should be set, in range [0, openmpt::module::get_num_instruments()[ if openmpt::module::get_num_instruments is not 0, otherwise in [0, openmpt::module::get_num_samples()[
	  \param mute The new mute status. true is muted, false is unmuted.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the instrument is outside the specified range.
	  \sa openmpt::ext::interactive::get_instrument_mute_status
	*/
	virtual void set_instrument_mute_status( std::int32_t instrument, bool mute ) = 0;

	//! Get the current mute status for an instrument
	/*!
	  \param instrument The instrument whose mute status should be retrieved, in range [0, openmpt::module::get_num_instruments()[ if openmpt::module::get_num_instruments is not 0, otherwise in [0, openmpt::module::get_num_samples()[
	  \return The current instrument mute status. true is muted, false is unmuted.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the instrument is outside the specified range.
	  \sa openmpt::ext::interactive::set_instrument_mute_status
	*/
	virtual bool get_instrument_mute_status( std::int32_t instrument ) const = 0;

	//! Play a note using the specified instrument
	/*!
	  \param instrument The instrument that should be played, in range [0, openmpt::module::get_num_instruments()[ if openmpt::module::get_num_instruments is not 0, otherwise in [0, openmpt::module::get_num_samples()[
	  \param note The note to play, in rage [0, 119]. 60 is the middle C.
	  \param volume The volume at which the note should be triggered, in range [0.0, 1.0]
	  \param panning The panning position at which the note should be triggered, in range [-1.0, 1.0], 0.0 is center.
	  \return The channel on which the note is played. This can pe be passed to openmpt::ext::interactive::stop_note to stop the note.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the instrument or note is outside the specified range.
	  \sa openmpt::ext::interactive::stop_note
	  \sa openmpt::ext::interactive2::note_off
	  \sa openmpt::ext::interactive2::note_fade
	*/
	virtual std::int32_t play_note( std::int32_t instrument, std::int32_t note, double volume, double panning ) = 0;

	//! Stop the note playing on the specified channel
	/*!
	  \param channel The channel on which the note should be stopped. This is the value returned by a previous play_note call.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel index is invalid.
	  \sa openmpt::ext::interactive::play_note
	  \sa openmpt::ext::interactive2::note_off
	  \sa openmpt::ext::interactive2::note_fade
	*/
	virtual void stop_note( std::int32_t channel ) = 0;

}; // class interactive


#ifndef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE2
#define LIBOPENMPT_EXT_INTERFACE_INTERACTIVE2
#endif

LIBOPENMPT_DECLARE_EXT_CXX_INTERFACE(interactive2)

class interactive2 {

	LIBOPENMPT_EXT_CXX_INTERFACE(interactive2)

	//! Sends a key-off command for the note playing on the specified channel
	/*!
	  \param channel The channel on which the key-off event should be triggered. This is the value returned by a previous play_note call.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel index is invalid.
	  \remarks This method releases envelopes and sample sustain loops. If the sample has no sustain loop, or if the module does not use instruments, it does nothing.
	  \sa openmpt::ext::interactive::play_note
	  \sa openmpt::ext::interactive::stop_note
	  \sa openmpt::ext::interactive2::note_fade
	  \since 0.6.0
	*/
	virtual void note_off(int32_t channel ) = 0;

	//! Sends a note fade command for the note playing on the specified channel
	/*!
	  \param channel The channel on which the note should be faded. This is the value returned by a previous play_note call.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel index is invalid.
	  \remarks This method uses the instrument's fade-out value. If the module does not use instruments, or the instrument's fade-out value is 0, it does nothing.
	  \sa openmpt::ext::interactive::play_note
	  \sa openmpt::ext::interactive::stop_note
	  \sa openmpt::ext::interactive2::note_off
	  \since 0.6.0
	*/
	virtual void note_fade(int32_t channel) = 0;

	//! Set the current panning position for a channel
	/*!
	  \param channel The channel whose panning will be changed, in range [0, openmpt::module::get_num_channels()[
	  \param panning The panning position to set on the channel, in range [-1.0, 1.0], 0.0 is center.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel index is invalid.
	  \remarks This command affects subsequent notes played on the same channel, and may itself be overridden by subsequent panning commands encountered in the module itself.
	  \sa openmpt::ext::interactive2::get_channel_panning
	  \since 0.6.0
	*/
	virtual void set_channel_panning(int32_t channel, double panning ) = 0;

	//! Get the current panning position for a channel
	/*!
	  \param channel The channel whose panning should be retrieved, in range [0, openmpt::module::get_num_channels()[
	  \return The current channel panning, in range [-1.0, 1.0], 0.0 is center.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel is outside the specified range.
	  \sa openmpt::ext::interactive2::set_channel_panning
	  \since 0.6.0
	*/
	virtual double get_channel_panning( int32_t channel ) = 0;
	
	//! Set the finetune for the currently playing note on a channel
	/*!
	  \param channel The channel whose finetune will be changed, in range [0, openmpt::module::get_num_channels()[
	  \param finetune The finetune to set on the channel, in range [-1.0, 1.0], 0.0 is center.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel index is invalid.
	  \remarks The finetune range depends on the pitch wheel depth of the instrument playing on the current channel; for sample-based modules, the depth of this command is fixed to +/-1 semitone.
	  \remarks This command does not affect subsequent notes played on the same channel, but may itself be overridden by subsequent finetune commands encountered in the module itself.
	  \sa openmpt::ext::interactive2::get_note_finetune
	  \since 0.6.0
	*/
	virtual void set_note_finetune(int32_t channel, double finetune ) = 0;

	//! Get the finetune for the currently playing note on a channel
	/*!
	  \param channel The channel whose finetune should be retrieved, in range [0, openmpt::module::get_num_channels()[
	  \return The current channel finetune, in range [-1.0, 1.0], 0.0 is center.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel is outside the specified range.
	  \remarks The finetune range depends on the pitch wheel depth of the instrument playing on the current channel; for sample-based modules, the depth of this command is fixed to +/-1 semitone.
	  \sa openmpt::ext::interactive2::set_note_finetune
	  \since 0.6.0
	*/
	virtual double get_note_finetune( int32_t channel ) = 0;

}; // class interactive2


#ifndef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE3
#define LIBOPENMPT_EXT_INTERFACE_INTERACTIVE3
#endif

LIBOPENMPT_DECLARE_EXT_CXX_INTERFACE(interactive3)

class interactive3 {

	LIBOPENMPT_EXT_CXX_INTERFACE(interactive3)

	//! Set the current module tempo
	/*!
	  \param tempo The new tempo in range [32, 512]. The exact meaning of the value depends on the tempo mode used by the module.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the tempo is outside the specified range.
	  \remarks The tempo may be reset by pattern commands at any time. Use openmpt::ext:interactive::set_tempo_factor to apply a tempo factor that is independent of pattern commands.
	  \sa openmpt::module::get_current_tempo2
		\since 0.7.0
	*/
	virtual void set_current_tempo2( double tempo ) = 0;

}; // class interactive3



/* add stuff here */



#undef LIBOPENMPT_DECLARE_EXT_CXX_INTERFACE
#undef LIBOPENMPT_EXT_CXX_INTERFACE

/*!
  @}
*/

} // namespace ext

} // namespace openmpt

#endif // LIBOPENMPT_EXT_HPP
