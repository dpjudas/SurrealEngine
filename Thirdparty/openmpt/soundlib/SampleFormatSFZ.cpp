/*
 * SampleFormatSFZ.cpp
 * -------------------
 * Purpose: Loading and saving SFZ instruments.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/TrackerSettings.h"
#include "mpt/io_file/inputfile.hpp"
#include "mpt/io_file_read/inputfile_filecursor.hpp"
#include "../common/mptFileIO.h"
#endif // MODPLUG_TRACKER
#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io_file/outputfile.hpp"
#include "../common/mptFileIO.h"
#ifdef MODPLUG_TRACKER
#include "mpt/fs/fs.hpp"
#endif // MODPLUG_TRACKER
#endif // !MODPLUG_NO_FILESAVE
#include "modsmp_ctrl.h"
#include "mpt/base/numbers.hpp"
#include "mpt/parse/parse.hpp"

#include <functional>

OPENMPT_NAMESPACE_BEGIN

#ifdef MPT_EXTERNAL_SAMPLES

template<size_t N>
static bool SFZStartsWith(const std::string_view &l, const char(&r)[N])
{
	return l.substr(0, N - 1) == r;
}

template <size_t N>
static bool SFZEndsWith(const std::string_view &l, const char (&r)[N])
{
	return l.size() >= (N - 1) && l.substr(l.size() - (N - 1), N - 1) == r;
}

static bool SFZIsNumeric(const std::string_view &str)
{
	return std::find_if(str.begin(), str.end(), [](char c) { return c < '0' || c > '9'; }) == str.end();
}

struct SFZControl
{
	std::string defaultPath;
	int8 octaveOffset = 0, noteOffset = 0;

	void Parse(const std::string_view key, const std::string &value)
	{
		if(key == "default_path")
			defaultPath = value;
		else if(key == "octave_offset")
			octaveOffset = mpt::parse<int8>(value);
		else if(key == "note_offset")
			noteOffset = mpt::parse<int8>(value);
	}
};

struct SFZFlexEG
{
	using PointIndex = decltype(InstrumentEnvelope().nLoopStart);

	std::vector<std::pair<double, double>> points;
	double amplitude = 0;  // percentage (100 = full volume range)
	double pan = 0;        // percentage (100 = full pan range)
	double pitch = 0;      // in cents
	double cutoff = 0;     // in cents
	PointIndex sustain = 0;

	void Parse(std::string_view key, const std::string &value)
	{
		key = key.substr(key.find('_') + 1);
		const double v = mpt::parse<double>(value);

		const bool isTime = SFZStartsWith(key, "time"), isLevel = SFZStartsWith(key, "level");
		std::string_view pointStr;
		if(isTime)
			pointStr = key.substr(4);
		else if(isLevel)
			pointStr = key.substr(5);

		if(!pointStr.empty() && SFZIsNumeric(pointStr))
		{
			PointIndex point = mpt::parse<PointIndex>(std::string(pointStr));
			if(point >= points.size() && point < MAX_ENVPOINTS)
				points.resize(point + 1);

			if(point < points.size())
			{
				if(isTime)
					points[point].first = v;
				else
					points[point].second = v;
			}
			return;
		}

		if(key == "points")
			points.resize(std::min(static_cast<PointIndex>(v), static_cast<PointIndex>(MAX_ENVPOINTS)));
		else if(key == "sustain")
			sustain = mpt::saturate_round<PointIndex>(v);
		else if(key == "amplitude" || key == "ampeg")
			amplitude = v;
		else if(key == "pan")
			pan = v;
		else if(key == "pitch")
			pitch = v;
		else if(key == "cutoff")
			cutoff = v;
	}

	void ConvertToMPT(ModInstrument *ins, const CSoundFile &sndFile) const
	{
		if(amplitude)
			ConvertToMPT(ins, sndFile, ENV_VOLUME, amplitude / 100.0, 0.0, 1.0);
		if(pan)
			ConvertToMPT(ins, sndFile, ENV_PANNING, pan / 100.0, -1.0, 1.0);
		if(pitch)
			ConvertToMPT(ins, sndFile, ENV_PITCH, pitch / 1600.0, -1.0, 1.0);
		if(cutoff)
			ConvertToMPT(ins, sndFile, ENV_PITCH, cutoff, 0.0, 1.0, true);
	}

	void ConvertToMPT(ModInstrument *ins, const CSoundFile &sndFile, EnvelopeType envType, double scale, double minVal, double maxVal, bool forceFilter = false) const
	{
		const double tickDuration = sndFile.m_PlayState.m_nSamplesPerTick / static_cast<double>(sndFile.GetSampleRate());
		if(tickDuration <= 0 || points.empty() || scale == 0.0)
			return;

		auto &env = ins->GetEnvelope(envType);
		std::function<double(double)> conversionFunc = Identity;
		if(forceFilter && envType == ENV_PITCH)
		{
			env.dwFlags.set(ENV_FILTER);
			conversionFunc = FilterConversionFunc(*ins, sndFile);
		}

		env.clear();
		env.reserve(points.size());

		const auto ToValue = std::bind(SFZFlexEG::ToValue, std::placeholders::_1, scale, minVal, maxVal, conversionFunc);

		int32 prevTick = -1;
		// If the first envelope point's time is greater than 0, we fade in from a neutral value
		if(points.front().first > 0)
		{
			env.push_back({0, ToValue(0.0)});
			prevTick = 0;
		}

		for(const auto &point : points)
		{
			const auto tick = mpt::saturate_cast<EnvelopeNode::tick_t>(prevTick + ToTicks(point.first, tickDuration));
			const auto value = ToValue(point.second);
			env.push_back({tick, value});
			prevTick = tick;
			if(tick == Util::MaxValueOfType(tick))
				break;
		}

		if(sustain < env.size() && !(envType == ENV_VOLUME && sustain == env.size() - 1u && env.back().value == 0))
		{
			env.nSustainStart = env.nSustainEnd = sustain;
			env.dwFlags.set(ENV_SUSTAIN);
		} else
		{
			env.dwFlags.reset(ENV_SUSTAIN);
		}
		env.dwFlags.set(ENV_ENABLED);

		if(envType == ENV_VOLUME && env.nSustainEnd > 0)
			env.nReleaseNode = env.nSustainEnd;
	}

protected:
	static EnvelopeNode::tick_t ToTicks(double duration, double tickDuration)
	{
		return std::max(EnvelopeNode::tick_t(1), mpt::saturate_round<EnvelopeNode::tick_t>(duration / tickDuration));
	}

	static EnvelopeNode::value_t ToValue(double value, double scale, double minVal, double maxVal, const std::function<double(double)> &conversionFunc)
	{
		value = conversionFunc((value * scale - minVal) / (maxVal - minVal)) * double(ENVELOPE_MAX) + double(ENVELOPE_MIN);
		Limit(value, double(ENVELOPE_MIN), double(ENVELOPE_MAX));
		return mpt::saturate_round<EnvelopeNode::value_t>(value);
	}

	static double Identity(double v) noexcept { return v; }

	static double CentsToFilterCutoff(double v, const CSoundFile &sndFile, int envBaseCutoff, float envBaseFreq)
	{
		const auto freq = static_cast<double>(envBaseFreq) * std::pow(2.0, v / 1200.0);
		return Util::muldivr(sndFile.FrequencyToCutOff(freq), 127, envBaseCutoff) / 127.0;
	}

	static std::function<double(double)> FilterConversionFunc(const ModInstrument &ins, const CSoundFile &sndFile)
	{
		const auto envBaseCutoff = ins.IsCutoffEnabled() ? ins.GetCutoff() : 127;
		const auto envBaseFreq = sndFile.CutOffToFrequency(envBaseCutoff);
		return std::bind(CentsToFilterCutoff, std::placeholders::_1, std::cref(sndFile), envBaseCutoff, envBaseFreq);
	}
};

struct SFZEnvelope
{
	double startLevel = 0, delay = 0, attack = 0, hold = 0;
	double decay = 0, sustainLevel = 100, release = 0, depth = 0;

	void Parse(std::string_view key, const std::string &value)
	{
		key = key.substr(key.find('_') + 1);
		double v = mpt::parse<double>(value);
		if(key == "depth")
			Limit(v, -12000.0, 12000.0);
		else if(key == "start" || key == "sustain")
			Limit(v, -100.0, 100.0);
		else
			Limit(v, 0.0, 100.0);

		if(key == "start")
			startLevel = v;
		else if(key == "delay")
			delay = v;
		else if(key == "attack")
			attack = v;
		else if(key == "hold")
			hold = v;
		else if(key == "decay")
			decay = v;
		else if(key == "sustain")
			sustainLevel = v;
		else if(key == "release")
			release = v;
		else if(key == "depth")
			depth = v;
	}

	void ConvertToMPT(ModInstrument *ins, const CSoundFile &sndFile, EnvelopeType envType, bool forceFilter = false) const
	{
		SFZFlexEG eg;
		if(envType == ENV_VOLUME)
			eg.amplitude = 1.0;
		else if(envType == ENV_PITCH && !forceFilter)
			eg.pitch = depth / 100.0;
		else if(envType == ENV_PITCH && forceFilter)
			eg.cutoff = depth / 100.0;

		auto &env = eg.points;
		if(attack > 0 || delay > 0)
		{
			env.push_back({0.0, startLevel});
			if(delay > 0)
				env.push_back({delay, env.back().second});
			env.push_back({attack, 100.0});
		}
		if(hold > 0)
		{
			if(env.empty())
				env.push_back({0.0, 100.0});
			env.push_back({hold, env.back().second});
		}
		if(env.empty())
			env.push_back({0.0, 100.0});
		if(env.back().second != sustainLevel)
			env.push_back({decay, sustainLevel});
		if(sustainLevel != 0)
		{
			eg.sustain = static_cast<SFZFlexEG::PointIndex>(env.size() - 1);
			env.push_back({release, 0.0});
		} else
		{
			eg.sustain = std::numeric_limits<SFZFlexEG::PointIndex>::max();
		}

		eg.ConvertToMPT(ins, sndFile);
	}
};


struct SFZRegion
{
	enum class LoopMode
	{
		kUnspecified,
		kContinuous,
		kOneShot,
		kSustain,
		kNoLoop
	};

	enum class LoopType
	{
		kUnspecified,
		kForward,
		kBackward,
		kAlternate,
	};

	size_t filenameOffset = 0;
	std::string filename, globalName, regionName;
	SFZEnvelope ampEnv, pitchEnv, filterEnv;
	std::vector<SFZFlexEG> flexEGs;
	SmpLength loopStart = 0, loopEnd = 0;
	SmpLength end = MAX_SAMPLE_LENGTH, offset = 0;
	LoopMode loopMode = LoopMode::kUnspecified;
	LoopType loopType = LoopType::kUnspecified;
	double loopCrossfade = 0.0;
	double cutoff = 0;         // in Hz
	double resonance = 0;      // 0...40dB
	double filterRandom = 0;   // 0...9600 cents
	double volume = 0;         // -144dB...+6dB
	double amplitude = 100.0;  // 0...100
	double pitchBend = 200;    // -9600...9600 cents
	double pitchLfoFade = 0;   // 0...100 seconds
	double pitchLfoDepth = 0;  // -1200...12000
	double pitchLfoFreq = 0;   // 0...20 Hz
	double panning = -128;     // -100...+100
	double finetune = 0;       // in cents
	int8 transpose = 0;
	uint8 keyLo = 0, keyHi = 127, keyRoot = 60;
	FilterMode filterType = FilterMode::Unchanged;
	uint8 polyphony = 255;
	bool useSampleKeyRoot = false;
	bool invertPhase = false;

	template<typename T, typename Tc>
	static void Read(const std::string &valueStr, T &value, Tc valueMin = std::numeric_limits<T>::min(), Tc valueMax = std::numeric_limits<T>::max())
	{
		double valueF = mpt::parse<double>(valueStr);
		if constexpr(std::numeric_limits<T>::is_integer)
		{
			valueF = mpt::round(valueF);
		}
		Limit(valueF, static_cast<double>(valueMin), static_cast<double>(valueMax));
		value = static_cast<T>(valueF);
	}

	static uint8 ReadKey(const std::string &value, const SFZControl &control)
	{
		if(value.empty())
			return 0;

		int key = 0;
		if(value[0] >= '0' && value[0] <= '9')
		{
			// MIDI key
			key = mpt::parse<uint8>(value);
		} else if(value.length() < 2)
		{
			return 0;
		} else
		{
			// Scientific pitch
			static constexpr int8 keys[] = { 9, 11, 0, 2, 4, 5, 7 };
			static_assert(std::size(keys) == 'g' - 'a' + 1);
			auto keyC = value[0];
			if(keyC >= 'A' && keyC <= 'G')
				key = keys[keyC - 'A'];
			if(keyC >= 'a' && keyC <= 'g')
				key = keys[keyC - 'a'];
			else
				return 0;

			uint8 octaveOffset = 1;
			if(value[1] == '#')
			{
				key++;
				octaveOffset = 2;
			} else if(value[1] == 'b' || value[1] == 'B')
			{
				key--;
				octaveOffset = 2;
			}
			if(octaveOffset >= value.length())
				return 0;

			int8 octave = mpt::parse<int8>(value.c_str() + octaveOffset);
			key += (octave + 1) * 12;
		}
		key += control.octaveOffset * 12 + control.noteOffset;
		return static_cast<uint8>(Clamp(key, 0, 127));
	}

	void Parse(const std::string_view key, const std::string &value, const SFZControl &control)
	{
		if(key == "sample")
		{
			filename = control.defaultPath + value;
			filenameOffset = control.defaultPath.size();
		}
		else if(key == "global_label")
			globalName = value;
		else if(key == "region_label")
			regionName = value;
		else if(key == "lokey")
			keyLo = ReadKey(value, control);
		else if(key == "hikey")
			keyHi = ReadKey(value, control);
		else if(key == "pitch_keycenter")
		{
			keyRoot = ReadKey(value, control);
			useSampleKeyRoot = (value == "sample");
		}
		else if(key == "key")
		{
			keyLo = keyHi = keyRoot = ReadKey(value, control);
			useSampleKeyRoot = false;
		}
		else if(key == "bend_up" || key == "bendup")
			Read(value, pitchBend, -9600.0, 9600.0);
		else if(key == "pitchlfo_fade")
			Read(value, pitchLfoFade, 0.0, 100.0);
		else if(key == "pitchlfo_depth")
			Read(value, pitchLfoDepth, -12000.0, 12000.0);
		else if(key == "pitchlfo_freq")
			Read(value, pitchLfoFreq, 0.0, 20.0);
		else if(key == "volume")
			Read(value, volume, -144.0, 6.0);
		else if(key == "amplitude")
			Read(value, amplitude, 0.0, 100.0);
		else if(key == "pan")
			Read(value, panning, -100.0, 100.0);
		else if(key == "transpose")
			Read(value, transpose, -127, 127);
		else if(key == "tune")
			Read(value, finetune, -100.0, 100.0);
		else if(key == "end")
			Read(value, end, SmpLength(0), MAX_SAMPLE_LENGTH);
		else if(key == "offset")
			Read(value, offset, SmpLength(0), MAX_SAMPLE_LENGTH);
		else if(key == "loop_start" || key == "loopstart")
			Read(value, loopStart, SmpLength(0), MAX_SAMPLE_LENGTH);
		else if(key == "loop_end" || key == "loopend")
			Read(value, loopEnd, SmpLength(0), MAX_SAMPLE_LENGTH);
		else if(key == "loop_crossfade" || key == "loopcrossfade")
			Read(value, loopCrossfade, 0.0, DBL_MAX);
		else if(key == "loop_mode" || key == "loopmode")
		{
			if(value == "loop_continuous")
				loopMode = LoopMode::kContinuous;
			else if(value == "one_shot")
				loopMode = LoopMode::kOneShot;
			else if(value == "loop_sustain")
				loopMode = LoopMode::kSustain;
			else if(value == "no_loop")
				loopMode = LoopMode::kNoLoop;
		}
		else if(key == "loop_type" || key == "looptype")
		{
			if(value == "forward")
				loopType = LoopType::kForward;
			else if(value == "backward")
				loopType = LoopType::kBackward;
			else if(value == "alternate")
				loopType = LoopType::kAlternate;
		}
		else if(key == "cutoff")
			Read(value, cutoff, 0.0, 96000.0);
		else if(key == "fil_random")
			Read(value, filterRandom, 0.0, 9600.0);
		else if(key == "resonance")
			Read(value, resonance, 0.0, 40.0);
		else if(key == "polyphony")
			Read(value, polyphony, 0, 255);
		else if(key == "phase")
			invertPhase = (value == "invert");
		else if(key == "fil_type" || key == "filtype")
		{
			if(value == "lpf_1p" || value == "lpf_2p" || value == "lpf_4p" || value == "lpf_6p")
				filterType = FilterMode::LowPass;
			else if(value == "hpf_1p" || value == "hpf_2p" || value == "hpf_4p" || value == "hpf_6p")
				filterType = FilterMode::HighPass;
			// Alternatives: bpf_2p, brf_2p
		}
		else if(SFZStartsWith(key, "ampeg_"))
			ampEnv.Parse(key, value);
		else if(SFZStartsWith(key, "fileg_"))
			filterEnv.Parse(key, value);
		else if(SFZStartsWith(key, "pitcheg_"))
			pitchEnv.Parse(key, value);
		else if(SFZStartsWith(key, "eg") && SFZIsNumeric(key.substr(2, 2)) && key.substr(4, 1) == "_")
		{
			uint8 eg = mpt::parse<uint8>(std::string(key.substr(2, 2)));
			if(eg >= flexEGs.size())
				flexEGs.resize(eg + 1);
			flexEGs[eg].Parse(key, value);
		}
	}
};

struct SFZInputFile
{
	FileReader file;
	std::unique_ptr<mpt::IO::InputFile> inputFile;  // FileReader has pointers into this so its address must not change
	std::string remain;

	SFZInputFile(FileReader f = {}, std::unique_ptr<mpt::IO::InputFile> i = {}, std::string r = {})
		: file{std::move(f)}, inputFile{std::move(i)}, remain{std::move(r)} {}
	SFZInputFile(SFZInputFile &&) = default;
};

bool CSoundFile::ReadSFZInstrument(INSTRUMENTINDEX nInstr, FileReader &file)
{
	file.Rewind();

	enum { kNone, kGlobal, kMaster, kGroup, kRegion, kControl, kCurve, kEffect, kUnknown } section = kNone;
	bool inMultiLineComment = false;
	SFZControl control;
	SFZRegion group, master, globals;
	std::vector<SFZRegion> regions;
	std::map<std::string, std::string> macros;
	std::vector<SFZInputFile> files;
	files.emplace_back(file);

	std::string s;
	while(!files.empty())
	{
		if(!files.back().file.ReadLine(s, 1024))
		{
			// Finished reading file, so back to remaining characters of the #include line from the previous file
			s = std::move(files.back().remain);
			files.pop_back();
		}

		if(inMultiLineComment)
		{
			if(auto commentEnd = s.find("*/"); commentEnd != std::string::npos)
			{
				s.erase(0, commentEnd + 2);
				inMultiLineComment = false;
			} else
			{
				continue;
			}
		}

		// First, terminate line at the start of a comment block
		if(auto commentPos = s.find("//"); commentPos != std::string::npos)
		{
			s.resize(commentPos);
		}

		// Now, read the tokens.
		// This format is so funky that no general tokenizer approach seems to work here...
		// Consider this jolly good example found at https://stackoverflow.com/questions/5923895/tokenizing-a-custom-text-file-format-file-using-c-sharp
		// <region>sample=piano C3.wav key=48 ampeg_release=0.7 // a comment here
		// <region>key = 49 sample = piano Db3.wav
		// <region>
		// group=1
		// key = 48
		//     sample = piano D3.ogg
		// The original sfz specification claims that spaces around = are not allowed, but a quick look into the real world tells us otherwise.

		while(!s.empty())
		{
			s.erase(0, s.find_first_not_of(" \t"));

			const bool isDefine = SFZStartsWith(s, "#define ") || SFZStartsWith(s, "#define\t");

			// Replace macros (unless this is a #define statement, to allow for macro re-definition)
			if(!isDefine)
			{
				for(const auto &[oldStr, newStr] : macros)
				{
					std::string::size_type pos = 0;
					while((pos = s.find(oldStr, pos)) != std::string::npos)
					{
						s.replace(pos, oldStr.length(), newStr);
						pos += newStr.length();
					}
				}
			}

			if(s.empty())
				break;

			std::string::size_type charsRead = 0;

			if(s[0] == '<' && (charsRead = s.find('>')) != std::string::npos)
			{
				// Section header
				const auto sec = std::string_view(s).substr(1, charsRead - 1);
				section = kUnknown;
				if(sec == "global")
				{
					section = kGlobal;
					// Reset global parameters
					globals = SFZRegion();
				} else if(sec == "master")
				{
					section = kMaster;
					// Reset master parameters
					master = globals;
				} else if(sec == "group")
				{
					section = kGroup;
					// Reset group parameters
					group = master;
				} else if(sec == "region")
				{
					section = kRegion;
					regions.push_back(group);
				} else if(sec == "control")
				{
					section = kControl;
				} else if(sec == "curve")
				{
					section = kCurve;
				} else if(sec == "effect")
				{
					section = kEffect;
				}
				charsRead++;
			} else if(isDefine)
			{
				// Macro definition
				charsRead += 8;
				auto keyStart = s.find_first_not_of(" \t", 8);
				auto keyEnd = s.find_first_of(" \t", keyStart);
				auto valueStart = s.find_first_not_of(" \t", keyEnd);
				if(keyStart != std::string::npos && valueStart != std::string::npos)
				{
					charsRead = s.find_first_of(" \t", valueStart);
					const auto key = s.substr(keyStart, keyEnd - keyStart);
					if(key.length() > 1 && key[0] == '$')
						macros[std::move(key)] = s.substr(valueStart, charsRead - valueStart);
				} else
				{
					break;
				}
			} else if(SFZStartsWith(s, "#include ") || SFZStartsWith(s, "#include\t"))
			{
				// Include other sfz file
				auto fileStart = s.find("\"", 9);  // Yes, there can be arbitrary characters before the opening quote, at least that's how sforzando does it.
				auto fileEnd = s.find("\"", fileStart + 1);
				if(fileStart != std::string::npos && fileEnd != std::string::npos)
				{
					charsRead = fileEnd + 1;
					fileStart++;
				} else
				{
					break;
				}

				std::string filenameU8 = s.substr(fileStart, fileEnd - fileStart);
				mpt::PathString filename = mpt::PathString::FromUTF8(filenameU8);
				if(!filename.empty())
				{
					if(filenameU8.find(':') == std::string::npos)
						filename = file.GetOptionalFileName().value_or(P_("")).GetDirectoryWithDrive() + filename;
					filename = filename.Simplify();
					// Avoid recursive #include
					if(std::find_if(files.begin(), files.end(), [&filename](const SFZInputFile &f) { return f.file.GetOptionalFileName().value_or(P_("")) == filename; }) == files.end())
					{
						auto f = std::make_unique<mpt::IO::InputFile>(filename);
						if(f->IsValid())
						{
							s.erase(0, charsRead);
							files.emplace_back(GetFileReader(*f), std::move(f), std::move(s));
							break;
						} else
						{
							AddToLog(LogWarning, U_("Unable to load include file: ") + filename.ToUnicode());
						}
					} else
					{
						AddToLog(LogWarning, U_("Recursive include file ignored: ") + filename.ToUnicode());
					}
				}
			} else if(SFZStartsWith(s, "/*"))
			{
				// Multi-line comment
				if(auto commentEnd = s.find("*/", charsRead + 2); commentEnd != std::string::npos)
				{
					charsRead = commentEnd;
				} else
				{
					inMultiLineComment = true;
					charsRead = s.length();
				}
			} else if(section == kNone)
			{
				// Garbage before any section, probably not an sfz file
				return false;
			} else if(s.find('=') != std::string::npos)
			{
				// Read key=value pair
				auto keyEnd = s.find_first_of(" \t=");
				auto valueStart = s.find_first_not_of(" \t=", keyEnd);
				if(valueStart == std::string::npos)
				{
					break;
				}
				const std::string key = mpt::ToLowerCaseAscii(s.substr(0, keyEnd));
				// Currently defined *_label opcodes are global_label, group_label, master_label, region_label, sw_label
				if(key == "sample" || key == "default_path" || SFZStartsWith(key, "label_cc") || SFZStartsWith(key, "label_key") || SFZEndsWith(key, "_label"))
				{
					// Sample / CC name may contain spaces...
					charsRead = s.find_first_of("=\t<", valueStart);
					if(charsRead != std::string::npos && s[charsRead] == '=')
					{
						// Backtrack to end of key
						while(charsRead > valueStart && s[charsRead] == ' ')
							charsRead--;
						// Backtrack to start of key
						while(charsRead > valueStart && s[charsRead] != ' ')
							charsRead--;
					}
				} else
				{
					charsRead = s.find_first_of(" \t<", valueStart);
				}
				const std::string value = s.substr(valueStart, charsRead - valueStart);

				switch(section)
				{
				case kGlobal:
					globals.Parse(key, value, control);
					[[fallthrough]];
				case kMaster:
					master.Parse(key, value, control);
					[[fallthrough]];
				case kGroup:
					group.Parse(key, value, control);
					break;
				case kRegion:
					regions.back().Parse(key, value, control);
					break;
				case kControl:
					control.Parse(key, value);
					break;
				case kNone:
				case kCurve:
				case kEffect:
				case kUnknown:
					break;
				}
			} else
			{
				// Garbage, probably not an sfz file
				return false;
			}

			// Remove the token(s) we just read
			s.erase(0, charsRead);
		}
	}

	if(regions.empty())
		return false;


	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
		return false;

	RecalculateSamplesPerTick();
	DestroyInstrument(nInstr, deleteAssociatedSamples);
	if(nInstr > m_nInstruments) m_nInstruments = nInstr;
	Instruments[nInstr] = pIns;
	pIns->name = mpt::ToCharset(GetCharsetInternal(), mpt::Charset::UTF8, globals.globalName);

	SAMPLEINDEX prevSmp = 0;
	for(auto &region : regions)
	{
		uint8 keyLo = region.keyLo, keyHi = region.keyHi;
		if(keyLo > keyHi)
			continue;
		Clamp<uint8, uint8>(keyLo, 0, NOTE_MAX - NOTE_MIN);
		Clamp<uint8, uint8>(keyHi, 0, NOTE_MAX - NOTE_MIN);
		SAMPLEINDEX smp = GetNextFreeSample(nInstr, prevSmp + 1);
		if(smp == SAMPLEINDEX_INVALID)
			break;
		prevSmp = smp;

		ModSample &sample = Samples[smp];
		sample.Initialize(MOD_TYPE_MPT);
		if(const auto synthSample = std::string_view(region.filename).substr(region.filenameOffset); SFZStartsWith(synthSample, "*"))
		{
			sample.nLength = 256;
			sample.nC5Speed = mpt::saturate_round<uint32>(sample.nLength * 261.6255653);
			sample.uFlags.set(CHN_16BIT);
			std::function<uint16(int32)> generator;
			if(synthSample == "*sine")
				generator = [](int32 i) { return mpt::saturate_round<int16>(std::sin(i * ((2.0 * mpt::numbers::pi) / 256.0)) * int16_max); };
			else if(synthSample == "*square")
				generator = [](int32 i) { return i < 128 ? int16_max : int16_min; };
			else if(synthSample == "*triangle" || synthSample == "*tri")
				generator = [](int32 i) { return static_cast<int16>(i < 128 ? ((63 - i) * 512) : ((i - 192) * 512)); };
			else if(synthSample == "*saw")
				generator = [](int32 i) { return static_cast<int16>((i - 128) * 256); };
			else if(synthSample == "*silence")
				generator = [](int32) { return int16(0); };
			else if(synthSample == "*noise")
			{
				sample.nLength = sample.nC5Speed;
				generator = [this](int32) { return mpt::random<int16>(AccessPRNG()); };
			} else
			{
				AddToLog(LogWarning, U_("Unknown sample type: ") + mpt::ToUnicode(mpt::Charset::UTF8, std::string(synthSample)));
				prevSmp--;
				continue;
			}
			if(sample.AllocateSample())
			{
				for(SmpLength i = 0; i < sample.nLength; i++)
				{
					sample.sample16()[i] = generator(static_cast<int32>(i));
				}
				if(smp > m_nSamples)
					m_nSamples = smp;
				region.offset = 0;
				region.loopMode = SFZRegion::LoopMode::kContinuous;
				region.loopStart = 0;
				region.loopEnd = sample.nLength - 1;
				region.loopCrossfade = 0;
				region.keyRoot = 60;
			}
		} else if(auto filename = mpt::PathString::FromUTF8(region.filename); !filename.empty())
		{
			if(region.filename.find(':') == std::string::npos)
			{
				filename = file.GetOptionalFileName().value_or(P_("")).GetDirectoryWithDrive() + filename;
			}
			filename = filename.Simplify();
			SetSamplePath(smp, filename);
			mpt::IO::InputFile f(filename, SettingCacheCompleteFileBeforeLoading());
			FileReader smpFile = GetFileReader(f);
			if(!ReadSampleFromFile(smp, smpFile, false))
			{
				AddToLog(LogWarning, U_("Unable to load sample: ") + filename.ToUnicode());
				prevSmp--;
				continue;
			}

			if(UseFinetuneAndTranspose())
				sample.TransposeToFrequency();

			sample.uFlags.set(SMP_KEEPONDISK, sample.HasSampleData());
		}

		if(!region.regionName.empty())
			m_szNames[smp] = mpt::ToCharset(GetCharsetInternal(), mpt::Charset::UTF8, region.regionName);
		if(!m_szNames[smp][0])
			m_szNames[smp] = mpt::ToCharset(GetCharsetInternal(), mpt::PathString::FromUTF8(region.filename).GetFilenameBase().ToUnicode());

		if(region.useSampleKeyRoot)
		{
			if(sample.rootNote != NOTE_NONE)
				region.keyRoot = sample.rootNote - NOTE_MIN;
			else
				region.keyRoot = 60;
		}

		const auto origSampleRate = sample.GetSampleRate(GetType());
		int8 transp = region.transpose + (60 - region.keyRoot);
		for(uint8 i = keyLo; i <= keyHi; i++)
		{
			pIns->Keyboard[i] = smp;
			if(GetType() != MOD_TYPE_XM)
				pIns->NoteMap[i] = NOTE_MIN + i + transp;
		}
		if(GetType() == MOD_TYPE_XM)
			sample.Transpose(transp / 12.0);

		pIns->filterMode = region.filterType;
		if(region.cutoff != 0)
			pIns->SetCutoff(FrequencyToCutOff(region.cutoff), true);
		if(region.resonance != 0)
			pIns->SetResonance(mpt::saturate_round<uint8>(region.resonance * 128.0 / 24.0), true);
		pIns->nCutSwing = mpt::saturate_round<uint8>(region.filterRandom * (m_SongFlags[SONG_EXFILTERRANGE] ? 20 : 24) / 1200.0);
		pIns->midiPWD = mpt::saturate_round<int8>(region.pitchBend / 100.0);

		pIns->nNNA = NewNoteAction::NoteOff;
		if(region.polyphony == 1)
		{
			pIns->nDNA = DuplicateNoteAction::NoteCut;
			pIns->nDCT = DuplicateCheckType::Sample;
		}
		region.ampEnv.ConvertToMPT(pIns, *this, ENV_VOLUME);
		if(region.pitchEnv.depth)
			region.pitchEnv.ConvertToMPT(pIns, *this, ENV_PITCH);
		else if(region.filterEnv.depth)
			region.filterEnv.ConvertToMPT(pIns, *this, ENV_PITCH, true);

		for(const auto &flexEG : region.flexEGs)
		{
			flexEG.ConvertToMPT(pIns, *this);
		}

		if(region.ampEnv.release > 0)
		{
			const double tickDuration = m_PlayState.m_nSamplesPerTick / static_cast<double>(GetSampleRate());
			pIns->nFadeOut = std::min(mpt::saturate_trunc<uint32>(32768.0 * tickDuration / region.ampEnv.release), uint32(32767));
			if(GetType() == MOD_TYPE_IT)
				pIns->nFadeOut = std::min((pIns->nFadeOut + 16u) & ~31u, uint32(8192));
		}
		
		sample.rootNote = region.keyRoot + NOTE_MIN;
		sample.nGlobalVol = mpt::saturate_round<decltype(sample.nGlobalVol)>(64.0 * Clamp(std::pow(10.0, region.volume / 20.0) * region.amplitude / 100.0, 0.0, 1.0));
		if(region.panning != -128)
		{
			sample.nPan = mpt::saturate_round<decltype(sample.nPan)>((region.panning + 100) * 256.0 / 200.0);
			sample.uFlags.set(CHN_PANNING);
		}
		sample.Transpose(region.finetune / 1200.0);

		if(region.pitchLfoDepth && region.pitchLfoFreq)
		{
			sample.nVibSweep = 255;
			if(region.pitchLfoFade > 0)
				sample.nVibSweep = mpt::saturate_round<uint8>(255.0 / region.pitchLfoFade);
			sample.nVibDepth = mpt::saturate_round<uint8>(region.pitchLfoDepth * 32.0 / 100.0);
			sample.nVibRate = mpt::saturate_round<uint8>(region.pitchLfoFreq * 4.0);
		}

		if(region.loopMode != SFZRegion::LoopMode::kUnspecified)
		{
			switch(region.loopMode)
			{
			case SFZRegion::LoopMode::kContinuous:
				sample.uFlags.set(CHN_LOOP);
				break;
			case SFZRegion::LoopMode::kSustain:
				sample.uFlags.set(CHN_SUSTAINLOOP);
				break;
			case SFZRegion::LoopMode::kNoLoop:
			case SFZRegion::LoopMode::kOneShot:
				sample.uFlags.reset(CHN_LOOP | CHN_SUSTAINLOOP);
				break;
			case SFZRegion::LoopMode::kUnspecified:
				MPT_ASSERT_NOTREACHED();
				break;
			}
		}
		if(region.loopEnd > region.loopStart)
		{
			// Loop may also be defined in file, in which case loopStart and loopEnd are unset.
			if(region.loopMode == SFZRegion::LoopMode::kSustain)
			{
				sample.nSustainStart = region.loopStart;
				sample.nSustainEnd = region.loopEnd + 1;
			} else if(region.loopMode == SFZRegion::LoopMode::kContinuous || region.loopMode == SFZRegion::LoopMode::kOneShot)
			{
				sample.nLoopStart = region.loopStart;
				sample.nLoopEnd = region.loopEnd + 1;
			}
		} else if(sample.nLoopEnd <= sample.nLoopStart && region.loopMode != SFZRegion::LoopMode::kUnspecified && region.loopMode != SFZRegion::LoopMode::kNoLoop)
		{
			sample.nLoopEnd = sample.nLength;
		}
		switch(region.loopType)
		{
		case SFZRegion::LoopType::kUnspecified:
			break;
		case SFZRegion::LoopType::kForward:
			sample.uFlags.reset(CHN_PINGPONGLOOP | CHN_PINGPONGSUSTAIN | CHN_REVERSE);
			break;
		case SFZRegion::LoopType::kBackward:
			sample.uFlags.set(CHN_REVERSE);
			break;
		case SFZRegion::LoopType::kAlternate:
			sample.uFlags.set(CHN_PINGPONGLOOP | CHN_PINGPONGSUSTAIN);
			break;
		default:
			break;
		}
		if(sample.nSustainEnd <= sample.nSustainStart && sample.nLoopEnd > sample.nLoopStart && region.loopMode == SFZRegion::LoopMode::kSustain)
		{
			// Turn normal loop (imported from sample) into sustain loop
			std::swap(sample.nSustainStart, sample.nLoopStart);
			std::swap(sample.nSustainEnd, sample.nLoopEnd);
			sample.uFlags.set(CHN_SUSTAINLOOP);
			sample.uFlags.set(CHN_PINGPONGSUSTAIN, sample.uFlags[CHN_PINGPONGLOOP]);
			sample.uFlags.reset(CHN_LOOP | CHN_PINGPONGLOOP);
		}

		mpt::PathString filenameModifier;

		// Loop cross-fade
		SmpLength fadeSamples = mpt::saturate_round<SmpLength>(region.loopCrossfade * origSampleRate);
		LimitMax(fadeSamples, sample.uFlags[CHN_SUSTAINLOOP] ? sample.nSustainStart : sample.nLoopStart);
		if(fadeSamples > 0)
		{
			ctrlSmp::XFadeSample(sample, fadeSamples, 50000, true, sample.uFlags[CHN_SUSTAINLOOP], *this);
			sample.uFlags.set(SMP_MODIFIED);
			filenameModifier += P_(" (cross-fade)");
		}

		const SmpLength origSampleLength = sample.nLength;

		// Sample offset
		if(region.offset && region.offset < sample.nLength)
		{
			auto offset = region.offset * sample.GetBytesPerSample();
			memmove(sample.sampleb(), sample.sampleb() + offset, sample.nLength * sample.GetBytesPerSample() - offset);
			if(region.end > region.offset)
				region.end -= region.offset;
			sample.nLength -= region.offset;
			sample.nLoopStart -= region.offset;
			sample.nLoopEnd -= region.offset;
			sample.uFlags.set(SMP_MODIFIED);
			filenameModifier += P_(" (offset)");
		}
		LimitMax(sample.nLength, region.end);

		if(sample.nLength < origSampleLength && (origSampleLength - sample.nLength) >= 128 * 1024)
		{
			// If the sample was trimmed excessively, re-allocate to save memory.
			// This is crucial for SFZs like those generated by Sforzando's SF2 conversion process,
			// as the whole SF2 sample data chunk ends up in a single WAV file that is then referenced by each region and sliced accordingly.
			if(auto newData = ModSample::AllocateSample(sample.nLength, sample.GetBytesPerSample()))
			{
				memcpy(newData, sample.samplev(), sample.nLength * sample.GetBytesPerSample());
				sample.FreeSample();
				sample.pData.pSample = newData;
			}
		}

		if(region.invertPhase)
		{
			ctrlSmp::InvertSample(sample, 0, sample.nLength, *this);
			sample.uFlags.set(SMP_MODIFIED);
			filenameModifier += P_(" (inverted)");
		}

		if(sample.uFlags.test_all(SMP_KEEPONDISK | SMP_MODIFIED))
		{
			// Avoid ruining the original samples
			if(auto filename = GetSamplePath(smp); !filename.empty())
			{
				filename = filename.GetDirectoryWithDrive() + filename.GetFilenameBase() + filenameModifier + filename.GetFilenameExtension();
				SetSamplePath(smp, filename);
			}
		}

		sample.PrecomputeLoops(*this, false);
		sample.Convert(MOD_TYPE_MPT, GetType());
	}

	pIns->Sanitize(MOD_TYPE_MPT);
	pIns->Convert(MOD_TYPE_MPT, GetType());
	return true;
}

#ifndef MODPLUG_NO_FILESAVE

static double SFZLinear2dB(double volume)
{
	return (volume > 0.0 ? 20.0 * std::log10(volume) : -144.0);
}

static void WriteSFZEnvelope(std::ostream &f, double tickDuration, int index, const InstrumentEnvelope &env, const char *type, double scale, std::function<double(int32)> convFunc)
{
	if(!env.dwFlags[ENV_ENABLED] || env.empty())
		return;

	const bool sustainAtEnd = (!env.dwFlags[ENV_SUSTAIN] || env.nSustainStart == (env.size() - 1)) && convFunc(env.back().value) != 0.0;

	const auto prefix = MPT_AFORMAT("\neg{}_")(mpt::afmt::dec0<2>(index));
	f << "\n" << prefix << type << "=" << scale;
	f << prefix << "points=" << (env.size() + (sustainAtEnd ? 1 : 0));
	EnvelopeNode::tick_t lastTick = 0;
	int nodeIndex = 0;
	for(const auto &node : env)
	{
		const double time = (node.tick - lastTick) * tickDuration;
		lastTick = node.tick;
		f << prefix << "time" << nodeIndex << "=" << time;
		f << prefix << "level" << nodeIndex << "=" << convFunc(node.value);
		nodeIndex++;
	}
	if(sustainAtEnd)
	{
		// Prevent envelope from going back to neutral
		f << prefix << "time" << nodeIndex << "=0";
		f << prefix << "level" << nodeIndex << "=" << convFunc(env.back().value);
	}
	// We always must write a sustain point, or the envelope will be sustained on the first point of the envelope
	f << prefix << "sustain=" << (env.dwFlags[ENV_SUSTAIN] ? env.nSustainStart : (env.size() - 1));

	if(env.dwFlags[ENV_LOOP])
		f << "\n// Loop: " << static_cast<uint32>(env.nLoopStart) << "-" << static_cast<uint32>(env.nLoopEnd);
	if(env.dwFlags[ENV_SUSTAIN] && env.nSustainEnd > env.nSustainStart)
		f << "\n// Sustain Loop: " << static_cast<uint32>(env.nSustainStart) << "-" << static_cast<uint32>(env.nSustainEnd);
	if(env.nReleaseNode != ENV_RELEASE_NODE_UNSET)
		f << "\n// Release Node: " << static_cast<uint32>(env.nReleaseNode);
}

static std::string SanitizeSFZString(std::string s, mpt::Charset sourceCharset)
{
	using namespace std::literals;
	// Remove characters could trip up the parser
	std::string::size_type pos = 0;
	while((pos = s.find_first_of("<=\r\n\t\0"sv, pos)) != std::string::npos)
	{
		s[pos++] = ' ';
	}
	return mpt::ToCharset(mpt::Charset::UTF8, sourceCharset, s);
}


bool CSoundFile::SaveSFZInstrument(INSTRUMENTINDEX nInstr, std::ostream &f, const mpt::PathString &filename, bool useFLACsamples) const
{
#ifdef MODPLUG_TRACKER
	const mpt::IO::FlushMode flushMode = mpt::IO::FlushModeFromBool(TrackerSettings::Instance().MiscFlushFileBuffersOnSave);
#else
	const mpt::IO::FlushMode flushMode = mpt::IO::FlushMode::Full;
#endif
	const ModInstrument *ins = Instruments[nInstr];
	if(ins == nullptr)
		return false;

	// Creating directory names with trailing spaces or dots is a bad idea, as they are difficult to remove in Windows.
	const mpt::RawPathString whitespaceDirName = PL_(" \n\r\t.");
	const mpt::PathString sampleBaseName = mpt::PathString::FromNative(mpt::trim(filename.GetFilenameBase().AsNative(), whitespaceDirName));
	const mpt::PathString sampleDirName = (sampleBaseName.empty() ? P_("Samples") : sampleBaseName)  + P_("/");
	const mpt::PathString sampleBasePath = filename.GetDirectoryWithDrive() + sampleDirName;
	if(!mpt::native_fs{}.is_directory(sampleBasePath) && !::CreateDirectory(sampleBasePath.AsNative().c_str(), nullptr))
		return false;

	const double tickDuration = m_PlayState.m_nSamplesPerTick / static_cast<double>(m_MixerSettings.gdwMixingFreq);

	f << std::setprecision(10);
	f << "// Created with " << mpt::ToCharset(mpt::Charset::UTF8, Version::Current().GetOpenMPTVersionString()) << "\n";
	f << "// Envelope tempo base: tempo " << m_PlayState.m_nMusicTempo.ToDouble();
	switch(m_nTempoMode)
	{
	case TempoMode::Classic:
		f << " (classic tempo mode)";
		break;
	case TempoMode::Alternative:
		f << " (alternative tempo mode)";
		break;
	case TempoMode::Modern:
		f << ", " << m_PlayState.m_nMusicSpeed << " ticks per row, " << m_PlayState.m_nCurrentRowsPerBeat << " rows per beat (modern tempo mode)";
		break;
	case TempoMode::NumModes:
		MPT_ASSERT_NOTREACHED();
		break;
	}

	f << "\n\n<control>\ndefault_path=" << sampleDirName.ToUTF8();
	if(const auto globalName = SanitizeSFZString(ins->name, GetCharsetInternal()); !globalName.empty())
	{
		f << "\n\n<global>\nglobal_label=" << globalName;
	}
	f << "\n\n<group>";
	f << "\nbend_up=" << ins->midiPWD * 100;
	f << "\nbend_down=" << -ins->midiPWD * 100;
	const uint32 cutoff = ins->IsCutoffEnabled() ? ins->GetCutoff() : 127;
	// If filter envelope is active but cutoff is not set, we still need to set the base cutoff frequency to be modulated by the envelope.
	if(ins->IsCutoffEnabled() || ins->PitchEnv.dwFlags[ENV_FILTER])
		f << "\ncutoff=" << CSoundFile::CutOffToFrequency(cutoff) << " // " << cutoff;
	if(ins->IsResonanceEnabled())
		f << "\nresonance=" << Util::muldivr_unsigned(ins->GetResonance(), 24, 128) << " // " << static_cast<int>(ins->GetResonance());
	if(ins->IsCutoffEnabled() || ins->IsResonanceEnabled())
		f << "\nfil_type=" << (ins->filterMode == FilterMode::HighPass ? "hpf_2p" : "lpf_2p");
	if(ins->dwFlags[INS_SETPANNING])
		f << "\npan=" << (Util::muldivr_unsigned(ins->nPan, 200, 256) - 100) << " // " << ins->nPan;
	if(ins->nGlobalVol != 64)
		f << "\nvolume=" << SFZLinear2dB(ins->nGlobalVol / 64.0) << " // " << ins->nGlobalVol;
	if(ins->nFadeOut)
	{
		f << "\nampeg_release=" << (32768.0 * tickDuration / ins->nFadeOut) << " // " << ins->nFadeOut;
		f << "\nampeg_release_shape=0";
	}

	if(ins->nDNA == DuplicateNoteAction::NoteCut && ins->nDCT != DuplicateCheckType::None)
		f << "\npolyphony=1";

	WriteSFZEnvelope(f, tickDuration, 1, ins->VolEnv, "amplitude", 100.0, [](int32 val) { return val / static_cast<double>(ENVELOPE_MAX); });
	WriteSFZEnvelope(f, tickDuration, 2, ins->PanEnv, "pan", 100.0, [](int32 val) { return 2.0 * (val - ENVELOPE_MID) / (ENVELOPE_MAX - ENVELOPE_MIN); });
	if(ins->PitchEnv.dwFlags[ENV_FILTER])
	{
		const auto envScale = 1200.0 * std::log(CutOffToFrequency(127, 256) / static_cast<double>(CutOffToFrequency(0, -256))) / mpt::numbers::ln2;
		const auto cutoffNormal = CutOffToFrequency(cutoff);
		WriteSFZEnvelope(f, tickDuration, 3, ins->PitchEnv, "cutoff", envScale, [this, cutoff, cutoffNormal, envScale](int32 val) {
			// Convert interval between center frequency and envelope into cents
			const auto freq = CutOffToFrequency(cutoff, (val - ENVELOPE_MID) * 256 / (ENVELOPE_MAX - ENVELOPE_MID));
			return 1200.0 * std::log(freq / static_cast<double>(cutoffNormal)) / mpt::numbers::ln2 / envScale;
		});
	} else
	{
		WriteSFZEnvelope(f, tickDuration, 3, ins->PitchEnv, "pitch", 1600.0, [](int32 val) { return 2.0 * (val - ENVELOPE_MID) / (ENVELOPE_MAX - ENVELOPE_MIN); });
	}

	size_t numSamples = 0;
	for(size_t i = 0; i < std::size(ins->Keyboard); i++)
	{
		if(ins->Keyboard[i] < 1 || ins->Keyboard[i] > GetNumSamples())
			continue;

		size_t endOfRegion = i + 1;
		while(endOfRegion < std::size(ins->Keyboard))
		{
			if(ins->Keyboard[endOfRegion] != ins->Keyboard[i] || ins->NoteMap[endOfRegion] != (ins->NoteMap[i] + endOfRegion - i))
				break;
			endOfRegion++;
		}
		endOfRegion--;

		const ModSample &sample = Samples[ins->Keyboard[i]];
		const bool isAdlib = sample.uFlags[CHN_ADLIB];

		if(!sample.HasSampleData())
		{
			i = endOfRegion;
			continue;
		}

		numSamples++;
		mpt::PathString sampleName = sampleBasePath + (sampleBaseName.empty() ? P_("Sample") : sampleBaseName) + P_(" ") + mpt::PathString::FromUnicode(mpt::ufmt::val(numSamples));
		if(isAdlib)
			sampleName += P_(".s3i");
		else if(useFLACsamples)
			sampleName += P_(".flac");
		else
			sampleName += P_(".wav");

		bool success = false;
		try
		{
			mpt::IO::SafeOutputFile sfSmp(sampleName, std::ios::binary, flushMode);
			if(sfSmp)
			{
				mpt::IO::ofstream &fSmp = sfSmp;
				fSmp.exceptions(fSmp.exceptions() | std::ios::badbit | std::ios::failbit);

				if(isAdlib)
					success = SaveS3ISample(ins->Keyboard[i], fSmp);
				else if(useFLACsamples)
					success = SaveFLACSample(ins->Keyboard[i], fSmp);
				else
					success = SaveWAVSample(ins->Keyboard[i], fSmp);
			}
		} catch(const std::exception &)
		{
			success = false;
		}
		if(!success)
		{
			AddToLog(LogError, MPT_USTRING("Unable to save sample: ") + sampleName.ToUnicode());
		}


		f << "\n\n<region>";
		if(const auto regionName = SanitizeSFZString(m_szNames[ins->Keyboard[i]], GetCharsetInternal()); !regionName.empty())
		{
			f << "\nregion_label=" << regionName;
		}
		f << "\nsample=" << sampleName.GetFilename().ToUTF8();
		f << "\nlokey=" << i;
		f << "\nhikey=" << endOfRegion;
		if(sample.rootNote != NOTE_NONE)
			f << "\npitch_keycenter=" << sample.rootNote - NOTE_MIN;
		else
			f << "\npitch_keycenter=" << NOTE_MIDDLEC + i - ins->NoteMap[i];
		if(sample.uFlags[CHN_PANNING])
			f << "\npan=" << (Util::muldivr_unsigned(sample.nPan, 200, 256) - 100) << " // " << sample.nPan;
		if(sample.nGlobalVol != 64)
			f << "\nvolume=" << SFZLinear2dB((ins->nGlobalVol * sample.nGlobalVol) / 4096.0) << " // " << sample.nGlobalVol;
		const char *loopMode = "no_loop", *loopType = "forward";
		SmpLength loopStart = 0, loopEnd = 0;
		if(sample.uFlags[CHN_SUSTAINLOOP])
		{
			loopMode = "loop_sustain";
			loopStart = sample.nSustainStart;
			loopEnd = sample.nSustainEnd;
			if(sample.uFlags[CHN_PINGPONGSUSTAIN])
				loopType = "alternate";
		} else if(sample.uFlags[CHN_LOOP])
		{
			loopMode = "loop_continuous";
			loopStart = sample.nLoopStart;
			loopEnd = sample.nLoopEnd;
			if(sample.uFlags[CHN_PINGPONGLOOP])
				loopType = "alternate";
			else if(sample.uFlags[CHN_REVERSE])
				loopType = "backward";
		}
		f << "\nloop_mode=" << loopMode;
		if(loopStart < loopEnd)
		{
			f << "\nloop_start=" << loopStart;
			f << "\nloop_end=" << (loopEnd - 1);
			f << "\nloop_type=" << loopType;
		}
		if(sample.uFlags.test_all(CHN_SUSTAINLOOP | CHN_LOOP))
		{
			f << "\n// Warning: Only sustain loop was exported!";
		}
		i = endOfRegion;
	}

	return true;
}

#endif // MODPLUG_NO_FILESAVE

#else
bool CSoundFile::ReadSFZInstrument(INSTRUMENTINDEX, FileReader &)
{
	return false;
}
#endif // MPT_EXTERNAL_SAMPLES

OPENMPT_NAMESPACE_END
