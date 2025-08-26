
#include "Precomp.h"
#include "D3D11FileResource.h"

// I probably should find a less brain dead way of doing this. :)

std::string D3D11FileResource::readAllText(const std::string& filename)
{
	if (filename == "shaders/Scene.vert")
	{
		return R"(
			struct Input
			{
				uint Flags : AttrFlags;
				float3 Position : AttrPos;
				float2 TexCoord : AttrTexCoordOne;
				float2 TexCoord2 : AttrTexCoordTwo;
				float2 TexCoord3 : AttrTexCoordThree;
				float2 TexCoord4 : AttrTexCoordFour;
				float4 Color : AttrColor;
			};

			struct Output
			{
				float4 pos : SV_Position;
				uint flags : PixelFlags;
				float2 texCoord : PixelTexCoordOne;
				float2 texCoord2 : PixelTexCoordTwo;
				float2 texCoord3 : PixelTexCoordThree;
				float2 texCoord4 : PixelTexCoordFour;
				float4 color : PixelColor;
				uint hitIndex : PixelHitIndex;
				float nearClipDistance : SV_ClipDistance0;
			};

			cbuffer Uniforms
			{
				float4x4 ObjectToProjection;
				float4x4 ObjectToView;
				float4 NearClip;
				uint HitIndex;
				uint Padding1, Padding2, Padding3;
			}

			Output main(Input input)
			{
				Output output;
				output.pos = mul(ObjectToProjection, float4(input.Position, 1.0));
				output.nearClipDistance = dot(NearClip, mul(ObjectToView, float4(input.Position, 1.0)));
				output.flags = input.Flags;
				output.texCoord = input.TexCoord;
				output.texCoord2 = input.TexCoord2;
				output.texCoord3 = input.TexCoord3;
				output.texCoord4 = input.TexCoord4;
				output.color = input.Color;
				output.hitIndex = HitIndex;
				return output;
			}
		)";
	}
	else if (filename == "shaders/Scene.frag")
	{
		return R"(
			struct Input
			{
				float4 pos : SV_Position;
				uint flags : PixelFlags;
				float2 texCoord : PixelTexCoordOne;
				float2 texCoord2 : PixelTexCoordTwo;
				float2 texCoord3 : PixelTexCoordThree;
				float2 texCoord4 : PixelTexCoordFour;
				float4 color : PixelColor;
				uint hitIndex : PixelHitIndex;
			};

			struct Output
			{
				float4 outColor : SV_Target0;
				uint outHitIndex : SV_Target1;
			};

			SamplerState samplerTex;
			SamplerState samplerTexLightmap;
			SamplerState samplerTexMacro;
			SamplerState samplerTexDetail;

			Texture2D tex;
			Texture2D texLightmap;
			Texture2D texMacro;
			Texture2D texDetail;

			float4 darkClamp(float4 c)
			{
				// Make all textures a little darker as some of the textures (i.e coronas) never become completely black as they should have
				float cutoff = 3.1/255.0;
				return float4(clamp((c.rgb - cutoff) / (1.0 - cutoff), 0.0, 1.0), c.a);
			}

			float4 textureTex(float2 uv) { return tex.Sample(samplerTex, uv); }
			float4 textureMacro(float2 uv) { return texMacro.Sample(samplerTexMacro, uv); }
			float4 textureLightmap(float2 uv) { return texLightmap.Sample(samplerTexLightmap, uv); }
			float4 textureDetail(float2 uv) { return texDetail.Sample(samplerTexDetail, uv); }

			Output main(Input input)
			{
				Output output;

				float actorXBlending = (input.flags & 32) != 0 ? 1.5 : 1.0;
				float oneXBlending = (input.flags & 64) != 0 ? 1.0 : 2.0;

				output.outColor = darkClamp(textureTex(input.texCoord)) * input.color;
				output.outColor.rgb *= actorXBlending;

				if ((input.flags & 2) != 0) // Macro texture
				{
					output.outColor *= darkClamp(textureMacro(input.texCoord3));
				}

				if ((input.flags & 1) != 0) // Lightmap
				{
					output.outColor.rgb *= textureLightmap(input.texCoord2).rgb * oneXBlending;
				}

				if ((input.flags & 4) != 0) // Detail texture
				{
					float fadedistance = 380.0f;
					float a = clamp(2.0f - (1.0f / input.pos.w) / fadedistance, 0.0f, 1.0f);
					float4 detailColor = (textureDetail(input.texCoord4) - 0.5) * 0.5 + 1.0;
					output.outColor.rgb = lerp(output.outColor.rgb, output.outColor.rgb * detailColor.rgb, a);
				}
				else if ((input.flags & 8) != 0) // Fog map
				{
					float4 fogcolor = textureDetail(input.texCoord4);
					output.outColor.rgb = fogcolor.rgb + output.outColor.rgb * (1.0 - fogcolor.a);
				}
				else if ((input.flags & 16) != 0) // Fog color
				{
					float4 fogcolor = float4(input.texCoord2, input.texCoord3);
					output.outColor.rgb = fogcolor.rgb + output.outColor.rgb * (1.0 - fogcolor.a);
				}

				#if defined(ALPHATEST)
				if (output.outColor.a < 0.5) discard;
				#endif

				output.outColor = clamp(output.outColor, 0.0, 1.0);

				output.outHitIndex = input.hitIndex;
				return output;
			}
		)";
	}
	else if (filename == "shaders/PPStep.vert")
	{
		return R"(
			struct Input
			{
				float2 pos : AttrPos;
			};

			struct Output
			{
				float4 pos : SV_Position;
				float2 texCoord : PixelTexCoord;
			};

			Output main(Input input)
			{
				Output output;
				output.pos = float4(input.pos, 0.0, 1.0);
				output.texCoord = input.pos * 0.5 + 0.5;
				return output;
			}
		)";
	}
	else if (filename == "shaders/Present.frag")
	{
		return R"(
			struct Input
			{
				float4 fragCoord : SV_Position;
				float2 texCoord : PixelTexCoord;
			};

			struct Output
			{
				float4 outColor : SV_Target;
			};

			cbuffer PresentPushConstants
			{
				float Contrast;
				float Saturation;
				float Brightness;
				float HdrScale;
				float4 GammaCorrection;
			}

			SamplerState samplerTex
			{
				Filter = MIN_MAG_MIP_LINEAR;
				AddressU = Clamp;
				AddressV = Clamp;
			};

			SamplerState samplerDither
			{
				Filter = MIN_MAG_MIP_POINT;
				AddressU = Wrap;
				AddressV = Wrap;
			};

			Texture2D tex;
			Texture2D texDither;

			float3 dither(float3 c, float4 FragCoord)
			{
				float2 texSize;
				texDither.GetDimensions(texSize.x, texSize.y);
				float threshold = texDither.Sample(samplerDither, FragCoord.xy / texSize).r;
				return floor(c.rgb * 255.0 + threshold) / 255.0;
			}

			float3 linearHdr(float3 c)
			{
				return pow(c, float3(2.2, 2.2, 2.2)) * HdrScale;
			}

			#if defined(GAMMA_MODE_D3D9)

			float3 gammaCorrect(float3 c)
			{
				return pow(c, GammaCorrection.xyz);
			}

			#elif defined(GAMMA_MODE_XOPENGL)

			// Returns maximum of first 3 components
			float max3( float3 v)
			{
				return max(max(v.x, v.y), v.z);
			}
			float max3( float4 v)
			{
				return max(max(v.x, v.y), v.z);
			}

			// Returns square of argument
			float square_f( float f)
			{
				return f*f;
			}

			float3 gammaCorrect(float3 c)
			{
				c = clamp(c, 0.0, 1.0); // XOpenGLDrv doesn't use a half-float scene buffer

				if (GammaCorrection.w > 1.0)
				{
					// Obtains a multiplier required to offset value according to the following
					// formula: ((1 - (2 * value - 1)^2) * 0.25)
					// It has the shape of a parabola with roots in 0,1 and maximum at f(x=0.5)=0.25
					float CCValue = max(max3(c), 0.001);
					float CC = (1.0 - square_f(2.0 * CCValue - 1.0)) * 0.25  * (GammaCorrection.w - 1.0);
					c = clamp( c * ((CCValue+CC) / CCValue), 0.0, 1.0);
				}
				else if (GammaCorrection.w < 1.0)
				{
					// Downscale brightness
					c *= GammaCorrection.w;
				}

				return pow(c, GammaCorrection.xyz);
			}

			#endif

			#if defined(COLOR_CORRECT_MODE0)
			float3 colorCorrect(float3 c)
			{
				float v = c.r + c.g + c.b;
				float3 valgray = float3(v, v, v) * (1 - Saturation) / 3 + c * Saturation;
				float3 val = valgray * Contrast - (Contrast - 1.0) * 0.5;
				val += Brightness * 0.5;
				return max(val, float3(0.0, 0.0, 0.0));
			}
			#elif defined(COLOR_CORRECT_MODE1)
			float3 colorCorrect(float3 c)
			{
				float v = dot(c, float3(0.3, 0.56, 0.14));
				float3 valgray = lerp(float3(v, v, v), c, Saturation);
				float3 val = valgray * Contrast - (Contrast - 1.0) * 0.5;
				val += Brightness * 0.5;
				return max(val, float3(0.0, 0.0, 0.0));
			}
			#elif defined(COLOR_CORRECT_MODE2)
			float3 colorCorrect(float3 c)
			{
				float v = pow(dot(pow(c, float3(2.2, 2.2, 2.2)), float3(0.2126, 0.7152, 0.0722)), 1.0/2.2);
				float3 valgray = lerp(float3(v, v, v), c, Saturation);
				float3 val = valgray * Contrast - (Contrast - 1.0) * 0.5;
				val += Brightness * 0.5;
				return max(val, float3(0.0, 0.0, 0.0));
			}
			#else
			float3 colorCorrect(float3 c) { return c; }
			#endif

			Output main(Input input)
			{
				Output output;
				float3 color = gammaCorrect(colorCorrect(tex.Sample(samplerTex, input.texCoord).rgb));
			#if defined(HDR_MODE)
				output.outColor = float4(linearHdr(color), 1.0f);
			#else
				output.outColor = float4(dither(color, input.fragCoord), 1.0f);
			#endif
				return output;
			}
		)";
	}
	else if (filename == "shaders/HitResolve.frag")
	{
		return R"(
			struct Input
			{
				float4 fragCoord : SV_Position;
				float2 texCoord : PixelTexCoord;
			};

			struct Output
			{
				uint outHitIndex : SV_Target;
			};

			Texture2DMS<uint> tex;

			Output main(Input input)
			{
				Output output;
				output.outHitIndex = tex.Load(int2(input.fragCoord.xy), 0);
				return output;
			}
		)";
	}
	else if (filename == "shaders/BloomExtract.frag")
	{
		return R"(
			struct Input
			{
				float4 fragCoord : SV_Position;
				float2 texCoord : PixelTexCoord;
			};

			struct Output
			{
				float4 outColor : SV_Target;
			};

			SamplerState samplerTex
			{
				Filter = MIN_MAG_MIP_LINEAR;
				AddressU = Clamp;
				AddressV = Clamp;
			};

			Texture2D tex;

			Output main(Input input)
			{
				Output output;
				output.outColor = float4(max(tex.Sample(samplerTex, input.texCoord).rgb - 1.0, 0.0), 0.0);
				return output;
			}
		)";
	}
	else if (filename == "shaders/BloomCombine.frag")
	{
		return R"(
			struct Input
			{
				float4 fragCoord : SV_Position;
				float2 texCoord : PixelTexCoord;
			};

			struct Output
			{
				float4 outColor : SV_Target;
			};

			SamplerState samplerTex
			{
				Filter = MIN_MAG_MIP_LINEAR;
				AddressU = Clamp;
				AddressV = Clamp;
			};

			Texture2D tex;

			Output main(Input input)
			{
				Output output;
				output.outColor = tex.Sample(samplerTex, input.texCoord);
				return output;
			}
		)";
	}
	else if (filename == "shaders/Blur.frag")
	{
		return R"(
			struct Input
			{
				float4 fragCoord : SV_Position;
				float2 texCoord : PixelTexCoord;
			};

			struct Output
			{
				float4 outColor : SV_Target;
			};

			cbuffer BloomPushConstants
			{
				float SampleWeights0;
				float SampleWeights1;
				float SampleWeights2;
				float SampleWeights3;
				float SampleWeights4;
				float SampleWeights5;
				float SampleWeights6;
				float SampleWeights7;
			}

			SamplerState samplerTex
			{
				Filter = MIN_MAG_MIP_LINEAR;
				AddressU = Clamp;
				AddressV = Clamp;
			};

			Texture2D tex;

			Output main(Input input)
			{
				Output output;
			#if defined(BLUR_HORIZONTAL)
				output.outColor =
					tex.Sample(samplerTex, input.texCoord, int2( 0, 0)) * SampleWeights0 +
					tex.Sample(samplerTex, input.texCoord, int2( 1, 0)) * SampleWeights1 +
					tex.Sample(samplerTex, input.texCoord, int2(-1, 0)) * SampleWeights2 +
					tex.Sample(samplerTex, input.texCoord, int2( 2, 0)) * SampleWeights3 +
					tex.Sample(samplerTex, input.texCoord, int2(-2, 0)) * SampleWeights4 +
					tex.Sample(samplerTex, input.texCoord, int2( 3, 0)) * SampleWeights5 +
					tex.Sample(samplerTex, input.texCoord, int2(-3, 0)) * SampleWeights6;
			#else
				output.outColor =
					tex.Sample(samplerTex, input.texCoord, int2(0, 0)) * SampleWeights0 +
					tex.Sample(samplerTex, input.texCoord, int2(0, 1)) * SampleWeights1 +
					tex.Sample(samplerTex, input.texCoord, int2(0,-1)) * SampleWeights2 +
					tex.Sample(samplerTex, input.texCoord, int2(0, 2)) * SampleWeights3 +
					tex.Sample(samplerTex, input.texCoord, int2(0,-2)) * SampleWeights4 +
					tex.Sample(samplerTex, input.texCoord, int2(0, 3)) * SampleWeights5 +
					tex.Sample(samplerTex, input.texCoord, int2(0,-3)) * SampleWeights6;
			#endif
				return output;
			}
		)";
	}

	return {};
}
