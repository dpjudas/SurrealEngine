import { Project, Target, File, FilePath, Directory, Environment } from "cppbuild";

var files = [
	"openal-soft.js",

	"common/albit.h",
	"common/albyte.h",
	"common/alcomplex.cpp",
	"common/alcomplex.h",
	"common/aldeque.h",
	"common/alfstream.cpp",
	"common/alfstream.h",
	"common/almalloc.cpp",
	"common/almalloc.h",
	"common/alnumbers.h",
	"common/alnumeric.h",
	"common/aloptional.h",
	"common/alspan.h",
	"common/alstring.cpp",
	"common/alstring.h",
	"common/altraits.h",
	"common/atomic.h",
	"common/comptr.h",
	"common/dynload.cpp",
	"common/dynload.h",
	"common/intrusive_ptr.h",
	"common/opthelpers.h",
	"common/phase_shifter.h",
	"common/polyphase_resampler.cpp",
	"common/polyphase_resampler.h",
	"common/pragmadefs.h",
	"common/ringbuffer.cpp",
	"common/ringbuffer.h",
	"common/strutils.cpp",
	"common/strutils.h",
	"common/threads.cpp",
	"common/threads.h",
	"common/vecmat.h",
	"common/vector.h",

	"al/auxeffectslot.cpp",
	"al/auxeffectslot.h",
	"al/buffer.cpp",
	"al/buffer.h",
	"al/effect.cpp",
	"al/effect.h",
	"al/effects/autowah.cpp",
	"al/effects/chorus.cpp",
	"al/effects/compressor.cpp",
	"al/effects/convolution.cpp",
	"al/effects/dedicated.cpp",
	"al/effects/distortion.cpp",
	"al/effects/echo.cpp",
	"al/effects/effects.cpp",
	"al/effects/effects.h",
	"al/effects/equalizer.cpp",
	"al/effects/fshifter.cpp",
	"al/effects/modulator.cpp",
	"al/effects/null.cpp",
	"al/effects/pshifter.cpp",
	"al/effects/reverb.cpp",
	"al/effects/vmorpher.cpp",
	"al/error.cpp",
	"al/event.cpp",
	"al/event.h",
	"al/extension.cpp",
	"al/filter.cpp",
	"al/filter.h",
	"al/listener.cpp",
	"al/listener.h",
	"al/source.cpp",
	"al/source.h",
	"al/state.cpp",

	"alc/alc.cpp",
	"alc/alu.cpp",
	"alc/alu.h",
	"alc/alconfig.cpp",
	"alc/alconfig.h",
	"alc/context.cpp",
	"alc/context.h",
	"alc/device.cpp",
	"alc/device.h",
	"alc/effects/base.h",
	"alc/effects/autowah.cpp",
	"alc/effects/chorus.cpp",
	"alc/effects/compressor.cpp",
	"alc/effects/convolution.cpp",
	"alc/effects/dedicated.cpp",
	"alc/effects/distortion.cpp",
	"alc/effects/echo.cpp",
	"alc/effects/equalizer.cpp",
	"alc/effects/fshifter.cpp",
	"alc/effects/modulator.cpp",
	"alc/effects/null.cpp",
	"alc/effects/pshifter.cpp",
	"alc/effects/reverb.cpp",
	"alc/effects/vmorpher.cpp",
	"alc/inprogext.h",
	"alc/panning.cpp",

	"alc/backends/base.cpp",
	"alc/backends/base.h",
	"alc/backends/loopback.cpp",
	"alc/backends/loopback.h",
	"alc/backends/null.cpp",
	"alc/backends/null.h",
	"alc/backends/wasapi.cpp",
	"alc/backends/wasapi.h",

	"core/ambdec.cpp",
	"core/ambdec.h",
	"core/ambidefs.cpp",
	"core/ambidefs.h",
	"core/async_event.h",
	"core/bformatdec.cpp",
	"core/bformatdec.h",
	"core/bs2b.cpp",
	"core/bs2b.h",
	"core/bsinc_defs.h",
	"core/bsinc_tables.cpp",
	"core/bsinc_tables.h",
	"core/bufferline.h",
	"core/buffer_storage.cpp",
	"core/buffer_storage.h",
	"core/context.cpp",
	"core/context.h",
	"core/converter.cpp",
	"core/converter.h",
	"core/cpu_caps.cpp",
	"core/cpu_caps.h",
	"core/cubic_defs.h",
	"core/cubic_tables.cpp",
	"core/cubic_tables.h",
	"core/devformat.cpp",
	"core/devformat.h",
	"core/device.cpp",
	"core/device.h",
	"core/effects/base.h",
	"core/effectslot.cpp",
	"core/effectslot.h",
	"core/except.cpp",
	"core/except.h",
	"core/filters/biquad.h",
	"core/filters/biquad.cpp",
	"core/filters/nfc.cpp",
	"core/filters/nfc.h",
	"core/filters/splitter.cpp",
	"core/filters/splitter.h",
	"core/fmt_traits.cpp",
	"core/fmt_traits.h",
	"core/fpu_ctrl.cpp",
	"core/fpu_ctrl.h",
	"core/front_stablizer.h",
	"core/helpers.cpp",
	"core/helpers.h",
	"core/hrtf.cpp",
	"core/hrtf.h",
	"core/logging.cpp",
	"core/logging.h",
	"core/mastering.cpp",
	"core/mastering.h",
	"core/mixer.cpp",
	"core/mixer.h",
	"core/resampler_limits.h",
	"core/uhjfilter.cpp",
	"core/uhjfilter.h",
	"core/uiddefs.cpp",
	"core/voice.cpp",
	"core/voice.h",
	"core/voice_change.h",

	"core/mixer/defs.h",
	"core/mixer/hrtfbase.h",
	"core/mixer/hrtfdefs.h",
	"core/mixer/mixer_c.cpp",

	"include/AL/al.h",
	"include/AL/alc.h",
	"include/AL/alext.h",
	"include/AL/efx.h",
	"include/AL/efx-presets.h",
];

var defines = [
	"AL_BUILD_LIBRARY",
	"AL_ALEXT_PROTOTYPES",
	"ALC_API=__declspec(dllexport)",
	"AL_API=__declspec(dllexport)",
	"_WINDLL",
	"_MBCS",
	"WIN32",
	"_WIN32",
	"_WINDOWS",
	"NOMINMAX",
	"_CRT_SECURE_NO_WARNINGS",
	"OpenAL_EXPORTS",
];

var includes = [
	".",
	"common",
	"include",
	Directory.buildPath("OpenALSoft"),
];

// We don't care about setting this too precisely as we only use manual compiling on Windows anyway

var configInDefines = [
	"ALSOFT_EMBED_HRTF_DATA",
	//"HAVE_POSIX_MEMALIGN",
	"HAVE__ALIGNED_MALLOC",
	//"HAVE_PROC_PIDPATH",
	//"HAVE_GETOPT",
	//"HAVE_RTKIT",
	//"HAVE_ALSA",
	//"HAVE_OSS",
	//"HAVE_PIPEWIRE",
	//"HAVE_SOLARIS",
	//"HAVE_SNDIO",
	"HAVE_WASAPI",
	//"HAVE_DSOUND",
	//"HAVE_WINMM",
	//"HAVE_PORTAUDIO",
	//"HAVE_PULSEAUDIO",
	//"HAVE_JACK",
	//"HAVE_COREAUDIO",
	//"HAVE_OPENSL",
	//"HAVE_OBOE",
	//"HAVE_WAVE",
	//"HAVE_SDL2",
	//"HAVE_DLFCN_H",
	//"HAVE_PTHREAD_NP_H",
	"HAVE_MALLOC_H",
	//"HAVE_CPUID_H",
	"HAVE_INTRIN_H",
	"HAVE_GUIDDEF_H",
	//"HAVE_INITGUID_H",
	//"HAVE_GCC_GET_CPUID",
	"HAVE_CPUID_INTRINSIC",
	//"HAVE_PTHREAD_SETSCHEDPARAM",
	//"HAVE_PTHREAD_SETNAME_NP",
	//"HAVE_PTHREAD_SET_NAME_NP",
	//"ALSOFT_INSTALL_DATADIR \"" + ALSOFT_INSTALL_DATADIR + "\"",
];

if (Environment.isARM64()) {
	configInDefines = configInDefines.concat([
		"HAVE_NEON",
	]);
	files = files.concat([
		"core/mixer/mixer_neon.cpp",
	]);
}
else if (Environment.isX64()) {
	configInDefines = configInDefines.concat([
		"HAVE_SSE_INTRINSICS",
		"HAVE_SSE",
		"HAVE_SSE2",
		"HAVE_SSE3",
		"HAVE_SSE4_1",
	]);
	files = files.concat([
		"core/mixer/mixer_sse.cpp",
		"core/mixer/mixer_sse2.cpp",
		"core/mixer/mixer_sse3.cpp",
		"core/mixer/mixer_sse41.cpp",
	]);
}

var versionInDefines = [
	"ALSOFT_VERSION \"1.23.1\"",
	"ALSOFT_VERSION_NUM 1,23,1,0",
	"ALSOFT_GIT_BRANCH \"UNKNOWN\"",
	"ALSOFT_GIT_COMMIT_HASH \"unknown\"",
];

var configH = "";
for (const define of configInDefines) {
	configH += "#define " + define + "\r\n";
}

var versionH = "";
for (const define of versionInDefines) {
	versionH += "#define " + define + "\r\n"
}

Directory.create(Directory.buildPath("OpenALSoft"));
File.writeAllText(Directory.buildPath("OpenALSoft/config.h"), configH);
File.writeAllText(Directory.buildPath("OpenALSoft/version.h"), versionH);

var defaultHrtf = File.readAllBytesAsHex("hrtf/Default HRTF.mhr");
// For each pair of characters, indent them and prepend the 0x prefix, and append a comma separateor.
defaultHrtf = defaultHrtf.replace(/(..)/g, "0x$1,");
defaultHrtf = defaultHrtf.replace(/(0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,)/g, "$1\n");
File.writeAllText(Directory.buildPath("OpenALSoft/default_hrtf.txt"), defaultHrtf);

if (Environment.isWindows()) {
	var openal = Target.addDynamicLibrary("OpenAL");
	openal.addFiles(files);
	openal.addDefines(defines);
	openal.addDefines(["RESTRICT="]);
	// openal.addDefines(["RESTRICT=__restrict"]);
	openal.addIncludePaths(includes);
	openal.addCompileOptions(["/std:c++14"]);
	openal.addCompileOptions(["/W4", "/w14640", "/wd4065", "/wd4127", "/wd4268", "/wd4324", "/wd5030", "/wd5051", "/wd4244", "/wd4267"]);
}
