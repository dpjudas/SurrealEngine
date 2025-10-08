/* Copyright (C) Teemu Suutari */

#include "SubBuffer.hpp"


namespace ancient::internal
{

template <>
uint8_t *GenericSubBuffer<Buffer>::data()
{
	return _base.data()+_start;
}

template <>
uint8_t *GenericSubBuffer<const Buffer>::data()
{
	throw InvalidOperationError();
}

}
