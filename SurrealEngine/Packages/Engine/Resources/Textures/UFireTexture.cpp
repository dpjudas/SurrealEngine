
#include "Precomp.h"
#include "UFireTexture.h"
#include <cstring>

void UFireTexture::Load(ObjectStream* stream)
{
	UFractalTexture::Load(stream);

	auto& sparks = Sparks;
	int size = stream->ReadIndex();
	for (int i = 0; i < size; i++)
	{
		Spark spark;
		spark.Type = (ESpark)stream->ReadUInt8();
		spark.Heat = stream->ReadUInt8();
		spark.X = stream->ReadUInt8();
		spark.Y = stream->ReadUInt8();
		spark.ByteA = stream->ReadUInt8();
		spark.ByteB = stream->ReadUInt8();
		spark.ByteC = stream->ReadUInt8();
		spark.ByteD = stream->ReadUInt8();
		sparks.push_back(spark);
	}
}

void UFireTexture::Save(PackageStreamWriter* stream)
{
	UFractalTexture::Save(stream);

	auto& sparks = Sparks;
	stream->WriteIndex((int)sparks.size());
	for (const Spark& spark : sparks)
	{
		stream->WriteUInt8((uint8_t)spark.Type);
		stream->WriteUInt8(spark.Heat);
		stream->WriteUInt8(spark.X);
		stream->WriteUInt8(spark.Y);
		stream->WriteUInt8(spark.ByteA);
		stream->WriteUInt8(spark.ByteB);
		stream->WriteUInt8(spark.ByteC);
		stream->WriteUInt8(spark.ByteD);
	}
}

static void SetPixel(uint8_t* pixels, int x, int y, int width, int height, uint8_t value)
{
	if (x >= 0 && y >= 0 && x < width && y < height)
	{
		pixels[x + y * width] = value;
	}
}

void UFireTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = UsedMipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();

		auto& sparks = Sparks;
		for (size_t i = 0; i < sparks.size(); i++)
		{
			Spark& spark = sparks[i];
			bool canEmit = sparks.size() + Particles.size() < (size_t)SparksLimit();
			switch (spark.Type)
			{
			default: // Always create some output as otherwise textures might completely disappear
			case ESpark::Eels: // This is the amp powerup effect
			case ESpark::Burn:
			{
				int x = spark.X;
				int y = spark.Y;
				SetPixel(pixels, x, y, width, height, RandomByteValue());
				break;
			}
			case ESpark::Pulse:
			case ESpark::Signal:
			case ESpark::Sparkle:
			{
				int x = (spark.X + (RandomByteValue() * spark.ByteA + 128) / 256) % width;
				int y = (spark.Y + (RandomByteValue() * spark.ByteB + 128) / 256) % height;
				SetPixel(pixels, x, y, width, height, spark.Heat);
				break;
			}
			case ESpark::Wheel:
			{
				if (canEmit)
				{
					SparkParticle particle;
					particle.Type = SparkParticleType::Twirl;
					particle.Twirl.X = spark.X + 0.5f;
					particle.Twirl.Y = spark.Y + 0.5f;
					particle.Twirl.Heat = spark.Heat;
					particle.Twirl.Angle = radians(spark.Wheel.Angle * (360.0f / 256));
					particle.Twirl.RotSpeed = radians(spark.Wheel.TwirlRotSpeed * (16.0f / 256) * (360.0f / 256));
					particle.Twirl.Age = spark.Wheel.TwirlAge;
					Particles.push_back(particle);
				}
				spark.Wheel.Angle += spark.Wheel.RotSpeed;
				break;
			}
			case ESpark::Emit:
			{
				if (canEmit && RandomByteValue() < 64)
				{
					SparkParticle particle;
					particle.Type = SparkParticleType::Drift;
					particle.Drift.X = spark.X + 0.5f;
					particle.Drift.Y = spark.Y + 0.5f;
					particle.Drift.Heat = spark.Heat;
					particle.Drift.HeatDecay = spark.Emit.HeatDecay;
					particle.Drift.SpeedX = ((int8_t)spark.Emit.SpeedX) * (1.0f / 128.0f);
					particle.Drift.SpeedY = ((int8_t)spark.Emit.SpeedY) * (1.0f / 128.0f);
					Particles.push_back(particle);
				}
				break;
			}
			case ESpark::OzHasSpoken:
			{
				if (canEmit && RandomByteValue() < 128)
				{
					SparkParticle particle;
					particle.Type = SparkParticleType::Drift;
					particle.Drift.X = spark.X + 0.5f;
					particle.Drift.Y = spark.Y + 0.5f;
					particle.Drift.Heat = spark.Heat;
					particle.Drift.HeatDecay = 5;
					particle.Drift.SpeedX = ((int)RandomByteValue() - 128) * (0.5f / 128.0f);
					particle.Drift.SpeedY = -0.5f;
					Particles.push_back(particle);
				}
				break;
			}
			case ESpark::Blaze:
			{
				if (canEmit && RandomByteValue() < 128)
				{
					SparkParticle particle;
					particle.Type = SparkParticleType::Drift;
					particle.Drift.X = spark.X + 0.5f;
					particle.Drift.Y = spark.Y + 0.5f;
					particle.Drift.Heat = spark.Heat;
					particle.Drift.HeatDecay = spark.Blaze.HeatDecay;
					particle.Drift.SpeedX = ((int)RandomByteValue() - 128) * (1.0f / 128.0f);
					particle.Drift.SpeedY = ((int)RandomByteValue() - 128) * (1.0f / 128.0f);
					Particles.push_back(particle);
				}
				break;
			}
			case ESpark::SphereLightning:
			{
				if (RandomByteValue() >= spark.SphereLightning.Frequency)
				{
					// Worst lightning line implementation ever, but it will do, maybe!
					float angle = radians(RandomByteValue() * (360.0f / 256));
					float radius = spark.SphereLightning.Radius * 0.50f;
					float x0 = spark.X + 0.5f;
					float y0 = spark.Y + 0.5f;
					float dx = std::cos(angle);
					float dy = std::sin(angle);
					int color0 = spark.Heat;
					int color1 = spark.Heat / 4;
					for (float i = 0; i < radius; i += 0.5f)
					{
						float t = i / radius;
						int c = (int)(color0 + (color1 - color0) * t + 0.5f);
						int x = (int)(x0 + dx * i);
						int y = (int)(y0 + dy * i);
						if (x < 0) x += width;
						else if (x >= width) x -= width;
						if (y < 0) y += height;
						else if (y >= height) y -= height;
						SetPixel(pixels, x, y, width, height, c);

						x0 += (float)(rand() * 2.0 / RAND_MAX - 1.0);
						y0 += (float)(rand() * 2.0 / RAND_MAX - 1.0);
					}
				}
				break;
			}
			}
		}

		for (size_t i = 0; i < Particles.size(); i++)
		{
			SparkParticle& particle = Particles[i];
			switch (particle.Type)
			{
			case SparkParticleType::Twirl:
			{
				if (particle.Twirl.Age > 0)
				{
					int x = (int)particle.Twirl.X;
					int y = (int)particle.Twirl.Y;
					if (x < 0) x += width; else if (x >= width) x -= width;
					if (y < 0) y += height; else if (y >= height) y -= height;
					SetPixel(pixels, x, y, width, height, particle.Twirl.Heat);

					float angle = particle.Twirl.Angle;
					float dx = std::sin(angle);
					float dy = std::cos(angle);

					particle.Twirl.X += dx * 0.5f;
					particle.Twirl.Y += dy * 0.5f;
					particle.Twirl.Angle += particle.Twirl.RotSpeed;
					particle.Twirl.Age--;
				}
				else
				{
					particle = Particles.back();
					Particles.pop_back();
				}
				break;
			}
			case SparkParticleType::Drift:
			{
				particle.Drift.Heat -= particle.Drift.HeatDecay;
				if (particle.Drift.Heat > 0)
				{
					int x = (int)particle.Drift.X;
					int y = (int)particle.Drift.Y;
					if (x < 0) x += width; else if (x >= width) x -= width;
					if (y < 0) y += height; else if (y >= height) y -= height;
					SetPixel(pixels, x, y, width, height, particle.Drift.Heat);

					particle.Drift.X += particle.Drift.SpeedX;
					particle.Drift.Y += particle.Drift.SpeedY;
				}
				else
				{
					particle = Particles.back();
					Particles.pop_back();
				}
				break;
			}
			}
		}

		if (CurrentRenderHeat != RenderHeat())
		{
			CurrentRenderHeat = RenderHeat();
			float heatLoss = 1.0f - (255 - CurrentRenderHeat) / 16.0f;
			for (int i = 0; i < 4 * 256; i++)
			{
				FadeTable[i] = (uint8_t)clamp(i * 0.25f + heatLoss, 0.0f, 255.0f);
			}
		}

		WorkBuffer.resize(width * height);
		uint8_t* buffer = WorkBuffer.data();

		int rising = bRising() ? 1 : 0;
		for (int y = 0; y < height; y++)
		{
			uint8_t* destLine = buffer + y * width;
			uint8_t* srcLine = pixels + ((y + rising) % height) * width;
			uint8_t* nextLine = pixels + ((y + rising + 1) % height) * width;
			{
				int left = srcLine[width - 1];
				int center = srcLine[0];
				int right = srcLine[1];
				int bottom = nextLine[0];
				destLine[0] = FadeTable[left + center + right + bottom];
			}
			for (int x = 1; x < width - 1; x++)
			{
				int left = srcLine[x - 1];
				int center = srcLine[x];
				int right = srcLine[x + 1];
				int bottom = nextLine[x];
				destLine[x] = FadeTable[left + center + right + bottom];
			}
			{
				int left = srcLine[width - 2];
				int center = srcLine[width - 1];
				int right = srcLine[0];
				int bottom = nextLine[width - 1];
				destLine[width - 1] = FadeTable[left + center + right + bottom];
			}
		}
		std::memcpy(pixels, buffer, width * height);

		TextureModified = true;
	}
}
