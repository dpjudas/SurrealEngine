
#include "Precomp.h"
#include "UWaterTexture.h"

void UWaterTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UpdateWater();

		// Show the current water in the texture:
		// To do: this probably shouldn't just show the depth, but rather the slope

		UnrealMipmap& mipmap = UsedMipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();

		for (int y = 0; y < height; y++)
		{
			const WaterPixel* waterline = &WaterDepth[CurrentWaterDepth][y * width];
			uint8_t* destline = pixels + y * width;
			for (int x = 0; x < width; x++)
			{
				// float u = 0.2f * waterline[x].XGradient;
				// float v = 0.2f * waterline[x].YGradient;
				vec3 normal = normalize(vec3(-waterline[x].XGradient, 0.2f, -waterline[x].YGradient));
				destline[x] = (uint8_t)clamp(std::abs(normal.y) * 255.0f + 128.0f, 0.0f, 255.0f);
			}
		}

		TextureModified = true;
	}
}

void UWaterTexture::UpdateWater()
{
	UnrealMipmap& mipmap = UsedMipmaps.front();

	int width = mipmap.Width;
	int height = mipmap.Height;
	uint8_t* pixels = (uint8_t*)mipmap.Data.data();

	WaterDepth[0].resize(width * height);
	WaterDepth[1].resize(width * height);

	auto drops = Drops();
	for (int i = 0, count = NumDrops(); i < count; i++)
	{
		ADrop& drop = drops[i];
		WaterPixel& water = WaterDepth[CurrentWaterDepth][drop.X * 2 + drop.Y * 2 * width];
		switch (drop.Type)
		{
		case ADropType::FixedDepth:
		{
			water.Pressure = ((int)drop.Depth - 128) * (1.0f / 255);
			break;
		}
		case ADropType::PhaseSpot:
		{
			drop.Depth += drop.ByteD;
			water.Pressure = std::sin(drop.Depth * (3.14f / 128)); // To do: use a table since there are only 256 possible values passed into std::sin here
			break;
		}
		case ADropType::ShallowSpot:
		{
			break;
		}
		case ADropType::HalfAmpl:
		{
			break;
		}
		case ADropType::RandomMover:
		{
			break;
		}
		case ADropType::FixedRandomSpot:
		{
			break;
		}
		case ADropType::WhirlyThing:
		{
			break;
		}
		case ADropType::BigWhirly:
		{
			break;
		}
		case ADropType::HorizontalLine:
		{
			break;
		}
		case ADropType::VerticalLine:
		{
			break;
		}
		case ADropType::DiagonalLine1:
		{
			break;
		}
		case ADropType::DiagonalLine2:
		{
			break;
		}
		case ADropType::HorizontalOsc:
		{
			break;
		}
		case ADropType::VerticalOsc:
		{
			break;
		}
		case ADropType::DiagonalOsc1:
		{
			break;
		}
		case ADropType::DiagonalOsc2:
		{
			break;
		}
		case ADropType::RainDrops:
		{
			break;
		}
		case ADropType::AreaClamp:
		{
			break;
		}
		case ADropType::LeakyTap:
		{
			break;
		}
		case ADropType::DrippyTap:
		{
			break;
		}
		default: break;
		}
	}

	int cur = CurrentWaterDepth;
	int next = (cur + 1) % 2;
	CurrentWaterDepth = next;

	for (int y = 0; y < height; y++)
	{
		const WaterPixel* srcline = &WaterDepth[cur][y * width];
		const WaterPixel* srclineup = &WaterDepth[cur][(y - 1 >= 0 ? y - 1 : height - 1) * width];
		const WaterPixel* srclinedown = &WaterDepth[cur][(y + 1 < height ? y + 1 : 0) * width];
		WaterPixel* destline = &WaterDepth[next][y * width];
		for (int x = 0; x < width; x++)
		{
			int xleft = x - 1 >= 0 ? x - 1 : width - 1;
			int xright = x + 1 < width ? x + 1 : 0;

			float velocity = srcline[x].Velocity;
			float pressure = srcline[x].Pressure;
			float pressureLeft = srcline[xleft].Pressure;
			float pressureRight = srcline[xright].Pressure;
			float pressureUp = srclineup[x].Pressure;
			float pressureDown = srclinedown[x].Pressure;

			const float delta = 1.0f; // Use a smaller number for a smaller timestep

			// Apply horizontal wave function
			velocity += delta * (-2.0f * pressure + pressureRight + pressureLeft) * 0.25f;

			// Apply vertical wave function
			velocity += delta * (-2.0f * pressure + pressureUp + pressureDown) * 0.25f;

			// Change pressure by pressure velocity
			pressure += delta * velocity;

			// "Spring" motion. This makes the waves look more like water waves and less like sound waves.
			velocity -= 0.005f * delta * pressure;

			// Velocity damping so things eventually calm down
			velocity *= 1.0f - 0.002f * delta;

			// Pressure damping to prevent it from building up forever.
			pressure *= 0.999f;

			destline[x].Pressure = pressure;
			destline[x].Velocity = velocity;
			destline[x].XGradient = (pressureRight - pressureLeft) * 0.5f;
			destline[x].YGradient = (pressureDown - pressureUp) * 0.5f;
		}
	}
}
