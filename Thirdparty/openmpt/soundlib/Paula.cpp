/*
* Paula.cpp
* ---------
* Purpose: Emulating the Amiga's sound chip, Paula, by implementing resampling using band-limited steps (BLEPs)
* Notes  : The BLEP table generator code is a translation of Antti S. Lankila's original Python code.
* Authors: OpenMPT Devs
*          Antti S. Lankila
* The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
*/

#include "stdafx.h"
#include "Paula.h"
#include "TinyFFT.h"
#include "Tables.h"
#include "mpt/base/numbers.hpp"

#include <complex>
#include <numeric>

OPENMPT_NAMESPACE_BEGIN

namespace Paula
{

namespace
{

MPT_NOINLINE std::vector<double> KaiserFIR(int numTaps, double cutoff, double beta)
{
	const double izeroBeta = Izero(beta);
	const double kPi = 4.0 * std::atan(1.0) * cutoff;
	const double xDiv = 1.0 / ((numTaps / 2) * (numTaps / 2));
	const int numTapsDiv2 = numTaps / 2;
	std::vector<double> result(numTaps);
	for(int i = 0; i < numTaps; i++)
	{
		double fsinc;
		if(i == numTapsDiv2)
		{
			fsinc = 1.0;
		} else
		{
			const double x = i - numTapsDiv2;
			const double xPi = x * kPi;
			//      -   sinc    -   -                   Kaiser window                   -   -sinc-
			fsinc = std::sin(xPi) * Izero(beta * std::sqrt(1 - x * x * xDiv)) / (izeroBeta * xPi);
		}

		result[i] = fsinc * cutoff;
	}
	return result;
}


MPT_NOINLINE void FIR_MinPhase(std::vector<double> &table, const TinyFFT &fft)
{
	std::vector<std::complex<double>> cepstrum(fft.Size());
	MPT_ASSERT(cepstrum.size() >= table.size());
	for(size_t i = 0; i < table.size(); i++)
		cepstrum[i] = table[i];
	// Compute the real cepstrum: fft -> abs + ln -> ifft -> real
	fft.FFT(cepstrum);
	for(auto &v : cepstrum)
		v = std::log(std::abs(v));
	fft.IFFT(cepstrum);
	fft.Normalize(cepstrum);

	// Window the cepstrum in such a way that anticausal components become rejected
	for(size_t i = 1; i < cepstrum.size() / 2; i++)
	{
		cepstrum[i] *= 2;
		cepstrum[i + cepstrum.size() / 2] *= 0;
	}

	// Now cancel the previous steps: fft -> exp -> ifft -> real
	fft.FFT(cepstrum);
	for(auto &v : cepstrum)
		v = std::exp(v);
	fft.IFFT(cepstrum);
	fft.Normalize(cepstrum);
	for(size_t i = 0; i < table.size(); i++)
		table[i] = cepstrum[i].real();
}


class BiquadFilter
{
	const double b0, b1, b2, a1, a2;
	double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;

	double Filter(double x0) noexcept
	{
		double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
		x2 = x1;
		x1 = x0;
		y2 = y1;
		y1 = y0;
		return y0;
	}

public:
	BiquadFilter(double b0_, double b1_, double b2_, double a1_, double a2_)
	    : b0(b0_), b1(b1_), b2(b2_), a1(a1_), a2(a2_)
	{ }

	std::vector<double> Run(std::vector<double> table)
	{
		x1 = 0.0;
		x2 = 0.0;
		y1 = 0.0;
		y2 = 0.0;

		// Initialize filter to stable state
		for(int i = 0; i < 10000; i++)
			Filter(table[0]);
		// Now run the filter
		for(auto &v : table)
			v = Filter(v);

		return table;
	}
};


// Observe: a and b are reversed here. To be absolutely clear:
// a is the nominator and b is the denominator. :-/
BiquadFilter ZTransform(double a0, double a1, double a2, double b0, double b1, double b2, double fc, double fs)
{
	// Prewarp s - domain coefficients
	const double wp = 2.0 * fs * std::tan(mpt::numbers::pi * fc / fs);
	a2 /= wp * wp;
	a1 /= wp;
	b2 /= wp * wp;
	b1 /= wp;

	// Compute bilinear transform and return it
	const double bd = 4 * b2 * fs * fs + 2 * b1 * fs + b0;
	return BiquadFilter(
	    (4 * a2 * fs * fs + 2 * a1 * fs + a0) / bd,
	    (2 * a0 - 8 * a2 * fs * fs) / bd,
	    (4 * a2 * fs * fs - 2 * a1 * fs + a0) / bd,
	    (2 * b0 - 8 * b2 * fs * fs) / bd,
	    (4 * b2 * fs * fs - 2 * b1 * fs + b0) / bd);
}


BiquadFilter MakeRCLowpass(double sampleRate, double freq)
{
	const double omega = (2.0 * mpt::numbers::pi) * freq / sampleRate;
	const double term = 1 + 1 / omega;
	return BiquadFilter(1 / term, 0.0, 0.0, -1.0 + 1.0 / term, 0.0);
}


BiquadFilter MakeButterworth(double fs, double fc, double res_dB = 0)
{
	// 2nd-order Butterworth s-domain coefficients are:
	//
	// b0 = 1.0  b1 = 0        b2 = 0
	// a0 = 1    a1 = sqrt(2)  a2 = 1
	//
	// by tweaking the a1 parameter, some resonance can be produced.

	const double res = std::pow(10.0, (-res_dB / 10.0 / 2.0));
	return ZTransform(1, 0, 0, 1, std::sqrt(2) * res, 1, fc, fs);
}


MPT_NOINLINE void Integrate(std::vector<double> &table)
{
	const double total = std::accumulate(table.begin(), table.end(), 0.0);
	double startVal = -total;

	for(auto &v : table)
	{
		startVal += v;
		v = startVal;
	}
}


MPT_NOINLINE void Quantize(const std::vector<double> &in, Paula::BlepArray &quantized)
{
	MPT_ASSERT(in.size() == Paula::BLEP_SIZE);
	constexpr int fact = 1 << Paula::BLEP_SCALE;
	const double cv = fact / (in.back() - in.front());

	for(int i = 0; i < Paula::BLEP_SIZE; i++)
	{
		double val = in[i] * cv;
#ifdef MPT_INTMIXER
		val = mpt::round(val);
#endif
		quantized[i] = static_cast<mixsample_t>(-val);
	}
}

}  // namespace

void BlepTables::InitTables()
{
	constexpr double sampleRate = Paula::PAULA_HZ;

	// Because Amiga only has 84 dB SNR, the noise floor is low enough with -90 dB.
	// A500 model uses slightly lower-quality kaiser window to obtain slightly
	// steeper stopband attenuation. The fixed filters attenuates the sidelobes by
	// 12 dB, compensating for the worse performance of the kaiser window.

	// 21 kHz stopband is not fully attenuated by 22 kHz. If the sampling frequency
	// is 44.1 kHz, all frequencies above 22 kHz will alias over 20 kHz, thus inaudible.
	// The output should be aliasingless for 48 kHz sampling frequency.
	auto unfilteredA500 = KaiserFIR(Paula::BLEP_SIZE, 21000.0 / sampleRate * 2.0, 8.0);
	auto unfilteredA1200 = KaiserFIR(Paula::BLEP_SIZE, 21000.0 / sampleRate * 2.0, 9.0);
	// Move filtering effects to start to allow IIRs more time to settle
	constexpr size_t padSize = 8;
	constexpr int fftSize = static_cast<int>(mpt::bit_width(size_t(Paula::BLEP_SIZE)) + mpt::bit_width(padSize) - 2);
	const TinyFFT fft(fftSize);
	FIR_MinPhase(unfilteredA500, fft);
	FIR_MinPhase(unfilteredA1200, fft);

	// Make digital models for the filters on Amiga 500 and 1200.
	auto filterFixed5kHz = MakeRCLowpass(sampleRate, 4900.0);
	// The leakage filter seems to reduce treble in both models a bit
	// The A500 filter seems to be well modelled only with a 4.9 kHz
	// filter although the component values would suggest 5 kHz filter.
	auto filterLeakage = MakeRCLowpass(sampleRate, 32000.0);
	auto filterLED = MakeButterworth(sampleRate, 3275.0, -0.70);

	// Apply fixed filter to A500
	auto amiga500Off = filterFixed5kHz.Run(unfilteredA500);
	// Produce the filtered outputs
	auto amiga1200Off = filterLeakage.Run(unfilteredA1200);

	// Produce LED filters
	auto amiga500On = filterLED.Run(amiga500Off);
	auto amiga1200On = filterLED.Run(amiga1200Off);

	// Integrate to produce blep
	Integrate(amiga500Off);
	Integrate(amiga500On);
	Integrate(amiga1200Off);
	Integrate(amiga1200On);
	Integrate(unfilteredA1200);

	// Quantize and scale
	Quantize(amiga500Off, WinSincIntegral[A500Off]);
	Quantize(amiga500On, WinSincIntegral[A500On]);
	Quantize(amiga1200Off, WinSincIntegral[A1200Off]);
	Quantize(amiga1200On, WinSincIntegral[A1200On]);
	Quantize(unfilteredA1200, WinSincIntegral[Unfiltered]);
}


const Paula::BlepArray &BlepTables::GetAmigaTable(Resampling::AmigaFilter amigaType, bool enableFilter) const
{
	if(amigaType == Resampling::AmigaFilter::A500)
		return enableFilter ? WinSincIntegral[A500On] : WinSincIntegral[A500Off];
	if(amigaType == Resampling::AmigaFilter::A1200)
		return enableFilter ? WinSincIntegral[A1200On] : WinSincIntegral[A1200Off];
	return WinSincIntegral[Unfiltered];
}


// we do not initialize blepState here
// cppcheck-suppress uninitMemberVar
State::State(uint32 sampleRate)
{
	double amigaClocksPerSample = static_cast<double>(PAULA_HZ) / sampleRate;
	numSteps = static_cast<int>(amigaClocksPerSample / MINIMUM_INTERVAL);
	stepRemainder = SamplePosition::FromDouble(amigaClocksPerSample - numSteps * MINIMUM_INTERVAL);
}


void State::Reset()
{
	remainder = SamplePosition(0);
	activeBleps = 0;
	firstBlep = MAX_BLEPS / 2u;
	globalOutputLevel = 0;
}


void State::InputSample(int16 sample)
{
	if(sample != globalOutputLevel)
	{
		// Start a new blep: level is the difference, age (or phase) is 0 clocks.
		firstBlep = (firstBlep - 1u) % MAX_BLEPS;
		if(activeBleps < std::size(blepState))
			activeBleps++;
		blepState[firstBlep].age = 0;
		blepState[firstBlep].level = sample - globalOutputLevel;
		globalOutputLevel = sample;
	}
}


// Return output simulated as series of bleps
int State::OutputSample(const BlepArray &WinSincIntegral)
{
	int output = globalOutputLevel * (1 << Paula::BLEP_SCALE);
	uint32 lastBlep = firstBlep + activeBleps;
	for(uint32 i = firstBlep; i != lastBlep; i++)
	{
		const auto &blep = blepState[i % MAX_BLEPS];
		output -= WinSincIntegral[blep.age] * blep.level;
	}
#ifdef MPT_INTMIXER
	output /= (1 << (Paula::BLEP_SCALE - 2));	// - 2 to compensate for the fact that we reduced the input sample bit depth
#endif

	return output;
}


// Advance the simulation by given number of clock ticks
void State::Clock(int cycles)
{
	uint32 lastBlep = firstBlep + activeBleps;
	for(uint32 i = firstBlep; i != lastBlep; i++)
	{
		auto &blep = blepState[i % MAX_BLEPS];
		blep.age += static_cast<uint16>(cycles);
		if(blep.age >= Paula::BLEP_SIZE)
		{
			activeBleps = static_cast<uint16>(i - firstBlep);
			return;
		}
	}
}

}

OPENMPT_NAMESPACE_END
