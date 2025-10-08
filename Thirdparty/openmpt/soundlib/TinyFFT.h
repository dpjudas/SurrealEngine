/*
 * TinyFFT.h
 * ---------
 * Purpose: A simple FFT implementation for power-of-two FFTs
 * Notes  : This is a C++ adaption of Ryuhei Mori's BSD 2-clause licensed TinyFFT
 *          available from https://github.com/ryuhei-mori/tinyfft
 * Authors: Ryuhei Mori
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"
#include <complex>

OPENMPT_NAMESPACE_BEGIN

class TinyFFT
{
	static constexpr std::complex<double> I{0.0, 1.0};
	std::vector<std::complex<double>> w;  // Pre-computed twiddle factors
	const uint32 k; // log2 of FFT size

	void GenerateTwiddleFactors(uint32 i, uint32 b, std::complex<double> z);

public:
	TinyFFT(const uint32 fftSize);

	uint32 Size() const noexcept;

	// Computes in-place FFT of size 2^k of A, result is in bit-reversed order.
	void FFT(std::vector<std::complex<double>> &A) const;

	// Computes in-place IFFT of size 2^k of A, input is expected to be in bit-reversed order.
	void IFFT(std::vector<std::complex<double>> &A) const;

	static void Normalize(std::vector<std::complex<double>> &data);
};

OPENMPT_NAMESPACE_END
