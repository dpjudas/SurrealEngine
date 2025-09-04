
#include "Precomp.h"
#include "VideoPlayer.h"
#include "AVIFileReader.h"
#include "Utils/File.h"
#include "UObject/UTexture.h"
#include "../../../SurrealVideo/SurrealVideo.h"

struct AVIMainHeader
{
	uint32_t MicroSecPerFrame = 0;
	uint32_t MaxBytesPerSec = 0;
	uint32_t PaddingGranularity = 0;
	uint32_t Flags = 0;
	uint32_t TotalFrames = 0;
	uint32_t InitialFrames = 0;
	uint32_t Streams = 0;
	uint32_t SuggestedBufferSize = 0;
	uint32_t Width = 0;
	uint32_t Height = 0;
	uint32_t Reserved[4] = {};
};

struct AVIStreamHeader
{
	std::string StreamType; // auds, mids, txts, vids
	std::string Codec;
	uint32_t Flags = 0;
	uint16_t Priority = 0;
	uint16_t Language = 0;
	uint32_t InitialFrames = 0;
	uint32_t Scale = 0;
	uint32_t Rate = 0;
	uint32_t Start = 0;
	uint32_t Length = 0;
	uint32_t SuggestedBufferSize = 0;
	uint32_t Quality = 0;
	uint32_t SampleSize = 0;
	struct
	{
		int32_t Left = 0;
		int32_t Top = 0;
		int32_t Right = 0;
		int32_t Bottom = 0;
	} Frame;
	struct
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint16_t Planes = 0;
		uint16_t BitCount = 0;
		uint32_t Compression = 0; // BI_RGB, BI_RLE8, BI_RLE4, BI_BITFIELDS, BI_JPEG, BI_PNG
		uint32_t ImageSize = 0;
		uint32_t XPixelsPerMeter = 0;
		uint32_t YPixelsPerMeter = 0;
		uint32_t ColorsUsed = 0;
		uint32_t ColorsImportant = 0;
	} Video;
	struct
	{
		uint16_t FormatTag = 0;
		uint16_t Channels = 0;
		uint32_t SamplesPerSec = 0;
		uint32_t AvgSamplesPerSec = 0;
		uint16_t BlockAlign = 0; // Channels * BitsPerSample / 8
		uint16_t BitsPerSample = 0; // 8 or 16
		uint16_t SizeExtra = 0;
		uint16_t SamplesPerBlock = 0;
		uint32_t ChannelMask = 0;
		uint8_t ADPCM[32] = {};
		uint8_t SubFormat[16] = {}; // GUID
	} Audio;
};

static void ReleaseVideoDecoder(IVideoDecoder* decoder) { decoder->Release(); }

class VideoPlayerImpl : public VideoPlayer
{
public:
	VideoPlayerImpl(const std::string& filename) : Decoder(CreateVideoDecoder(), &ReleaseVideoDecoder)
	{
		reader = std::make_unique<AVIFileReader>(File::read_all_bytes(filename));

		reader->PushChunk("RIFF");
		reader->ReadTag("AVI ");

		// AVI header must be first chunk
		reader->SkipJunk();
		reader->PushChunk("LIST");
		{
			reader->ReadTag("hdrl");

			reader->PushChunk("avih");
			if (reader->GetChunkSize() < 56)
				throw std::runtime_error("Unsupported AVI header size");
			mainHeader.MicroSecPerFrame = reader->ReadUint32();
			mainHeader.MaxBytesPerSec = reader->ReadUint32();
			mainHeader.PaddingGranularity = reader->ReadUint32();
			mainHeader.Flags = reader->ReadUint32();
			mainHeader.TotalFrames = reader->ReadUint32();
			mainHeader.InitialFrames = reader->ReadUint32();
			mainHeader.Streams = reader->ReadUint32();
			mainHeader.SuggestedBufferSize = reader->ReadUint32();
			mainHeader.Width = reader->ReadUint32();
			mainHeader.Height = reader->ReadUint32();
			mainHeader.Reserved[0] = reader->ReadUint32();
			mainHeader.Reserved[1] = reader->ReadUint32();
			mainHeader.Reserved[2] = reader->ReadUint32();
			mainHeader.Reserved[3] = reader->ReadUint32();
			reader->PopChunk();

			reader->SkipJunk();
			while (!reader->IsEndOfChunk())
			{
				reader->PushChunk("LIST");
				reader->ReadTag("strl");

				AVIStreamHeader streamHeader;
				reader->PushChunk("strh");
				if (reader->GetChunkSize() < 56)
					throw std::runtime_error("Unsupported AVI stream header size");
				streamHeader.StreamType = reader->ReadTag();
				streamHeader.Codec = reader->ReadTag();
				streamHeader.Flags = reader->ReadUint32();
				streamHeader.Priority = reader->ReadUint16();
				streamHeader.Language = reader->ReadUint16();
				streamHeader.InitialFrames = reader->ReadUint32(); // How far ahead the audio compared to the video
				streamHeader.Scale = reader->ReadUint32();
				streamHeader.Rate = reader->ReadUint32(); // Rate / Scale = samples per second
				streamHeader.Start = reader->ReadUint32(); // Start time
				streamHeader.Length = reader->ReadUint32(); // Length of video
				streamHeader.SuggestedBufferSize = reader->ReadUint32();
				streamHeader.Quality = reader->ReadUint32();
				streamHeader.SampleSize = reader->ReadUint32(); // Block align for audio, zero if varying
				streamHeader.Frame.Left = reader->ReadInt16();
				streamHeader.Frame.Top = reader->ReadInt16();
				streamHeader.Frame.Right = reader->ReadInt16();
				streamHeader.Frame.Bottom = reader->ReadInt16();
				reader->PopChunk();

				reader->PushChunk("strf");
				if (streamHeader.StreamType == "vids")
				{
					if (reader->GetChunkSize() < 40)
						throw std::runtime_error("Unsupported AVI video stream header size");
					int size = reader->ReadUint32(); // cbSize in BITMAPINFOHEADER
					if (size < 40)
						throw std::runtime_error("Unsupported AVI video stream header size");
					streamHeader.Video.Width = reader->ReadUint32();
					streamHeader.Video.Height = reader->ReadUint32();
					streamHeader.Video.Planes = reader->ReadUint16();
					streamHeader.Video.BitCount = reader->ReadUint16();
					streamHeader.Video.Compression = reader->ReadUint32();
					streamHeader.Video.ImageSize = reader->ReadUint32();
					streamHeader.Video.XPixelsPerMeter = reader->ReadUint32();
					streamHeader.Video.YPixelsPerMeter = reader->ReadUint32();
					streamHeader.Video.ColorsUsed = reader->ReadUint32();
					streamHeader.Video.ColorsImportant = reader->ReadUint32();

				}
				else if (streamHeader.StreamType == "auds")
				{
					if (reader->GetChunkSize() < 18)
						throw std::runtime_error("Unsupported AVI audio stream header size");
					streamHeader.Audio.FormatTag = reader->ReadUint16();
					streamHeader.Audio.Channels = reader->ReadUint16();
					streamHeader.Audio.SamplesPerSec = reader->ReadUint32();
					streamHeader.Audio.AvgSamplesPerSec = reader->ReadUint32();
					streamHeader.Audio.BlockAlign = reader->ReadUint16();
					streamHeader.Audio.BitsPerSample = reader->ReadUint16();
					streamHeader.Audio.SizeExtra = reader->ReadUint16();
					if (streamHeader.Audio.FormatTag == 2 /*WAVE_FORMAT_ADPCM*/ && streamHeader.Audio.SizeExtra == 32)
					{
						// This nasty thing:
						// https://wiki.multimedia.cx/index.php/Microsoft_ADPCM
					}
					else if (streamHeader.Audio.FormatTag == 0xfffe /*WAVE_FORMAT_EXTENSIBLE*/ && streamHeader.Audio.SizeExtra >= 22)
					{
						streamHeader.Audio.SamplesPerBlock = reader->ReadUint16();
						streamHeader.Audio.ChannelMask = reader->ReadUint32();
						reader->Read(streamHeader.Audio.SubFormat, 16);
					}
				}
				reader->PopChunk();

				/*
				if (!reader->IsEndOfChunk())
				{
					std::string tag = reader->ReadTag();
					//reader->ReadTag("strd");
					// additional header data
				}
				if (!reader->IsEndOfChunk())
				{
					reader->ReadTag("strn");
					// stream name
				}
				*/

				streamHeaders.push_back(streamHeader);
				reader->PopChunk();
				reader->SkipJunk();
			}
		}
		reader->PopChunk();

		// movi list must be second chunk
		reader->SkipJunk();
		reader->PushChunk("LIST");
		reader->ReadTag("movi");
		reader->SkipJunk();
	}

	bool Decode() override
	{
		while (!reader->IsEndOfChunk())
		{
			std::string type = reader->PushChunk();
			if (type == "LIST")
			{
				reader->ReadTag("rec ");
				reader->SkipJunk();
				while (!reader->IsEndOfChunk())
				{
					type = reader->PushChunk();

					size_t packetSize = reader->GetChunkSize();
					if (packetData.size() < packetSize)
						packetData.resize(packetSize);
					reader->Read(packetData.data(), packetSize);

					if (type.substr(2) == "wb") // Audio frame
					{
						DecodeAudio(packetData.data(), packetSize);
					}
					else if (type.substr(2) == "dc" || type.substr(2) == "db") // Video frame
					{
						DecodeVideo(packetData.data(), packetSize);
					}

					reader->PopChunk();
					reader->SkipJunk();
				}
			}
			else
			{
				size_t packetSize = reader->GetChunkSize();
				if (packetData.size() < packetSize)
					packetData.resize(packetSize);
				reader->Read(packetData.data(), packetSize);

				if (type.substr(2) == "wb") // Audio frame
				{
					DecodeAudio(packetData.data(), packetSize);
				}
				else if (type.substr(2) == "dc" || type.substr(2) == "db") // Video frame
				{
					DecodeVideo(packetData.data(), packetSize);
				}
			}
			reader->PopChunk();
			reader->SkipJunk();
			return true;
		}
		return false;
	}

	UnrealMipmap* NextVideoFrame()
	{
		if (Video.DecodedFrames.empty())
			return nullptr;
		if (Video.CurrentFrame)
			Video.FreeFrames.push_back(std::move(Video.CurrentFrame));
		Video.CurrentFrame = std::move(Video.DecodedFrames.front());
		Video.DecodedFrames.erase(Video.DecodedFrames.begin());
		return Video.CurrentFrame.get();
	}

	void DecodeAudio(const void* packetData, size_t packetSize)
	{
		// To do: decode packet
		// To do: put this in an audio buffer
	}

	void DecodeVideo(const void* packetData, size_t packetSize)
	{
		VideoDecoderResult result = Decoder->Decode(packetData, packetSize);
		if (result == VideoDecoderResult::Error)
			throw std::runtime_error("Video decode failed");

		if (result == VideoDecoderResult::DecodedFrame)
		{
			int width = Decoder->GetWidth();
			int height = Decoder->GetHeight();
			const uint32_t* pixels = Decoder->GetPixels();

			std::unique_ptr<UnrealMipmap> frame;
			if (!Video.FreeFrames.empty())
			{
				frame = std::move(Video.FreeFrames.back());
				Video.FreeFrames.pop_back();
			}
			else
			{
				frame = std::make_unique<UnrealMipmap>();
			}
			frame->Width = width;
			frame->Height = height;
			frame->Data.resize(width * height * sizeof(uint32_t));
			memcpy(frame->Data.data(), pixels, width * height * sizeof(uint32_t));
			Video.DecodedFrames.push_back(std::move(frame));
		}
	}

	std::unique_ptr<IVideoDecoder, decltype(&ReleaseVideoDecoder)> Decoder;

	struct
	{
		std::unique_ptr<UnrealMipmap> CurrentFrame;
		std::vector<std::unique_ptr<UnrealMipmap>> DecodedFrames;
		std::vector<std::unique_ptr<UnrealMipmap>> FreeFrames;
	} Video;

	std::unique_ptr<AVIFileReader> reader;
	AVIMainHeader mainHeader;
	Array<AVIStreamHeader> streamHeaders;
	std::vector<uint8_t> packetData;
};

std::unique_ptr<VideoPlayer> VideoPlayer::Create(const std::string& filename)
{
	return std::make_unique<VideoPlayerImpl>(filename);
}
