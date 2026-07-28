
#include "Precomp.h"
#include "UActor.h"
#include "VM/ScriptCall.h"

void UActor::TickRotating(float elapsed)
{
	if (bRotateToDesired())
	{
		if (Rotation() != DesiredRotation())
		{
			Rotator rot = Rotation();
			if (bFixedRotationDir())
			{
				rot.Yaw = Rotator::TurnToFixed(rot.Yaw, DesiredRotation().Yaw, (int)(RotationRate().Yaw * elapsed));
				rot.Pitch = Rotator::TurnToFixed(rot.Pitch, DesiredRotation().Pitch, (int)(RotationRate().Pitch * elapsed));
				rot.Roll = Rotator::TurnToFixed(rot.Roll, DesiredRotation().Roll, (int)(RotationRate().Roll * elapsed));
			}
			else
			{
				rot.Yaw = Rotator::TurnToShortest(rot.Yaw, DesiredRotation().Yaw, (int)std::abs(RotationRate().Yaw * elapsed));
				rot.Pitch = Rotator::TurnToShortest(rot.Pitch, DesiredRotation().Pitch, (int)std::abs(RotationRate().Pitch * elapsed));
				rot.Roll = Rotator::TurnToShortest(rot.Roll, DesiredRotation().Roll, (int)std::abs(RotationRate().Roll * elapsed));
			}
			Rotation() = rot;

			if (Rotation() == DesiredRotation())
			{
				CallEvent(this, EventName::EndedRotation);
			}
		}
	}
	else if (bFixedRotationDir())
	{
		Rotation() += RotationRate() * elapsed;
	}
}
