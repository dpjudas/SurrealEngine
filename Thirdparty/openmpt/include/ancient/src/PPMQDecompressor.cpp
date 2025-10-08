/* Copyright (C) Teemu Suutari */

#include "common/Common.hpp"
#include "PPMQDecompressor.hpp"
#include "RangeDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "RangeDecoder.hpp"
#include "FrequencyTree.hpp"

#include <map>
#include <list>
#include <array>

namespace ancient::internal
{

bool PPMQDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("PPMQ");
}

std::shared_ptr<XPKDecompressor> PPMQDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<PPMQDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

PPMQDecompressor::PPMQDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &PPMQDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-PPMQ: PPM compressor"};
	return name;
}

void PPMQDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	class BitReader : public RangeDecoder::BitReader
	{
	public:
		BitReader(ForwardInputStream &stream) noexcept :
			_reader{stream}
		{
			// nothing needed
		}
		~BitReader() noexcept=default;

		uint32_t readBit() final
		{
			return _reader.readBitsBE32(1);
		}

		uint32_t readBits(uint32_t bitCount)
		{
			return _reader.readBitsBE32(bitCount);
		}

	private:
		MSBBitReader<ForwardInputStream>	_reader;
	};


	ForwardInputStream inputStream{_packedData,0,_packedData.size(),16U};
	ForwardOutputStream outputStream{rawData,0,rawData.size()};
	BitReader bitReader{inputStream};

	uint32_t history{0};
	uint8_t history5{0};
	auto addToHistory=[&](uint8_t ch)
	{
		history5=history>>24U;
		history=(history<<8U)|ch;
	};

	for (uint32_t i=0;i<5U;i++)
	{
		// files shorter than 5 bytes are not supported by the encoder.
		// In practice this most probably just means padding
		if (outputStream.eof()) return;
		uint8_t ch{uint8_t(bitReader.readBits(8U))};
		outputStream.writeByte(ch);
		addToHistory(ch);
	}

	RangeDecoder decoder{bitReader,uint16_t(bitReader.readBits(16))};

	class InclusionList
	{
	public:
		class InclusionCallback
		{
		public:
			InclusionCallback(InclusionList &parent) noexcept
			{
				parent.registerCallback(this);
			}
			~InclusionCallback() noexcept=default;

			virtual void symbolIncluded(uint8_t symbol) noexcept=0;
			virtual void symbolExcluded(uint8_t symbol) noexcept=0;
		};

		InclusionList() noexcept
		{
			for (uint32_t i=0;i<256U;i++)
				_tree.set(i,1);
		}
		~InclusionList() noexcept=default;

		void reset() noexcept
		{
			_tree.onNotOne([&](uint32_t i)
			{
				_tree.set(i,1);
				for (auto callback : _callbacks) callback->symbolIncluded(i);
			});
		}

		void exclude(uint8_t symbol) noexcept
		{
			if (_tree[symbol])
			{
				_tree.set(symbol,0);
				for (auto callback : _callbacks) callback->symbolExcluded(symbol);
			}
		}

		bool isIncluded(uint8_t symbol) const noexcept
		{
			return _tree[symbol];
		}

		uint32_t getTotal() const noexcept
		{
			return _tree.getTotal();
		}

		uint8_t decode(uint16_t value,uint16_t &low,uint16_t &freq) const
		{
			return _tree.decode(value,low,freq);
		}

	private:
		void registerCallback(InclusionCallback *callback) noexcept
		{
			_callbacks.push_back(callback);
		}

		FrequencyTree<uint16_t,uint8_t,256>	_tree;
		std::vector<InclusionCallback*>		_callbacks;
	};

	class ShadedFrequencyTree : public InclusionList::InclusionCallback
	{
	public:
		ShadedFrequencyTree(InclusionList &inclusionList) noexcept :
			InclusionCallback{inclusionList},
			_inclusionList{inclusionList}
		{
			for (uint32_t i=0;i<256U;i++) _charCounts[i]=0;
		}
		~ShadedFrequencyTree() noexcept=default;

		uint16_t operator[](uint8_t symbol) const
		{
			return _tree[symbol];
		}

		void add(uint8_t symbol,int16_t freq)
		{
			if (_inclusionList.isIncluded(symbol))
				_tree.add(symbol,freq);
			_charCounts[symbol]+=freq;
		}

		void set(uint8_t symbol,uint16_t freq)
		{
			if (_inclusionList.isIncluded(symbol))
				_tree.set(symbol,freq);
			_charCounts[symbol]=freq;
		}

		uint16_t getTotal() const noexcept
		{
			return _tree.getTotal();
		}

		void excludeAll() noexcept
		{
			_tree.onNotZero([&](uint32_t i)
			{
				_inclusionList.exclude(i);
			});
		}

		uint8_t decode(uint16_t value,uint16_t &low,uint16_t &freq) const
		{
			return _tree.decode(value,low,freq);
		}

	private:
		void symbolIncluded(uint8_t symbol) noexcept final
		{
			_tree.set(uint16_t(symbol),_charCounts[symbol]);
		}

		void symbolExcluded(uint8_t symbol) noexcept final
		{
			_tree.set(uint16_t(symbol),0);
		}

		InclusionList				&_inclusionList;
		FrequencyTree<uint16_t,uint8_t,256U>	_tree;
		std::array<uint16_t,256>		_charCounts;
	};

	// Sparse frequency map with MTF, no good data type for this one.
	// Note that there is no sense to do MTF here. It is just something
	// the original implementation did (maybe to optimize linked list traversing)
	// Worse yet, this embeds to the exclusion logic in the original.
	// By using just tree and separate MFT, the mem-usage would be in gigabytes -> lets not do that
	// No matter how I rotate this rubics cube, the star's wont align to do something nice
	// so lets do something ugly
	class ShadedSparseMTFFrequencyList
	{
	public:
		struct Node
		{
			uint16_t			freq;
			uint8_t				symbol;
		};

		ShadedSparseMTFFrequencyList(InclusionList &inclusionList) noexcept :
			_inclusionList{inclusionList}
		{
			// nothing needed
		}
		~ShadedSparseMTFFrequencyList() noexcept=default;

		uint16_t getTotal() const noexcept
		{
			uint16_t ret{0};
			for (auto &node : _nodes)
			{
				if (_inclusionList.isIncluded(node.symbol))
					ret+=node.freq;
			}
			return ret;
		}

		size_t size() const noexcept
		{
			return _nodes.size();
		}

		void excludeAll() noexcept
		{
			for (auto &node : _nodes)
				_inclusionList.exclude(node.symbol);
		}

		void scale() noexcept
		{
			for (auto it=_nodes.begin();it!=_nodes.end();)
			{
				it->freq>>=1U;
				if (!it->freq)
				{
					it = _nodes.erase(it);
				} else {
					it++;
				}
			}
		}

		Node &decode(uint16_t value,uint16_t &low,uint16_t &freq)
		{
			uint16_t tmp{0};
			for (auto it=_nodes.begin();it!=_nodes.end();it++)
			{
				auto &node{*it};
				if (_inclusionList.isIncluded(node.symbol))
				{
					if (value<tmp+node.freq)
					{
						Node nodeCopy{node};
						freq=node.freq;
						low=tmp;
						_nodes.erase(it);
						_nodes.insert(_nodes.begin(),nodeCopy);
						return _nodes.front();
					}
					tmp+=node.freq;
				}
			}
			throw Decompressor::DecompressionError();
		}

		Node &front()
		{
			return _nodes.front();
		}

		void addNew(uint8_t symbol)
		{
			_nodes.emplace(_nodes.begin(),Node{1U,symbol});
		}

	private:
		std::list<Node>				_nodes;
		InclusionList				&_inclusionList;
	};
	// After some eye-bleach we are ready to continue

	class Model
	{
	public:
		Model(RangeDecoder &decoder,InclusionList &inclusionList) noexcept :
			_decoder{decoder},
			_inclusionList{inclusionList}
		{
			// nothing needed
		}
		virtual ~Model() noexcept=default;

		virtual bool decode(uint32_t history,uint8_t history5,uint8_t &ch)=0;
		virtual void mark(uint8_t ch) noexcept=0;

	protected:
		RangeDecoder				&_decoder;
		InclusionList				&_inclusionList;
	};

	class Model2 : public Model
	{
	public:
		using contextFunc=std::tuple<uint32_t,uint16_t,uint8_t>(*)(uint32_t,uint8_t) noexcept;

		Model2(RangeDecoder &decoder,InclusionList &inclusionList,contextFunc cf) noexcept :
			Model{decoder,inclusionList},
			_cf{cf}
		{
			for (uint32_t i=0;i<32;i++) for (uint32_t j=0;j<18;j++)
			{
				_freqs[i][j]=1U;
				_totals[i][j]=j?(j<<2)+1U:2U;
			}
		}
		~Model2() noexcept=default;

		bool decode(uint32_t history,uint8_t history5,uint8_t &ch) final
		{
			auto context{_cf(history,history5)};

			auto scale=[&](Context &ctx,uint16_t total)
			{
				if (total+ctx.escapeFreq==0x4000U)
				{
					ctx.escapeFreq=(ctx.escapeFreq>>1U)+1U;
					ctx.nodes.scale();
				}
			};

			if (auto it=_contexts.find(context);it!=_contexts.end())
			{
				Context &ctx{it->second};
				if (ctx.nodes.size()==1U)
				{
					auto &node{ctx.nodes.front()};
					uint16_t count{std::min(node.freq,uint16_t{17U})};
					uint32_t index{std::get<0>(context)&0x1fU};
					if (_totals[index][count]>16300U)
					{
						_freqs[index][count]>>=1U;
						_totals[index][count]>>=1U;
						if (!_freqs[index][count])
						{
							_freqs[index][count]++;
							_totals[index][count]+=20U;
						}
					}
					if (node.freq>16300U) node.freq>>=1U;
					if (_inclusionList.isIncluded(node.symbol))
					{
						uint16_t freq{_freqs[index][count]};
						uint16_t total{_totals[index][count]};

						uint16_t value{_decoder.decode(total)};
						if (value<freq)
						{
							_decoder.scale(0,freq,total);
							_inclusionList.exclude(node.symbol);
						} else {
							_decoder.scale(freq,total,total);
							node.freq++;
							_totals[index][count]+=20U;
							ch=node.symbol;
							return true;
						}
					}
					ctx.escapeFreq++;		// does not check scale (under the limit?)
					_freqs[index][count]+=20U;
					_totals[index][count]+=20U;
					_delayedContext=context;
					_addNewContext=true;
					return false;
				}
				uint16_t total{ctx.nodes.getTotal()};
				uint16_t value{_decoder.decode(total+ctx.escapeFreq)};
				if (value<ctx.escapeFreq)
				{
					_decoder.scale(0,ctx.escapeFreq,total+ctx.escapeFreq);
					ctx.nodes.excludeAll();
					ctx.escapeFreq++;
					scale(ctx,total);
					_delayedContext=context;
					_addNewContext=true;
					return false;
				} else {
					uint16_t low,freq;
					auto &node{ctx.nodes.decode(value-ctx.escapeFreq,low,freq)};
					_decoder.scale(ctx.escapeFreq+low,ctx.escapeFreq+low+freq,total+ctx.escapeFreq);
					if (node.freq==1U && ctx.escapeFreq>1U) ctx.escapeFreq--;
					node.freq++;
					ch=node.symbol;
					scale(ctx,total+1U);
					return true;
				}
			}
			_delayedContext=context;
			_addNewContext=true;
			return false;
		}

		void mark(uint8_t ch) noexcept final
		{
			if (_addNewContext)
			{
				if (auto it=_contexts.find(_delayedContext);it!=_contexts.end())
				{
					it->second.nodes.addNew(ch);
				} else {
					_contexts.emplace(_delayedContext,Context{_inclusionList,ch});
				}
				_addNewContext=false;
			}
		}

	private:
		struct Context
		{
			Context(InclusionList &inclusionList,uint8_t ch) noexcept :
				escapeFreq{1U},
				nodes{inclusionList}
			{
				nodes.addNew(ch);
			};
			~Context() noexcept=default;

			uint16_t			escapeFreq;
			ShadedSparseMTFFrequencyList	nodes;
		};

		contextFunc				_cf;
		bool					_addNewContext=false;
		std::tuple<uint32_t,uint16_t,uint8_t>	_delayedContext;
		std::map<std::tuple<uint32_t,uint16_t,uint8_t>,Context> _contexts;
		std::array<std::array<uint16_t,18>,32>	_freqs;
		std::array<std::array<uint16_t,18>,32>	_totals;
	};

	class Model1 : public Model
	{
	public:
		using contextFunc=std::pair<uint32_t,uint16_t>(*)(uint32_t) noexcept;

		Model1(RangeDecoder &decoder,InclusionList &inclusionList,contextFunc cf) noexcept :
			Model{decoder,inclusionList},
			_cf{cf}
		{
			// nothing needed
		}
		~Model1() noexcept=default;

		bool decode(uint32_t history,uint8_t history5,uint8_t &ch) final
		{
			auto context{_cf(history)};

			auto scale=[&](Context &ctx,uint16_t total)
			{
				if (total+ctx.escapeFreq==0x4000U)
				{
					ctx.escapeFreq=(ctx.escapeFreq>>1U)+1U;
					ctx.nodes.scale();
				}
			};

			if (auto it=_contexts.find(context);it!=_contexts.end())
			{
				Context &ctx{it->second};
				uint16_t total{ctx.nodes.getTotal()};
				uint16_t value{_decoder.decode(total+ctx.escapeFreq)};
				if (value<ctx.escapeFreq)
				{
					_decoder.scale(0,ctx.escapeFreq,total+ctx.escapeFreq);
					ctx.nodes.excludeAll();
					ctx.escapeFreq++;
					scale(ctx,total);
					_delayedContext=context;
					_addNewContext=true;
					return false;
				} else {
					uint16_t low,freq;
					auto &node{ctx.nodes.decode(value-ctx.escapeFreq,low,freq)};
					_decoder.scale(ctx.escapeFreq+low,ctx.escapeFreq+low+freq,total+ctx.escapeFreq);
					if (node.freq==1U && ctx.escapeFreq>1U) ctx.escapeFreq--;
					node.freq++;
					ch=node.symbol;
					scale(ctx,total+1U);
					return true;
				}
			}
			_delayedContext=context;
			_addNewContext=true;
			return false;
		}

		void mark(uint8_t ch) noexcept final
		{
			if (_addNewContext)
			{
				if (auto it=_contexts.find(_delayedContext);it!=_contexts.end())
				{
					it->second.nodes.addNew(ch);
				} else {
					_contexts.emplace(_delayedContext,Context{_inclusionList,ch});
				}
				_addNewContext=false;
			}
		}

	private:
		struct Context
		{
			Context(InclusionList &inclusionList,uint8_t ch) noexcept :
				escapeFreq{1U},
				nodes{inclusionList}
			{
				nodes.addNew(ch);
			};
			~Context() noexcept=default;

			uint16_t			escapeFreq;
			ShadedSparseMTFFrequencyList	nodes;
		};

		contextFunc				_cf;
		bool					_addNewContext=false;
		std::pair<uint32_t,uint16_t>		_delayedContext;
		std::map<std::pair<uint32_t,uint16_t>,Context> _contexts;
	};

	// A simple arithmetic encoder (but with sparse array)
	// Fallback built in, since they have unhealthy dependency to this model
	class Model0 : public Model
	{
	public:
		Model0(RangeDecoder &decoder,InclusionList &inclusionList) noexcept :
			Model{decoder,inclusionList},
			_tree{inclusionList}
		{
			// nothing needed
		}

		~Model0() noexcept=default;

		bool decode(uint32_t history,uint8_t history5,uint8_t &ch) final
		{
			uint16_t value{_decoder.decode(_tree.getTotal()+_escapeFreq)};
			if (value<_escapeFreq)
			{
				_decoder.scale(0,_escapeFreq,_tree.getTotal()+_escapeFreq);

				// since we could not decode it, it is no symbol we know and
				// we can exclude them all
				_tree.excludeAll();

				decodeFallback(ch);
			} else {
				uint16_t low,freq;
				uint8_t symbol{_tree.decode(value-_escapeFreq,low,freq)};
				_decoder.scale(_escapeFreq+low,_escapeFreq+low+freq,_tree.getTotal()+_escapeFreq);

				switch (_tree[symbol])
				{
					case 0:
					_escapeFreq++;
					break;

					case 1:
					 if (_escapeFreq>1U) _escapeFreq--;
					break;

					default:
					// nothing needed
					break;
				}
				_tree.add(symbol,1);
				ch=symbol;
			}
			return true;
		}

		void mark(uint8_t ch) noexcept final
		{
			// nothing needed
		}

	private:
		void decodeFallback(uint8_t &ch)
		{
			uint16_t value{_decoder.decode(_inclusionList.getTotal())};
			uint16_t low,freq;
			ch=_inclusionList.decode(value,low,freq);
			_decoder.scale(low,low+freq,_inclusionList.getTotal());

			_tree.set(uint16_t(ch),1U);
			_escapeFreq++;
		}

		uint16_t				_escapeFreq{1U};
		ShadedFrequencyTree			_tree;
	};

	InclusionList inclusionList;

	// Different sources put different names on the models: PPM, PPMI etc. with different contexts
	// Also, nothing seems to be exactly same as some public sources. The xpk library is "special"
	// -> I just call them with numbers and letters so I don't get confused
	Model2 model2A{decoder,inclusionList,[](uint32_t c,uint8_t c5)noexcept{return std::make_tuple(c,uint16_t(c^(c>>15U)),c5);}};
	Model2 model2B{decoder,inclusionList,[](uint32_t c,uint8_t c5)noexcept{return std::make_tuple(c,uint16_t(c^(c>>15U)),uint8_t{0});}};
	Model1 model1A{decoder,inclusionList,[](uint32_t c)noexcept{return std::make_pair(c&0xff'ffffU,uint16_t(c^(c>>7U)));}};
	Model1 model1B{decoder,inclusionList,[](uint32_t c)noexcept{return std::make_pair(c&0xffffU,uint16_t(c&0xffffU));}};
	Model1 model1C{decoder,inclusionList,[](uint32_t c)noexcept{return std::make_pair(c&0xffU,uint16_t(c&0xffU));}};
	Model0 model0{decoder,inclusionList};
	const std::array<Model*,6> models{&model2A,&model2B,&model1A,&model1B,&model1C,&model0};

	while (!outputStream.eof())
	{
		inclusionList.reset();
		for (auto model : models)
		{
			uint8_t ch;
			if (model->decode(history,history5,ch))
			{
				for (auto backModel : models)
				{
					if (backModel==model) break;
					backModel->mark(ch);
				}
				addToHistory(ch);
				outputStream.writeByte(ch);
				break;
			}
		}
	}
}

}
