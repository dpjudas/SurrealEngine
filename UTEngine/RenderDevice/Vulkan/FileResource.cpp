
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

			layout(location = 0) flat out uint flags;
			layout(location = 1) out vec2 texCoord;
			layout(location = 2) out vec2 texCoord2;
			layout(location = 3) out vec2 texCoord3;
			layout(location = 4) out vec2 texCoord4;
			layout(location = 5) out vec4 color;

			void main()
			{
				if ((aFlags & 16) == 0)
					gl_Position = objectToProjection * vec4(aPosition, 1.0);
				else
					gl_Position = vec4(aPosition, 1.0);
				flags = aFlags;
				texCoord = aTexCoord;
				texCoord2 = aTexCoord2;
				texCoord3 = aTexCoord3;
				texCoord4 = aTexCoord4;
				color = aColor;
			}
		)";
	}
	else if (filename == "shaders/Scene.frag")
	{
		return R"(
			struct Light
			{
				vec3 location;
				float unused;
				float brightness;
				float hue;
				float saturation;
				float radius;
			};

			layout(binding = 0) uniform sampler2D tex;
			layout(binding = 1) uniform sampler2D texLightmap;
			layout(binding = 2) uniform sampler2D texMacro;
			layout(binding = 3) uniform sampler2D texDetail;

			layout(set = 1, binding = 0, std140) buffer LightBuffer
			{
				Light lights[];
			};
			layout(set = 1, binding = 1, std140) buffer SurfaceLights
			{
				int surfaceLights[];
			};
			layout(set = 1, binding = 2) uniform accelerationStructureEXT TopLevelAS;

			layout(location = 0) flat in uint flags;
			layout(location = 1) in vec2 texCoord;
			layout(location = 2) in vec2 texCoord2;
			layout(location = 3) in vec2 texCoord3;
			layout(location = 4) in vec2 texCoord4;
			layout(location = 5) in vec4 color;

			layout(location = 0) out vec4 outColor;

			vec3 linear(vec3 c)
			{
				return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(c, vec3(0.04045)));
			}

			void main()
			{
				outColor = texture(tex, texCoord);
				//outColor.rgb = linear(outColor.rgb);

				if ((flags & 2) != 0) // Macro texture
				{
					outColor *= texture(texMacro, texCoord3);
				}

				if ((flags & 1) != 0) // Lightmap
				{
					outColor *= texture(texLightmap, texCoord2);
				}

				if ((flags & 4) != 0) // Detail texture
				{
					/* To do: apply fade out: Min( appRound(100.f * (NearZ / Poly->Pts[i]->Point.Z - 1.f)), 255) */

					float detailScale = 1.0;
					for (int i = 0; i < 3; i++)
					{
						outColor *= texture(texDetail, texCoord4 * detailScale) + 0.5;
						detailScale *= 4.223f;
					}
				}
				else if ((flags & 8) != 0) // Fog map
				{
					vec4 fogcolor = texture(texDetail, texCoord4);
					outColor = fogcolor + outColor * (1.0 - fogcolor.a);
				}

				outColor *= color;

				#if defined(ALPHATEST)
				if (outColor.a < 0.5) discard;
				#endif
			}
		)";
	}
	else if (filename == "shaders/Shadowmap.vert")
	{
		return R"(
			layout(push_constant) uniform ScenePushConstants
			{
				mat4 objectToProjection;
			};

			layout(location = 1) in vec3 aPosition;
			layout(location = 0) out float depth;

			void main()
			{
				gl_Position = objectToProjection * vec4(aPosition, 1.0);
				depth = gl_Position.w;
			}
		)";
	}
	else if (filename == "shaders/Shadowmap.frag")
	{
		return R"(
			layout(location = 0) in float depth;
			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = vec4(depth, 0.0, 0.0, 1.0);
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

			void main()
			{
				outColor = vec4(dither(pow(texture(texSampler, texCoord).rgb, vec3(InvGamma))), 1.0f);
			}
		)";
	}

	return {};
}
