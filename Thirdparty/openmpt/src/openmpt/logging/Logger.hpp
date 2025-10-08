/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/source_location.hpp"
#include "mpt/string/types.hpp"

OPENMPT_NAMESPACE_BEGIN

enum LogLevel
{
	LogDebug = 5,
	LogInformation = 4,
	LogNotification = 3,
	LogWarning = 2,
	LogError = 1
};

class ILogger
{
protected:
	virtual ~ILogger() = default;

public:
	virtual bool IsLevelActive(LogLevel level) const noexcept = 0;
	// facility: ASCII
	virtual bool IsFacilityActive(const char *facility) const noexcept = 0;
	// facility: ASCII
	virtual void SendLogMessage(const mpt::source_location &loc, LogLevel level, const char *facility, const mpt::ustring &text) const = 0;
};

#define MPT_LOG(logger, level, facility, text) \
	do \
	{ \
		if((logger).IsLevelActive((level))) \
		{ \
			if((logger).IsFacilityActive((facility))) \
			{ \
				(logger).SendLogMessage(MPT_SOURCE_LOCATION_CURRENT(), (level), (facility), (text)); \
			} \
		} \
	} while(0)

OPENMPT_NAMESPACE_END
