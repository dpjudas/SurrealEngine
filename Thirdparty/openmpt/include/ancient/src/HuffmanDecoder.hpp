/* Copyright (C) Teemu Suutari */

#ifndef HUFFMANDECODER_HPP
#define HUFFMANDECODER_HPP

#include <cstddef>
#include <cstdint>

#include <vector>
#include <utility>
#include <array>

// For exception
#include "Decompressor.hpp"

namespace ancient::internal
{

template<typename T>
struct HuffmanCode
{
	uint32_t	length;
	uint32_t	code;

	T		value;

	HuffmanCode(uint32_t _length,uint32_t _code,T _value) noexcept :
		length{_length},
		code{_code},
		value{_value}
	{
		// nothing needed
	}
	~HuffmanCode() noexcept=default;
};

template<typename T> class OptionalHuffmanDecoder;

template<typename T>
class HuffmanDecoder
{
friend class OptionalHuffmanDecoder<T>;
private:
	struct Node
	{
		uint32_t	left,right;
		T		value;

		Node(uint32_t _left,uint32_t _right,T _value) :
			left{_left},
			right{_right},
			value{_value}
		{
			// nothing needed
		}

		Node(Node &&source) :
			left{source.left},
			right{source.right},
			value{source.value}
		{
			// nothing needed
		}

		Node& operator=(Node &&source)
		{
			if (this!=&source)
			{
				left=source.left;
				right=source.right;
				value=source.value;
			}
			return *this;
		}
	};

public:
	HuffmanDecoder() noexcept=default;

	template<typename ...Args>
	HuffmanDecoder(const Args&& ...args) :
		HuffmanDecoder()
	{
		const HuffmanCode<T> list[sizeof...(args)]={args...};
		for (auto &item : list)
			insert(item);
	}

	~HuffmanDecoder() noexcept=default;

	void reset()
	{
		_table.clear();
	}

	template<typename F>
	const T &decode(F bitReader) const
	{
		if (!_table.size())
			throw Decompressor::DecompressionError();
		uint32_t i{0};
		while (_table[i].left || _table[i].right)
		{
			i=bitReader()?_table[i].right:_table[i].left;
			if (!i)
				throw Decompressor::DecompressionError();
		}
		return _table[i].value;
	}

	void insert(const HuffmanCode<T> &code)
	{
		uint32_t i{0};
		uint32_t length={uint32_t(_table.size())};
		for (int32_t currentBit=code.length;currentBit>=0;currentBit--)
		{
			uint32_t codeBit={(currentBit && ((code.code>>(currentBit-1U))&1U))?1U:0};
			if (i!=length)
			{
				if (!currentBit || (!_table[i].left && !_table[i].right))
					throw Decompressor::DecompressionError();
				uint32_t &tmp{codeBit?_table[i].right:_table[i].left};
				if (!tmp) tmp=i=length;
					else i=tmp;
			} else {
				_table.emplace_back((currentBit&&!codeBit)?length+1:0,(currentBit&&codeBit)?length+1:0,currentBit?T():code.value);
				length++;
				i++;
			}
		}
	}

	// create orderly Huffman table, as used by Deflate and Bzip2 (and many others)
	template<size_t N>
	void createOrderlyHuffmanTable(const std::array<uint8_t,N> &bitLengths,uint32_t bitTableLength)
	{
		if (bitTableLength>N)
			throw Decompressor::DecompressionError();
		uint8_t minDepth{32};
		uint8_t maxDepth{0};
		// some optimization: more tables
		std::array<uint16_t,33> firstIndex;
		std::array<uint16_t,33> lastIndex;
		std::vector<uint16_t> nextIndex(bitTableLength);
		for (uint32_t i=1;i<33;i++)
			firstIndex[i]=0xffffU;

		uint32_t realItems{0};
		for (uint32_t i=0;i<bitTableLength;i++)
		{
			uint8_t length{bitLengths[i]};
			if (length>32)
				throw Decompressor::DecompressionError();
			if (length)
			{
				if (length<minDepth) minDepth=length;
				if (length>maxDepth) maxDepth=length;
				if (firstIndex[length]==0xffffU)
				{
					firstIndex[length]=i;
					lastIndex[length]=i;
				} else {
					nextIndex[lastIndex[length]]=i;
					lastIndex[length]=i;
				}
				realItems++;
			}
		}
		if (!maxDepth)
			throw Decompressor::DecompressionError();
		// optimization, the multiple depends how sparse the tree really is. (minimum is *2)
		// usually it is sparse.
		_table.reserve(realItems*3);

		uint32_t code{0};
		for (uint32_t depth=minDepth;depth<=maxDepth;depth++)
		{
			if (firstIndex[depth]!=0xffffU)
				nextIndex[lastIndex[depth]]=bitTableLength;

			for (uint32_t i=firstIndex[depth];i<bitTableLength;i=nextIndex[i])
			{
				insert(HuffmanCode<T>{depth,code>>(maxDepth-depth),T(i)});
				code+=1<<(maxDepth-depth);
			}
		}
	}

private:
	std::vector<Node>	_table;
};

template<typename T>
class OptionalHuffmanDecoder
{
public:
	OptionalHuffmanDecoder() noexcept=default;
	~OptionalHuffmanDecoder() noexcept=default;

	void reset()
	{
		_base.reset();
	}

	void setEmpty(T value)
	{
		reset();
		_emptyValue=value;
	}

	template<typename F>
	T decode(F bitReader) const
	{
		if (!_base._table.size()) return _emptyValue;
			else return _base.decode(bitReader);
	}

	void insert(const HuffmanCode<T> &code)
	{
		_base.insert(code);
	}

	
	template<size_t N>
	void createOrderlyHuffmanTable(const std::array<uint8_t,N> &bitLengths,uint32_t bitTableLength)
	{
		_base.createOrderlyHuffmanTable(bitLengths,bitTableLength);
	}

private:
	HuffmanDecoder<T>	_base;
	T			_emptyValue{0};
};

}

#endif
