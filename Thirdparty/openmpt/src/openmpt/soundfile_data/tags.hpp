/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/string/types.hpp"



OPENMPT_NAMESPACE_BEGIN



struct FileTags
{

	mpt::ustring encoder;

	mpt::ustring title;
	mpt::ustring comments;

	mpt::ustring bpm;

	mpt::ustring artist;
	mpt::ustring album;
	mpt::ustring trackno;
	mpt::ustring year;
	mpt::ustring url;

	mpt::ustring genre;
};



OPENMPT_NAMESPACE_END
