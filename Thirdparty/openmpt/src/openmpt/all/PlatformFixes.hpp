/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once



#include "openmpt/all/BuildSettings.hpp"



// immediate quirk fixups

#if defined(MPT_LIBC_QUIRK_REQUIRES_SYS_TYPES_H)
#include <sys/types.h>
#endif
