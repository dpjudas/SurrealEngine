
#include "Precomp.h"
#include "UTexture.h"
#include "UFont.h"
#include "Engine.h"
#include "VM/ScriptCall.h"

void UTexture::Load(ObjectStream* stream)
{
	UBitmap::Load(stream);

	int mipsCount = stream->ReadUInt8();
	UncompressedMipmaps.resize(mipsCount);
	for (UnrealMipmap& mipmap : UncompressedMipmaps)
	{
		uint32_t widthoffset = 0;
		if (stream->GetVersion() >= 63)
			widthoffset = stream->ReadInt32();
		int bytes = stream->ReadIndex();
		mipmap.Data.resize(bytes);
		stream->ReadBytes(mipmap.Data.data(), bytes);
		mipmap.Width = stream->ReadUInt32();
		mipmap.Height = stream->ReadUInt32();
		mipmap.UBits = stream->ReadUInt8();
		mipmap.VBits = stream->ReadUInt8();
	}

	if (HasProperty("bHasComp") && GetBool("bHasComp"))
	{
		mipsCount = stream->ReadUInt8();
		CompressedMipmaps.resize(mipsCount);
		for (UnrealMipmap& mipmap : CompressedMipmaps)
		{
			uint32_t widthoffset = 0;
			if (stream->GetVersion() >= 68)
				widthoffset = stream->ReadInt32();
			int bytes = stream->ReadIndex();
			mipmap.Data.resize(bytes);
			stream->ReadBytes(mipmap.Data.data(), bytes);
			mipmap.Width = stream->ReadUInt32();
			mipmap.Height = stream->ReadUInt32();
			mipmap.UBits = stream->ReadUInt8();
			mipmap.VBits = stream->ReadUInt8();
		}

		UsedFormat = (TextureFormat)GetByte("CompFormat");
		UsedMipmaps = CompressedMipmaps;
	}
	else
	{
		UsedFormat = (TextureFormat)GetByte("Format");
		UsedMipmaps = UncompressedMipmaps;
	}
}

void UTexture::Save(PackageStreamWriter* stream)
{
	UBitmap::Save(stream);

	stream->WriteUInt8((uint8_t)UncompressedMipmaps.size());
	for (const UnrealMipmap& mipmap : UncompressedMipmaps)
	{
		if (stream->GetVersion() >= 63)
			stream->BeginSkipOffset();
		stream->WriteIndex((int)mipmap.Data.size());
		stream->WriteBytes(mipmap.Data.data(), (int)mipmap.Data.size());
		stream->WriteUInt32(mipmap.Width);
		stream->WriteUInt32(mipmap.Height);
		stream->WriteUInt8(mipmap.UBits);
		stream->WriteUInt8(mipmap.VBits);
		if (stream->GetVersion() >= 63)
			stream->EndSkipOffset();
	}

	if (HasProperty("bHasComp") && GetBool("bHasComp"))
	{
		stream->WriteUInt8((uint8_t)CompressedMipmaps.size());
		for (UnrealMipmap& mipmap : CompressedMipmaps)
		{
			if (stream->GetVersion() >= 68)
				stream->BeginSkipOffset();
			stream->WriteIndex((int)mipmap.Data.size());
			stream->WriteBytes(mipmap.Data.data(), (int)mipmap.Data.size());
			stream->WriteUInt32(mipmap.Width);
			stream->WriteUInt32(mipmap.Height);
			stream->WriteUInt8(mipmap.UBits);
			stream->WriteUInt8(mipmap.VBits);
			if (stream->GetVersion() >= 68)
				stream->EndSkipOffset();
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

	UsedFormat = TextureFormat::P8;
	UsedMipmaps.resize(1);

	int width = GetInt("UClamp");
	int height = GetInt("VClamp");

	UnrealMipmap& mipmap = UsedMipmaps.front();
	mipmap.Width = width;
	mipmap.Height = height;
	mipmap.Data.resize((size_t)mipmap.Width * mipmap.Height);
	uint8_t* pixels = (uint8_t*)mipmap.Data.data();
	memset(pixels, 0, (size_t)width * height);
}

void UFractalTexture::Save(PackageStreamWriter* stream)
{
	UTexture::Save(stream);
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

void UFireTexture::Save(PackageStreamWriter* stream)
{
	UFractalTexture::Save(stream);

	stream->WriteIndex((int)Sparks.size());
	for (const Spark& spark : Sparks)
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

		for (size_t i = 0; i < Sparks.size(); i++)
		{
			Spark& spark = Sparks[i];
			bool canEmit = Sparks.size() + Particles.size() < (size_t)SparksLimit();
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
		UnrealMipmap& mipmap = UsedMipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();
		int count = width * height;

		UTexture* tex = SourceTexture();
		if (tex && !tex->UsedMipmaps.empty() && tex->UsedMipmaps.front().Width == mipmap.Width && tex->UsedMipmaps.front().Height == mipmap.Height)
		{
			const uint8_t* srcpixels = (const uint8_t*)tex->UsedMipmaps.front().Data.data();
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

	ADrop* drops = Drops();
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

/////////////////////////////////////////////////////////////////////////////

void UWaveTexture::UpdateFrame()
{
	// What is the difference between a water texture and a wave texture?
	UWaterTexture::UpdateFrame();
}

/////////////////////////////////////////////////////////////////////////////

void UWetTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UpdateWater();

		UnrealMipmap& mipmap = UsedMipmaps.front();

		UTexture* tex = SourceTexture();
		if (tex && !tex->UsedMipmaps.empty() && tex->UsedMipmaps.front().Width == mipmap.Width && tex->UsedMipmaps.front().Height == mipmap.Height)
		{
			int width = mipmap.Width;
			int height = mipmap.Height;
			uint8_t* pixels = (uint8_t*)mipmap.Data.data();
			const uint8_t* srcpixels = (const uint8_t*)tex->UsedMipmaps.front().Data.data();
			for (int y = 0; y < height; y++)
			{
				const WaterPixel* waterline = &WaterDepth[CurrentWaterDepth][y * width];
				const uint8_t* srcline = srcpixels + y * width;
				uint8_t* destline = pixels + y * width;
				for (int x = 0; x < width; x++)
				{
					// Use water as displacement

					int water = (int)(0.5f * waterline[x].XGradient * width);
					int srcx = clamp(x + water, 0, width - 1);
					destline[x] = srcline[srcx];
				}
			}
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

void UPalette::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	stream->WriteIndex((int)Colors.size());
	stream->WriteBytes(Colors.data(), (uint32_t)Colors.size() * 4);
}

uint8_t UPalette::FindBestColor(const Color& color) const
{
	int r = color.R;
	int g = color.G;
	int b = color.B;
	int count = (int)Colors.size();
	const uint32_t* palette = Colors.data();
	int bestcolor = 0;
	int bestdist = 257 * 257 + 257 * 257 + 257 * 257;
	for (int color = 0; color < count; color++)
	{
		int sr = (palette[color] & 0xff);
		int sg = ((palette[color] >> 8) & 0xff);
		int sb = ((palette[color] >> 16) & 0xff);
		int x = r - sr;
		int y = g - sg;
		int z = b - sb;
		int dist = x * x + y * y + z * z;
		if (dist < bestdist)
		{
			if (dist == 0)
				return color;

			bestdist = dist;
			bestcolor = color;
		}
	}
	return bestcolor;
}

/////////////////////////////////////////////////////////////////////////////

void UScriptedTexture::Load(ObjectStream* stream)
{
	UTexture::Load(stream);

	UsedFormat = TextureFormat::P8;
	UsedMipmaps.resize(1);

	int width = GetInt("UClamp");
	int height = GetInt("VClamp");

	UnrealMipmap& mipmap = UsedMipmaps.front();
	mipmap.Width = width;
	mipmap.Height = height;
	mipmap.Data.resize((size_t)mipmap.Width * mipmap.Height);
	uint8_t* pixels = (uint8_t*)mipmap.Data.data();
	memset(pixels, 0, (size_t)width * height);
}

void UScriptedTexture::Save(PackageStreamWriter* stream)
{
	UTexture::Save(stream);
}

void UScriptedTexture::UpdateFrame()
{
	if (TextureModified)
		return;

	if (engine->LaunchInfo.engineVersion < 469)
	{
		double timeSinceLastUpdate = engine->TotalTime - LastUpdate;
		if (timeSinceLastUpdate < 1.0 / 80.0) // Rate limit to about 80 fps updates as 436 unrealscript code breaks if called too often
			return;
		LastUpdate = engine->TotalTime;
	}

	UTexture* sourceTex = SourceTexture();
	if (sourceTex)
	{
		Palette() = sourceTex->Palette();
		DrawTileP8(sourceTex, 0.0f, 0.0f, (float)UsedMipmaps.front().Width, (float)UsedMipmaps.front().Height, 0.0f, 0.0f, (float)sourceTex->UsedMipmaps.front().Width, (float)sourceTex->UsedMipmaps.front().Height);
	}

	UActor* actor = NotifyActor();
	if (actor)
		CallEvent(actor, "RenderTexture", { ExpressionValue::ObjectValue(this) });

	TextureModified = true;
}

void UScriptedTexture::DrawColoredText(float X, float Y, const std::string& Text, UFont* Font, const Color& FontColor)
{
	if (!Font)
		return;

	uint8_t color = Palette()->FindBestColor(FontColor);

	for (char c : Text)
	{
		FontGlyph glyph = Font->GetGlyph(c);
		if (!glyph.Texture)
			continue;

		float StartU = (float)glyph.StartU;
		float StartV = (float)glyph.StartV;
		float USize = (float)glyph.USize;
		float VSize = (float)glyph.VSize;
		DrawTileP8Color(glyph.Texture, X, Y, USize, VSize, StartU, StartV, USize, VSize, color);
		X += USize;
	}
}

void UScriptedTexture::DrawText(float X, float Y, const std::string& Text, UFont* Font)
{
	if (!Font)
		return;

	for (char c : Text)
	{
		FontGlyph glyph = Font->GetGlyph(c);
		if (!glyph.Texture)
			continue;

		float StartU = (float)glyph.StartU;
		float StartV = (float)glyph.StartV;
		float USize = (float)glyph.USize;
		float VSize = (float)glyph.VSize;
		DrawTileP8Masked(glyph.Texture, X, Y, USize, VSize, StartU, StartV, USize, VSize);
		X += USize;
	}
}

void UScriptedTexture::DrawTile(float X, float Y, float XL, float YL, float U, float V, float UL, float VL, UTexture* Tex, bool bMasked)
{
	if (bMasked)
		DrawTileP8Masked(Tex, X, Y, XL, YL, U, V, UL, VL);
	else
		DrawTileP8(Tex, X, Y, XL, YL, U, V, UL, VL);
}

void UScriptedTexture::ReplaceTexture(UTexture* Tex)
{
	// What does this do? Nothing calls it in UT it seems.
}

void UScriptedTexture::TextSize(const std::string& Text, float& XL, float& YL, UFont* Font)
{
	if (!Font)
		return;

	int x = 0;
	int y = 0;
	for (char c : Text)
	{
		FontGlyph glyph = Font->GetGlyph(c);
		x += glyph.USize;
		y = std::max(y, glyph.VSize);
	}

	XL = (float)x;
	YL = (float)y;
}

void UScriptedTexture::DrawTileP8(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight)
{
	if (!texture || width <= 0.0f || height <= 0.0f || UsedFormat != TextureFormat::P8 || texture->UsedFormat != TextureFormat::P8)
		return;

	int swidth = texture->UsedMipmaps.front().Width;
	const uint8_t* src = (const uint8_t*)texture->UsedMipmaps.front().Data.data();

	int dwidth = UsedMipmaps.front().Width;
	int dheight = UsedMipmaps.front().Height;
	uint8_t* dest = (uint8_t*)UsedMipmaps.front().Data.data();

	int x0 = std::max((int)left, 0);
	int x1 = std::min((int)(left + width), dwidth);
	int y0 = std::max((int)top, 0);
	int y1 = std::min((int)(top + height), dheight);
	if (x1 <= x0 || y1 <= y0)
		return;

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint8_t* sline = src + ((int)vpix) * swidth;
		uint8_t* dline = dest + y * dwidth;

		for (int x = x0; x < x1; x++)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint8_t spixel = sline[(int)upix];
			dline[x] = spixel;
		}
	}
}

void UScriptedTexture::DrawTileP8Masked(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight)
{
	if (!texture || width <= 0.0f || height <= 0.0f || UsedFormat != TextureFormat::P8 || texture->UsedFormat != TextureFormat::P8)
		return;

	int swidth = texture->UsedMipmaps.front().Width;
	const uint8_t* src = (const uint8_t*)texture->UsedMipmaps.front().Data.data();

	int dwidth = UsedMipmaps.front().Width;
	int dheight = UsedMipmaps.front().Height;
	uint8_t* dest = (uint8_t*)UsedMipmaps.front().Data.data();

	int x0 = std::max((int)left, 0);
	int x1 = std::min((int)(left + width), dwidth);
	int y0 = std::max((int)top, 0);
	int y1 = std::min((int)(top + height), dheight);
	if (x1 <= x0 || y1 <= y0)
		return;

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint8_t* sline = src + ((int)vpix) * swidth;
		uint8_t* dline = dest + y * dwidth;

		for (int x = x0; x < x1; x++)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint8_t spixel = sline[(int)upix];
			if (spixel != 0)
				dline[x] = spixel;
		}
	}
}

void UScriptedTexture::DrawTileP8Color(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight, uint8_t color)
{
	if (!texture || width <= 0.0f || height <= 0.0f || UsedFormat != TextureFormat::P8 || texture->UsedFormat != TextureFormat::P8)
		return;

	int swidth = texture->UsedMipmaps.front().Width;
	const uint8_t* src = (const uint8_t*)texture->UsedMipmaps.front().Data.data();

	int dwidth = UsedMipmaps.front().Width;
	int dheight = UsedMipmaps.front().Height;
	uint8_t* dest = (uint8_t*)UsedMipmaps.front().Data.data();

	int x0 = std::max((int)left, 0);
	int x1 = std::min((int)(left + width), dwidth);
	int y0 = std::max((int)top, 0);
	int y1 = std::min((int)(top + height), dheight);
	if (x1 <= x0 || y1 <= y0)
		return;

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint8_t* sline = src + ((int)vpix) * swidth;
		uint8_t* dline = dest + y * dwidth;

		for (int x = x0; x < x1; x++)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint8_t spixel = sline[(int)upix];
			if (spixel != 0)
				dline[x] = color;
		}
	}
}
