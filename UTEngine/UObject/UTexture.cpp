
#include "Precomp.h"
#include "UTexture.h"

void UTexture::Load(ObjectStream* stream)
{
	UBitmap::Load(stream);

	ActualFormat = (TextureFormat)GetByte("Format");

	int mipsCount = stream->ReadUInt8();
	Mipmaps.resize(mipsCount);

	for (UnrealMipmap& mipmap : Mipmaps)
	{
		uint32_t widthoffset = 0;
		if (stream->GetVersion() >= 63)
			widthoffset = stream->ReadInt32();
		int bytes = stream->ReadIndex();
		mipmap.Data.resize(bytes);
		stream->ReadBytes(mipmap.Data.data(), bytes);
		mipmap.Width = stream->ReadUInt32();
		mipmap.Height = stream->ReadUInt32();
		uint8_t UBits = stream->ReadUInt8();
		uint8_t VBits = stream->ReadUInt8();
	}

	if (HasProperty("bHasComp") && GetBool("bHasComp"))
	{
		ActualFormat = (TextureFormat)GetByte("CompFormat");

		mipsCount = stream->ReadUInt8();
		Mipmaps.resize(mipsCount);
		for (UnrealMipmap& mipmap : Mipmaps)
		{
			uint32_t widthoffset = 0;
			if (stream->GetVersion() >= 68)
				widthoffset = stream->ReadInt32();
			int bytes = stream->ReadIndex();
			mipmap.Data.resize(bytes);
			stream->ReadBytes(mipmap.Data.data(), bytes);
			mipmap.Width = stream->ReadUInt32();
			mipmap.Height = stream->ReadUInt32();
			uint8_t UBits = stream->ReadUInt8();
			uint8_t VBits = stream->ReadUInt8();
		}
	}
}

void UTexture::Update(float elapsed)
{
	float animationSpeed = 0.0f;
	if (MaxFrameRate() != 0.0f)
		animationSpeed = 1.0f / MaxFrameRate();

	if (animationSpeed > 0.0f)
		Accumulator() += elapsed;

	while (animationSpeed <= 0.0f || Accumulator() > animationSpeed)
	{
		UpdateFrame();
		if (animationSpeed <= 0.0f)
			break;
		Accumulator() -= animationSpeed;
	}
}

void UTexture::UpdateFrame()
{
	// Loop textures
	UTexture* cur = AnimCurrent();
	if (!cur) cur = this;
	cur = cur->AnimNext();
	if (!cur) cur = this;
	AnimCurrent() = cur;
}


/////////////////////////////////////////////////////////////////////////////

void UFractalTexture::Load(ObjectStream* stream)
{
	UTexture::Load(stream);

	ActualFormat = TextureFormat::P8;
	Mipmaps.resize(1);

	int width = GetInt("UClamp");
	int height = GetInt("VClamp");

	UnrealMipmap& mipmap = Mipmaps.front();
	mipmap.Width = width;
	mipmap.Height = height;
	mipmap.Data.resize((size_t)mipmap.Width * mipmap.Height);
	uint8_t* pixels = (uint8_t*)mipmap.Data.data();
	memset(pixels, 0, (size_t)width * height);
}

/////////////////////////////////////////////////////////////////////////////

void UFireTexture::Load(ObjectStream* stream)
{
	UFractalTexture::Load(stream);

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
		Sparks.push_back(spark);
	}
}

void UFireTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = Mipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();

		for (size_t i = 0; i < Sparks.size(); i++)
		{
			Spark& spark = Sparks[i];
			bool canEmit = Sparks.size() + Particles.size() < (size_t)SparksLimit();
			switch (spark.Type)
			{
			case ESpark::Burn:
			{
				int x = spark.X;
				int y = spark.Y;
				pixels[x + y * width] = RandomByteValue();
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
						pixels[x + y * width] = c;

						x0 += rand() * 2.0f / (float)RAND_MAX - 1.0f;
						y0 += rand() * 2.0f / (float)RAND_MAX - 1.0f;
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
					pixels[x + y * width] = particle.Twirl.Heat;

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
					pixels[x + y * width] = particle.Drift.Heat;

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
				FadeTable[i] = (uint8_t)std::round(clamp((i + 0.5f) * 0.25f + heatLoss, 0.0f, 255.0f));
			}
		}

		WorkBuffer.resize(width * height);
		uint8_t* buffer = WorkBuffer.data();
		int riseAmount = bRising() ? 1 : 0;
		for (int y = 0; y < height; y++)
		{
			uint8_t* destLine = buffer + y * width;
			uint8_t* srcLine = pixels + ((y + riseAmount) % height) * width;
			uint8_t* nextLine = pixels + ((y + riseAmount + 1) % height) * width;
			for (int x = 0; x < width; x++)
			{
				int left = srcLine[x != 0 ? x - 1 : width - 1];
				int center = srcLine[x];
				int right = srcLine[x != width - 1 ? x + 1 : 0];
				int bottom = nextLine[x];
				destLine[x] = FadeTable[left + center + right + bottom];
			}
		}
		memcpy(pixels, buffer, width * height);

		TextureModified = true;
	}
}

/////////////////////////////////////////////////////////////////////////////

void UIceTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = Mipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();
		int count = width * height;

		UTexture* tex = SourceTexture();
		if (tex && !tex->Mipmaps.empty() && tex->Mipmaps.front().Width == mipmap.Width && tex->Mipmaps.front().Height == mipmap.Height)
		{
			const uint8_t* srcpixels = (const uint8_t*)tex->Mipmaps.front().Data.data();
			for (int i = 0; i < count; i++)
			{
				pixels[i] = srcpixels[i];
			}
		}
		else
		{
			for (int i = 0; i < count; i++)
			{
				pixels[i] = 200;
			}
		}

		TextureModified = true;
	}
}

/////////////////////////////////////////////////////////////////////////////

void UWaterTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = Mipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();
		int count = width * height;
		for (int i = 0; i < count; i++)
		{
			pixels[i] = 200;
		}

		TextureModified = true;
	}
}

/////////////////////////////////////////////////////////////////////////////

void UWaveTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = Mipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();
		int count = width * height;
		for (int i = 0; i < count; i++)
		{
			pixels[i] = 200;
		}

		TextureModified = true;
	}
}

/////////////////////////////////////////////////////////////////////////////

void UWetTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = Mipmaps.front();

		UTexture* tex = SourceTexture();
		if (tex && !tex->Mipmaps.empty() && tex->Mipmaps.front().Width == mipmap.Width && tex->Mipmaps.front().Height == mipmap.Height)
		{
			int width = mipmap.Width;
			int height = mipmap.Height;
			uint8_t* pixels = (uint8_t*)mipmap.Data.data();
			const uint8_t* srcpixels = (const uint8_t*)tex->Mipmaps.front().Data.data();
			int count = width * height;
			for (int i = 0; i < count; i++)
			{
				pixels[i] = srcpixels[i];
			}
		}
		else
		{
			UWaterTexture::UpdateFrame();
		}

		TextureModified = true;
	}
}

/////////////////////////////////////////////////////////////////////////////

void UPalette::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadIndex();
	Colors.resize(count);
	stream->ReadBytes(Colors.data(), count * 4);

	if (stream->GetVersion() < 66)
	{
		for (uint32_t& c : Colors)
			c |= 0xff000000;
	}
}
