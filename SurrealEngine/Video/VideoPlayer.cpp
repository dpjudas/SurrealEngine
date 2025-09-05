
#include "Precomp.h"
#include "VideoPlayer.h"
#include "AVIFileReader.h"
#include "AVIHeaders.h"
#include "Utils/File.h"
#include "UObject/UTexture.h"
#include "../../../SurrealVideo/SurrealVideo.h"

static void ReleaseVideoDecoder(IVideoDecoder* decoder) { decoder->Release(); }

class VideoPlayerImpl : public VideoPlayer
{
public:
	VideoPlayerImpl(const std::string& filename) : Decoder(CreateVideoDecoder(), &ReleaseVideoDecoder)
	{
		reader = std::make_unique<AVIFileReader>(File::read_all_bytes(filename));
		ReadHeaders();
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
					ReadPacket(type);
					reader->PopChunk();
					reader->SkipJunk();
				}
			}
			else
			{
				ReadPacket(type);
			}
			reader->PopChunk();
			reader->SkipJunk();
			return true;
		}
		return false;
	}

	void ReadPacket(const std::string& type)
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

	void ReadHeaders()
	{
		reader->PushChunk("RIFF");
		reader->ReadTag("AVI ");

		// AVI header must be first chunk
		reader->SkipJunk();
		reader->PushChunk("LIST");
		{
			reader->ReadTag("hdrl");

			reader->PushChunk("avih");
			ReadMainHeader();
			reader->PopChunk();

			reader->SkipJunk();
			while (!reader->IsEndOfChunk())
			{
				reader->PushChunk("LIST");
				reader->ReadTag("strl");
				ReadStreamHeader();
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

	void ReadMainHeader()
	{
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
	}

	void ReadStreamHeader()
	{
		AVIStreamHeader streamHeader;
		reader->PushChunk("strh");
		if (reader->GetChunkSize() < 56)
			throw std::runtime_error("Unsupported AVI stream header size");
		streamHeader.StreamType = reader->ReadTag();
		streamHeader.Codec = reader->ReadTag();
		streamHeader.Flags = reader->ReadUint32();
		streamHeader.Priority = reader->ReadUint16();
		streamHeader.Language = reader->ReadUint16();
		streamHeader.InitialFrames = reader->ReadUint32();
		streamHeader.Scale = reader->ReadUint32();
		streamHeader.Rate = reader->ReadUint32();
		streamHeader.Start = reader->ReadUint32();
		streamHeader.Length = reader->ReadUint32();
		streamHeader.SuggestedBufferSize = reader->ReadUint32();
		streamHeader.Quality = reader->ReadUint32();
		streamHeader.SampleSize = reader->ReadUint32();
		streamHeader.Frame.Left = reader->ReadInt16();
		streamHeader.Frame.Top = reader->ReadInt16();
		streamHeader.Frame.Right = reader->ReadInt16();
		streamHeader.Frame.Bottom = reader->ReadInt16();
		reader->PopChunk();
		reader->PushChunk("strf");
		if (streamHeader.StreamType == "vids")
		{
			ReadVideoStreamHeader(streamHeader);
		}
		else if (streamHeader.StreamType == "auds")
		{
			ReadAudioStreamHeader(streamHeader);
		}
		reader->PopChunk();
		streamHeaders.push_back(streamHeader);
	}

	void ReadVideoStreamHeader(AVIStreamHeader& streamHeader)
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

	void ReadAudioStreamHeader(AVIStreamHeader& streamHeader)
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
			reader->Read(streamHeader.Audio.ADPCM, 32);
		}
		else if (streamHeader.Audio.FormatTag == 0xfffe /*WAVE_FORMAT_EXTENSIBLE*/ && streamHeader.Audio.SizeExtra >= 22)
		{
			streamHeader.Audio.SamplesPerBlock = reader->ReadUint16();
			streamHeader.Audio.ChannelMask = reader->ReadUint32();
			reader->Read(streamHeader.Audio.SubFormat, 16);
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
