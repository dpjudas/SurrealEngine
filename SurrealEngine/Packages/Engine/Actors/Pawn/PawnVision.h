#pragma once

#include "Math/vec.h"

class PawnVision
{
public:
	static inline bool IsWithinVisionCone(const vec3& observerLocation, const vec3& targetLocation, const vec3& viewDirection, float peripheralVision)
	{
		if (!std::isfinite(peripheralVision) ||
			!std::isfinite(observerLocation.x) || !std::isfinite(observerLocation.y) || !std::isfinite(observerLocation.z) ||
			!std::isfinite(targetLocation.x) || !std::isfinite(targetLocation.y) || !std::isfinite(targetLocation.z) ||
			!std::isfinite(viewDirection.x) || !std::isfinite(viewDirection.y) || !std::isfinite(viewDirection.z))
		{
			return false;
		}

		const double toTargetX = static_cast<double>(targetLocation.x) - observerLocation.x;
		const double toTargetY = static_cast<double>(targetLocation.y) - observerLocation.y;
		const double toTargetZ = static_cast<double>(targetLocation.z) - observerLocation.z;
		const double targetLengthSquared =
			toTargetX * toTargetX +
			toTargetY * toTargetY +
			toTargetZ * toTargetZ;
		const double viewLengthSquared =
			static_cast<double>(viewDirection.x) * viewDirection.x +
			static_cast<double>(viewDirection.y) * viewDirection.y +
			static_cast<double>(viewDirection.z) * viewDirection.z;
		const double minimumLengthSquared = static_cast<double>(FLT_EPSILON) * FLT_EPSILON;
		if (targetLengthSquared <= minimumLengthSquared || viewLengthSquared <= minimumLengthSquared)
			return false;

		const double directionDot =
			static_cast<double>(viewDirection.x) * toTargetX +
			static_cast<double>(viewDirection.y) * toTargetY +
			static_cast<double>(viewDirection.z) * toTargetZ;
		const float cosine = static_cast<float>(std::clamp(
			directionDot / (std::sqrt(viewLengthSquared) * std::sqrt(targetLengthSquared)),
			-1.0,
			1.0));
		return cosine >= peripheralVision;
	}
};
