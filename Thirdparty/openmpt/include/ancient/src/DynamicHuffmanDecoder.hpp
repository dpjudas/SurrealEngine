/* Copyright (C) Teemu Suutari */

#ifndef DYNAMICHUFFMANDECODER_HPP
#define DYNAMICHUFFMANDECODER_HPP

#include <cstddef>
#include <cstdint>

#include <array>

// For exception
#include "Decompressor.hpp"

namespace ancient::internal
{

template<uint32_t maxCount>
class DynamicHuffmanDecoder
{
public:
	DynamicHuffmanDecoder(uint32_t initialCount=maxCount) :
		_initialCount{initialCount}
	{
		if (_initialCount>maxCount)
			throw Decompressor::DecompressionError();
		reset();
	}

	~DynamicHuffmanDecoder() noexcept=default;

	void reset()
	{
		_count=_initialCount;
		if (!_count) return;
		for (uint32_t i=0;i<_count;i++)
		{
			_nodes[i].frequency=1;
			_nodes[i].index=i+(maxCount-_count)*2;
			_nodes[i].parent=maxCount*2-_count+(i>>1);
			_nodes[i].leftLeaf=0;
			_nodes[i].rightLeaf=0;
			_codeMap[i+(maxCount-_count)*2]=i;
		}
		for (uint32_t i=maxCount*2-_count,j=0;i<maxCount*2-1;i++,j+=2)
		{
			uint32_t l{(j>=_count)?j+(maxCount-_count)*2:j};
			uint32_t r{(j+1>=_count)?j+1+(maxCount-_count)*2:(j+1)};
			_nodes[i].frequency=_nodes[l].frequency+_nodes[r].frequency;
			_nodes[i].index=i;
			_nodes[i].parent=maxCount+(i>>1);
			_nodes[i].leftLeaf=l;
			_nodes[i].rightLeaf=r;
			_codeMap[i]=i;
		}
	}

	template<typename F>
	uint32_t decode(F bitReader) const
	{
		if (!_count)
			throw Decompressor::DecompressionError();
		if (_count==1) return 0;
		uint32_t code{maxCount*2-2};
		while (code>=maxCount)
			code=bitReader()?_nodes[code].rightLeaf:_nodes[code].leftLeaf;
		return code;
	}

	void update(uint32_t code)
	{

		if (code>=_count)
			throw Decompressor::DecompressionError();
		// this is a bug in LH2. Nobody else uses this codepath, so we can let it be...
		if (_count==1)
		{
			_nodes[0].frequency=1;
			return;
		}

		while (code!=maxCount*2-2)
		{
			_nodes[code].frequency++;

			uint32_t index{_nodes[code].index};
			uint32_t destIndex{index};
			uint32_t freq{_nodes[code].frequency};

			while (destIndex!=maxCount*2-2 && freq>_nodes[_codeMap[destIndex+1]].frequency) destIndex++;
			if (index!=destIndex)
			{
				auto getParentLeaf=[&](uint32_t currentCode)->uint32_t&
				{
					Node &parent{_nodes[_nodes[currentCode].parent]};
					return parent.leftLeaf==currentCode?parent.leftLeaf:parent.rightLeaf;
				};

				uint32_t destCode{_codeMap[destIndex]};
				std::swap(_nodes[code].index,_nodes[destCode].index);
				std::swap(_codeMap[index],_codeMap[destIndex]);
				std::swap(getParentLeaf(code),getParentLeaf(destCode));
				std::swap(_nodes[code].parent,_nodes[destCode].parent);
			}
			code=_nodes[code].parent;
		}
		_nodes[code].frequency++;
	}

	// halve the frequencies rounding upwards
	void halve()
	{
		if (!_count) return;
		else if (_count==1)
		{
			_nodes[0].frequency=(_nodes[0].frequency+1)>>1;
			return;
		}

		for (uint32_t i=(maxCount-_count)*2,j=(maxCount-_count)*2;i<maxCount*2-1&&j<maxCount*2-_count;i++)
			if (_codeMap[i]<maxCount) _nodes[_codeMap[i]].index=j++;

		for (uint32_t i=0;i<_count;i++)
		{
			_nodes[i].frequency=(_nodes[i].frequency+1)>>1;
			_nodes[i].parent=maxCount+(_nodes[i].index>>1);
			_codeMap[_nodes[i].index]=i;
		}
		for (uint32_t i=maxCount*2-_count,j=(maxCount-_count)*2;i<maxCount*2-1;i++,j+=2)
		{
			uint32_t l{_codeMap[j]};
			uint32_t r{_codeMap[j+1]};
			uint32_t freq{_nodes[l].frequency+_nodes[r].frequency};
			_nodes[i].frequency=freq;
			_nodes[i].index=i;
			_nodes[i].parent=maxCount+(i>>1);
			_nodes[i].leftLeaf=l;
			_nodes[i].rightLeaf=r;
			_codeMap[i]=i;

			for (uint32_t k=i;freq<_nodes[_codeMap[k-1]].frequency;k--)
			{
				uint32_t &code{_codeMap[k]};
				uint32_t &destCode{_codeMap[k-1]};
				std::swap(_nodes[code].index,_nodes[destCode].index);
				std::swap(_nodes[code].parent,_nodes[destCode].parent);
				std::swap(code,destCode);
			}	
		}
	}

	// Defined as in LH2
	void addCode()
	{
		if (_count>=maxCount)
			throw Decompressor::DecompressionError();
		uint32_t newIndex{(maxCount-_count-1)*2};
		if (!_count)
		{
			_nodes[0].frequency=0;
			_nodes[0].index=newIndex-1;
			_nodes[0].parent=maxCount*2-2;
			_nodes[0].leftLeaf=0;
			_nodes[0].rightLeaf=0;
			_codeMap[newIndex-1]=0;
			_count++;
		} else {
			_nodes[_count].frequency=0;
			_nodes[_count].index=newIndex;
			_nodes[_count].parent=maxCount*2-_count-1;
			_nodes[_count].leftLeaf=0;
			_nodes[_count].rightLeaf=0;
			_codeMap[newIndex]=_count;

			uint32_t insertIndex{newIndex+2};

			uint32_t repNode;
			uint32_t parentNode;
			uint32_t insertNode{maxCount*2-_count-1};
			if (_count>1)
			{
				_codeMap[insertIndex-1]=_codeMap[insertIndex];
				_nodes[_codeMap[insertIndex-1]].index--;

				repNode=_codeMap[(maxCount-_count)*2];
				parentNode=_nodes[repNode].parent;
				(_nodes[parentNode].leftLeaf==repNode?_nodes[parentNode].leftLeaf:_nodes[parentNode].rightLeaf)=insertNode;
				_nodes[repNode].parent=insertNode;
			} else {
				repNode=0;
				parentNode=maxCount*2-1;
			}

			_nodes[insertNode].frequency=_nodes[repNode].frequency;
			_nodes[insertNode].index=insertIndex;
			_nodes[insertNode].parent=parentNode;
			_nodes[insertNode].leftLeaf=_count;
			_nodes[insertNode].rightLeaf=repNode;
			_codeMap[insertIndex]=insertNode;

			Node &parent{_nodes[parentNode]};
			if (_count>1 && _nodes[parent.leftLeaf].index>_nodes[parent.rightLeaf].index)
				std::swap(parent.leftLeaf,parent.rightLeaf);
			_count++;
		}
	}

	uint32_t getMaxFrequency() const noexcept
	{
		return _nodes[maxCount*2-2].frequency;
	}

private:
	struct Node
	{
		uint32_t	frequency;
		uint32_t	index;
		uint32_t	parent;
		uint32_t	leftLeaf;
		uint32_t	rightLeaf;
	};

	uint32_t		_initialCount;
	uint32_t		_count;
	std::array<Node,maxCount*2-1> _nodes;
	std::array<uint32_t,maxCount*2-1> _codeMap;
};

}

#endif
