//$ nobt
//$ nocpp

/**
 * @file CDSPFracInterpolator.h
 *
 * @brief Fractional delay interpolator and filter bank classes.
 *
 * This file includes fractional delay interpolator class.
 *
 * r8brain-free-src Copyright (c) 2013-2019 Aleksey Vaneev
 * See the "License.txt" file for license.
 */

#ifndef R8B_CDSPFRACINTERPOLATOR_INCLUDED
#define R8B_CDSPFRACINTERPOLATOR_INCLUDED

#include "CDSPSincFilterGen.h"
#include "CDSPProcessor.h"

namespace r8b {

#if R8B_FLTTEST
	extern int InterpFilterFracs; ///< Force this number of fractional filter
		///< positions. -1 - use default.
		///<
	extern int InterpFilterFracsThird; ///< Force this number of fractional
		///< filter positions for one-third filters. -1 - use default.
		///<
#endif // R8B_FLTTEST

/**
 * @brief Sinc function-based fractional delay filter bank class.
 *
 * Class implements storage and initialization of a bank of sinc-based
 * fractional delay filters, expressed as 0th, 1st, 2nd or 3rd order
 * polynomial interpolation coefficients. The filters are windowed by the
 * "Kaiser" power-raised window function.
 */

class CDSPFracDelayFilterBank : public R8B_BASECLASS
{
	R8BNOCTOR( CDSPFracDelayFilterBank );

	friend class CDSPFracDelayFilterBankCache;

public:
	/**
	 * Constructor.
	 *
	 * @param aFilterFracs The number of fractional delay positions to sample,
	 * -1 - use default.
	 * @param aElementSize The size of each filter's tap, in "double" values.
	 * This parameter corresponds to the complexity of interpolation. 4 should
	 * be set for 3rd order, 3 for 2nd order, 2 for linear interpolation, 1
	 * for whole-numbered stepping.
	 * @param aInterpPoints The number of points the interpolation is based
	 * on. This value should not be confused with the ElementSize. Set to 2
	 * for linear or no interpolation.
	 * @param aReqAtten Required filter attentuation.
	 * @param aIsThird "True" if one-third filter is required.
	 */

	CDSPFracDelayFilterBank( const int aFilterFracs, const int aElementSize,
		const int aInterpPoints, const double aReqAtten, const bool aIsThird )
		: InitFilterFracs( aFilterFracs )
		, ElementSize( aElementSize )
		, InterpPoints( aInterpPoints )
		, ReqAtten( aReqAtten )
		, IsThird( aIsThird )
		, Next( NULL )
		, RefCount( 1 )
	{
		R8BASSERT( ElementSize >= 1 && ElementSize <= 4 );

		// Kaiser window function Params, for half and third-band.

		const double* const Params = getWinParams( ReqAtten, IsThird,
			FilterLen );

		FilterSize = FilterLen * ElementSize;

		if( InitFilterFracs == -1 )
		{
			FilterFracs = (int) ceil( 1.792462178761753 *
				exp( 0.033300466782047 * ReqAtten ));

			#if R8B_FLTTEST

			if( IsThird )
			{
				if( InterpFilterFracsThird != -1 )
				{
					FilterFracs = InterpFilterFracsThird;
				}
			}
			else
			{
				if( InterpFilterFracs != -1 )
				{
					FilterFracs = InterpFilterFracs;
				}
			}

			#endif // R8B_FLTTEST
		}
		else
		{
			FilterFracs = InitFilterFracs;
		}

		Table.alloc( FilterSize * ( FilterFracs + InterpPoints ));

		CDSPSincFilterGen sinc;
		sinc.Len2 = FilterLen / 2;

		double* p = Table;
		const int pc2 = InterpPoints / 2;
		int i;

		for( i = -pc2 + 1; i <= FilterFracs + pc2; i++ )
		{
			sinc.FracDelay = (double) ( FilterFracs - i ) / FilterFracs;
			sinc.initFrac( CDSPSincFilterGen :: wftKaiser, Params, true );
			sinc.generateFrac( p, &CDSPSincFilterGen :: calcWindowKaiser,
				ElementSize );

			normalizeFIRFilter( p, FilterLen, 1.0, ElementSize );
			p += FilterSize;
		}

		const int TablePos2 = FilterSize;
		const int TablePos3 = FilterSize * 2;
		const int TablePos4 = FilterSize * 3;
		const int TablePos5 = FilterSize * 4;
		const int TablePos6 = FilterSize * 5;
		const int TablePos7 = FilterSize * 6;
		const int TablePos8 = FilterSize * 7;
		double* const TableEnd = Table + ( FilterFracs + 1 ) * FilterSize;
		p = Table;

		if( InterpPoints == 8 )
		{
			if( ElementSize == 3 )
			{
				// Calculate 2nd order spline (polynomial) interpolation
				// coefficients using 8 points.

				while( p < TableEnd )
				{
					calcSpline2p8Coeffs( p, p[ 0 ], p[ TablePos2 ],
						p[ TablePos3 ], p[ TablePos4 ], p[ TablePos5 ],
						p[ TablePos6 ], p[ TablePos7 ], p[ TablePos8 ]);

					p += ElementSize;
				}
			}
			else
			if( ElementSize == 4 )
			{
				// Calculate 3rd order spline (polynomial) interpolation
				// coefficients using 8 points.

				while( p < TableEnd )
				{
					calcSpline3p8Coeffs( p, p[ 0 ], p[ TablePos2 ],
						p[ TablePos3 ], p[ TablePos4 ], p[ TablePos5 ],
						p[ TablePos6 ], p[ TablePos7 ], p[ TablePos8 ]);

					p += ElementSize;
				}
			}
		}
		else
		{
			if( ElementSize == 2 )
			{
				// Calculate linear interpolation coefficients.

				while( p < TableEnd )
				{
					p[ 1 ] = p[ TablePos2 ] - p[ 0 ];
					p += ElementSize;
				}
			}
		}

		R8BCONSOLE( "CDSPFracDelayFilterBank: fracs=%i order=%i taps=%i "
			"att=%.1f third=%i\n", FilterFracs, ElementSize - 1, FilterLen,
			ReqAtten, (int) IsThird );
	}

	~CDSPFracDelayFilterBank()
	{
		delete Next;
	}

	/**
	 * Function "rounds" the specified attenuation to the nearest effective
	 * value.
	 *
	 * @param[in,out] att Required filter attentuation. Will be rounded to the
	 * nearest value.
	 * @param aIsThird "True" if one-third filter is required.
	 */

	static void roundReqAtten( double& att, const bool aIsThird )
	{
		int tmp;
		getWinParams( att, aIsThird, tmp );
	}

	/**
	 * Function returns the length of the filter.
	 */

	int getFilterLen() const
	{
		return( FilterLen );
	}

	/**
	 * Function returns the number of fractional positions sampled by the
	 * bank.
	 */

	int getFilterFracs() const
	{
		return( FilterFracs );
	}

	/**
	 * @param i Filter index, in the range 0 to FilterFracs, inclusive.
	 * @return Reference to the filter.
	 */

	const double& operator []( const int i ) const
	{
		R8BASSERT( i >= 0 && i <= FilterFracs );

		return( Table[ i * FilterSize ]);
	}

	/**
	 * This function should be called when the filter obtained via the
	 * filter bank cache is no longer needed.
	 */

	void unref();

private:
	int FilterLen; ///< Filter length.
		///<
	int FilterFracs; ///< Fractional position count.
		///<
	int InitFilterFracs; ///< Fractional position count as supplied to the
		///< constructor, may equal -1.
		///<
	int ElementSize; ///< Filter element size.
		///<
	int InterpPoints; ///< Interpolation points to use.
		///<
	double ReqAtten; ///< Filter's attentuation.
		///<
	bool IsThird; ///< "True" if one-third filter is in use.
		///<
	int FilterSize; ///< This constant specifies the "size" of a single filter
		///< in "double" elements.
		///<
	CFixedBuffer< double > Table; ///< The table of fractional delay filters
		///< for all discrete fractional x = 0..1 sample positions, and
		///< interpolation coefficients.
		///<
	CDSPFracDelayFilterBank* Next; ///< Next filter bank in cache's list.
		///<
	int RefCount; ///< The number of references made to *this filter bank.
		///< Not considered for "static" filter bank objects.
		///<

	/**
	 * Function returns windowing function parameters for the specified
	 * attenuation and filter type.
	 *
	 * @param[in,out] att Required filter attentuation. Will be rounded to the
	 * nearest value.
	 * @param aIsThird "True" if one-third filter is required.
	 * @param[out] fltlen Resulting filter length.
	 */

	static const double* getWinParams( double& att, const bool aIsThird,
		int& fltlen )
	{
		const int CoeffCount = 13;
		static const double Coeffs[ CoeffCount ][ 3 ] = {
			{ 2.6504246356892924, 1.9035845248358245, 51.7280 }, // 0.0516
			{ 4.0759654812373016, 1.5747323142948524, 67.1095 }, // 0.0048
			{ 4.9036508646352033, 1.6207644759455790, 81.8379 }, // 0.0009
			{ 5.6131421124830716, 1.6947677220415129, 96.4021 }, // 0.0002
			{ 5.9433751253133691, 1.8730186383321272, 111.1300 }, // 0.0000
			{ 6.8308658253825660, 1.8549555120377224, 125.4649 }, // 0.0000
			{ 7.6648458853758372, 1.8565765953924642, 139.7378 }, // 0.0000
			{ 8.2038730802326842, 1.9269521308895179, 154.0532 }, // 0.0000
			{ 8.7865151489187561, 1.9775307528231671, 168.2101 }, // 0.0000
			{ 9.5945013206755156, 1.9718457932433306, 182.1076 }, // 0.0000
			{ 10.5163048616210250, 1.9504085061576968, 195.5668 }, // 0.0000
			{ 10.2382664677006100, 2.1608878780497056, 209.0609 }, // 0.0000
			{ 10.9976663155261660, 2.1536415815428249, 222.5009 }, // 0.0000
		};

		const int CoeffCountThird = 10;
		static const double CoeffsThird[ CoeffCountThird ][ 3 ] = {
			{ 4.0738201365282452, 1.5774150265957998, 67.2431 }, // 0.0050
			{ 4.9502289040040495, 1.7149006172407628, 86.4870 }, // 0.0008
			{ 5.5995071332976192, 1.8930163359641823, 106.1171 }, // 0.0001
			{ 6.3627287856776054, 1.9945748303811506, 125.2304 }, // 0.0000
			{ 7.4299554386534528, 1.9893399585993299, 144.3469 }, // 0.0000
			{ 8.0667710807396436, 2.0928202837610885, 163.4098 }, // 0.0000
			{ 8.7469991933128526, 2.1640274270903488, 181.0694 }, // 0.0000
			{ 10.0823164330540570, 2.0896732996403280, 199.2880 }, // 0.0000
			{ 19.1718281840114810, 1.2030083075440616, 215.2990 }, // 0.0000
			{ 21.0914128488567630, 1.1919045429676862, 233.9152 }, // 0.0000
		};

		const double* Params;
		int i = 0;

		if( aIsThird )
		{
			while( i != CoeffCountThird - 1 && CoeffsThird[ i ][ 2 ] < att )
			{
				i++;
			}

			Params = &CoeffsThird[ i ][ 0 ];
			att = CoeffsThird[ i ][ 2 ];
		}
		else
		{
			while( i != CoeffCount - 1 && Coeffs[ i ][ 2 ] < att )
			{
				i++;
			}

			Params = &Coeffs[ i ][ 0 ];
			att = Coeffs[ i ][ 2 ];
		}

		fltlen = ( i + 3 ) * 2;

		return( Params );
	}
};

/**
 * @brief Fractional delay filter cache class.
 *
 * Class implements cache storage of fractional delay filter banks.
 */

class CDSPFracDelayFilterBankCache : public R8B_BASECLASS
{
	R8BNOCTOR( CDSPFracDelayFilterBankCache );

	friend class CDSPFracDelayFilterBank;

public:
	/**
	 * @return The number of filters present in the cache now. This value can
	 * be monitored for debugging "forgotten" filters.
	 */

	static int getObjCount()
	{
		R8BSYNC( StateSync );

		return( ObjCount );
	}

	/**
	 * Function calculates or returns reference to a previously calculated
	 * (cached) fractional delay filter bank.
	 *
	 * @param aFilterFracs The number of fractional delay positions to sample,
	 * -1 - use default.
	 * @param aElementSize The size of each filter's tap, in "double" values.
	 * @param aInterpPoints The number of points the interpolation is based
	 * on.
	 * @param ReqAtten Required filter attentuation.
	 * @param IsThird "True" if one-third filter is required.
	 * @param IsStatic "True" if a permanent static filter should be returned
	 * that is never removed from the cache until application terminates.
	 */

	static CDSPFracDelayFilterBank& getFilterBank( const int aFilterFracs,
		const int aElementSize, const int aInterpPoints,
		double ReqAtten, const bool IsThird, const bool IsStatic )
	{
		CDSPFracDelayFilterBank :: roundReqAtten( ReqAtten, IsThird );

		R8BSYNC( StateSync );

		if( IsStatic )
		{
			CDSPFracDelayFilterBank* CurObj = StaticObjects;

			while( CurObj != NULL )
			{
				if( CurObj -> InitFilterFracs == aFilterFracs &&
					CurObj -> ElementSize == aElementSize &&
					CurObj -> InterpPoints == aInterpPoints &&
					CurObj -> ReqAtten == ReqAtten &&
					CurObj -> IsThird == IsThird )
				{
					return( *CurObj );
				}

				CurObj = CurObj -> Next;
			}

			// Create a new filter bank and build it.

			CurObj = new CDSPFracDelayFilterBank( aFilterFracs, aElementSize,
				aInterpPoints, ReqAtten, IsThird );

			// Insert the bank at the start of the list.

			CurObj -> Next = StaticObjects.unkeep();
			StaticObjects = CurObj;

			return( *CurObj );
		}

		CDSPFracDelayFilterBank* PrevObj = NULL;
		CDSPFracDelayFilterBank* CurObj = Objects;

		while( CurObj != NULL )
		{
			if( CurObj -> InitFilterFracs == aFilterFracs &&
				CurObj -> ElementSize == aElementSize &&
				CurObj -> InterpPoints == aInterpPoints &&
				CurObj -> ReqAtten == ReqAtten &&
				CurObj -> IsThird == IsThird )
			{
				break;
			}

			if( CurObj -> Next == NULL && ObjCount >= R8B_FRACBANK_CACHE_MAX )
			{
				if( CurObj -> RefCount == 0 )
				{
					// Delete the last bank which is not used.

					PrevObj -> Next = NULL;
					delete CurObj;
					ObjCount--;
				}
				else
				{
					// Move the last bank to the top of the list since it
					// seems to be in use for a long time.

					PrevObj -> Next = NULL;
					CurObj -> Next = Objects.unkeep();
					Objects = CurObj;
				}

				CurObj = NULL;
				break;
			}

			PrevObj = CurObj;
			CurObj = CurObj -> Next;
		}

		if( CurObj != NULL )
		{
			CurObj -> RefCount++;

			if( PrevObj == NULL )
			{
				return( *CurObj );
			}

			// Remove the bank from the list temporarily.

			PrevObj -> Next = CurObj -> Next;
		}
		else
		{
			// Create a new filter bank (with RefCount == 1) and build it.

			CurObj = new CDSPFracDelayFilterBank( aFilterFracs, aElementSize,
				aInterpPoints, ReqAtten, IsThird );

			ObjCount++;
		}

		// Insert the bank at the start of the list.

		CurObj -> Next = Objects.unkeep();
		Objects = CurObj;

		return( *CurObj );
	}

private:
	static CSyncObject StateSync; ///< Cache state synchronizer.
		///<
	static CPtrKeeper< CDSPFracDelayFilterBank* > Objects; ///< The chain of
		///< cached objects.
		///<
	static CPtrKeeper< CDSPFracDelayFilterBank* > StaticObjects; ///< The
		///< chain of static objects.
		///<
	static int ObjCount; ///< The number of objects currently preset in the
		///< Objects cache.
		///<
};

// ---------------------------------------------------------------------------
// CDSPFracDelayFilterBank PUBLIC
// ---------------------------------------------------------------------------

inline void CDSPFracDelayFilterBank :: unref()
{
	R8BSYNC( CDSPFracDelayFilterBankCache :: StateSync );

	RefCount--;
}

/**
 * @param l Number 1.
 * @param s Number 2.
 * @param[out] GCD Resulting GCD.
 * @return "True" if the greatest common denominator of 2 numbers was
 * found.
 */

inline bool findGCD( double l, double s, double& GCD )
{
	int it = 0;

	while( it < 50 )
	{
		if( s <= 0.0 )
		{
			GCD = l;
			return( true );
		}

		const double r = l - s;
		l = s;
		s = ( r < 0.0 ? -r : r );
		it++;
	}

	return( false );
}

/**
 * Function evaluates source and destination sample rate ratio and returns
 * the required input and output stepping. Function returns "false" if
 * *this class cannot be used to perform interpolation using these sample
 * rates.
 *
 * @param SSampleRate Source sample rate.
 * @param DSampleRate Destination sample rate.
 * @param[out] ResInStep Resulting input step.
 * @param[out] ResOutStep Resulting output step.
 * @return "True" if stepping was acquired.
 */

inline bool getWholeStepping( const double SSampleRate,
	const double DSampleRate, int& ResInStep, int& ResOutStep )
{
	double GCD;

	if( !findGCD( SSampleRate, DSampleRate, GCD ) || GCD < 1.0 )
	{
		return( false );
	}

	const double InStep0 = SSampleRate / GCD;
	ResInStep = (int) InStep0;
	const double OutStep0 = DSampleRate / GCD;
	ResOutStep = (int) OutStep0;

	if( InStep0 != ResInStep || OutStep0 != ResOutStep )
	{
		return( false );
	}

	if( ResOutStep > 1500 )
	{
		// Do not allow large output stepping due to low cache
		// performance of large filter banks.

		return( false );
	}

	return( true );
}

/**
 * @brief Fractional delay filter bank-based interpolator class.
 *
 * Class implements the fractional delay interpolator. This implementation at
 * first puts the input signal into a ring buffer and then performs
 * interpolation. The interpolation is performed using sinc-based fractional
 * delay filters. These filters are contained in a bank, and for higher
 * precision they are interpolated between adjacent filters.
 *
 * To increase sample timing precision, this class uses "resettable counter"
 * approach. This gives zero overall sample timing error. With the
 * R8B_FASTTIMING configuration option enabled, the sample timing experiences
 * a very minor drift.
 */

class CDSPFracInterpolator : public CDSPProcessor
{
public:
	/**
	 * Constructor initalizes the interpolator. It is important to call the
	 * getMaxOutLen() function afterwards to obtain the optimal output buffer
	 * length.
	 *
	 * @param aSrcSampleRate Source sample rate.
	 * @param aDstSampleRate Destination sample rate.
	 * @param ReqAtten Required filter attentuation.
	 * @param IsThird "True" if one-third filter is required.
	 * @param PrevLatency Latency, in samples (any value >=0), which was left
	 * in the output signal by a previous process. This latency will be
	 * consumed completely.
	 */

	CDSPFracInterpolator( const double aSrcSampleRate,
		const double aDstSampleRate, const double ReqAtten,
		const bool IsThird, const double PrevLatency )
		: SrcSampleRate( aSrcSampleRate )
		, DstSampleRate( aDstSampleRate )
	#if R8B_FASTTIMING
		, FracStep( aSrcSampleRate / aDstSampleRate )
	#endif // R8B_FASTTIMING
	{
		R8BASSERT( SrcSampleRate > 0.0 );
		R8BASSERT( DstSampleRate > 0.0 );
		R8BASSERT( PrevLatency >= 0.0 );
		R8BASSERT( BufLenBits >= 5 );
		R8BASSERT(( 1 << BufLenBits ) >= FilterLen * 3 );

		InitFracPos = PrevLatency;
		Latency = (int) InitFracPos;
		InitFracPos -= Latency;

		#if R8B_FLTTEST

			IsWhole = false;
			LatencyFrac = 0.0;
			FilterBank = new CDSPFracDelayFilterBank( -1, 3, 8, ReqAtten,
				IsThird );

		#else // R8B_FLTTEST

			IsWhole = getWholeStepping( SrcSampleRate, DstSampleRate, InStep,
				OutStep );

			if( IsWhole )
			{
				InitFracPosW = (int) ( InitFracPos * OutStep );
				LatencyFrac = InitFracPos - (double) InitFracPosW / OutStep;
				FilterBank = &CDSPFracDelayFilterBankCache :: getFilterBank(
					OutStep, 1, 2, ReqAtten, IsThird, false );
			}
			else
			{
				LatencyFrac = 0.0;
				FilterBank = &CDSPFracDelayFilterBankCache :: getFilterBank(
					-1, 3, 8, ReqAtten, IsThird, true );
			}

		#endif // R8B_FLTTEST

		FilterLen = FilterBank -> getFilterLen();
		fl2 = FilterLen >> 1;
		fll = fl2 - 1;
		flo = fll + fl2;

		static const CConvolveFn FltConvFn0[ 13 ] = {
			&CDSPFracInterpolator :: convolve0< 6 >,
			&CDSPFracInterpolator :: convolve0< 8 >,
			&CDSPFracInterpolator :: convolve0< 10 >,
			&CDSPFracInterpolator :: convolve0< 12 >,
			&CDSPFracInterpolator :: convolve0< 14 >,
			&CDSPFracInterpolator :: convolve0< 16 >,
			&CDSPFracInterpolator :: convolve0< 18 >,
			&CDSPFracInterpolator :: convolve0< 20 >,
			&CDSPFracInterpolator :: convolve0< 22 >,
			&CDSPFracInterpolator :: convolve0< 24 >,
			&CDSPFracInterpolator :: convolve0< 26 >,
			&CDSPFracInterpolator :: convolve0< 28 >,
			&CDSPFracInterpolator :: convolve0< 30 >
		};

		convfn = ( IsWhole ? FltConvFn0[ fl2 - 3 ] :
			&CDSPFracInterpolator :: convolve2 );

		R8BCONSOLE( "CDSPFracInterpolator: src=%.2f dst=%.2f taps=%i "
			"fracs=%i third=%i step=%.6f\n", SrcSampleRate, DstSampleRate,
			FilterLen, ( IsWhole ? OutStep : FilterBank -> getFilterFracs() ),
			(int) IsThird, aSrcSampleRate / aDstSampleRate );

		clear();
	}

	virtual ~CDSPFracInterpolator()
	{
		#if R8B_FLTTEST

			delete FilterBank;

		#else // R8B_FLTTEST

			FilterBank -> unref();

		#endif // R8B_FLTTEST
	}

	virtual int getLatency() const
	{
		return( 0 );
	}

	virtual double getLatencyFrac() const
	{
		return( LatencyFrac );
	}

	virtual int getMaxOutLen( const int MaxInLen ) const
	{
		R8BASSERT( MaxInLen >= 0 );

		return( (int) ceil( MaxInLen * DstSampleRate / SrcSampleRate ) + 1 );
	}

	virtual void clear()
	{
		LatencyLeft = Latency;
		BufLeft = 0;
		WritePos = 0;
		ReadPos = BufLen - fll; // Set "read" position to account for filter's
			// latency at zero fractional delay.

		memset( &Buf[ ReadPos ], 0, fll * sizeof( double ));

		if( IsWhole )
		{
			InPosFracW = InitFracPosW;
		}
		else
		{
			InPosFrac = InitFracPos;

			#if !R8B_FASTTIMING
				InCounter = 0;
				InPosInt = 0;
				InPosShift = InitFracPos * DstSampleRate / SrcSampleRate;
			#endif // !R8B_FASTTIMING
		}
	}

	virtual int process( double* ip, int l, double*& op0 )
	{
		R8BASSERT( l >= 0 );
		R8BASSERT( ip != op0 || l == 0 || SrcSampleRate > DstSampleRate );

		if( LatencyLeft > 0 )
		{
			if( LatencyLeft >= l )
			{
				LatencyLeft -= l;
				return( 0 );
			}

			l -= LatencyLeft;
			ip += LatencyLeft;
			LatencyLeft = 0;
		}

		double* op = op0;

		while( l > 0 )
		{
			// Add new input samples to both halves of the ring buffer.

			const int b = min( min( l, BufLen - WritePos ),
				BufLen - fll - BufLeft );

			double* const wp1 = Buf + WritePos;
			memcpy( wp1, ip, b * sizeof( double ));

			if( WritePos < flo )
			{
				const int c = min( b, flo - WritePos );
				memcpy( wp1 + BufLen, wp1, c * sizeof( double ));
			}

			ip += b;
			WritePos = ( WritePos + b ) & BufLenMask;
			l -= b;
			BufLeft += b;

			// Produce as many output samples as possible.

			op = ( *this.*convfn )( op );
		}

		#if !R8B_FASTTIMING

			if( !IsWhole && InCounter > 1000 )
			{
				// Reset the interpolation position counter to achieve a
				// higher sample timing precision.

				InCounter = 0;
				InPosInt = 0;
				InPosShift = InPosFrac * DstSampleRate / SrcSampleRate;
			}

		#endif // !R8B_FASTTIMING

		return( (int) ( op - op0 ));
	}

private:
	static const int BufLenBits = 8; ///< The length of the ring buffer,
		///< expressed as Nth power of 2. This value can be reduced if it is
		///< known that only short input buffers will be passed to the
		///< interpolator. The minimum value of this parameter is 5, and
		///< 1 << BufLenBits should be at least 3 times larger than the
		///< FilterLen. However, this condition can be easily met if the input
		///< signal is suitably downsampled first before the interpolation is
		///< performed.
		///<
	static const int BufLen = 1 << BufLenBits; ///< The length of the ring
		///< buffer. The actual length is twice as long to allow "beyond max
		///< position" positioning.
		///<
	static const int BufLenMask = BufLen - 1; ///< Mask used for quick buffer
		///< position wrapping.
		///<
	int FilterLen; ///< Filter length, in taps. Even value.
		///<
	int fl2; ///< Right-side (half) filter length.
		///<
	int fll; ///< Input latency.
		///<
	int flo; ///< Overrun length.
		///<
	double Buf[ BufLen + 29 ]; ///< The ring buffer, including overrun
		///< protection for maximal filter length.
		///<
	double SrcSampleRate; ///< Source sample rate.
		///<
	double DstSampleRate; ///< Destination sample rate.
		///<
	bool IsWhole; ///< "True" if whole-number stepping is in use.
		///<
	int InStep; ///< Input whole-number stepping.
		///<
	int OutStep; ///< Output whole-number stepping (corresponds to filter bank
		///< size).
		///<
	double InitFracPos; ///< Initial fractional position, in samples, in the
		///< range [0; 1).
		///<
	int InitFracPosW; ///< Initial fractional position for whole-number
		///< stepping.
		///<
	int Latency; ///< Initial latency that should be removed from the input.
		///<
	double LatencyFrac; ///< Left-over fractional latency.
		///<
	int BufLeft; ///< The number of samples left in the buffer to process.
		///<
	int WritePos; ///< The current buffer write position. Incremented together
		///< with the BufLeft variable.
		///<
	int ReadPos; ///< The current buffer read position.
		///<
	int LatencyLeft; ///< Input latency left to remove.
		///<
	double InPosFrac; ///< Interpolation position (fractional part).
		///<
	int InPosFracW; ///< Interpolation position (fractional part) for
		///< whole-number stepping. Corresponds to the index into the filter
		///< bank.
		///<
	CDSPFracDelayFilterBank* FilterBank; ///< Filter bank in use, may be
		///< whole-number stepping filter bank or static bank.
		///<
#if R8B_FASTTIMING
	double FracStep; ///< Fractional sample timing step.
#else // R8B_FASTTIMING
	int InCounter; ///< Interpolation step counter.
		///<
	int InPosInt; ///< Interpolation position (integer part).
		///<
	double InPosShift; ///< Interpolation position fractional shift.
		///<
#endif // R8B_FASTTIMING

	typedef double*( CDSPFracInterpolator :: *CConvolveFn )( double* op ); ///<
		///< Convolution funtion type.
		///<
	CConvolveFn convfn; ///< Convolution function in use.
		///<

	/**
	 * Convolution function for 0th order resampling.
	 *
	 * @param[out] op Output buffer.
	 * @return Advanced "op" value.
	 * @tparam fltlen Filter length.
	 */

	template< int fltlen >
	double* convolve0( double* op )
	{
		while( BufLeft > fl2 )
		{
			const double* const ftp = &(*FilterBank)[ InPosFracW ];
			const double* const rp = Buf + ReadPos;
			double s = 0.0;
			int i;

			for( i = 0; i < fltlen; i++ )
			{
				s += ftp[ i ] * rp[ i ];
			}

			*op = s;
			op++;

			InPosFracW += InStep;
			const int PosIncr = InPosFracW / OutStep;
			InPosFracW -= PosIncr * OutStep;

			ReadPos = ( ReadPos + PosIncr ) & BufLenMask;
			BufLeft -= PosIncr;
		}

		return( op );
	}

	/**
	 * Convolution function for 2nd order resampling.
	 *
	 * @param[out] op Output buffer.
	 * @return Advanced "op" value.
	 */

	double* convolve2( double* op )
	{
		while( BufLeft > fl2 )
		{
			double x = InPosFrac * FilterBank -> getFilterFracs();
			const int fti = (int) x; // Function table index.
			x -= fti; // Coefficient for interpolation between
				// adjacent fractional delay filters.
			const double x2 = x * x;
			const double* const ftp = &(*FilterBank)[ fti ];
			const double* const rp = Buf + ReadPos;
			double s = 0.0;
			int ii = 0;
			int i;

			for( i = 0; i < FilterLen; i++ )
			{
				s += ( ftp[ ii ] + ftp[ ii + 1 ] * x +
					ftp[ ii + 2 ] * x2 ) * rp[ i ];

				ii += 3;
			}

			*op = s;
			op++;

			#if R8B_FASTTIMING

				InPosFrac += FracStep;
				const int PosIncr = (int) InPosFrac;
				InPosFrac -= PosIncr;

			#else // R8B_FASTTIMING

				InCounter++;
				const double NextInPos = ( InCounter + InPosShift ) *
					SrcSampleRate / DstSampleRate;

				const int NextInPosInt = (int) NextInPos;
				const int PosIncr = NextInPosInt - InPosInt;
				InPosInt = NextInPosInt;
				InPosFrac = NextInPos - NextInPosInt;

			#endif // R8B_FASTTIMING

			ReadPos = ( ReadPos + PosIncr ) & BufLenMask;
			BufLeft -= PosIncr;
		}

		return( op );
	}
};

// ---------------------------------------------------------------------------

} // namespace r8b

#endif // R8B_CDSPFRACINTERPOLATOR_INCLUDED
