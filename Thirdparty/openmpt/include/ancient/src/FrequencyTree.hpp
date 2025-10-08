/* Copyright (C) Teemu Suutari */

#ifndef FREQUENCYTREE_HPP
#define FREQUENCYTREE_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <array>

// For exception
#include "Decompressor.hpp"

namespace ancient::internal
{

template<typename T,typename U,size_t V>
class FrequencyTree
{
public:
	FrequencyTree() noexcept
	{
		for (uint32_t i=0;i<_size;i++)
			_tree[i]=0;
	}
	~FrequencyTree() noexcept=default;

	U decode(T value,T &low,T &freq) const
	{
		if (value>=_tree[_size-1])
			throw Decompressor::DecompressionError();
		U symbol{0};
		low=0;
		for (uint32_t i=_levels-2;;i--)
		{
			T tmp{_tree[_levelOffsets[i]+symbol]};
			if (uint32_t(symbol+1)<_levelSizes[i] && value>=tmp)
			{
				symbol++;
				low+=tmp;
				value-=tmp;
			}
			if (!i) break;
			symbol<<=1;
		}
		freq=_tree[symbol];
		return symbol;
	}

	template <typename F>
	void onNotZero(F func)
	{
		uint32_t step{1U<<(_levels-1)};
		uint32_t level{_levels-1};

		for (uint32_t symbol=0;symbol<V;)
		{
			while (_tree[_levelOffsets[level]+(symbol>>level)])
			{
				if (level)
				{
					level--;
					step>>=1;
				} else {
					func(symbol);
					break;
				}
			}
			symbol+=step;
			if (!(symbol&step))
			{
				level++;
				step<<=1;
			}
		}
	}

	// only works with tree of zeros and ones (and I don't want to specialize this on bool. that would also be ugly)
	template <typename F>
	void onNotOne(F func)
	{
		uint32_t step{1U<<(_levels-1)};
		uint32_t level{_levels-1};

		for (uint32_t symbol=0;symbol<V;)
		{
			while (_tree[_levelOffsets[level]+(symbol>>level)]!=std::min(step,uint32_t(V)-symbol))
			{
				if (level)
				{
					level--;
					step>>=1;
				} else {
					func(symbol);
					break;
				}
			}
			symbol+=step;
			if (!(symbol&step))
			{
				level++;
				step<<=1;
			}
		}
	}

	T operator[](U symbol) const
	{
		if (symbol>=V)
			throw Decompressor::DecompressionError();
		return _tree[symbol];
	}
	
	void add(U symbol,typename std::make_signed<T>::type freq)
	{
		if (symbol>=V)
			throw Decompressor::DecompressionError();
		if (!freq) return;
		for (uint32_t i=0;i<_levels;i++)
		{
			_tree[_levelOffsets[i]+symbol]+=freq;
			symbol>>=1;
		}
	}

	void set(U symbol,T freq)
	{
		if (symbol>=V)
			throw Decompressor::DecompressionError();
		// TODO: check behavior on large numbers
		typename std::make_signed<T>::type delta=freq-_tree[symbol];
		add(symbol,delta);
	}

	T getTotal() const noexcept
	{
		return _tree[_size-1];
	}

private:
	static constexpr uint32_t levelSize(uint32_t level)
	{
		uint32_t ret=V;
		for (uint32_t i=0;i<level;i++)
			ret=(ret+1)>>1;
		return ret;
	}

	static constexpr uint32_t levels()
	{
		uint32_t ret{0};
		while (levelSize(ret)!=1) ret++;
		return ret+1;
	}

	static constexpr uint32_t size()
	{
		uint32_t ret{0};
		for (uint32_t i=0;i<levels();i++)
			ret+=levelSize(i);
		return ret;
	}

	static constexpr uint32_t levelOffset(uint32_t level)
	{
		uint32_t ret{0};
		for (uint32_t i=0;i<level;i++)
			ret+=levelSize(i);
		return ret;
	}

	template<uint32_t... I>
	static constexpr auto makeLevelOffsetSequence(std::integer_sequence<uint32_t,I...>)
	{
		return std::integer_sequence<uint32_t,levelOffset(I)...>{};
	}

	template<uint32_t... I>
	static constexpr auto makeLevelSizeSequence(std::integer_sequence<uint32_t,I...>)
	{
		return std::integer_sequence<uint32_t,levelSize(I)...>{};
	}
 	
	template<uint32_t... I>
	static constexpr std::array<uint32_t,sizeof...(I)> makeArray(std::integer_sequence<uint32_t,I...>)
	{
		return std::array<uint32_t,sizeof...(I)>{{I...}};
	}

	static constexpr uint32_t			_size{size()};
	static constexpr uint32_t			_levels{levels()};
	static constexpr std::array<uint32_t,_levels>	_levelOffsets{makeArray(makeLevelOffsetSequence(std::make_integer_sequence<uint32_t,levels()>{}))};
	static constexpr std::array<uint32_t,_levels>	_levelSizes{makeArray(makeLevelSizeSequence(std::make_integer_sequence<uint32_t,levels()>{}))};

	std::array<T,_size>					_tree;
};

}

#endif
