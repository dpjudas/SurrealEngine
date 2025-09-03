
#include "Precomp.h"
#include "VideoPlayer.h"
#include "AVIFileReader.h"
#include "Utils/File.h"

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
};

void VideoPlayer::Test()
{
	AVIMainHeader mainHeader;
	Array<AVIStreamHeader> streamHeaders;

	AVIFileReader reader(File::read_all_bytes("C:\\Games\\Star Trek - Klingon Honor Guard\\System\\INTRO.AVI"));

	reader.PushChunk("RIFF");
	reader.ReadTag("AVI ");

	// AVI header must be first chunk
	reader.SkipJunk();
	reader.PushChunk("LIST");
	{
		reader.ReadTag("hdrl");

		reader.PushChunk("avih");
		if (reader.GetChunkSize() < 56)
			throw std::runtime_error("Unsupported AVI header size");
		mainHeader.MicroSecPerFrame = reader.ReadUint32();
		mainHeader.MaxBytesPerSec = reader.ReadUint32();
		mainHeader.PaddingGranularity = reader.ReadUint32();
		mainHeader.Flags = reader.ReadUint32();
		mainHeader.TotalFrames = reader.ReadUint32();
		mainHeader.InitialFrames = reader.ReadUint32();
		mainHeader.Streams = reader.ReadUint32();
		mainHeader.SuggestedBufferSize = reader.ReadUint32();
		mainHeader.Width = reader.ReadUint32();
		mainHeader.Height = reader.ReadUint32();
		mainHeader.Reserved[0] = reader.ReadUint32();
		mainHeader.Reserved[1] = reader.ReadUint32();
		mainHeader.Reserved[2] = reader.ReadUint32();
		mainHeader.Reserved[3] = reader.ReadUint32();
		reader.PopChunk();

		reader.SkipJunk();
		while (!reader.IsEndOfChunk())
		{
			reader.PushChunk("LIST");
			reader.ReadTag("strl");

			AVIStreamHeader streamHeader;
			reader.PushChunk("strh");
			if (reader.GetChunkSize() < 64)
				throw std::runtime_error("Unsupported AVI stream header size");
			streamHeader.StreamType = reader.ReadTag();
			streamHeader.Codec = reader.ReadTag();
			streamHeader.Flags = reader.ReadUint32();
			streamHeader.Priority = reader.ReadUint16();
			streamHeader.Language = reader.ReadUint16();
			streamHeader.InitialFrames = reader.ReadUint32();
			streamHeader.Scale = reader.ReadUint32();
			streamHeader.Rate = reader.ReadUint32();
			streamHeader.Start = reader.ReadUint32();
			streamHeader.Length = reader.ReadUint32();
			streamHeader.SuggestedBufferSize = reader.ReadUint32();
			streamHeader.Quality = reader.ReadUint32();
			streamHeader.SampleSize = reader.ReadUint32();
			streamHeader.Frame.Left = reader.ReadInt32();
			streamHeader.Frame.Top = reader.ReadInt32();
			streamHeader.Frame.Right = reader.ReadInt32();
			streamHeader.Frame.Bottom = reader.ReadInt32();
			reader.PopChunk();

			reader.PushChunk("strf");
			int size = reader.GetChunkSize();
			// BITMAPINFO for video, WAVEFORMATEX for audio
			reader.PopChunk();

			/*
			if (!reader.IsEndOfChunk())
			{
				std::string tag = reader.ReadTag();
				//reader.ReadTag("strd");
				// additional header data
			}
			if (!reader.IsEndOfChunk())
			{
				reader.ReadTag("strn");
				// stream name
			}
			*/

			streamHeaders.push_back(streamHeader);
			reader.PopChunk();
			reader.SkipJunk();
		}
	}
	reader.PopChunk();

	// movi list must be second chunk
	reader.SkipJunk();
	reader.PushChunk("LIST");
	{
		reader.ReadTag("movi");
		reader.SkipJunk();
		while (!reader.IsEndOfChunk())
		{
			std::string type = reader.PushChunk();
			if (type == "LIST")
			{
				reader.ReadTag("rec ");
				reader.SkipJunk();
				while (!reader.IsEndOfChunk())
				{
					type = reader.PushChunk();

					if (type.substr(2) == "wb")
					{
						// Audio frame
						int size = reader.GetChunkSize();
					}
					else if (type.substr(2) == "dc")
					{
						// Video frame
						int size = reader.GetChunkSize();
					}

					reader.PopChunk();
					reader.SkipJunk();
				}
			}
			else if (type.substr(2) == "wb")
			{
				// Audio frame
				int size = reader.GetChunkSize();
			}
			else if (type.substr(2) == "dc")
			{
				// Video frame
				int size = reader.GetChunkSize();
			}
			reader.PopChunk();
			reader.SkipJunk();
		}
	}
	reader.PopChunk();

	// At the end there should be seek indexes
	reader.SkipJunk();
	while (!reader.IsEndOfChunk())
	{
		std::string type = reader.PushChunk();
		if (type == "idx1") // AVI 1.0 index
		{
		}
		else if (type == "indx") // AVI 2.0 (OpenDML) index
		{
		}
		else if (type == "LIST") // Seems this also happens?
		{
		}
		reader.PopChunk();
		reader.SkipJunk();
	}

	reader.PopChunk(); // RIFF
}
