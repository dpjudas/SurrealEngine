/*
 * Tagging.h
 * ---------
 * Purpose: Structure holding a superset of tags for all supported output sample or stream files or types.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "openmpt/soundfile_data/tags.hpp"


OPENMPT_NAMESPACE_BEGIN




mpt::ustring GetSampleNameFromTags(const FileTags &tags);


OPENMPT_NAMESPACE_END
