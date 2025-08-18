
#include "Precomp.h"
#include "FileResource.h"

// I probably should find a less brain dead way of doing this. :)

std::string FileResource::readAllText(const std::string& filename)
{
	if (filename == "shaders/Scene.vert")
	{
		return R"(
			layout(push_constant) uniform ScenePushConstants
			{
				mat4 objectToProjection;
				vec4 nearClip;
				uint uHitIndex;
				uint padding1, padding2, padding3;
			};

			layout(location = 0) in uint aFlags;
			layout(location = 1) in vec3 aPosition;
			layout(location = 2) in vec2 aTexCoord;
			layout(location = 3) in vec2 aTexCoord2;
			layout(location = 4) in vec2 aTexCoord3;
			layout(location = 5) in vec2 aTexCoord4;
			layout(location = 6) in vec4 aColor;
			layout(location = 7) in ivec4 aTextureBinds;

			layout(location = 0) flat out uint flags;
			layout(location = 1) out vec2 texCoord;
			layout(location = 2) out vec2 texCoord2;
			layout(location = 3) out vec2 texCoord3;
			layout(location = 4) out vec2 texCoord4;
			layout(location = 5) out vec4 color;
			layout(location = 6) flat out uint hitIndex;
			layout(location = 7) flat out ivec4 textureBinds;

			void main()
			{
				gl_Position = objectToProjection * vec4(aPosition, 1.0);
				gl_ClipDistance[0] = dot(nearClip, vec4(aPosition, 1.0));
				flags = aFlags;
				texCoord = aTexCoord;
				texCoord2 = aTexCoord2;
				texCoord3 = aTexCoord3;
				texCoord4 = aTexCoord4;
				color = aColor;
				hitIndex = uHitIndex;
				textureBinds = aTextureBinds;
			}
		)";
	}
	else if (filename == "shaders/Scene.frag")
	{
		return R"(
			layout(binding = 0) uniform sampler2D textures[];

			layout(location = 0) flat in uint flags;
			layout(location = 1) centroid in vec2 texCoord;
			layout(location = 2) in vec2 texCoord2;
			layout(location = 3) in vec2 texCoord3;
			layout(location = 4) in vec2 texCoord4;
			layout(location = 5) in vec4 color;
			layout(location = 6) flat in uint hitIndex;
			layout(location = 7) flat in ivec4 textureBinds;

			layout(location = 0) out vec4 outColor;
			layout(location = 1) out uint outHitIndex;

			vec4 darkClamp(vec4 c)
			{
				// Make all textures a little darker as some of the textures (i.e coronas) never become completely black as they should have
				float cutoff = 3.1/255.0;
				return vec4(clamp((c.rgb - cutoff) / (1.0 - cutoff), 0.0, 1.0), c.a);
			}

			vec4 textureTex(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.x)], uv); }
			vec4 textureMacro(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.y)], uv); }
			vec4 textureDetail(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.z)], uv); }
			vec4 textureLightmap(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.w)], uv); }

			void main()
			{
				float actorXBlending = (flags & 32) != 0 ? 1.5 : 1.0;
				float oneXBlending = (flags & 64) != 0 ? 1.0 : 2.0;

				outColor = darkClamp(textureTex(texCoord)) * color;
				outColor.rgb *= actorXBlending;

				if ((flags & 2) != 0) // Macro texture
				{
					outColor *= darkClamp(textureMacro(texCoord3));
				}

				if ((flags & 1) != 0) // Lightmap
				{
					outColor.rgb *= clamp(textureLightmap(texCoord2).rgb, 0.0, 1.0) * oneXBlending;
				}

				if ((flags & 4) != 0) // Detail texture
				{
					float fadedistance = 380.0f;
					float a = clamp(2.0f - (1.0f / gl_FragCoord.w) / fadedistance, 0.0f, 1.0f);
					vec4 detailColor = (textureDetail(texCoord4) - 0.5) * 0.8 + 1.0;
					outColor.rgb = mix(outColor.rgb, outColor.rgb * detailColor.rgb, a);
				}
				else if ((flags & 8) != 0) // Fog map
				{
					vec4 fogcolor = textureDetail(texCoord4);
					outColor.rgb = fogcolor.rgb + outColor.rgb * (1.0 - fogcolor.a);
				}
				else if ((flags & 16) != 0) // Fog color
				{
					vec4 fogcolor = vec4(texCoord2, texCoord3);
					outColor.rgb = fogcolor.rgb + outColor.rgb * (1.0 - fogcolor.a);
				}

				#if defined(ALPHATEST)
				if (outColor.a < 0.5) discard;
				#endif

				outColor = clamp(outColor, 0.0, 1.0);

				outHitIndex = hitIndex;
			}
		)";
	}
	else if (filename == "shaders/PPStep.vert")
	{
		return R"(
			layout(location = 0) out vec2 texCoord;

			vec2 positions[6] = vec2[](
				vec2(-1.0, -1.0),
				vec2( 1.0, -1.0),
				vec2(-1.0,  1.0),
				vec2(-1.0,  1.0),
				vec2( 1.0, -1.0),
				vec2( 1.0,  1.0)
			);

			void main()
			{
				vec4 pos = vec4(positions[gl_VertexIndex], 0.0, 1.0);
				gl_Position = pos;
				texCoord = pos.xy * 0.5 + 0.5;
			}
		)";
	}
	else if (filename == "shaders/Present.frag")
	{
		return R"(
			layout(push_constant) uniform PresentPushConstants
			{
				float Contrast;
				float Saturation;
				float Brightness;
				float HdrScale;
				vec4 GammaCorrection;
			};

			layout(binding = 0) uniform sampler2D texSampler;
			layout(binding = 1) uniform sampler2D texDither;
			layout(location = 0) in vec2 texCoord;
			layout(location = 0) out vec4 outColor;

			vec3 dither(vec3 c)
			{
				vec2 texSize = vec2(textureSize(texDither, 0));
				float threshold = texture(texDither, gl_FragCoord.xy / texSize).r;
				return floor(c.rgb * 255.0 + threshold) / 255.0;
			}

			vec3 linearHdr(vec3 c)
			{
				return pow(c, vec3(2.2)) * HdrScale;
			}

			#if defined(GAMMA_MODE_D3D9)

			vec3 gammaCorrect(vec3 c)
			{
				return pow(c, GammaCorrection.xyz);
			}

			#elif defined(GAMMA_MODE_XOPENGL)

			// Returns maximum of first 3 components
			float max3(vec3 v)
			{
				return max(max(v.x, v.y), v.z);
			}
			float max3(vec4 v)
			{
				return max(max(v.x, v.y), v.z);
			}

			// Returns square of argument
			float square_f( float f)
			{
				return f*f;
			}

			vec3 gammaCorrect(vec3 c)
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
			vec3 colorCorrect(vec3 c)
			{
				float v = c.r + c.g + c.b;
				vec3 valgray = vec3(v, v, v) * (1 - Saturation) / 3 + c * Saturation;
				vec3 val = valgray * Contrast - (Contrast - 1.0) * 0.5;
				val += Brightness * 0.5;
				return max(val, vec3(0.0, 0.0, 0.0));
			}
			#elif defined(COLOR_CORRECT_MODE1)
			vec3 colorCorrect(vec3 c)
			{
				float v = dot(c, vec3(0.3, 0.56, 0.14));
				vec3 valgray = mix(vec3(v, v, v), c, Saturation);
				vec3 val = valgray * Contrast - (Contrast - 1.0) * 0.5;
				val += Brightness * 0.5;
				return max(val, vec3(0.0, 0.0, 0.0));
			}
			#elif defined(COLOR_CORRECT_MODE2)
			vec3 colorCorrect(vec3 c)
			{
				float v = pow(dot(pow(c, vec3(2.2, 2.2, 2.2)), vec3(0.2126, 0.7152, 0.0722)), 1.0/2.2);
				vec3 valgray = mix(vec3(v, v, v), c, Saturation);
				vec3 val = valgray * Contrast - (Contrast - 1.0) * 0.5;
				val += Brightness * 0.5;
				return max(val, vec3(0.0, 0.0, 0.0));
			}
			#else
			vec3 colorCorrect(vec3 c) { return c; }
			#endif

			void main()
			{
				vec3 color = gammaCorrect(colorCorrect(texture(texSampler, texCoord).rgb));
			#if defined(HDR_MODE)
				outColor = vec4(linearHdr(color), 1.0f);
			#else
				outColor = vec4(dither(color), 1.0f);
			#endif
			}
		)";
	}
	else if (filename == "shaders/BloomExtract.frag")
	{
		return R"(
			layout(push_constant) uniform BloomPushConstants
			{
				float SampleWeights0;
				float SampleWeights1;
				float SampleWeights2;
				float SampleWeights3;
				float SampleWeights4;
				float SampleWeights5;
				float SampleWeights6;
				float SampleWeights7;
			};

			layout(binding = 0) uniform sampler2D texSampler;
			layout(location = 0) in vec2 texCoord;
			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = vec4(max(texture(texSampler, texCoord).rgb - 1.0, 0.0), 0.0);
			}
		)";
	}
	else if (filename == "shaders/BloomCombine.frag")
	{
		return R"(
			layout(push_constant) uniform BloomPushConstants
			{
				float SampleWeights0;
				float SampleWeights1;
				float SampleWeights2;
				float SampleWeights3;
				float SampleWeights4;
				float SampleWeights5;
				float SampleWeights6;
				float SampleWeights7;
			};

			layout(binding = 0) uniform sampler2D texSampler;
			layout(location = 0) in vec2 texCoord;
			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = texture(texSampler, texCoord);
			}
		)";
	}
	else if (filename == "shaders/Blur.frag")
	{
		return R"(
			layout(push_constant) uniform BloomPushConstants
			{
				float SampleWeights0;
				float SampleWeights1;
				float SampleWeights2;
				float SampleWeights3;
				float SampleWeights4;
				float SampleWeights5;
				float SampleWeights6;
				float SampleWeights7;
			};

			layout(binding = 0) uniform sampler2D texSampler;
			layout(location = 0) in vec2 texCoord;
			layout(location = 0) out vec4 outColor;

			void main()
			{
			#if defined(BLUR_HORIZONTAL)
				outColor =
					textureOffset(texSampler, texCoord, ivec2( 0, 0)) * SampleWeights0 +
					textureOffset(texSampler, texCoord, ivec2( 1, 0)) * SampleWeights1 +
					textureOffset(texSampler, texCoord, ivec2(-1, 0)) * SampleWeights2 +
					textureOffset(texSampler, texCoord, ivec2( 2, 0)) * SampleWeights3 +
					textureOffset(texSampler, texCoord, ivec2(-2, 0)) * SampleWeights4 +
					textureOffset(texSampler, texCoord, ivec2( 3, 0)) * SampleWeights5 +
					textureOffset(texSampler, texCoord, ivec2(-3, 0)) * SampleWeights6;
			#else
				outColor =
					textureOffset(texSampler, texCoord, ivec2(0, 0)) * SampleWeights0 +
					textureOffset(texSampler, texCoord, ivec2(0, 1)) * SampleWeights1 +
					textureOffset(texSampler, texCoord, ivec2(0,-1)) * SampleWeights2 +
					textureOffset(texSampler, texCoord, ivec2(0, 2)) * SampleWeights3 +
					textureOffset(texSampler, texCoord, ivec2(0,-2)) * SampleWeights4 +
					textureOffset(texSampler, texCoord, ivec2(0, 3)) * SampleWeights5 +
					textureOffset(texSampler, texCoord, ivec2(0,-3)) * SampleWeights6;
			#endif
			}
		)";
	}

	return {};
}
