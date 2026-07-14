
#include "Precomp.h"
#include "VRSubsystem.h"

#ifdef USE_OPENXR
#include "OpenXRSubsystem.h"
#endif

std::unique_ptr<VRSubsystem> VRSubsystem::Create(bool enabled)
{
#ifdef USE_OPENXR
	if (enabled)
	{
		auto vr = std::make_unique<OpenXRSubsystem>();
		if (vr->CreateInstance())
			return vr;
	}
#endif
	return std::make_unique<NullVRSubsystem>();
}
