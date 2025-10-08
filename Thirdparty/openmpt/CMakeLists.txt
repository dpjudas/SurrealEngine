cmake_minimum_required(VERSION 3.15)
project(openmpt)

set(LIBOPENMPT_SOURCES
	common/BuildSettings.h
    common/BuildSettingsCompiler.h
    common/ComponentManager.h
    common/FileReader.h
    common/FileReaderFwd.h
    common/GzipWriter.h
    common/Logging.h
    common/Profiler.h
    common/misc_util.h
    common/mptAssert.h
    common/mptBaseMacros.h
    common/mptBaseTypes.h
    common/mptBaseUtils.h
    common/mptCPU.h
    common/mptFileIO.h
    common/mptFileTemporary.h
    common/mptFileType.h
    common/mptPathString.h
    common/mptRandom.h
    common/mptString.h
    common/mptStringBuffer.h
    common/mptStringFormat.h
    common/mptTime.h
    common/serialization_utils.h
    common/version.h
    common/versionNumber.h
    libopenmpt/libopenmpt.h
    libopenmpt/libopenmpt.hpp
    libopenmpt/libopenmpt_config.h
    libopenmpt/libopenmpt_ext.h
    libopenmpt/libopenmpt_ext.hpp
    libopenmpt/libopenmpt_ext_impl.hpp
    libopenmpt/libopenmpt_impl.hpp
    libopenmpt/libopenmpt_internal.h
    libopenmpt/libopenmpt_stream_callbacks_buffer.h
    libopenmpt/libopenmpt_stream_callbacks_fd.h
    libopenmpt/libopenmpt_stream_callbacks_file.h
    libopenmpt/libopenmpt_stream_callbacks_file_mingw.h
    libopenmpt/libopenmpt_stream_callbacks_file_msvcrt.h
    libopenmpt/libopenmpt_stream_callbacks_file_posix.h
    libopenmpt/libopenmpt_stream_callbacks_file_posix_lfs64.h
    libopenmpt/libopenmpt_version.h
    sounddsp/AGC.h
    sounddsp/DSP.h
    sounddsp/EQ.h
    sounddsp/Reverb.h
    soundlib/AudioCriticalSection.h
    soundlib/AudioReadTarget.h
    soundlib/BitReader.h
    soundlib/Container.h
    soundlib/Dlsbank.h
    soundlib/FloatMixer.h
    soundlib/ITCompression.h
    soundlib/ITTools.h
    soundlib/InstrumentSynth.h
    soundlib/IntMixer.h
    soundlib/Loaders.h
    soundlib/MIDIEvents.h
    soundlib/MIDIMacroParser.h
    soundlib/MIDIMacros.h
    soundlib/MODTools.h
    soundlib/MPEGFrame.h
    soundlib/Message.h
    soundlib/MixFuncTable.h
    soundlib/Mixer.h
    soundlib/MixerInterface.h
    soundlib/MixerLoops.h
    soundlib/MixerSettings.h
    soundlib/ModChannel.h
    soundlib/ModInstrument.h
    soundlib/ModSample.h
    soundlib/ModSampleCopy.h
    soundlib/ModSequence.h
    soundlib/OPL.h
    soundlib/OggStream.h
    soundlib/Paula.h
    soundlib/PlayState.h
    soundlib/PlaybackTest.h
    soundlib/Resampler.h
    soundlib/RowVisitor.h
    soundlib/S3MTools.h
    soundlib/SampleCopy.h
    soundlib/SampleIO.h
    soundlib/SampleNormalize.h
    soundlib/Snd_defs.h
    soundlib/Sndfile.h
    soundlib/SoundFilePlayConfig.h
    soundlib/Tables.h
    soundlib/Tagging.h
    soundlib/TinyFFT.h
    soundlib/UMXTools.h
    soundlib/WAVTools.h
    soundlib/WindowedFIR.h
    soundlib/XMTools.h
    soundlib/mod_specifications.h
    soundlib/modcommand.h
    soundlib/modsmp_ctrl.h
    soundlib/opal.h
    soundlib/pattern.h
    soundlib/patternContainer.h
    soundlib/plugins/DigiBoosterEcho.h
    soundlib/plugins/LFOPlugin.h
    soundlib/plugins/PlugInterface.h
    soundlib/plugins/PluginManager.h
    soundlib/plugins/PluginMixBuffer.h
    soundlib/plugins/PluginStructs.h
    soundlib/plugins/SymMODEcho.h
    soundlib/plugins/dmo/Chorus.h
    soundlib/plugins/dmo/Compressor.h
    soundlib/plugins/dmo/DMOPlugin.h
    soundlib/plugins/dmo/DMOUtils.h
    soundlib/plugins/dmo/Distortion.h
    soundlib/plugins/dmo/Echo.h
    soundlib/plugins/dmo/Flanger.h
    soundlib/plugins/dmo/Gargle.h
    soundlib/plugins/dmo/I3DL2Reverb.h
    soundlib/plugins/dmo/ParamEq.h
    soundlib/plugins/dmo/WavesReverb.h
    soundlib/tuning.h
    soundlib/tuningbase.h
    soundlib/tuningcollection.h
    src/mpt/arch/arch.hpp
    src/mpt/arch/feature_fence.hpp
    src/mpt/arch/feature_flags.hpp
    src/mpt/arch/x86_amd64.hpp
    src/mpt/audio/sample.hpp
    src/mpt/audio/span.hpp
    src/mpt/base/algorithm.hpp
    src/mpt/base/aligned_array.hpp
    src/mpt/base/alloc.hpp
    src/mpt/base/arithmetic_shift.hpp
    src/mpt/base/array.hpp
    src/mpt/base/bit.hpp
    src/mpt/base/check_platform.hpp
    src/mpt/base/compiletime_warning.hpp
    src/mpt/base/constexpr_throw.hpp
    src/mpt/base/debugging.hpp
    src/mpt/base/detect.hpp
    src/mpt/base/detect_arch.hpp
    src/mpt/base/detect_compiler.hpp
    src/mpt/base/detect_libc.hpp
    src/mpt/base/detect_libcxx.hpp
    src/mpt/base/detect_os.hpp
    src/mpt/base/detect_quirks.hpp
    src/mpt/base/float.hpp
    src/mpt/base/integer.hpp
    src/mpt/base/macros.hpp
    src/mpt/base/math.hpp
    src/mpt/base/memory.hpp
    src/mpt/base/namespace.hpp
    src/mpt/base/numbers.hpp
    src/mpt/base/numeric.hpp
    src/mpt/base/pointer.hpp
    src/mpt/base/preprocessor.hpp
    src/mpt/base/saturate_cast.hpp
    src/mpt/base/saturate_round.hpp
    src/mpt/base/secure.hpp
    src/mpt/base/semantic_version.hpp
    src/mpt/base/size.hpp
    src/mpt/base/source_location.hpp
    src/mpt/base/span.hpp
    src/mpt/base/tests/tests_base_arithmetic_shift.hpp
    src/mpt/base/tests/tests_base_bit.hpp
    src/mpt/base/tests/tests_base_math.hpp
    src/mpt/base/tests/tests_base_numeric.hpp
    src/mpt/base/tests/tests_base_saturate_cast.hpp
    src/mpt/base/tests/tests_base_saturate_round.hpp
    src/mpt/base/tests/tests_base_wrapping_divide.hpp
    src/mpt/base/type_traits.hpp
    src/mpt/base/utility.hpp
    src/mpt/base/version.hpp
    src/mpt/base/wrapping_divide.hpp
    src/mpt/binary/base64.hpp
    src/mpt/binary/base64url.hpp
    src/mpt/binary/hex.hpp
    src/mpt/binary/tests/tests_binary.hpp
    src/mpt/check/compiler.hpp
    src/mpt/check/libc.hpp
    src/mpt/check/libcxx.hpp
    src/mpt/check/mfc.hpp
    src/mpt/check/windows.hpp
    src/mpt/crc/crc.hpp
    src/mpt/crc/tests/tests_crc.hpp
    src/mpt/detect/dl.hpp
    src/mpt/detect/ltdl.hpp
    src/mpt/detect/mfc.hpp
    src/mpt/detect/nlohmann_json.hpp
    src/mpt/endian/floatingpoint.hpp
    src/mpt/endian/int24.hpp
    src/mpt/endian/integer.hpp
    src/mpt/endian/tests/tests_endian_floatingpoint.hpp
    src/mpt/endian/tests/tests_endian_int24.hpp
    src/mpt/endian/tests/tests_endian_integer.hpp
    src/mpt/endian/type_traits.hpp
    src/mpt/environment/environment.hpp
    src/mpt/exception/exception.hpp
    src/mpt/exception/exception_text.hpp
    src/mpt/exception/logic_error.hpp
    src/mpt/exception/runtime_error.hpp
    src/mpt/format/concat.hpp
    src/mpt/format/default_floatingpoint.hpp
    src/mpt/format/default_formatter.hpp
    src/mpt/format/default_integer.hpp
    src/mpt/format/default_string.hpp
    src/mpt/format/helpers.hpp
    src/mpt/format/join.hpp
    src/mpt/format/message.hpp
    src/mpt/format/message_macros.hpp
    src/mpt/format/simple.hpp
    src/mpt/format/simple_floatingpoint.hpp
    src/mpt/format/simple_integer.hpp
    src/mpt/format/simple_spec.hpp
    src/mpt/format/tests/tests_format_message.hpp
    src/mpt/format/tests/tests_format_simple.hpp
    src/mpt/io/base.hpp
    src/mpt/io/io.hpp
    src/mpt/io/io_span.hpp
    src/mpt/io/io_stdstream.hpp
    src/mpt/io/io_virtual_wrapper.hpp
    src/mpt/io/tests/tests_io.hpp
    src/mpt/io_file/fileref.hpp
    src/mpt/io_file/fstream.hpp
    src/mpt/io_file/inputfile.hpp
    src/mpt/io_file/outputfile.hpp
    src/mpt/io_file_adapter/fileadapter.hpp
    src/mpt/io_file_read/inputfile_filecursor.hpp
    src/mpt/io_file_unique/unique_basename.hpp
    src/mpt/io_file_unique/unique_tempfilename.hpp
    src/mpt/io_read/callbackstream.hpp
    src/mpt/io_read/filecursor.hpp
    src/mpt/io_read/filecursor_callbackstream.hpp
    src/mpt/io_read/filecursor_filename_traits.hpp
    src/mpt/io_read/filecursor_memory.hpp
    src/mpt/io_read/filecursor_stdstream.hpp
    src/mpt/io_read/filecursor_traits_filedata.hpp
    src/mpt/io_read/filecursor_traits_memory.hpp
    src/mpt/io_read/filedata.hpp
    src/mpt/io_read/filedata_base.hpp
    src/mpt/io_read/filedata_base_buffered.hpp
    src/mpt/io_read/filedata_base_seekable.hpp
    src/mpt/io_read/filedata_base_unseekable.hpp
    src/mpt/io_read/filedata_base_unseekable_buffer.hpp
    src/mpt/io_read/filedata_callbackstream.hpp
    src/mpt/io_read/filedata_memory.hpp
    src/mpt/io_read/filedata_stdstream.hpp
    src/mpt/io_read/filereader.hpp
    src/mpt/io_write/buffer.hpp
    src/mpt/mutex/mutex.hpp
    src/mpt/osinfo/class.hpp
    src/mpt/osinfo/dos_memory.hpp
    src/mpt/osinfo/dos_version.hpp
    src/mpt/osinfo/windows_version.hpp
    src/mpt/osinfo/windows_wine_version.hpp
    src/mpt/out_of_memory/out_of_memory.hpp
    src/mpt/parse/parse.hpp
    src/mpt/parse/split.hpp
    src/mpt/parse/tests/tests_parse.hpp
    src/mpt/path/basic_path.hpp
    src/mpt/path/native_path.hpp
    src/mpt/path/os_path.hpp
    src/mpt/path/os_path_long.hpp
    src/mpt/path/path.hpp
    src/mpt/random/any_engine.hpp
    src/mpt/random/crand.hpp
    src/mpt/random/default_engines.hpp
    src/mpt/random/device.hpp
    src/mpt/random/engine.hpp
    src/mpt/random/engine_lcg.hpp
    src/mpt/random/random.hpp
    src/mpt/random/seed.hpp
    src/mpt/random/tests/tests_random.hpp
    src/mpt/string/buffer.hpp
    src/mpt/string/tests/tests_string_buffer.hpp
    src/mpt/string/tests/tests_string_utility.hpp
    src/mpt/string/types.hpp
    src/mpt/string/utility.hpp
    src/mpt/string_transcode/macros.hpp
    src/mpt/string_transcode/tests/tests_string_transcode.hpp
    src/mpt/string_transcode/transcode.hpp
    src/mpt/system_error/system_error.hpp
    src/mpt/uuid/guid.hpp
    src/mpt/uuid/tests/tests_uuid.hpp
    src/mpt/uuid/uuid.hpp
    src/openmpt/all/BuildSettings.hpp
    src/openmpt/all/PlatformFixes.hpp
    src/openmpt/base/Endian.hpp
    src/openmpt/base/FlagSet.hpp
    src/openmpt/base/Int24.hpp
    src/openmpt/base/Types.hpp
    src/openmpt/fileformat_base/magic.hpp
    src/openmpt/logging/Logger.hpp
    src/openmpt/random/ModPlug.hpp
    src/openmpt/soundbase/Copy.hpp
    src/openmpt/soundbase/CopyMix.hpp
    src/openmpt/soundbase/Dither.hpp
    src/openmpt/soundbase/DitherModPlug.hpp
    src/openmpt/soundbase/DitherNone.hpp
    src/openmpt/soundbase/DitherSimple.hpp
    src/openmpt/soundbase/MixSample.hpp
    src/openmpt/soundbase/MixSampleConvert.hpp
    src/openmpt/soundbase/SampleClip.hpp
    src/openmpt/soundbase/SampleClipFixedPoint.hpp
    src/openmpt/soundbase/SampleConvert.hpp
    src/openmpt/soundbase/SampleConvertFixedPoint.hpp
    src/openmpt/soundbase/SampleDecode.hpp
    src/openmpt/soundbase/SampleEncode.hpp
    src/openmpt/soundbase/SampleFormat.hpp
    src/openmpt/soundfile_data/tags.hpp
    src/openmpt/soundfile_data/wav.hpp
    common/ComponentManager.cpp
    common/Logging.cpp
    common/Profiler.cpp
    common/mptFileType.cpp
    common/mptPathString.cpp
    common/mptRandom.cpp
    common/mptStringBuffer.cpp
    common/mptTime.cpp
    common/serialization_utils.cpp
    common/version.cpp
    libopenmpt/libopenmpt_c.cpp
    libopenmpt/libopenmpt_cxx.cpp
    libopenmpt/libopenmpt_ext_impl.cpp
    libopenmpt/libopenmpt_impl.cpp
    sounddsp/AGC.cpp
    sounddsp/DSP.cpp
    sounddsp/EQ.cpp
    sounddsp/Reverb.cpp
    soundlib/AudioCriticalSection.cpp
    soundlib/ContainerMMCMP.cpp
    soundlib/ContainerPP20.cpp
    soundlib/ContainerUMX.cpp
    soundlib/ContainerXPK.cpp
    soundlib/Dlsbank.cpp
    soundlib/Fastmix.cpp
    soundlib/ITCompression.cpp
    soundlib/ITTools.cpp
    soundlib/InstrumentExtensions.cpp
    soundlib/InstrumentSynth.cpp
    soundlib/Load_667.cpp
    soundlib/Load_669.cpp
    soundlib/Load_amf.cpp
    soundlib/Load_ams.cpp
    soundlib/Load_c67.cpp
    soundlib/Load_cba.cpp
    soundlib/Load_dbm.cpp
    soundlib/Load_digi.cpp
    soundlib/Load_dmf.cpp
    soundlib/Load_dsm.cpp
    soundlib/Load_dsym.cpp
    soundlib/Load_dtm.cpp
    soundlib/Load_etx.cpp
    soundlib/Load_far.cpp
    soundlib/Load_fc.cpp
    soundlib/Load_fmt.cpp
    soundlib/Load_ftm.cpp
    soundlib/Load_gdm.cpp
    soundlib/Load_gmc.cpp
    soundlib/Load_gt2.cpp
    soundlib/Load_ice.cpp
    soundlib/Load_imf.cpp
    soundlib/Load_ims.cpp
    soundlib/Load_it.cpp
    soundlib/Load_itp.cpp
    soundlib/Load_kris.cpp
    soundlib/Load_mdl.cpp
    soundlib/Load_med.cpp
    soundlib/Load_mid.cpp
    soundlib/Load_mo3.cpp
    soundlib/Load_mod.cpp
    soundlib/Load_mt2.cpp
    soundlib/Load_mtm.cpp
    soundlib/Load_mus_km.cpp
    soundlib/Load_okt.cpp
    soundlib/Load_plm.cpp
    soundlib/Load_psm.cpp
    soundlib/Load_pt36.cpp
    soundlib/Load_ptm.cpp
    soundlib/Load_puma.cpp
    soundlib/Load_rtm.cpp
    soundlib/Load_s3m.cpp
    soundlib/Load_sfx.cpp
    soundlib/Load_stk.cpp
    soundlib/Load_stm.cpp
    soundlib/Load_stp.cpp
    soundlib/Load_symmod.cpp
    soundlib/Load_tcb.cpp
    soundlib/Load_uax.cpp
    soundlib/Load_ult.cpp
    soundlib/Load_unic.cpp
    soundlib/Load_wav.cpp
    soundlib/Load_xm.cpp
    soundlib/Load_xmf.cpp
    soundlib/MIDIEvents.cpp
    soundlib/MIDIMacroParser.cpp
    soundlib/MIDIMacros.cpp
    soundlib/MODTools.cpp
    soundlib/MPEGFrame.cpp
    soundlib/Message.cpp
    soundlib/MixFuncTable.cpp
    soundlib/MixerLoops.cpp
    soundlib/MixerSettings.cpp
    soundlib/ModChannel.cpp
    soundlib/ModInstrument.cpp
    soundlib/ModSample.cpp
    soundlib/ModSequence.cpp
    soundlib/OPL.cpp
    soundlib/OggStream.cpp
    soundlib/Paula.cpp
    soundlib/PlayState.cpp
    soundlib/PlaybackTest.cpp
    soundlib/RowVisitor.cpp
    soundlib/S3MTools.cpp
    soundlib/SampleFormatBRR.cpp
    soundlib/SampleFormatFLAC.cpp
    soundlib/SampleFormatMP3.cpp
    soundlib/SampleFormatMediaFoundation.cpp
    soundlib/SampleFormatOpus.cpp
    soundlib/SampleFormatSFZ.cpp
    soundlib/SampleFormatVorbis.cpp
    soundlib/SampleFormats.cpp
    soundlib/SampleIO.cpp
    soundlib/Snd_flt.cpp
    soundlib/Snd_fx.cpp
    soundlib/Sndfile.cpp
    soundlib/Sndmix.cpp
    soundlib/SoundFilePlayConfig.cpp
    soundlib/Tables.cpp
    soundlib/Tagging.cpp
    soundlib/TinyFFT.cpp
    soundlib/UMXTools.cpp
    soundlib/UpgradeModule.cpp
    soundlib/WAVTools.cpp
    soundlib/WindowedFIR.cpp
    soundlib/XMTools.cpp
    soundlib/load_j2b.cpp
    soundlib/mod_specifications.cpp
    soundlib/modcommand.cpp
    soundlib/modsmp_ctrl.cpp
    soundlib/pattern.cpp
    soundlib/patternContainer.cpp
    soundlib/plugins/DigiBoosterEcho.cpp
    soundlib/plugins/LFOPlugin.cpp
    soundlib/plugins/PlugInterface.cpp
    soundlib/plugins/PluginManager.cpp
    soundlib/plugins/SymMODEcho.cpp
    soundlib/plugins/dmo/Chorus.cpp
    soundlib/plugins/dmo/Compressor.cpp
    soundlib/plugins/dmo/DMOPlugin.cpp
    soundlib/plugins/dmo/DMOUtils.cpp
    soundlib/plugins/dmo/Distortion.cpp
    soundlib/plugins/dmo/Echo.cpp
    soundlib/plugins/dmo/Flanger.cpp
    soundlib/plugins/dmo/Gargle.cpp
    soundlib/plugins/dmo/I3DL2Reverb.cpp
    soundlib/plugins/dmo/ParamEq.cpp
    soundlib/plugins/dmo/WavesReverb.cpp
    soundlib/tuning.cpp
    soundlib/tuningCollection.cpp

    # dependencies
    ../dr_flac.h
    ../dr_wav.h
    ../minimp3.h
    ../stb_vorbis.h
    ../miniz/miniz.c
    ../miniz/miniz.h
)

set(LIBOPENMPT_WIN32_SOURCES
    common/stdafx.h
)

source_group("common" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/common/.+")
source_group("libopenmpt" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/.+")
source_group("sounddsp" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/sounddsp/.+")
source_group("soundlib" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/.+")
source_group("soundlib\\plugins" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/plugins/.+")
source_group("soundlib\\plugins\\dmo" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/plugins/dmo/.+")
source_group("src\\openmpt\\all" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/all/.+")
source_group("src\\openmpt\\base" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/base/.+")
source_group("src\\openmpt\\fileformat_base" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/fileformat_base/.+")
source_group("src\\openmpt\\logging" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/logging/.+")
source_group("src\\openmpt\\random" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/random/.+")
source_group("src\\openmpt\\soundbase" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/soundbase/.+")
source_group("src\\openmpt\\sounddevice" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/sounddevice/.+")
source_group("src\\openmpt\\soundfile_data" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/soundfile_data/.+")
source_group("src\\openmpt\\soundfile_write" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/soundfile_write/.+")
source_group("src\\openmpt\\streamencoder" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/streamencoder/.+")
source_group("src\\mpt" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/.+")
source_group("src\\mpt\\arch" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/arch/.+")
source_group("src\\mpt\\audio" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/audio/.+")
source_group("src\\mpt\\base" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/base/.+")
source_group("src\\mpt\\binary" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/binary/.+")
source_group("src\\mpt\\check" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/check/.+")
source_group("src\\mpt\\crc" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/crc/.+")
source_group("src\\mpt\\crypto" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/crypto/.+")
source_group("src\\mpt\\detect" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/detect/.+")
source_group("src\\mpt\\endian" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/endian/.+")
source_group("src\\mpt\\environment" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/environment/.+")
source_group("src\\mpt\\exception" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/exception/.+")
source_group("src\\mpt\\format" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/format/.+")
source_group("src\\mpt\\fs" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/fs/.+")
source_group("src\\mpt\\io" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/io/.+")
source_group("src\\mpt\\io_file" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/io_file/.+")
source_group("src\\mpt\\io_file_adapter" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/io_file_adapter/.+")
source_group("src\\mpt\\io_file_read" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/io_file_read/.+")
source_group("src\\mpt\\io_file_unique" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/io_file_unique/.+")
source_group("src\\mpt\\io_read" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/io_read/.+")
source_group("src\\mpt\\io_write" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/io_write/.+")
source_group("src\\mpt\\json" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/json/.+")
source_group("src\\mpt\\library" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/library/.+")
source_group("src\\mpt\\main" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/main/.+")
source_group("src\\mpt\\mutex" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/mutex/.+")
source_group("src\\mpt\\osinfo" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/osinfo/.+")
source_group("src\\mpt\\out_of_memory" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/out_of_memory/.+")
source_group("src\\mpt\\parse" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/parse/.+")
source_group("src\\mpt\\path" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/path/.+")
source_group("src\\mpt\\random" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/random/.+")
source_group("src\\mpt\\string" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/string/.+")
source_group("src\\mpt\\string_transcode" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/string_transcode/.+")
source_group("src\\mpt\\system_error" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/system_error/.+")
source_group("src\\mpt\\test" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/test/.+")
source_group("src\\mpt\\uuid" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/uuid/.+")
source_group("src\\mpt\\uuid_namespace" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/mpt/uuid_namespace/.+")

include_directories(openmpt ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ common src)

# TODO: MPT_WITH_FLAC but using dr_flac.h
add_definitions(-DLIBOPENMPT_BUILD -DMPT_WITH_MINIZ -DMPT_WITH_MINIMP3 -DMPT_WITH_STBVORBIS)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

if(WIN32)
    set(LIBOPENMPT_SOURCES ${LIBOPENMPT_SOURCES} ${LIBOPENMPT_WIN32_SOURCES})
    add_definitions(-DUNICODE -D_UNICODE -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1)
    if (MSVC)
	    # Use all cores for compilation
	    set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}")
        add_definitions(-DMPT_BUILD_MSVC -DMPT_CHECK_CXX_IGNORE_PREPROCESSOR)
    endif()
endif()

add_library(openmpt STATIC ${LIBOPENMPT_SOURCES})
set_target_properties(openmpt PROPERTIES CXX_STANDARD 20)