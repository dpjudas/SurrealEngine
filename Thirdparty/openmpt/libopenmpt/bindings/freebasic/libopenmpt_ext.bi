/'
 ' libopenmpt_ext.bi
 ' -----------------
 ' Purpose: libopenmpt public FreeBASIC interface for libopenmpt extensions
 ' Notes  : (currently none)
 ' Authors: Johannes Schultz
 '          OpenMPT Devs
 ' The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 '/

#Pragma Once

#Include Once "libopenmpt.bi"

Extern "C"

'* \brief Opaque type representing a libopenmpt extension module
Type openmpt_module_ext
	opaque As Any Ptr
End Type

/'* \brief Construct an openmpt_module_ext

  \param stream_callbacks Input stream callback operations.
  \param stream Input stream to load the module from.
  \param logfunc Logging function where warning and errors are written. The logging function may be called throughout the lifetime of openmpt_module_ext. May be NULL.
  \param loguser User-defined data associated with this module. This value will be passed to the logging callback function (logfunc)
  \param errfunc Error function to define error behaviour. May be NULL.
  \param erruser Error function user context.
  \param errorcode Pointer to an integer where an error may get stored. May be NULL.
  \param error_message Pointer to a string pointer where an error message may get stored. May be NULL.
  \param ctls A map of initial ctl values, see openmpt_module_get_ctls.
  \return A pointer to the constructed openmpt_module_ext, or NULL on failure.
  \remarks The input data can be discarded after an openmpt_module_ext has been constructed successfully.
  \sa openmpt_stream_callbacks
  \since 0.3.0
'/
Declare Function openmpt_module_ext_create(ByVal stream_callbacks As openmpt_stream_callbacks, ByVal stream As Any Ptr, ByVal logfunc As openmpt_log_func, ByVal loguser As Any Ptr, ByVal errfunc As openmpt_error_func, ByVal erruser As Any Ptr, ByVal Error As Long Ptr, ByVal error_message As Const ZString Ptr Ptr, ByVal ctls As Const openmpt_module_initial_ctl Ptr) As openmpt_module_ext Ptr

/'* \brief Construct an openmpt_module_ext

  \param filedata Data to load the module from.
  \param filesize Amount of data available.
  \param logfunc Logging function where warning and errors are written. The logging function may be called throughout the lifetime of openmpt_module_ext.
  \param loguser User-defined data associated with this module. This value will be passed to the logging callback function (logfunc)
  \param errfunc Error function to define error behaviour. May be NULL.
  \param erruser Error function user context.
  \param errorcode Pointer to an integer where an error may get stored. May be NULL.
  \param error_message Pointer to a string pointer where an error message may get stored. May be NULL.
  \param ctls A map of initial ctl values, see openmpt_module_get_ctls.
  \return A pointer to the constructed openmpt_module_ext, or NULL on failure.
  \remarks The input data can be discarded after an openmpt_module_ext has been constructed successfully.
  \since 0.3.0
'/
Declare Function openmpt_module_ext_create_from_memory(ByVal filedata As Const Any Ptr, ByVal filesize As UInteger, ByVal logfunc As openmpt_log_func, ByVal loguser As Any Ptr, ByVal errfunc As openmpt_error_func, ByVal erruser As Any Ptr, ByVal Error As Long Ptr, ByVal error_message As Const ZString Ptr Ptr, ByVal ctls As Const openmpt_module_initial_ctl Ptr) As openmpt_module_ext Ptr

/'* \brief Unload a previously created openmpt_module_ext from memory.

  \param mod_ext The module to unload.
  \since 0.3.0
'/
Declare Sub openmpt_module_ext_destroy(ByVal mod_ext As openmpt_module_ext Ptr)

/'* \brief Retrieve the openmpt_module handle from an openmpt_module_ext handle.

  \param mod_ext The extension module handle to convert
  \return An equivalent openmpt_module handle to pass to standard libopenmpt functions 
  \since 0.3.0
'/
Declare Function openmpt_module_ext_get_module(ByVal mod_ext As openmpt_module_ext Ptr) As openmpt_module Ptr

/'* Retrieve a libopenmpt extension.

  \param mod_ext The module handle to work on.
  \param interface_id The name of the extension interface to retrieve (e.g. LIBOPENMPT_EXT_C_INTERFACE_PATTERN_VIS).
  \param interface Appropriate structure of interface function pointers which is to be filled by this function (e.g. a pointer to a openmpt_module_ext_interface_pattern_vis structure).
  \param interface_size Size of the interface's structure of function pointers (e.g. sizeof(openmpt_module_ext_interface_pattern_vis)).
  \return 1 on success, 0 if the interface was not found.
  \since 0.3.0
'/
Declare Function openmpt_module_ext_get_interface(ByVal mod_ext As openmpt_module_ext Ptr, ByVal interface_id As Const ZString Ptr, ByVal interface As Any Ptr, ByVal interface_size As UInteger) As Long

#define LIBOPENMPT_EXT_C_INTERFACE_PATTERN_VIS "pattern_vis"

'* Pattern command type
Const OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_UNKNOWN = 0
Const OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_GENERAL = 1
Const OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_GLOBAL = 2
Const OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_VOLUME = 3
Const OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_PANNING = 4
Const OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_PITCH = 5

Type openmpt_module_ext_interface_pattern_vis
	/'* Get pattern command type for pattern highlighting

	  \param mod_ext The module handle to work on.
	  \param pattern The pattern whose data should be retrieved.
	  \param row The row from which the data should be retrieved.
	  \param channel The channel from which the data should be retrieved.
	  \return The command type in the effect column at the given pattern position (see OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_*)
	  \sa get_pattern_row_channel_effect_type
	'/
	get_pattern_row_channel_volume_effect_type As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal pattern As Long, ByVal row As Long, ByVal channel As Long) As Long

	/'* Get pattern command type for pattern highlighting

	  \param mod_ext The module handle to work on.
	  \param pattern The pattern whose data should be retrieved.
	  \param row The row from which the data should be retrieved.
	  \param channel The channel from which the data should be retrieved.
	  \return The command type in the effect column at the given pattern position (see OPENMPT_MODULE_EXT_INTERFACE_PATTERN_VIS_EFFECT_TYPE_*)
	  \sa get_pattern_row_channel_volume_effect_type
	'/
	get_pattern_row_channel_effect_type As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal pattern As Long, ByVal row As Long, ByVal channel As Long) As Long
End Type

#define LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE "interactive"

Type openmpt_module_ext_interface_interactive
	/'* Set the current ticks per row (speed)

	  \param mod_ext The module handle to work on.
	  \param speed The new tick count in range [1, 65535].
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the speed is outside the specified range.
	  \return 1 on success, 0 on failure.
	  \sa openmpt_module_get_current_speed
	'/
	set_current_speed As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal speed As Long) As Long

	/'* Set the current module tempo

	  \param mod_ext The module handle to work on.
	  \param tempo The new tempo in range [32, 512]. The exact meaning of the value depends on the tempo mode used by the module.
	  \return 1 on success, 0 on failure.
	  \remarks The tempo may be reset by pattern commands at any time. Use set_tempo_factor to apply a tempo factor that is independent of pattern commands.
	  \sa openmpt_module_get_current_tempo
	  \deprecated Please use openmpt_module_ext_interface_interactive3.set_current_tempo2().
	'/
	set_current_tempo As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal tempo As Long) As Long

	/'* Set the current module tempo factor without affecting playback pitch

	  \param mod_ext The module handle to work on.
	  \param factor The new tempo factor in range ]0.0, 4.0] - 1.0 means unmodified tempo.
	  \return 1 on success, 0 on failure.
	  \remarks Modifying the tempo without applying the same pitch factor using set_pitch_factor may cause rhythmic samples (e.g. drum loops) to go out of sync.
	  \sa openmpt_module_ext_interface_interactive.get_tempo_factor
	'/
	set_tempo_factor As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal factor As Double) As Long

	/'* Gets the current module tempo factor

	  \param mod_ext The module handle to work on.
	  \return The current tempo factor.
	  \sa openmpt_module_ext_interface_interactive.set_tempo_factor
	'/
	get_tempo_factor As Function(ByVal mod_ext As openmpt_module_ext Ptr) As Double

	/'* Set the current module pitch factor without affecting playback speed

	  \param mod_ext The module handle to work on.
	  \param factor The new pitch factor in range ]0.0, 4.0] - 1.0 means unmodified pitch.
	  \return 1 on success, 0 on failure.
	  \remarks Modifying the pitch without applying the the same tempo factor using set_tempo_factor may cause rhythmic samples (e.g. drum loops) to go out of sync.
	  \remarks To shift the pich by `n` semitones, the parameter can be calculated as follows: `2.0 ^ ( n / 12.0 )`
	  \sa openmpt_module_ext_interface_interactive.get_pitch_factor
	'/
	set_pitch_factor As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal factor As Double) As Long

	/'* Gets the current module pitch factor

	  \param mod_ext The module handle to work on.
	  \return The current pitch factor.
	  \sa openmpt_module_ext_interface_interactive.set_pitch_factor
	'/
	get_pitch_factor As Function(ByVal mod_ext As openmpt_module_ext Ptr) As Double

	/'* Set the current global volume

	  \param mod_ext The module handle to work on.
	  \param volume The new global volume in range [0.0, 1.0]
	  \return 1 on success, 0 on failure.
	  \remarks The global volume may be reset by pattern commands at any time. Use openmpt_module_set_render_param to apply a global overall volume factor that is independent of pattern commands.
	  \sa openmpt_module_ext_interface_interactive.get_global_volume
	'/
	set_global_volume As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal volume As Double) As Long

	/'* Get the current global volume

	  \param mod_ext The module handle to work on.
	  \return The current global volume in range [0.0, 1.0]
	  \sa openmpt_module_ext_interface_interactive.set_global_volume
	'/
	get_global_volume As Function(ByVal mod_ext As openmpt_module_ext Ptr) As Double

	/'* Set the current channel volume for a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel whose volume should be set, in range [0, openmpt_module_get_num_channels()[
	  \param volume The new channel volume in range [0.0, 1.0]
	  \return 1 on success, 0 on failure (channel out of range).
	  \remarks The channel volume may be reset by pattern commands at any time.
	  \sa openmpt_module_ext_interface_interactive.get_channel_volume
	'/
	set_channel_volume As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long, ByVal volume As Double) As Long

	/'* Get the current channel volume for a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel whose volume should be retrieved, in range [0, openmpt_module_get_num_channels()[
	  \return The current channel volume in range [0.0, 1.0]
	  \sa openmpt_module_ext_interface_interactive.set_channel_volume
	'/
	get_channel_volume As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long) As Double

	/'* Set the current mute status for a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel whose mute status should be set, in range [0, openmpt_module_get_num_channels()[
	  \param mute The new mute status. true is muted, false is unmuted.
	  \return 1 on success, 0 on failure (channel out of range).
	  \sa openmpt_module_ext_interface_interactive.get_channel_mute_status
	'/
	set_channel_mute_status As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long, ByVal mute As Long) As Long

	/'* Get the current mute status for a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel whose mute status should be retrieved, in range [0, openmpt_module_get_num_channels()[
	  \return The current channel mute status. true is muted, false is unmuted.
	  \sa openmpt_module_ext_interface_interactive.set_channel_mute_status
	'/
	get_channel_mute_status As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long) As Long

	/'* Set the current mute status for an instrument

	  \param mod_ext The module handle to work on.
	  \param instrument The instrument whose mute status should be set, in range [0, openmpt_module_get_num_instruments()[ if openmpt_module_get_num_instruments is not 0, otherwise in [0, openmpt_module_get_num_samples()[
	  \param mute The new mute status. true is muted, false is unmuted.
	  \return 1 on success, 0 on failure (instrument out of range).
	  \sa openmpt_module_ext_interface_interactive.get_instrument_mute_status
	'/
	set_instrument_mute_status As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal instrument As Long, ByVal mute As Long) As Long

	/'* Get the current mute status for an instrument

	  \param mod_ext The module handle to work on.
	  \param instrument The instrument whose mute status should be retrieved, in range [0, openmpt_module_get_num_instruments()[ if openmpt_module_get_num_instruments is not 0, otherwise in [0, openmpt_module_get_num_samples()[
	  \return The current instrument mute status. 1 is muted, 0 is unmuted, -1 means the instrument was out of range.
	  \sa openmpt_module_ext_interface_interactive.set_instrument_mute_status
	'/
	get_instrument_mute_status As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal instrument As Long) As Long

	/'* Play a note using the specified instrument

	  \param mod_ext The module handle to work on.
	  \param instrument The instrument that should be played, in range [0, openmpt_module_get_num_instruments()[ if openmpt_module_get_num_instruments is not 0, otherwise in [0, openmpt_module_get_num_samples()[
	  \param note The note to play, in rage [0, 119]. 60 is the middle C.
	  \param volume The volume at which the note should be triggered, in range [0.0, 1.0]
	  \param panning The panning position at which the note should be triggered, in range [-1.0, 1.0], 0.0 is center.
	  \return The channel on which the note is played. This can pe be passed to stop_note to stop the note. -1 means that no channel could be allocated and the note is not played.
	  \sa openmpt_module_ext_interface_interactive.stop_note
	  \sa openmpt_module_ext_interface_interactive2.note_off
	  \sa openmpt_module_ext_interface_interactive2.note_fade
	'/
	play_note As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal instrument As Long, ByVal note As Long, ByVal volume As Double, ByVal panning As Double) As Long

	/'* Stop the note playing on the specified channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel on which the note should be stopped. This is the value returned by a previous play_note call.
	  \return 1 on success, 0 on failure (channel out of range).
	  \sa openmpt_module_ext_interface_interactive.play_note
	  \sa openmpt_module_ext_interface_interactive.note_off
	  \sa openmpt_module_ext_interface_interactive2.note_fade
	'/
	stop_note As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long) As Long
End Type

#define LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE2 "interactive2"

Type openmpt_module_ext_interface_interactive2

	/'* Sends a key-off command for the note playing on the specified channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel on which the key-off event should be triggered. This is the value returned by a previous play_note call.
	  \return 1 on success, 0 on failure (channel out of range).
	  \remarks This method releases envelopes and sample sustain loops. If the sample has no sustain loop, or if the module does not use instruments, it does nothing.
	  \sa openmpt_module_ext_interface_interactive.play_note
	  \sa openmpt_module_ext_interface_interactive.stop_note
	  \sa note_fade
	 '/
	note_off As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long) As Long

	/'* Sends a note fade command for the note playing on the specified channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel on which the note should be faded. This is the value returned by a previous play_note call.
	  \return 1 on success, 0 on failure (channel out of range).
	  \remarks This method uses the instrument's fade-out value. If the module does not use instruments, or the instrument's fade-out value is 0, it does nothing.
	  \sa openmpt_module_ext_interface_interactive.play_note
	  \sa openmpt_module_ext_interface_interactive.stop_note
	  \sa note_fade
	 '/
	note_fade As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long) As Long

	/'* Set the current panning for a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel that should be panned. This is the value returned by a previous play_note call.
	  \param panning The panning position to set on the channel, in range [-1.0, 1.0], 0.0 is center.
	  \return 1 on success, 0 on failure (channel out of range).
	  \remarks This command affects subsequent notes played on the same channel, and may itself be overridden by subsequent panning commands encountered in the module itself.
	  \sa openmpt_module_ext_interface_interactive2.get_channel_panning
	 '/
	set_channel_panning As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long, ByVal panning As Double) As Long

	/'* Get the current panning position for a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel whose panning should be retrieved. This is the value returned by a previous play_note call.
	  \return The current channel panning, in range [-1.0, 1.0], 0.0 is center.
	  \sa openmpt_module_ext_interface_interactive2.set_channel_panning
	 '/
	get_channel_panning As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long) As Double

	/'* Set the finetune for the currently playing note on a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel whose finetune will be changed, in range [0, openmpt::module::get_num_channels()[
	  \param finetune The finetune to set on the channel, in range [-1.0, 1.0], 0.0 is center.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel index is invalid.
	  \remarks The finetune range depends on the pitch wheel depth of the instrument playing on the current channel; for sample-based modules, the depth of this command is fixed to +/-1 semitone.
	  \remarks This command does not affect subsequent notes played on the same channel, but may itself be overridden by subsequent finetune commands encountered in the module itself.
	  \sa openmpt_module_ext_interface_interactive2.get_note_finetune
	 '/
	set_note_finetune As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long, ByVal finetune As Double) As Long

	/'* Get the finetune for the currently playing note on a channel

	  \param mod_ext The module handle to work on.
	  \param channel The channel whose finetune should be retrieved, in range [0, openmpt::module::get_num_channels()[
	  \return The current channel finetune, in range [-1.0, 1.0], 0.0 is center.
	  \remarks The finetune range depends on the pitch wheel depth of the instrument playing on the current channel; for sample-based modules, the depth of this command is fixed to +/-1 semitone.
	  \throws openmpt::exception Throws an exception derived from openmpt::exception if the channel is outside the specified range.
	  \sa openmpt_module_ext_interface_interactive2.set_note_finetune
	 '/
	get_note_finetune As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal channel As Long) As Double
End Type

#define LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE3 "interactive3"

Type openmpt_module_ext_interface_interactive3

	/'* Set the current module tempo

	  \param mod_ext The module handle to work on.
	  \param tempo The new tempo in range [32, 512]. The exact meaning of the value depends on the tempo mode used by the module.
	  \return 1 on success, 0 on failure.
	  \remarks The tempo may be reset by pattern commands at any time. Use set_tempo_factor to apply a tempo factor that is independent of pattern commands.
	  \sa openmpt_module_get_current_tempo2
	  \since 0.7.0
	'/
	set_current_tempo As Function(ByVal mod_ext As openmpt_module_ext Ptr, ByVal tempo As Double) As Long

End Type

End Extern

/'* \brief Construct an openmpt_module_ext

  \param file The FreeBASIC file handle to load from.
  \param logfunc Logging function where warning and errors are written. May be NULL.
  \param loguser Logging function user context. Used to pass any user-defined data associated with this module to the logging function.
  \param errfunc Error function to define error behaviour. May be NULL.
  \param erruser Error function user context. Used to pass any user-defined data associated with this module to the error function.
  \param errorcode Pointer to an integer where an error may get stored. May be NULL.
  \param error_message Pointer to a string pointer where an error message may get stored. May be NULL.
  \param ctls A map of initial ctl values, see openmpt_module_get_ctls.
  \return A pointer to the constructed openmpt_module, or NULL on failure.
  \remarks The file handle can be closed after an openmpt_module has been constructed successfully.
  \sa openmpt_module_ext_create
'/
Function openmpt_module_ext_create_from_fbhandle(_
		ByVal file As Integer,_
		ByVal logfunc As openmpt_log_func = 0,_
		ByVal loguser As Any Ptr = 0,_
		ByVal errfunc As openmpt_error_func = 0,_
		ByVal erruser As Any Ptr = 0,_
		ByVal errorcode As Long Ptr = 0,_
		ByVal error_message As Const ZString Ptr Ptr = 0,_
		ByVal ctls As Const openmpt_module_initial_ctl Ptr = 0) As openmpt_module_ext Ptr
	Return openmpt_module_ext_create(openmpt_stream_get_file_callbacks(), Cast(FILE Ptr, FileAttr(file, fbFileAttrHandle)), logfunc, loguser, errfunc, erruser, errorcode, error_message, ctls)
End Function

/'* \brief Construct an openmpt_module_ext

  \param filename The file to load from.
  \param logfunc Logging function where warning and errors are written. May be NULL.
  \param loguser Logging function user context. Used to pass any user-defined data associated with this module to the logging function.
  \param errfunc Error function to define error behaviour. May be NULL.
  \param erruser Error function user context. Used to pass any user-defined data associated with this module to the error function.
  \param errorcode Pointer to an integer where an error may get stored. May be NULL.
  \param error_message Pointer to a string pointer where an error message may get stored. May be NULL.
  \param ctls A map of initial ctl values, see openmpt_module_get_ctls.
  \return A pointer to the constructed openmpt_module, or NULL on failure.
  \sa openmpt_module_ext_create
'/
Function openmpt_module_ext_create_from_filename(_
		ByRef filename As String,_
		ByVal logfunc As openmpt_log_func = 0,_
		ByVal loguser As Any Ptr = 0,_
		ByVal errfunc As openmpt_error_func = 0,_
		ByVal erruser As Any Ptr = 0,_
		ByVal errorcode As Long Ptr = 0,_
		ByVal error_message As Const ZString Ptr Ptr = 0,_
		ByVal ctls As Const openmpt_module_initial_ctl Ptr = 0) As openmpt_module_ext Ptr
	Var file = fopen(filename, "rb")
	Var retval = CPtr(openmpt_module Ptr, 0)
	If(file <> 0) Then
		retval = openmpt_module_ext_create(openmpt_stream_get_file_callbacks(), file, logfunc, loguser, errfunc, erruser, errorcode, error_message, ctls)
		fclose(file)
	EndIf
	Return retval
End Function
