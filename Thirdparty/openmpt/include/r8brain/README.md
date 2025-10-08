# r8brain-free-src - High-Quality, Fast Resampler #

## Introduction ##

Open source (under the MIT license) high-quality professional audio sample
rate converter (SRC) / resampler C++ library.  Features routines for SRC,
both up- and downsampling, to/from any sample rate, including non-integer
sample rates: it can be also used for conversion to/from SACD/DSD sample
rates, and even go beyond that.  SRC routines were implemented in a
multi-platform C++ code, and have a high level of optimality. Also suitable
for fast general-purpose 1D time-series resampling / interpolation (with
relaxed filter parameters).

The structure of this library's objects is such that they can be frequently
created and destroyed in large applications with a minimal performance impact
due to a high level of reusability of its most "initialization-expensive"
objects: the fast Fourier transform and FIR filter objects.

The SRC algorithm at first produces 2X oversampled (relative to the source
sample rate, or the destination sample rate if the downsampling is performed)
signal, then performs interpolation using a bank of short (8 to 30 taps,
depending on the required precision) polynomial-interpolated sinc
function-based fractional delay filters.  This puts the algorithm into the
league of the fastest among the most precise SRC algorithms.  The more precise
alternative being only the whole number-factored SRC, which can be slower.

P.S. Please credit the creator of this library in your documentation in the
following way: "Sample rate converter designed by Aleksey Vaneev of Voxengo".

## Requirements ##

C++ compiler and system with the "double" floating point type (53-bit
mantissa) support.  No explicit code for the "float" type is present in this
library, because as practice has shown the "float"-based code performs
considerably slower on a modern processor, at least in this library.  This
library does not have dependencies beside the standard C library, the
"windows.h" on Windows and the "pthread.h" on macOS and Linux.

## Links ##

* [Documentation](https://www.voxengo.com/public/r8brain-free-src/Documentation/)
* [Discussion](https://www.kvraudio.com/forum/viewtopic.php?t=389711)

## Usage Information ##

The sample rate converter (resampler) is represented by the
**r8b::CDSPResampler** class, which is a single front-end class for the
whole library.  You do not basically need to use nor understand any other
classes beside this class.  Several derived classes that have varying levels
of precision are also available (for full-resolution 16-bit and 24-bit
resampling).

The code of the library resides in the "r8b" C++ namespace, effectively
isolating it from all other code.  The code is thread-safe.  A separate
resampler object should be created for each audio channel or stream being
processed concurrently.

Note that you will need to compile the "r8bbase.cpp" source file and include
the resulting object file into your application build.  This source file
includes definitions of several global static objects used by the library.
You may also need to include to your project: the "Kernel32" library (on
Windows) and the "pthread" library on macOS and Linux.

The library is able to process signal of any scale and loudness: it is not
limited to just a "usual" -1.0 to 1.0 range.

By defining the `R8B_IPP` configuration macro it is possible to enable Intel
IPP back-end for FFT functions, instead of the default Ooura FFT.  IPP FFT
makes sample rate conversion faster by 23% on average.

    #define R8B_IPP 1

If a larger initial processing delay and a very minor sample-timing error are
not an issue, for the most efficiency you can define these macros at
the beginning of the `r8bconf.h` file, or during compilation:

    #define R8B_IPP 1
    #define R8B_FASTTIMING 1
    #define R8B_EXTFFT 1

If you do not have access to the Intel IPP then you may consider enabling the
PFFFT which is only slightly slower than Intel IPP FFT in performance.  There
are two macros available: `R8B_PFFFT` and `R8B_PFFFT_DOUBLE`.  The first macro
enables PFFFT that works in single-precision resolution, thus limiting the
overall resampler's precision to 24-bit sample rate conversions (for
mission-critical professional audio applications, using the `R8B_PFFFT` macro
is not recommended as its peak error is quite large).  The second macro
enables PFFFT implementation that works in double-precision resolution, making
use of SSE2, AVX, and NEON intrinsics, yielding precision that is equal to
both Intel IPP and Ooura FFT implementations.

To use the PFFFT, define the `R8B_PFFFT` or `R8B_PFFFT_DOUBLE` macro, compile
and include the supplied `pffft.cpp` or `pffft_double/pffft_double.c` file to
your project build.

    #define R8B_PFFFT 1

    or

    #define R8B_PFFFT_DOUBLE 1

The code of this library was commented in the [Doxygen](http://www.doxygen.org/)
style.  To generate the documentation locally you may run the
`doxygen ./other/r8bdoxy.txt` command from the library's folder.

Preliminary tests show that the r8b::CDSPResampler24 resampler class achieves
`38*n_cores` Mrops (`56*n_cores` for Intel IPP FFT) when converting 1 channel
of 24-bit audio from 44100 to 96000 sample rate (2% transition band), on a
Ryzen 3700X processor-based 64-bit system.  This approximately translates to a
real-time resampling of `860*n_cores` (`1270*n_cores`) audio streams, at 100%
CPU load.  Performance when converting to other sample rates may vary greatly.
When comparing performance of this resampler library to another library make
sure that the competing library is also tuned to produce a fully linear-phase
response, has similar stop-band characteristics and similar sample-timing
precision.

## Dynamic Link Library ##

The functions of this SRC library are also accessible in simplified form via
the DLL file on Windows, requiring a processor with SSE2 support (Win64
version includes AVX2 auto-dispatch code).  Delphi Pascal interface unit file
for the DLL file is available.  DLL and C LIB files are distributed in the DLL
folder on the project's homepage.  On non-Windows systems it is preferrable
to use the C++ library directly.  Note that the DLL was compiled with the
Intel IPP enabled.

## Real-Time Applications ##

The resampler class of this library was designed as an asynchronous processor:
it may produce any number of output samples, depending on the input sample
data length and the resampling parameters.  The resampler must be fed with the
input sample data until enough output sample data were produced, with any
excess output samples used before feeding the resampler with more input data.
A "relief" factor here is that the resampler removes the initial processing
latency automatically, and that after initial moments of processing the output
becomes steady, with only minor output sample data length fluctuations.

So, while for an off-line resampling a "push" method can be used,
demonstrated in the `example.cpp` file, for a real-time resampling a "pull"
method should be used which calls the resampling process until the output
buffer is filled.

## Notes ##

When using the **r8b::CDSPResampler** class directly, you may select the
transition band/steepness of the low-pass (reconstruction) filter, expressed
as a percentage of the full spectral bandwidth of the input signal (or the
output signal if the downsampling is performed), and the desired stop-band
attenuation in decibel.

The transition band is specified as the normalized spectral space of the input
signal (or the output signal if the downsampling is performed) between the
low-pass filter's -3 dB point and the Nyquist frequency, and ranges from 0.5%
to 45%.  Stop-band attenuation can be specified in the range from 49 to 218
decibel.  Both the transition band and stop-band attenuation affect
resampler's overall performance and initial output delay.  For your
information, transition frequency range spans 175% of the specified transition
band, which means that for 2% transition band, frequency response below
0.965\*Nyquist is linear.

This SRC library also implements a much faster "power of 2" resampling (e.g.
2X, 4X, 8X, 16X, 3X, 3\*2X, 3\*4X, 3\*8X, etc. upsampling and downsampling),
which is engaged automatically if the resampling parameters permit.

This library was tested for compatibility with [GNU C++](https://gcc.gnu.org/),
[Microsoft Visual C++](https://visualstudio.microsoft.com/),
[LLVM](https://llvm.org/) and [Intel C++](https://software.intel.com/en-us/c-compilers)
compilers, on 32- and 64-bit Windows, macOS, and CentOS Linux.

Most code is "inline", without the need to compile many source files. The
memory footprint is quite modest.

## Acknowledgements ##

r8brain-free-src is bundled with the following code:

* FFT routines Copyright (c) 1996-2001 Takuya OOURA.
[Homepage](http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html)
* PFFFT Copyright (c) 2013 Julien Pommier.
[Homepage](https://bitbucket.org/jpommier/pffft)
* PFFFT DOUBLE Copyright (c) 2020 Hayati Ayguen, Dario Mambro.
[Homepage](https://github.com/marton78/pffft)

## Users ##

This library is used by:

* [REAPER](https://reaper.fm/)
* [AUDIRVANA](https://audirvana.com/)
* [Red Dead Redemption 2](https://www.rockstargames.com/reddeadredemption2/credits)
* [Mini Piano Lite](https://play.google.com/store/apps/details?id=umito.android.minipiano)
* [OpenMPT](https://openmpt.org/)
* [Boogex Guitar Amp audio plugin](https://www.voxengo.com/product/boogex/)
* [r8brain free sample rate converter](https://www.voxengo.com/product/r8brain/)
* [Voice Aloud Reader](https://play.google.com/store/apps/details?id=com.hyperionics.avarLic)
* [Zynewave Podium](https://zynewave.com/podium/)
* [Phonometrica](http://www.phonometrica-ling.org/index.html)
* [Ripcord](https://cancel.fm/ripcord/)
* [TensorVox](https://github.com/ZDisket/TensorVox)
* [Curvessor](https://github.com/unevens/Curvessor)

Please send me a note via aleksey.vaneev@gmail.com and I will include a link
to your software product to this list of users. This list is important in
maintaining confidence in this library among the interested parties. The
inclusion into this list is not mandatory.

## Change Log ##

Version 6.2:

* Fixed miscalculation in the recently introduced getInLenBeforeOutPos()
function for minimum-phase filters.
* Fixed a mistake in the getInputRequiredForOutput() function.
* Fixed a long-standing mistake in LatencyFrac value of whole-stepping
interpolation. However, this mistake gave no practical issues before (absent
for linear-phase filters, and minor for minimum-phase filters).

Version 6.1:

* Made a micro-optimization of the "whole stepping" interpolation yielding 18%
performance increase in some conversions (e.g., 44100 to 96000).
* Implemented the getInLenBeforeOutPos() function which is an ultra-fast and
flexible replacement for the getInLenBeforeOutStart() function (that became a
legacy function now). Also added the getInputRequiredForOutput() helper
function.
* Updated comment sections across the codebase, to match the latest Doxygen
version.
* Reintroduced the r8b_inlen() function in the DLL.

Version 6.0:

* Added SSE and NEON implementations to `CDSPHBDownsampler` yielding 5-16%
performance improvement of power-of-2 downsampling.
* Further optimization of filter calculation making it 15% faster.
* Upped "SpinCount" in Windows mutex to 2000, to be on a safer side when the
filter cache is fully filled.
* Made the latest used "static" filter bank pop to the top of the list, for
cases when multiple "ReqAtten" values are in use in an application.

Version 5.9:

* Optimized filter calculation (Kaiser window function) with negligible change
in filtering results.
* Optimized min-phase filter's group delay calculation.
* Reduced "SpinCount" in Windows mutex to 1000.
* Made non-essential changes across the codebase and comments.

Version 5.8:

* Rearranged FFT macros, added `R8B_PFFFT` and `R8B_PFFFT_DOUBLE` collision
check.

Version 5.7:

* Removed the `defined( __ARM_NEON )` macro detection so that the code
compiles on non-ARM64 platforms.

Version 5.6:

* Added SSE and NEON implementations to `CDSPHBUpsampler` yielding 15%
performance improvement of power-of-2 upsampling.
* Added SSE and NEON implementations to the `CDSPRealFFT::multiplyBlocksZP`
function, for 2-3% performance improvement.
* Added intermediate interpolator's transition band limitation, for logical
hardness (not practically needed).
* Added the `aDoConsumeLatency` parameter to `CDSPHBUpsampler` constructor,
for "inline" DSP uses of the class.
* Made various minor changes across the codebase.

Version 5.5:

* Hardened positional logic of fractional filter calculation, removed
redundant multiplications.
* Removed unnecessary function templating from the `CDSPSincFilterGen` class.
* Added the `__ARM_NEON` macro to NEON availability detection.

Version 5.4:

* Added compiler specializations to previously optimized inner loops.
"Shuffled" SIMD interpolation code is not efficient on Apple M1. Intel C++
Compiler vectorizes "whole stepping" interpolation as good as a
manually-written SSE.
* Reorganized SIMD instructions for a slightly better performance.
* Changed internal buffer sizes of half-band resamplers (1-2% performance
boost).
* Fixed compiler warnings in PFFFT code.
* Added several asserts to the code.

Version 5.3:

* Optimized inner loops of the fractional interpolator, added SSE2 and NEON
intrinsics, resulting in a measurable (8-25%) performance gain.
* Optimized filter calculation functions: changed some divisions by a constant
to multiplications.
* Renamed M_PI macros to R8B_PI, to avoid macro collisions.
* Removed redundant code and macros.

Version 5.2:

* Modified `PFFFT` and `PFFFT DOUBLE` conditional pre-processor directives to
always enable NEON on `aarch64`/`arm64` (this includes code built for
Apple M1).

Version 5.1:

* Changed alignment in the `CFixedBuffer` class to 64 bytes. This improves AVX
performance of the `PFFFT DOUBLE` implementation by a few percent.
* Removed redundant files from the `pffft_double` folder, integrated the
`pffft_common.c` file into the `pffft_double.c` file.

Version 5.0:

* Removed a long-outdated macros from the `r8bconf.h` file.
* Changed a conditional pre-processor directive in the `pf_sse2_double.h` file
as per PFFFT DOUBLE author's suggestion, to allow SSE2 intrinsics in most
compilers.
* Fixed "License.txt" misnaming in the source files to "LICENSE".

Version 4.10:

* Added the `PFFFT DOUBLE` implementation support. Now available via the
`R8B_PFFFT_DOUBLE` definition macro.

Version 4.9:

* Reoptimized half-band and fractional interpolation filters with a stricter
frequency response linearity constraints. This did not impact the average
speed performance.

Version 4.8:

* Added a limit to the intermediate filter's transition band, to keep the
latency under control at any resampling ratio.

Version 4.7:

* Added `#ifndef _USE_MATH_DEFINES` to `pffft.cpp`.
* Moved `#include "pffft.h"` to `CDSPRealFFT.h`.

Version 4.6:

* Removed the `MaxInLen` parameter from the `oneshot()` function.
* Decreased intermediate low-pass filter's transition band slightly, for more
stable quality.

Version 4.5:

* Fixed VS2017 compiler warnings.

Version 4.4:

* Fixed the "Declaration hides a member" Intel C++ compiler warnings.

Version 4.3:

* Added //$ markers for internal debugging purposes.

Version 4.2:

* Backed-off max transition band to 45 and MinAtten to 49.
* Implemented Wave64 and AIFF file input in the `r8bfreesrc` bench tool. The
tool is now compiled with the `R8B_IPP 1` and `R8B_EXTFFT 1` macros to
demonstrate the maximal achievable performance.

Version 4.1:

* Updated allowed ReqAtten range to 52-218, ReqTransBand 0.5-56. It is
possible to specify filter parameters slightly beyond these values, but the
resulting filter will be slightly out of specification as well.
* Optimized static filter banks allocation.

Version 4.0:

* A major overhaul of interpolation classes: now templated parameters are not
used, all required parameters are calculated at runtime. Static filter bank
object is not used, it is created when necessary, and then cached.
* Implemented one-third interpolation filters, however, this did not
measurably increase resampler's speed.

Version 3.7:

* Used ippsMul_64f_I() in the CDSPRealFFT::multiplyBlockZ() function for a
minor conversion speed increase in Intel IPP mode.

Version 3.6:

* Added memory alignment to allocated buffers which boosts performance by 1.5%
when Intel IPP FFT is in use.
* Implemented PFFFT support.

Version 3.5:

* Improved resampling speed very slightly.
* Updated the `r8bfreesrc` benchmark tool to support RF64 WAV files.

Version 3.4:

* Added a more efficient half-band filters for >= 256 resampling ratios.

Version 3.3:

* Made minor fix to downsampling for some use cases of CDSPBlockConvolver,
did not affect resampler.
* Converted CDSPHBUpsampler and CDSPHBDownsampler's inner functions to
static functions, which boosted high-ratio resampling performance measurably.

Version 3.2:

* Minor fix to the latency consumption mechanism.

Version 3.1:

* Reoptimized fractional delay filter's windowing function.

Version 3.0:

* Implemented a new variant of the getInLenBeforeOutStart() function.
* Reimplemented oneshot() function to support `float` buffer types.
* Considerably improved downsampling performance at high resampling ratios.
* Implemented intermediate interpolation technique which boosted upsampling
performance for most resampling ratios considerably.
* Removed the ConvCount constant - now resampler supports virtually any
resampling ratios.
* Removed the UsePower2 parameter from the resampler constructor.
* Now resampler's process() function always returns pointer to the internal
buffer, input buffer is returned only if no resampling happens.
* Resampler's getMaxOutLen() function can now be used to obtain the maximal
output length that can be produced by the resampler in a single call.
* Added a more efficient "one-third" filters to half-band upsampler and
downsampler.

Version 2.1:

* Optimized 2X half-band downsampler.

Version 2.0:

* Optimized power-of-2 upsampling.

Version 1.9:

* Optimized half-band downsampling filter.
* Implemented whole-number stepping resampling.
* Added `R8B_EXTFFT` configuration option.
* Fixed initial sub-sample offseting on downsampling.

Version 1.8:

* Added `R8B_FASTTIMING` configuration option.

Version 1.7:

* Improved sample timing precision.
* Increased CDSPResampler :: ConvCountMax to 28 to support a lot wider
resampling ratios.
* Added `bench` tools.
* Removed getInLenBeforeOutStart() due to incorrect calculation.
