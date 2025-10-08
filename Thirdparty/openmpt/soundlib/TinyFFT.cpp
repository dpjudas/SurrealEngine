/*
 * TinyFFT.cpp
 * -----------
 * Purpose: A simple FFT implementation for power-of-two FFTs
 * Notes  : This is a C++ adaption of Ryuhei Mori's BSD 2-clause licensed TinyFFT
 *          available from https://github.com/ryuhei-mori/tinyfft
 * Authors: Ryuhei Mori
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "TinyFFT.h"

OPENMPT_NAMESPACE_BEGIN

void TinyFFT::GenerateTwiddleFactors(uint32 i, uint32 b, std::complex<double> z)
{
	if(b == 0)
		w[i] = z;
	else
	{
		GenerateTwiddleFactors(i, b >> 1, z);
		GenerateTwiddleFactors(i | b, b >> 1, z * w[b]);
	}
}


TinyFFT::TinyFFT(const uint32 fftSize)
    : w(std::size_t(1) << (fftSize - 1))
    , k(fftSize)
{
	const uint32 m = 1 << k;
	constexpr double PI2_ = 6.28318530717958647692;
	const double arg = -PI2_ / m;
	for(uint32 i = 1, j = m / 4; j; i <<= 1, j >>= 1)
	{
		w[i] = std::exp(I * (arg * j));
	}
	GenerateTwiddleFactors(0, m / 4, 1);
}


uint32 TinyFFT::Size() const noexcept
{
	return 1 << k;
}


// Computes in-place FFT of size 2^k of A, result is in bit-reversed order.
void TinyFFT::FFT(std::vector<std::complex<double>> &A) const
{
	MPT_ASSERT(A.size() == (std::size_t(1) << k));
	const uint32 m = 1 << k;
	uint32 u = 1;
	uint32 v = m / 4;
	if(k & 1)
	{
		for(uint32 j = 0; j < m / 2; j++)
		{
			auto Ajv = A[j + (m / 2)];
			A[j + (m / 2)] = A[j] - Ajv;
			A[j] += Ajv;
		}
		u <<= 1;
		v >>= 1;
	}
	for(uint32 i = k & ~1; i > 0; i -= 2)
	{
		for(uint32 jh = 0; jh < u; jh++)
		{
			auto wj = w[jh << 1];
			auto wj2 = w[jh];
			auto wj3 = wj2 * wj;
			for(uint32 j = jh << i, je = j + v; j < je; j++)
			{
				auto tmp0 = A[j];
				auto tmp1 = wj * A[j + v];
				auto tmp2 = wj2 * A[j + 2 * v];
				auto tmp3 = wj3 * A[j + 3 * v];

				auto ttmp0 = tmp0 + tmp2;
				auto ttmp2 = tmp0 - tmp2;
				auto ttmp1 = tmp1 + tmp3;
				auto ttmp3 = -I * (tmp1 - tmp3);

				A[j] = ttmp0 + ttmp1;
				A[j + v] = ttmp0 - ttmp1;
				A[j + 2 * v] = ttmp2 + ttmp3;
				A[j + 3 * v] = ttmp2 - ttmp3;
			}
		}
		u <<= 2;
		v >>= 2;
	}
}


// Computes in-place IFFT of size 2^k of A, input is expected to be in bit-reversed order.
void TinyFFT::IFFT(std::vector<std::complex<double>> &A) const
{
	MPT_ASSERT(A.size() == (std::size_t(1) << k));
	const uint32 m = 1 << k;
	uint32 u = m / 4;
	uint32 v = 1;
	for(uint32 i = 2; i <= k; i += 2)
	{
		for(uint32 jh = 0; jh < u; jh++)
		{
			auto wj = std::conj(w[jh << 1]);
			auto wj2 = std::conj(w[jh]);
			auto wj3 = wj2 * wj;
			for(uint32 j = jh << i, je = j + v; j < je; j++)
			{
				auto tmp0 = A[j];
				auto tmp1 = A[j + v];
				auto tmp2 = A[j + 2 * v];
				auto tmp3 = A[j + 3 * v];

				auto ttmp0 = tmp0 + tmp1;
				auto ttmp1 = tmp0 - tmp1;
				auto ttmp2 = tmp2 + tmp3;
				auto ttmp3 = I * (tmp2 - tmp3);

				A[j] = ttmp0 + ttmp2;
				A[j + v] = wj * (ttmp1 + ttmp3);
				A[j + 2 * v] = wj2 * (ttmp0 - ttmp2);
				A[j + 3 * v] = wj3 * (ttmp1 - ttmp3);
			}
		}
		u >>= 2;
		v <<= 2;
	}
	if(k & 1)
	{
		for(uint32 j = 0; j < m / 2; j++)
		{
			auto Ajv = A[j + (m / 2)];
			A[j + (m / 2)] = A[j] - Ajv;
			A[j] += Ajv;
		}
	}
}


void TinyFFT::Normalize(std::vector<std::complex<double>> &data)
{
	const double s = static_cast<double>(data.size());
	for(auto &v : data)
		v /= s;
}

OPENMPT_NAMESPACE_END
