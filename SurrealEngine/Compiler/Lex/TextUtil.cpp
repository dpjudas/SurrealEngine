
#include "TextUtil.h"

std::u32string TextUtil::utf8_to_utf32(const std::string &utf8)
{
	uint8_t *utf8Data = (uint8_t*)utf8.data();
	size_t length = utf8.length();
	size_t current_position = 0;

	std::u32string utf32;
	utf32.reserve(utf8.length());

	while (current_position < length)
	{
		size_t trailing_bytes = trailing_bytes_for_utf8[utf8Data[current_position]];
		if (trailing_bytes == 0 && (utf8Data[current_position] & 0x80) == 0x80)
			throw TextException("Invalid character in UTF-8 encoded string");

		if (current_position + 1 + trailing_bytes > length)
			throw TextException("Invalid character in UTF-8 encoded string");

		unsigned int ucs4 = (utf8Data[current_position] & bitmask_leadbyte_for_utf8[trailing_bytes]);
		for (size_t i = 0; i < trailing_bytes; i++)
		{
			if ((utf8Data[current_position+1+i] & 0xC0) == 0x80)
				ucs4 = (ucs4 << 6) + (utf8Data[current_position+1+i] & 0x3f);
			else
				throw TextException("Invalid character in UTF-8 encoded string");
		}

		// To do: verify that the ucs4 value is in the valid range for the trailing_bytes specified in the lead byte.

		current_position += 1 + trailing_bytes;
		utf32.push_back(ucs4);
	}

	return utf32;
}

std::string TextUtil::utf32_to_utf8(const std::u32string &utf32)
{
	size_t length = utf32.length();

	std::string utf8;
	utf8.reserve((length + 1) / 2); // Assume that we mostly get ASCII characters

	for (size_t i = 0; i < length; i++)
	{
		char32_t value = utf32[i];

		if ( (value < 0x80) && (value > 0) )
		{
			utf8.push_back((char) value);
		}
		else if( value < 0x800 )
		{
			utf8.push_back((char) (  0xc0 | ( value >> 6 )));
			utf8.push_back((char) (  0x80 | ( value & 0x3f )));
		}
		else if( value < 0x10000 )
		{
			utf8.push_back((char) (  0xe0 | ( value >> 12 )));
			utf8.push_back((char) (  0x80 | ( (value >> 6) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( value & 0x3f )));
		}
		else if( value < 0x200000 )
		{
			utf8.push_back((char) (  0xf0 | ( value >> 18 )));
			utf8.push_back((char) (  0x80 | ( (value >> 12) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( (value >> 6) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( value & 0x3f )));

		}
		else if( value < 0x4000000 )
		{
			utf8.push_back((char) (  0xf8 | ( value >> 24 )));
			utf8.push_back((char) (  0x80 | ( (value >> 18) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( (value >> 12) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( (value >> 6) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( value & 0x3f )));

		}
		else if( value < 0x80000000 )
		{
			utf8.push_back((char) (  0xfc | ( value >> 30 )));
			utf8.push_back((char) (  0x80 | ( (value >> 24) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( (value >> 18) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( (value >> 12) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( (value >> 6) & 0x3f )));
			utf8.push_back((char) (  0x80 | ( value & 0x3f )));
		}
		else
		{
			throw TextException("Invalid UTF-32 character");
		}
	}

	return utf8;
}

const char TextUtil::trailing_bytes_for_utf8[256] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

const unsigned char TextUtil::bitmask_leadbyte_for_utf8[6] =
{
	0x7f,
	0x1f,
	0x0f,
	0x07,
	0x03,
	0x01
};
