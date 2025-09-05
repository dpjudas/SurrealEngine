#pragma once

#include <cstdint>

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
	std::string StreamType;             // auds, mids, txts, vids
	std::string Codec;
	uint32_t Flags = 0;
	uint16_t Priority = 0;
	uint16_t Language = 0;
	uint32_t InitialFrames = 0;         // How far ahead the audio compared to the video
	uint32_t Scale = 0;
	uint32_t Rate = 0;                  // Rate / Scale = samples per second
	uint32_t Start = 0;                 // Start time
	uint32_t Length = 0;                // Length of video
	uint32_t SuggestedBufferSize = 0;
	uint32_t Quality = 0;
	uint32_t SampleSize = 0;            // Block align for audio, zero if varying
	struct
	{
		int16_t Left = 0;
		int16_t Top = 0;
		int16_t Right = 0;
		int16_t Bottom = 0;
	} Frame;
	struct
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint16_t Planes = 0;
		uint16_t BitCount = 0;
		uint32_t Compression = 0;        // BI_RGB, BI_RLE8, BI_RLE4, BI_BITFIELDS, BI_JPEG, BI_PNG
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
		uint16_t BlockAlign = 0;          // Channels * BitsPerSample / 8
		uint16_t BitsPerSample = 0;       // 8 or 16
		uint16_t SizeExtra = 0;
		uint16_t SamplesPerBlock = 0;
		uint32_t ChannelMask = 0;
		uint8_t ADPCM[32] = {};
		uint8_t SubFormat[16] = {};       // GUID
	} Audio;
};
