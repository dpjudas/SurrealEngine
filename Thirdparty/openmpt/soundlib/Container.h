/*
 * Container.h
 * -----------
 * Purpose: General interface for MDO container and/or packers.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "../common/FileReader.h"

#include <vector>

OPENMPT_NAMESPACE_BEGIN


struct ContainerItem
{
	mpt::ustring name;
	FileReader file;
	std::unique_ptr<std::vector<char> > data_cache; // may be empty
};


enum ContainerLoadingFlags
{
	ContainerOnlyVerifyHeader = 0x00,
	ContainerUnwrapData       = 0x01,
};


#if !defined(MPT_WITH_ANCIENT)
bool UnpackXPK(std::vector<ContainerItem> &containerItems, FileReader &file, ContainerLoadingFlags loadFlags);
bool UnpackPP20(std::vector<ContainerItem> &containerItems, FileReader &file, ContainerLoadingFlags loadFlags);
bool UnpackMMCMP(std::vector<ContainerItem> &containerItems, FileReader &file, ContainerLoadingFlags loadFlags);
#endif // !MPT_WITH_ANCIENT
bool UnpackUMX(std::vector<ContainerItem> &containerItems, FileReader &file, ContainerLoadingFlags loadFlags);


OPENMPT_NAMESPACE_END
