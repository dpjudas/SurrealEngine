/*
 * PluginMixBuffer.h
 * -----------------
 * Purpose: Helper class for managing plugin audio input and output buffers.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include <algorithm>
#include <array>

#if defined(MPT_ENABLE_ARCH_INTRINSICS) || defined(MPT_WITH_VST)
#include "mpt/base/aligned_array.hpp"
#endif // MPT_ENABLE_ARCH_INTRINSICS || MPT_WITH_VST


OPENMPT_NAMESPACE_BEGIN


// At least this part of the code is ready for double-precision rendering... :>
// buffer_t: Sample buffer type (float, double, ...)
// bufferSize: Buffer size in samples
template<typename buffer_t, uint32 bufferSize>
class PluginMixBuffer
{

private:

#if defined(MPT_ENABLE_ARCH_INTRINSICS) || defined(MPT_WITH_VST)
	static constexpr std::align_val_t alignment = std::align_val_t{16};
	static_assert(sizeof(mpt::aligned_array<buffer_t, bufferSize, alignment>) == sizeof(std::array<buffer_t, bufferSize>));
	static_assert(alignof(mpt::aligned_array<buffer_t, bufferSize, alignment>) == static_cast<std::size_t>(alignment));
#endif // MPT_ENABLE_ARCH_INTRINSICS || MPT_WITH_VST

protected:

#if defined(MPT_ENABLE_ARCH_INTRINSICS) || defined(MPT_WITH_VST)
	std::vector<mpt::aligned_array<buffer_t, bufferSize, alignment>> inputs;
	std::vector<mpt::aligned_array<buffer_t, bufferSize, alignment>> outputs;
#else // !(MPT_ENABLE_ARCH_INTRINSICS || MPT_WITH_VST)
	std::vector<std::array<buffer_t, bufferSize>> inputs;
	std::vector<std::array<buffer_t, bufferSize>> outputs;
#endif // MPT_ENABLE_ARCH_INTRINSICS || MPT_WITH_VST
	std::vector<buffer_t*> inputsarray;
	std::vector<buffer_t*> outputsarray;

public:

	// Allocate input and output buffers
	bool Initialize(uint32 numInputs, uint32 numOutputs)
	{
		// Short cut - we do not need to recreate the buffers.
		if(inputs.size() == numInputs && outputs.size() == numOutputs)
		{
			return true;
		}

		try
		{
			inputs.resize(numInputs);
			outputs.resize(numOutputs);
			inputsarray.resize(numInputs);
			outputsarray.resize(numOutputs);
		} catch(mpt::out_of_memory e)
		{
			mpt::delete_out_of_memory(e);
			inputs.clear();
			inputs.shrink_to_fit();
			outputs.clear();
			outputs.shrink_to_fit();
			inputsarray.clear();
			inputsarray.shrink_to_fit();
			outputsarray.clear();
			outputsarray.shrink_to_fit();
			return false;
		}

		for(uint32 i = 0; i < numInputs; i++)
		{
			inputsarray[i] = inputs[i].data();
		}

		for(uint32 i = 0; i < numOutputs; i++)
		{
			outputsarray[i] = outputs[i].data();
		}

		return true;
	}

	// Silence all input buffers.
	void ClearInputBuffers(uint32 numSamples)
	{
		MPT_ASSERT(numSamples <= bufferSize);
		for(size_t i = 0; i < inputs.size(); i++)
		{
			std::fill(inputs[i].data(), inputs[i].data() + numSamples, buffer_t{0});
		}
	}

	// Silence all output buffers.
	void ClearOutputBuffers(uint32 numSamples)
	{
		MPT_ASSERT(numSamples <= bufferSize);
		for(size_t i = 0; i < outputs.size(); i++)
		{
			std::fill(outputs[i].data(), outputs[i].data() + numSamples, buffer_t{0});
		}
	}

	PluginMixBuffer()
	{
		Initialize(2, 0);
	}

	// Return pointer to a given input or output buffer
	const buffer_t *GetInputBuffer(uint32 index) const { return inputs[index].data(); }
	const buffer_t *GetOutputBuffer(uint32 index) const { return outputs[index].data(); }
	buffer_t *GetInputBuffer(uint32 index) { return inputs[index].data(); }
	buffer_t *GetOutputBuffer(uint32 index) { return outputs[index].data(); }

	// Return pointer array to all input or output buffers
	buffer_t **GetInputBufferArray() { return inputs.empty() ? nullptr : inputsarray.data(); }
	buffer_t **GetOutputBufferArray() { return outputs.empty() ? nullptr : outputsarray.data(); }

	bool Ok() const { return (inputs.size() + outputs.size()) > 0; }

};


OPENMPT_NAMESPACE_END
