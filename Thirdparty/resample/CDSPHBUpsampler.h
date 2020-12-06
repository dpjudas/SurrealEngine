//$ nobt
//$ nocpp

/**
 * @file CDSPHBUpsampler.h
 *
 * @brief Half-band upsampling class.
 *
 * This file includes half-band upsampling class.
 *
 * r8brain-free-src Copyright (c) 2019 Aleksey Vaneev
 * See the "License.txt" file for license.
 */

#ifndef R8B_CDSPHBUPSAMPLER_INCLUDED
#define R8B_CDSPHBUPSAMPLER_INCLUDED

#include "CDSPProcessor.h"

namespace r8b {

/**
 * @brief Half-band upsampling class.
 *
 * Class implements brute-force half-band 2X upsampling that uses small
 * sparse symmetric FIR filters. It is very efficient and should be used at
 * latter upsampling steps after initial steep 2X upsampling.
 */

class CDSPHBUpsampler : public CDSPProcessor
{
public:
	/**
	 * Function that provides filter data for various steepness indices and
	 * attenuations.
	 *
	 * @param ReqAtten Required half-band filter attentuation.
	 * @param SteepIndex Steepness index - 0=steepest. Corresponds to general
	 * upsampling/downsampling ratio, e.g. at 4x 0 is used, at 8x 1 is used,
	 * etc.
	 */

	static void getHBFilter( const double ReqAtten, const int SteepIndex,
		const double*& flt, int& fltt, double& att )
	{
		static const int FltCount = 11;
		static const double HBKernel_4[ 4 ] = { // att -64.9241 dB, frac 4.0
			6.1073830069265711e-001, -1.4463982571471876e-001,
			4.1136036923118187e-002, -7.4740105856914872e-003 };
		static const double HBKernel_5[ 5 ] = { // att -87.4775 dB, frac 4.0
			6.1553054142504338e-001, -1.5591352339398118e-001,
			5.2404802661298266e-002, -1.4230574348726146e-002,
			2.2457593805377831e-003 };
		static const double HBKernel_6[ 6 ] = { // att -104.5154 dB, frac 4.0
			6.1883766561934184e-001, -1.6396282655874558e-001,
			6.1104571279325129e-002, -2.0317756445217543e-002,
			5.0264527466018826e-003, -6.9392938429507279e-004 };
		static const double HBKernel_7[ 7 ] = { // att -120.6199 dB, frac 4.0
			6.2125313688727779e-001, -1.6999763849273491e-001,
			6.8014108060738196e-002, -2.5679821316697125e-002,
			7.9798828249699784e-003, -1.7871060154498470e-003,
			2.1836606459564009e-004 };
		static const double HBKernel_8[ 8 ] = { // att -136.5151 dB, frac 4.0
			6.2309299085367287e-001, -1.7468969193368433e-001,
			7.3628746444973150e-002, -3.0378268550055314e-002,
			1.0908085227657214e-002, -3.1287343330312556e-003,
			6.3632014609722092e-004, -6.9597139145649502e-005 };
		static const double HBKernel_9[ 9 ] = { // att -152.3240 dB, frac 4.0
			6.2454069594794803e-001, -1.7844303649890664e-001,
			7.8279410808762842e-002, -3.4501119561829857e-002,
			1.3717889826645487e-002, -4.6090109007760798e-003,
			1.2192752061406873e-003, -2.2647618541786664e-004,
			2.2395554542567748e-005 };
		static const double HBKernel_10[ 10 ] = { // att -168.0859 dB, frac 4.0
			6.2570883988448611e-001, -1.8151274643053061e-001,
			8.2191863294185458e-002, -3.8131779329357615e-002,
			1.6367492549512565e-002, -6.1530178832078578e-003,
			1.9277693942420303e-003, -4.7165916432255402e-004,
			8.0491894752808465e-005, -7.2581515842465856e-006 };
		static const double HBKernel_11[ 11 ] = { // att -183.7962 dB, frac 4.0
			6.2667167706646965e-001, -1.8407153341833782e-001,
			8.5529995600327216e-002, -4.1346831452173063e-002,
			1.8844831683400131e-002, -7.7125170314919214e-003,
			2.7268674834296570e-003, -7.9745028391855826e-004,
			1.8116344571770699e-004, -2.8569149678673122e-005,
			2.3667021922861879e-006 };
		static const double HBKernel_12[ 12 ] = { // att -199.4768 dB, frac 4.0
			6.2747849729182659e-001, -1.8623616781335248e-001,
			8.8409755856508648e-002, -4.4207468780136254e-002,
			2.1149175912217915e-002, -9.2551508154301194e-003,
			3.5871562052326249e-003, -1.1923167600753576e-003,
			3.2627812001613326e-004, -6.9106902008709490e-005,
			1.0122897772888322e-005, -7.7531878091292963e-007 };
		static const double HBKernel_13[ 13 ] = { // att -215.1364 dB, frac 4.0
			6.2816416238782957e-001, -1.8809076918442266e-001,
			9.0918539368474965e-002, -4.6765502172995604e-002,
			2.3287520069933797e-002, -1.0760626940880943e-002,
			4.4853921118213676e-003, -1.6438774496992904e-003,
			5.1441308429384374e-004, -1.3211724349740752e-004,
			2.6191316362108199e-005, -3.5802424384280469e-006,
			2.5491272423372411e-007 };
		static const double HBKernel_14[ 14 ] = { // att -230.7526 dB, frac 4.0
			6.2875473147254901e-001, -1.8969942008858576e-001,
			9.3126095475258408e-002, -4.9067252227455962e-002,
			2.5273009767563311e-002, -1.2218646838258702e-002,
			5.4048946497798353e-003, -2.1409921992386689e-003,
			7.4250304371305991e-004, -2.1924546773651068e-004,
			5.3015823597863675e-005, -9.8743070771832892e-006,
			1.2650397198764347e-006, -8.4146728313072455e-008 };
		static const double FltAttens[ FltCount ] = {
			64.9241, 87.4775, 104.5154, 120.6199, 136.5151, 152.3240,
			168.0859, 183.7962, 199.4768, 215.1364, 230.7526 };
		static const double* const FltPtrs[ FltCount ] = { HBKernel_4,
			HBKernel_5, HBKernel_6, HBKernel_7, HBKernel_8, HBKernel_9,
			HBKernel_10, HBKernel_11, HBKernel_12, HBKernel_13,
			HBKernel_14 };

		static const int FltCountB = 7; // 0.125
		static const double HBKernel_2b[ 2 ] = { // StopAtten = -46.2556 dB
			5.6965643437574798e-001, -7.0243561190601822e-002 };
		static const double HBKernel_3b[ 3 ] = { // StopAtten = -93.6536 dB
			5.9040785316467748e-001, -1.0462338733801557e-001,
			1.4234900265846395e-002 };
		static const double HBKernel_4b[ 4 ] = { // StopAtten = -123.4514 dB
			6.0140277542879073e-001, -1.2564483854573050e-001,
			2.7446500598030887e-002, -3.2051079559036744e-003 };
		static const double HBKernel_5b[ 5 ] = { // StopAtten = -152.4403 dB
			6.0818642429044178e-001, -1.3981140187082763e-001,
			3.8489164053787661e-002, -7.6218861795043225e-003,
			7.5772358126258155e-004 };
		static const double HBKernel_6b[ 6 ] = { // StopAtten = -181.2501 dB
			6.1278392271870352e-001, -1.5000053763409249e-001,
			4.7575323519283508e-002, -1.2320702806281281e-002,
			2.1462442604041065e-003, -1.8425092396978648e-004 };
		static const double HBKernel_7b[ 7 ] = { // StopAtten = -209.9472 dB
			6.1610372237019151e-001, -1.5767891821295410e-001,
			5.5089690570484962e-002, -1.6895755290596615e-002,
			3.9416641999499014e-003, -6.0603620400878633e-004,
			4.5632598748568398e-005 };
		static const double HBKernel_8b[ 8 ] = { // StopAtten = -238.5612 dB
			6.1861282849648180e-001, -1.6367179296640288e-001,
			6.1369859727781417e-002, -2.1184465440918565e-002,
			5.9623352367661475e-003, -1.2483096884685629e-003,
			1.7099294398059683e-004, -1.1448310399897466e-005 };
		static const double FltAttensB[ FltCountB ] = {
			46.2556, 93.6536, 123.4514, 152.4403, 181.2501, 209.9472,
			238.5612 };
		static const double* const FltPtrsB[ FltCountB ] = { HBKernel_2b,
			HBKernel_3b, HBKernel_4b, HBKernel_5b, HBKernel_6b, HBKernel_7b,
			HBKernel_8b };

		static const int FltCountC = 5; // 0.0625
		static const double HBKernel_2c[ 2 ] = { // StopAtten = -87.9438 dB
			5.6430278013478086e-001, -6.4338068855764208e-002 };
		static const double HBKernel_3c[ 3 ] = { // StopAtten = -130.8862 dB
			5.8706402915553113e-001, -9.9362380958695873e-002,
			1.2298637065878193e-002 };
		static const double HBKernel_4c[ 4 ] = { // StopAtten = -172.3191 dB
			5.9896586135108265e-001, -1.2111680603660968e-001,
			2.4763118077755664e-002, -2.6121758134936002e-003 };
		static const double HBKernel_5c[ 5 ] = { // StopAtten = -213.4984 dB
			6.0626808278478261e-001, -1.3588224019070938e-001,
			3.5544305138258458e-002, -6.5127022013993230e-003,
			5.8255449020627736e-004 };
		static const double HBKernel_6c[ 6 ] = { // StopAtten = -254.5179 dB
			6.1120171157732273e-001, -1.4654486624691154e-001,
			4.4582957343679119e-002, -1.0840542911916273e-002,
			1.7343703931622656e-003, -1.3363015552414481e-004 };
		static const double FltAttensC[ FltCountC ] = {
			87.9438, 130.8862, 172.3191, 213.4984, 254.5179 };
		static const double* const FltPtrsC[ FltCountC ] = { HBKernel_2c,
			HBKernel_3c, HBKernel_4c, HBKernel_5c, HBKernel_6c };

		static const int FltCountD = 3; // 0.03125
		static const double HBKernel_2d[ 2 ] = { // StopAtten = -113.1456 dB
			5.6295152180538044e-001, -6.2953706070191726e-002 };
		static const double HBKernel_3d[ 3 ] = { // StopAtten = -167.1446 dB
			5.8621968728761675e-001, -9.8080551656624410e-002,
			1.1860868762030571e-002 };
		static const double HBKernel_4d[ 4 ] = { // StopAtten = -220.6519 dB
			5.9835028661892165e-001, -1.1999986095168852e-001,
			2.4132530901858028e-002, -2.4829565783680927e-003 };
		static const double FltAttensD[ FltCountD ] = {
			113.1456, 167.1446, 220.6519 };
		static const double* const FltPtrsD[ FltCountD ] = { HBKernel_2d,
			HBKernel_3d, HBKernel_4d };

		static const int FltCountE = 4; // 0.015625
		static const double HBKernel_1e[ 1 ] = { // StopAtten = -60.9962 dB
			5.0030136284718241e-001 };
		static const double HBKernel_2e[ 2 ] = { // StopAtten = -137.3130 dB
			5.6261293163934145e-001, -6.2613067826625832e-002 };
		static const double HBKernel_3e[ 3 ] = { // StopAtten = -203.2997 dB
			5.8600808139033378e-001, -9.7762185874608526e-002,
			1.1754104552667852e-002 };
		static const double HBKernel_4e[ 4 ] = { // StopAtten = -268.8561 dB
			5.9819599535791312e-001, -1.1972157884617740e-001,
			2.3977307400990484e-002, -2.4517239127622593e-003 };
		static const double FltAttensE[ FltCountE ] = {
			60.9962, 137.3130, 203.2997, 268.8561 };
		static const double* const FltPtrsE[ FltCountE ] = { HBKernel_1e,
			HBKernel_2e, HBKernel_3e, HBKernel_4e };

		static const int FltCountG = 3;
		static const double HBKernel_1g[ 1 ] = { // att -93.9165 dB, frac 256.0
			5.0001882524896712e-001 };
		static const double HBKernel_2g[ 2 ] = { // att -185.4886 dB, frac 256.0
			5.6250705922473820e-001, -6.2507059756319761e-002 };
		static const double HBKernel_3g[ 3 ] = { // att -275.5531 dB, frac 256.0
			5.8594191093025305e-001, -9.7662866644414148e-002,
			1.1720955714177778e-002 };
		static const double FltAttensG[ FltCountG ] = {
			93.9165, 185.4886, 275.5531 };
		static const double* const FltPtrsG[ FltCountG ] = { HBKernel_1g,
			HBKernel_2g, HBKernel_3g };

		int k = 0;

		if( SteepIndex <= 0 )
		{
			while( k != FltCount - 1 && FltAttens[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrs[ k ];
			fltt = 4 + k;
			att = FltAttens[ k ];
		}
		else
		if( SteepIndex == 1 )
		{
			while( k != FltCountB - 1 && FltAttensB[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsB[ k ];
			fltt = 2 + k;
			att = FltAttensB[ k ];
		}
		else
		if( SteepIndex == 2 )
		{
			while( k != FltCountC - 1 && FltAttensC[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsC[ k ];
			fltt = 2 + k;
			att = FltAttensC[ k ];
		}
		else
		if( SteepIndex == 3 )
		{
			while( k != FltCountD - 1 && FltAttensD[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsD[ k ];
			fltt = 2 + k;
			att = FltAttensD[ k ];
		}
		else
		if( SteepIndex == 4 || SteepIndex == 5 )
		{
			while( k != FltCountE - 1 && FltAttensE[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsE[ k ];
			fltt = 1 + k;
			att = FltAttensE[ k ];
		}
		else
		{
			while( k != FltCountG - 1 && FltAttensG[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsG[ k ];
			fltt = 1 + k;
			att = FltAttensG[ k ];
		}
	}

	/**
	 * Function that provides filter data for various steepness indices and
	 * attenuations. For 1/3 resamplings.
	 *
	 * @param ReqAtten Required half-band filter attentuation.
	 * @param SteepIndex Steepness index - 0=steepest. Corresponds to general
	 * upsampling/downsampling ratio, e.g. at 4x 0 is used, at 8x 1 is used,
	 * etc.
	 */

	static void getHBFilterThird( const double ReqAtten, const int SteepIndex,
		const double*& flt, int& fltt, double& att )
	{
		static const int FltCount = 7;
		static const double HBKernel_3[ 3 ] = { // att -75.0994 dB, frac 6.0
			5.9381789425210385e-001, -1.1030344037819353e-001,
			1.6601396044066741e-002 };
		static const double HBKernel_4[ 4 ] = { // att -102.5310 dB, frac 6.0
			6.0388679447131843e-001, -1.3043900369548017e-001,
			3.0518777984447295e-002, -3.9738477033171900e-003 };
		static const double HBKernel_5[ 5 ] = { // att -126.5360 dB, frac 6.0
			6.1014115058940344e-001, -1.4393081816630204e-001,
			4.1760642892854860e-002, -8.9692183234068596e-003,
			9.9871340618369218e-004 };
		static const double HBKernel_6[ 6 ] = { // att -150.1830 dB, frac 6.0
			6.1439563420561982e-001, -1.5360187826939378e-001,
			5.0840891346007507e-002, -1.4053648740740480e-002,
			2.6771286587896391e-003, -2.5815816045721899e-004 };
		static const double HBKernel_7[ 7 ] = { // att -173.7067 dB, frac 6.0
			6.1747493476475102e-001, -1.6087373733655960e-001,
			5.8263075644905349e-002, -1.8872408175697929e-002,
			4.7421376553202421e-003, -8.0196529637661137e-004,
			6.7964807425180754e-005 };
		static const double HBKernel_8[ 8 ] = { // att -197.1454 dB, frac 6.0
			6.1980610946074488e-001, -1.6654070574184196e-001,
			6.4416567396953492e-002, -2.3307744316524541e-002,
			6.9909157209589430e-003, -1.5871946236745982e-003,
			2.4017727258609085e-004, -1.8125308111373566e-005 };
		static const double HBKernel_9[ 9 ] = { // att -220.5199 dB, frac 6.0
			6.2163188987470752e-001, -1.7108115412330563e-001,
			6.9588371105224839e-002, -2.7339625869282957e-002,
			9.2954473703765472e-003, -2.5537181861669997e-003,
			5.2572296540671394e-004, -7.1813366796731157e-005,
			4.8802392556669750e-006 };
		static const double FltAttens[ FltCount ] = {
			75.0994, 102.5310, 126.5360, 150.1830, 173.7067, 197.1454,
			220.5199 };
		static const double* const FltPtrs[ FltCount ] = { HBKernel_3,
			HBKernel_4, HBKernel_5, HBKernel_6, HBKernel_7, HBKernel_8,
			HBKernel_9 };

		static const int FltCountB = 5;
		static const double HBKernel_2b[ 2 ] = { // att -75.4413 dB, frac 12.0
			5.6569875353984056e-001, -6.5811416441328888e-002 };
		static const double HBKernel_3b[ 3 ] = { // att -115.7198 dB, frac 12.0
			5.8793612182667099e-001, -1.0070583248877137e-001,
			1.2771337947163270e-002 };
		static const double HBKernel_4b[ 4 ] = { // att -152.1528 dB, frac 12.0
			5.9960155600859322e-001, -1.2228154335192955e-001,
			2.5433718917658079e-002, -2.7537562530760588e-003 };
		static const double HBKernel_5b[ 5 ] = { // att -188.2914 dB, frac 12.0
			6.0676859170270769e-001, -1.3689667009297382e-001,
			3.6288512627614941e-002, -6.7838855288962756e-003,
			6.2345167652090897e-004 };
		static const double HBKernel_6b[ 6 ] = { // att -224.2705 dB, frac 12.0
			6.1161456377889145e-001, -1.4743902036519768e-001,
			4.5344160828746795e-002, -1.1207372108402218e-002,
			1.8328498006058664e-003, -1.4518194076022933e-004 };
		static const double FltAttensB[ FltCountB ] = {
			75.4413, 115.7198, 152.1528, 188.2914, 224.2705 };
		static const double* const FltPtrsB[ FltCountB ] = { HBKernel_2b,
			HBKernel_3b, HBKernel_4b, HBKernel_5b, HBKernel_6b };

		static const int FltCountC = 4;
		static const double HBKernel_2c[ 2 ] = { // att -102.9806 dB, frac 24.0
			5.6330232648142842e-001, -6.3309247177420730e-002 };
		static const double HBKernel_3c[ 3 ] = { // att -152.1187 dB, frac 24.0
			5.8643891113575064e-001, -9.8411593011501639e-002,
			1.1972706651455891e-002 };
		static const double HBKernel_4c[ 4 ] = { // att -200.6182 dB, frac 24.0
			5.9851012364429712e-001, -1.2028885240905723e-001,
			2.4294521088349529e-002, -2.5157924167197453e-003 };
		static const double HBKernel_5c[ 5 ] = { // att -248.8728 dB, frac 24.0
			6.0590922849004858e-001, -1.3515953371903033e-001,
			3.5020856634677522e-002, -6.3256195330255094e-003,
			5.5506812768978109e-004 };
		static const double FltAttensC[ FltCountC ] = {
			102.9806, 152.1187, 200.6182, 248.8728 };
		static const double* const FltPtrsC[ FltCountC ] = { HBKernel_2c,
			HBKernel_3c, HBKernel_4c, HBKernel_5c };

		static const int FltCountD = 4;
		static const double HBKernel_1d[ 1 ] = { // att -48.6615 dB, frac 48.0
			5.0053598654836240e-001 };
		static const double HBKernel_2d[ 2 ] = { // att -127.3033 dB, frac 48.0
			5.6270074379958679e-001, -6.2701174487726163e-002 };
		static const double HBKernel_3d[ 3 ] = { // att -188.2989 dB, frac 48.0
			5.8606296210257025e-001, -9.7844644764129421e-002,
			1.1781683046197223e-002 };
		static const double HBKernel_4d[ 4 ] = { // att -248.8578 dB, frac 48.0
			5.9823601283411165e-001, -1.1979369067338455e-001,
			2.4017459011435899e-002, -2.4597811725236445e-003 };
		static const double FltAttensD[ FltCountD ] = {
			48.6615, 127.3033, 188.2989, 248.8578 };
		static const double* const FltPtrsD[ FltCountD ] = { HBKernel_1d,
			HBKernel_2d, HBKernel_3d, HBKernel_4d };

		static const int FltCountE = 3;
		static const double HBKernel_1e[ 1 ] = { // att -73.2782 dB, frac 96.0
			5.0013388897382527e-001 };
		static const double HBKernel_2e[ 2 ] = { // att -151.4076 dB, frac 96.0
			5.6255019604318290e-001, -6.2550222932385172e-002 };
		static const double HBKernel_3e[ 3 ] = { // att -224.4366 dB, frac 96.0
			5.8596887233874539e-001, -9.7703321108182931e-002,
			1.1734448775437802e-002 };
		static const double FltAttensE[ FltCountE ] = {
			73.2782, 151.4076, 224.4366 };
		static const double* const FltPtrsE[ FltCountE ] = { HBKernel_1e,
			HBKernel_2e, HBKernel_3e };

		static const int FltCountG = 3;
		static const double HBKernel_1g[ 1 ] = { // att -101.2873 dB, frac 384.0
			5.0000836666064941e-001 };
		static const double HBKernel_2g[ 2 ] = { // att -199.5761 dB, frac 384.0
			5.6250313744967606e-001, -6.2503137554676916e-002 };
		static const double HBKernel_3g[ 3 ] = { // att -296.4833 dB, frac 384.0
			5.8593945769687561e-001, -9.7659186594368730e-002,
			1.1719728897494584e-002 };
		static const double FltAttensG[ FltCountG ] = {
			101.2873, 199.5761, 296.4833 };
		static const double* const FltPtrsG[ FltCountG ] = { HBKernel_1g,
			HBKernel_2g, HBKernel_3g };

		int k = 0;

		if( SteepIndex <= 0 )
		{
			while( k != FltCount - 1 && FltAttens[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrs[ k ];
			fltt = 3 + k;
			att = FltAttens[ k ];
		}
		else
		if( SteepIndex == 1 )
		{
			while( k != FltCountB - 1 && FltAttensB[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsB[ k ];
			fltt = 2 + k;
			att = FltAttensB[ k ];
		}
		else
		if( SteepIndex == 2 )
		{
			while( k != FltCountC - 1 && FltAttensC[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsC[ k ];
			fltt = 2 + k;
			att = FltAttensC[ k ];
		}
		else
		if( SteepIndex == 3 )
		{
			while( k != FltCountD - 1 && FltAttensD[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsD[ k ];
			fltt = 1 + k;
			att = FltAttensD[ k ];
		}
		else
		if( SteepIndex == 4 || SteepIndex == 5 )
		{
			while( k != FltCountE - 1 && FltAttensE[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsE[ k ];
			fltt = 1 + k;
			att = FltAttensE[ k ];
		}
		else
		{
			while( k != FltCountG - 1 && FltAttensG[ k ] < ReqAtten )
			{
				k++;
			}

			flt = FltPtrsG[ k ];
			fltt = 1 + k;
			att = FltAttensG[ k ];
		}
	}

	/**
	 * Constructor initalizes the half-band upsampler.
	 *
	 * @param ReqAtten Required half-band filter attentuation.
	 * @param SteepIndex Steepness index - 0=steepest. Corresponds to general
	 * upsampling ratio, e.g. at 4x upsampling 0 is used, at 8x upsampling 1
	 * is used, etc.
	 * @param IsThird "True" if 1/3 resampling is performed.
	 * @param PrevLatency Latency, in samples (any value >=0), which was left
	 * in the output signal by a previous process. Whole-number latency will
	 * be consumed by *this object while remaining fractional latency can be
	 * obtained via the getLatencyFrac() function.
	 */

	CDSPHBUpsampler( const double ReqAtten, const int SteepIndex,
		const bool IsThird, const double PrevLatency )
	{
		static const CConvolveFn FltConvFn[ 14 ] = {
			&CDSPHBUpsampler :: convolve1, &CDSPHBUpsampler :: convolve2,
			&CDSPHBUpsampler :: convolve3, &CDSPHBUpsampler :: convolve4,
			&CDSPHBUpsampler :: convolve5, &CDSPHBUpsampler :: convolve6,
			&CDSPHBUpsampler :: convolve7, &CDSPHBUpsampler :: convolve8,
			&CDSPHBUpsampler :: convolve9, &CDSPHBUpsampler :: convolve10,
			&CDSPHBUpsampler :: convolve11, &CDSPHBUpsampler :: convolve12,
			&CDSPHBUpsampler :: convolve13, &CDSPHBUpsampler :: convolve14 };

		int fltt;
		double att;

		if( IsThird )
		{
			getHBFilterThird( ReqAtten, SteepIndex, fltp, fltt, att );
		}
		else
		{
			getHBFilter( ReqAtten, SteepIndex, fltp, fltt, att );
		}

		convfn = FltConvFn[ fltt - 1 ];
		fll = fltt - 1;
		fl2 = fltt;
		flo = fll + fl2;

		LatencyFrac = PrevLatency * 2.0;
		Latency = (int) LatencyFrac;
		LatencyFrac -= Latency;

		R8BCONSOLE( "CDSPHBUpsampler: sti=%i third=%i taps=%i att=%.1f "
			"io=2/1\n", SteepIndex, (int) IsThird, fltt, att );

		clear();
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

		return( MaxInLen * 2 );
	}

	virtual void clear()
	{
		LatencyLeft = Latency;
		BufLeft = 0;
		WritePos = 0;
		ReadPos = BufLen - fll; // Set "read" position to
			// account for filter's latency.

		memset( &Buf[ ReadPos ], 0, fll * sizeof( double ));
	}

	virtual int process( double* ip, int l, double*& op0 )
	{
		R8BASSERT( l >= 0 );

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

			if( BufLeft > fl2 )
			{
				const int c = BufLeft - fl2;

				double* const opend = op + c + c;
				( *convfn )( op, opend, fltp, Buf + fll, ReadPos );

				op = opend;
				BufLeft -= c;
			}
		}

		int ol = (int) ( op - op0 );

		if( LatencyLeft > 0 )
		{
			if( LatencyLeft >= ol )
			{
				LatencyLeft -= ol;
				return( 0 );
			}

			ol -= LatencyLeft;
			op0 += LatencyLeft;
			LatencyLeft = 0;
		}

		return( ol );
	}

private:
	static const int BufLenBits = 8; ///< The length of the ring buffer,
		///< expressed as Nth power of 2. This value can be reduced if it is
		///< known that only short input buffers will be passed to the
		///< interpolator. The minimum value of this parameter is 5, and
		///< 1 << BufLenBits should be at least 3 times larger than the
		///< FilterLen.
		///<
	static const int BufLen = 1 << BufLenBits; ///< The length of the ring
		///< buffer. The actual length is twice as long to allow "beyond max
		///< position" positioning.
		///<
	static const int BufLenMask = BufLen - 1; ///< Mask used for quick buffer
		///< position wrapping.
		///<
	double Buf[ BufLen + 27 ]; ///< The ring buffer, including overrun
		///< protection for the largest filter.
		///<
	const double* fltp; ///< Half-band filter taps.
		///<
	int fll; ///< Input latency.
		///<
	int fl2; ///< Right-side filter length.
		///<
	int flo; ///< Overrrun length.
		///<
	int Latency; ///< Initial latency that should be removed from the output.
		///<
	double LatencyFrac; ///< Fractional latency left on the output.
		///<
	int BufLeft; ///< The number of samples left in the buffer to process.
		///< When this value is below FilterLenD2Plus1, the interpolation
		///< cycle ends.
		///<
	int WritePos; ///< The current buffer write position. Incremented together
		///< with the BufLeft variable.
		///<
	int ReadPos; ///< The current buffer read position.
		///<
	int LatencyLeft; ///< Latency left to remove.
		///<
	typedef void( *CConvolveFn )( double* op, double* const opend,
		const double* const flt, const double* const rp0, int& ReadPos0 ); ///<
		///< Convolution funtion type.
		///<
	CConvolveFn convfn; ///< Convolution function in use.
		///<

#define R8BHBC1( fn ) \
	static void fn( double* op, double* const opend, const double* const flt, \
		const double* const rp0, int& ReadPos0 ) \
	{ \
		int rpos = ReadPos0; \
		while( op < opend ) \
		{ \
			const double* const rp = rp0 + rpos; \
			op[ 0 ] = rp[ 0 ]; \
			op[ 1 ] =

#define R8BHBC2 \
			rpos = ( rpos + 1 ) & BufLenMask; \
			op += 2; \
		} \
		ReadPos0 = rpos; \
	}

	R8BHBC1( convolve1 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]);
	R8BHBC2

	R8BHBC1( convolve2 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]);
	R8BHBC2

	R8BHBC1( convolve3 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]);
	R8BHBC2

	R8BHBC1( convolve4 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]);
	R8BHBC2

	R8BHBC1( convolve5 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]);
	R8BHBC2

	R8BHBC1( convolve6 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]);
	R8BHBC2

	R8BHBC1( convolve7 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]);
	R8BHBC2

	R8BHBC1( convolve8 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]) +
				flt[ 7 ] * ( rp[ 8 ] + rp[ -7 ]);
	R8BHBC2

	R8BHBC1( convolve9 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]) +
				flt[ 7 ] * ( rp[ 8 ] + rp[ -7 ]) +
				flt[ 8 ] * ( rp[ 9 ] + rp[ -8 ]);
	R8BHBC2

	R8BHBC1( convolve10 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]) +
				flt[ 7 ] * ( rp[ 8 ] + rp[ -7 ]) +
				flt[ 8 ] * ( rp[ 9 ] + rp[ -8 ]) +
				flt[ 9 ] * ( rp[ 10 ] + rp[ -9 ]);
	R8BHBC2

	R8BHBC1( convolve11 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]) +
				flt[ 7 ] * ( rp[ 8 ] + rp[ -7 ]) +
				flt[ 8 ] * ( rp[ 9 ] + rp[ -8 ]) +
				flt[ 9 ] * ( rp[ 10 ] + rp[ -9 ]) +
				flt[ 10 ] * ( rp[ 11 ] + rp[ -10 ]);
	R8BHBC2

	R8BHBC1( convolve12 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]) +
				flt[ 7 ] * ( rp[ 8 ] + rp[ -7 ]) +
				flt[ 8 ] * ( rp[ 9 ] + rp[ -8 ]) +
				flt[ 9 ] * ( rp[ 10 ] + rp[ -9 ]) +
				flt[ 10 ] * ( rp[ 11 ] + rp[ -10 ]) +
				flt[ 11 ] * ( rp[ 12 ] + rp[ -11 ]);
	R8BHBC2

	R8BHBC1( convolve13 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]) +
				flt[ 7 ] * ( rp[ 8 ] + rp[ -7 ]) +
				flt[ 8 ] * ( rp[ 9 ] + rp[ -8 ]) +
				flt[ 9 ] * ( rp[ 10 ] + rp[ -9 ]) +
				flt[ 10 ] * ( rp[ 11 ] + rp[ -10 ]) +
				flt[ 11 ] * ( rp[ 12 ] + rp[ -11 ]) +
				flt[ 12 ] * ( rp[ 13 ] + rp[ -12 ]);
	R8BHBC2

	R8BHBC1( convolve14 )
				flt[ 0 ] * ( rp[ 1 ] + rp[ 0 ]) +
				flt[ 1 ] * ( rp[ 2 ] + rp[ -1 ]) +
				flt[ 2 ] * ( rp[ 3 ] + rp[ -2 ]) +
				flt[ 3 ] * ( rp[ 4 ] + rp[ -3 ]) +
				flt[ 4 ] * ( rp[ 5 ] + rp[ -4 ]) +
				flt[ 5 ] * ( rp[ 6 ] + rp[ -5 ]) +
				flt[ 6 ] * ( rp[ 7 ] + rp[ -6 ]) +
				flt[ 7 ] * ( rp[ 8 ] + rp[ -7 ]) +
				flt[ 8 ] * ( rp[ 9 ] + rp[ -8 ]) +
				flt[ 9 ] * ( rp[ 10 ] + rp[ -9 ]) +
				flt[ 10 ] * ( rp[ 11 ] + rp[ -10 ]) +
				flt[ 11 ] * ( rp[ 12 ] + rp[ -11 ]) +
				flt[ 12 ] * ( rp[ 13 ] + rp[ -12 ]) +
				flt[ 13 ] * ( rp[ 14 ] + rp[ -13 ]);
	R8BHBC2

#undef R8BHBC1
#undef R8BHBC2
};

// ---------------------------------------------------------------------------

} // namespace r8b

#endif // R8B_CDSPHBUPSAMPLER_INCLUDED
