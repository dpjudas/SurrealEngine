
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
			};

			layout(location = 0) in uint aFlags;
			layout(location = 1) in vec3 aPosition;
			layout(location = 2) in vec2 aTexCoord;
			layout(location = 3) in vec2 aTexCoord2;
			layout(location = 4) in vec2 aTexCoord3;
			layout(location = 5) in vec2 aTexCoord4;
			layout(location = 6) in vec4 aColor;
			#if defined(BINDLESS_TEXTURES)
			layout(location = 7) in ivec4 aTextureBinds;
			#endif

			layout(location = 0) flat out uint flags;
			layout(location = 1) out vec2 texCoord;
			layout(location = 2) out vec2 texCoord2;
			layout(location = 3) out vec2 texCoord3;
			layout(location = 4) out vec2 texCoord4;
			layout(location = 5) out vec4 color;
			#if defined(BINDLESS_TEXTURES)
			layout(location = 6) flat out ivec4 textureBinds;
			#endif

			void main()
			{
				gl_Position = objectToProjection * vec4(aPosition, 1.0);
				flags = aFlags;
				texCoord = aTexCoord;
				texCoord2 = aTexCoord2;
				texCoord3 = aTexCoord3;
				texCoord4 = aTexCoord4;
				color = aColor;
				#if defined(BINDLESS_TEXTURES)
				textureBinds = aTextureBinds;
				#endif
			}
		)";
	}
	else if (filename == "shaders/Scene.frag")
	{
		return R"(
			#if defined(BINDLESS_TEXTURES)
			layout(binding = 0) uniform sampler2D textures[];
			#else
			layout(binding = 0) uniform sampler2D tex;
			layout(binding = 1) uniform sampler2D texLightmap;
			layout(binding = 2) uniform sampler2D texMacro;
			layout(binding = 3) uniform sampler2D texDetail;
			#endif

			layout(location = 0) flat in uint flags;
			layout(location = 1) centroid in vec2 texCoord;
			layout(location = 2) in vec2 texCoord2;
			layout(location = 3) in vec2 texCoord3;
			layout(location = 4) in vec2 texCoord4;
			layout(location = 5) in vec4 color;
			#if defined(BINDLESS_TEXTURES)
			layout(location = 6) flat in ivec4 textureBinds;
			#endif

			layout(location = 0) out vec4 outColor;

			vec3 linear(vec3 c)
			{
				return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(c, vec3(0.04045)));
			}

			vec4 darkClamp(vec4 c)
			{
				// Make all textures a little darker as some of the textures (i.e coronas) never become completely black as they should have
				float cutoff = 3.1/255.0;
				return vec4(clamp((c.rgb - cutoff) / (1.0 - cutoff), 0.0, 1.0), c.a);
			}

			#if defined(BINDLESS_TEXTURES)
			vec4 textureTex(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.x)], uv); }
			vec4 textureMacro(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.y)], uv); }
			vec4 textureDetail(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.z)], uv); }
			vec4 textureLightmap(vec2 uv) { return texture(textures[nonuniformEXT(textureBinds.w)], uv); }
			#else
			vec4 textureTex(vec2 uv) { return texture(tex, uv); }
			vec4 textureMacro(vec2 uv) { return texture(texMacro, uv); }
			vec4 textureLightmap(vec2 uv) { return texture(texLightmap, uv); }
			vec4 textureDetail(vec2 uv) { return texture(texDetail, uv); }
			#endif

			void main()
			{
				float actorXBlending = (flags & 32) != 0 ? 1.5 : 1.0;
				float oneXBlending = (flags & 64) != 0 ? 1.0 : 2.0;

				outColor = darkClamp(textureTex(texCoord)) * darkClamp(color) * actorXBlending;

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
				float InvGamma;
				float padding1;
				float padding2;
				float padding3;
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

			vec3 srgb(vec3 c)
			{
				return mix(c * 12.92, 1.055 * pow(c, vec3(1.0/2.4)) - 0.055, step(c, vec3(0.0031308)));
			}

			void main()
			{
				outColor = vec4(dither(pow(texture(texSampler, texCoord).rgb, vec3(InvGamma))), 1.0f);
			}
		)";
	}

	return {};
}
