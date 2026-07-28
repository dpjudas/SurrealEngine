
#include "Precomp.h"
#include "UActor.h"
#include "Utils/Logger.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Packages/Engine/Resources/Mesh/USkeletalMesh.h"
#include "Engine.h"

bool UActor::HasAnim(const NameString& sequence)
{
	return Mesh() && Mesh()->GetSequence(sequence);
}

bool UActor::IsAnimating()
{
	return AnimRate() != 0.0f;
}

bool UActor::IsAnimating_HP(std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.IsAnimating_HP");
	return IsAnimating();
}

void UActor::FinishAnim()
{
	if (bAnimLoop())
	{
		bAnimLoop() = false;
		bAnimFinished() = false;
	}

	if (StateFrame)
		StateFrame->LatentState = LatentRunState::FinishAnim;
}

void UActor::FinishAnim_HP(std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.FinishAnim_HP");
	FinishAnim();
}

NameString UActor::GetAnimGroup(const NameString& sequence)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
			return seq->Group;
	}
	return {};
}

// UnrealScript variables controlling animation:
// 
// Tweening means animating (using vertex interpolation) from the last animation's frame to the current animation's first frame
//
// Mesh          - the mesh the animation belongs to
// AnimSequence  - current active animation sequence
// AnimFrame     - how far we've gotten in an animation 0.0 to 1.0 for current animation, negative for interpolation from old animation when tweening
// AnimLast      - end point for AnimFrame (when to stop/loop). It is zero when only tweening (don't play the animation). It is the start of the last frame (1-1/numframes) when playing an animation
// AnimRate      - how far AnimFrame moves in 1 second (AnimFrame += AnimRate * timeElapsed). If negative it is a scale factor used to convert Velocity length to animation speed
// AnimMinRate   - the minimum animation speed when AnimRate is negative (negative AnimRate means it should use length(Velocity) * abs(AnimRate) as the anim speed)
// TweenRate     - how fast to move when AnimFrame is negative (AnimFrame += TweenRate * timeElapsed)
// OldAnimRate   - AnimRate from previous call to PlayAnim/LoopAnim/TweenAnim
// bAnimLoop     - true if the animation should loop when AnimLast is reached
// bAnimNotify   - true if animation notify events should be fired when animating
// bAnimFinished - true if AnimLast was reached and there's no looping

void UActor::PlayAnim(const NameString& sequence, float rate, float tweenTime)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			SetTweenFromAnimFrame();

			AnimSequence() = sequence;

			if (seq->NumFrames > 1)
			{
				AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
				AnimLast() = 1.0f - 1.0f / seq->NumFrames;
				AnimRate() = rate * seq->Rate / seq->NumFrames;
				TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
				bAnimNotify() = !seq->Notifys.empty();
				OldAnimRate() = AnimRate();
			}
			else
			{
				// Special case for 1 frame animations. Simply keep drawing the animation for 0.1 second (or tween duration, if tweening).

				AnimFrame() = -1.0f;
				AnimLast() = 0.0f;
				AnimRate() = 0.0f;
				TweenRate() = tweenTime > 0.0f ? 1.0f / tweenTime : 10.0f;
				bAnimNotify() = false;
				OldAnimRate() = 0.0f;
				AnimMinRate() = 0.0f;
			}

			bAnimLoop() = false;
			bAnimFinished() = false;
		}
	}
}

void UActor::PlayBlendAnim(const NameString& sequenceName, float rate, float tweenTime, int blendSlot)
{
	LogUnimplemented("Actor.PlayBlendAnim");
	if (blendSlot < 0 || blendSlot > 3)
	{
		LogMessage("Invalid channel for PlayBlendAnim!");
		return;
	}
	if (!Mesh())
	{
		LogMessage("No mesh for PlayBlendAnim");
		return;
	}

	MeshAnimSeq* sequence = Mesh()->GetSequence(sequenceName);
	if (!sequence)
	{
		LogMessage("Sequence not found for PlayBlendAnim");
		return;
	}

	int numFrames = sequence->NumFrames;
	float sequenceRate = sequence->Rate;

	SetTweenFromBlendAnimFrame(blendSlot);

	BlendAnimSequence()[blendSlot] = sequenceName;

	BlendAnimFrame()[blendSlot] = -1.0f / numFrames;

	BlendAnimRate()[blendSlot] = (rate * sequenceRate) / numFrames;

	BlendAnimLast()[blendSlot] = 1.0f - (1.0f / numFrames);

	if (BlendAnimLast()[blendSlot] == 0.0f)
	{
		BlendAnimRate()[blendSlot] = 0.0f;
		BlendAnimFrame()[blendSlot] = 0.0f;

		BlendTweenRate()[blendSlot] = (tweenTime <= 0.0f) ? 10.0f : (1.0f / tweenTime);
	}
	else if (tweenTime <= 0.0f)
	{
		if (tweenTime == -1.0f)
		{
			BlendAnimFrame()[blendSlot] = 0.0f;
			if (BlendAnimMinRate()[blendSlot] <= 0.0f)
			{
				if (BlendAnimMinRate()[blendSlot] == 0.0f)
				{
					BlendTweenRate()[blendSlot] = 1.0f / (numFrames * 0.025f);
				}
				else
				{
					float speed = length(Velocity());
					float computed = speed * (-BlendAnimMinRate()[blendSlot]);
					float minVal = BlendAnimRate()[blendSlot] * 0.5f;

					BlendTweenRate()[blendSlot] = std::max(computed, minVal);
				}
			}
			else
			{
				BlendTweenRate()[blendSlot] = BlendAnimMinRate()[blendSlot];
			}
		}
		else
		{
			BlendTweenRate()[blendSlot] = 0.0f;
			BlendAnimFrame()[blendSlot] = 0.001f;
		}
	}
	else
	{
		BlendTweenRate()[blendSlot] = 1.0f / (numFrames * tweenTime);
	}

	float oldX = SimBlendAnim()[blendSlot].x;
	float oldY = SimBlendAnim()[blendSlot].y;
	float oldZ = SimBlendAnim()[blendSlot].z;
	float oldW = SimBlendAnim()[blendSlot].w;

	SimBlendAnim()[blendSlot].z = BlendAnimFrame()[blendSlot] * 10000.0f;
	SimBlendAnim()[blendSlot].w = BlendAnimRate()[blendSlot] * 10000.0f;
	SimBlendAnim()[blendSlot].x = BlendTweenRate()[blendSlot] * 1000.0f;
	SimBlendAnim()[blendSlot].y = BlendAnimLast()[blendSlot] * 10000.0f;

	if (oldZ == SimBlendAnim()[blendSlot].z && oldW == SimBlendAnim()[blendSlot].w && oldX == SimBlendAnim()[blendSlot].x && oldY == SimBlendAnim()[blendSlot].y)
	{
		SimBlendAnim()[blendSlot].y += 1.0f;
	}

	OldBlendAnimRate()[blendSlot] = BlendAnimRate()[blendSlot];
}

void UActor::TweenBlendAnim(const NameString& sequenceName, float time, int blendSlot)
{
	if (blendSlot < 0 || blendSlot > 3)
	{
		LogMessage("Invalid channel for TweenBlendAnim!");
		return;
	}
	if (!Mesh())
	{
		LogMessage("No mesh for TweenBlendAnim");
		return;
	}

	MeshAnimSeq* sequence = Mesh()->GetSequence(sequenceName);
	if (!sequence || sequence->Name != sequenceName)
	{
		LogMessage("TweenBlendAnim: Sequence '" + sequenceName.ToString() + "' not found in mesh for slot " + std::to_string(blendSlot));
		return;
	}
	int numFrames = sequence->NumFrames;
	LogMessage("TweenBlendAnim: seq='" + sequenceName.ToString() + "' slot=" + std::to_string(blendSlot) + " time=" + std::to_string(time) + " numFrames=" + std::to_string(numFrames) + " StartFrame=" + std::to_string(sequence->StartFrame));

	BlendAnimSequence()[blendSlot] = sequenceName;
	BlendAnimLast()[blendSlot] = 0.0;
	BlendAnimMinRate()[blendSlot] = 0.0;
	BlendAnimRate()[blendSlot] = 0.0;
	OldBlendAnimRate()[blendSlot] = 0.0;
	if (time <= 0.0)
	{
		BlendTweenRate()[blendSlot] = 0.0;
		BlendAnimFrame()[blendSlot] = 0.0;
	}
	else
	{
		BlendTweenRate()[blendSlot] = 1.0f / (numFrames * time);
		BlendAnimFrame()[blendSlot] = 1.0f / numFrames;
	}
	// Don't worry about simblendanim for now
	return;
}

void UActor::LoopAnim(const NameString& sequence, float rate, float tweenTime, float minRate)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			if (AnimSequence() == sequence && IsAnimating() && bAnimLoop())
			{
				if (seq->NumFrames > 1)
				{
					AnimRate() = rate * seq->Rate / seq->NumFrames;
					AnimMinRate() = minRate * seq->Rate / seq->NumFrames;
					TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
					OldAnimRate() = AnimRate();
				}
			}
			else
			{
				SetTweenFromAnimFrame();

				AnimSequence() = sequence;
				if (seq->NumFrames > 1)
				{
					AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
					AnimLast() = 1.0f - 1.0f / seq->NumFrames;
					bAnimNotify() = !seq->Notifys.empty();
					AnimRate() = rate * seq->Rate / seq->NumFrames;
					AnimMinRate() = minRate * seq->Rate / seq->NumFrames;
					TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
					OldAnimRate() = AnimRate();
				}
				else
				{
					// Special case for 1 frame animations. Simply keep drawing the animation for 0.1 second (or tween duration, if tweening).

					AnimFrame() = -1.0f;
					AnimLast() = 0.0f;
					AnimRate() = 0.0f;
					TweenRate() = tweenTime > 0.0f ? 1.0f / tweenTime : 10.0f;
					bAnimNotify() = false;
					OldAnimRate() = 0.0f;
					AnimMinRate() = 0.0f;
				}
				bAnimFinished() = false;
				bAnimLoop() = true;
			}
		}
	}
}

void UActor::TweenAnim(const NameString& sequence, float tweenTime)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			SetTweenFromAnimFrame();

			AnimSequence() = sequence;
			AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
			AnimLast() = 0.0f;
			AnimRate() = 0.0f;
			AnimMinRate() = 0.0f;
			TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
			OldAnimRate() = AnimRate();
			bAnimNotify() = false;
			bAnimFinished() = false;
			bAnimLoop() = false;
		}
	}
}

void UActor::PlayAnim_HP(const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<EAnimType> Type, std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.PlayAnim_HP");
}

void UActor::LoopAnim_HP(const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<float> MinRate, std::optional<EAnimType> Type, std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.LoopAnim_HP");
}

void UActor::TickAnimation(float elapsed)
{
	if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
	{
		if (!IsAnimating() || AnimFrame() >= AnimLast())
			StateFrame->LatentState = LatentRunState::Continue;
	}

	for (int i = 0; elapsed > 0.0f && i < 10; i++)
	{
		// If AnimFrame is positive we are doing a normal animation. If it is negative we are doing a tween animation.
		float fromAnimTime = AnimFrame();
		if (fromAnimTime >= 0.0f)
		{
			// If AnimRate is positive we are animating at a fixed rate. If it is negative we animate based on velocity (using AnimRate as a speed scale factor)
			float animRate = (AnimRate() >= 0) ? AnimRate() : std::max(AnimMinRate(), -AnimRate() * length(Velocity()));
			if (animRate == 0.0f)
				break;

			// Find what time will we be at the end of the animation
			float toAnimTime = fromAnimTime + animRate * elapsed;

			// Stop at the next notify event, if any
			if (Mesh() && bAnimNotify())
			{
				MeshAnimSeq* seq = Mesh()->GetSequence(AnimSequence());
				if (seq)
				{
					bool foundEvent = false;
					for (const MeshAnimNotify& n : seq->Notifys)
					{
						if (n.Time > fromAnimTime && n.Time <= toAnimTime)
						{
							if (FindEventFunction(this, n.Function))
							{
								toAnimTime = n.Time;
								elapsed -= (toAnimTime - fromAnimTime) / animRate;
								AnimFrame() = toAnimTime;
								foundEvent = true;
								CallEvent(this, n.Function);
								break;
							}
						}
					}
					if (foundEvent)
						continue;
				}
			}

			// Looped animations got their AnimEnd notify event at the AnimLast point, NOT when the loop finishes!
			if (bAnimLoop() && AnimLast() > fromAnimTime && AnimLast() <= toAnimTime)
			{
				toAnimTime = AnimLast();
				elapsed -= (toAnimTime - fromAnimTime) / animRate;
				AnimFrame() = toAnimTime;

				if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
					StateFrame->LatentState = LatentRunState::Continue;

				CallEvent(this, EventName::AnimEnd);
				continue;
			}

			// Clamp elapsed time to the animation end. This differs for looping animations as they also have to take the last frame into account before looping.
			float animEndTime = bAnimLoop() ? 1.0f : AnimLast();
			if (toAnimTime < fromAnimTime) // This can happen if FinishAnim is called after a looping animation made it past the AnimLast point
			{
				toAnimTime = fromAnimTime;
				animEndTime = fromAnimTime;
				elapsed = 0.0f;
			}
			else if (toAnimTime >= animEndTime)
			{
				elapsed -= (animEndTime - fromAnimTime) / animRate;
				toAnimTime = animEndTime;
			}
			else
			{
				elapsed = 0.0f;
			}

			AnimFrame() = toAnimTime;

			if (toAnimTime == animEndTime)
			{
				if (bAnimLoop())
				{
					AnimFrame() = 0.0f;
				}
				else
				{
					AnimRate() = 0.0f;
					bAnimFinished() = true;
				}
			}

			if (!bAnimLoop() && fromAnimTime < animEndTime && toAnimTime >= animEndTime)
			{
				if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
					StateFrame->LatentState = LatentRunState::Continue;

				CallEvent(this, EventName::AnimEnd);
			}
		}
		else
		{
			float tweenRate = TweenRate();
			if (tweenRate == 0.0f)
				break;

			float toAnimTime = fromAnimTime + tweenRate * elapsed;

			float animEndTime = 0.0f;
			if (toAnimTime >= animEndTime)
			{
				elapsed -= (animEndTime - fromAnimTime) / tweenRate;
				toAnimTime = animEndTime;
			}
			else
			{
				elapsed = 0.0f;
			}

			AnimFrame() = toAnimTime;

			if (toAnimTime == animEndTime && AnimRate() == 0.0f)
			{
				if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
					StateFrame->LatentState = LatentRunState::Continue;

				bAnimFinished() = true;
				//engine->LogMessage("CallEvent(AnimEnd) for " + Class->FriendlyName.ToString() + "");
				CallEvent(this, EventName::AnimEnd);
			}
		}
	}
}

void UActor::TickBlendAnimation(float elapsed)
{
	for (int i = 0; elapsed > 0.0f && i < 4; i++)
	{
		if (BlendAnimSequence()[i].IsNone())
			continue;

		if (BlendAnimFrame()[i] >= BlendAnimLast()[i])
			continue;

		float oldFrame = BlendAnimFrame()[i];

		if (BlendAnimFrame()[i] < 0.0f)
		{
			BlendAnimFrame()[i] += elapsed * BlendTweenRate()[i];

			if (BlendAnimFrame()[i] < 0.0f)
				continue;

			BlendAnimFrame()[i] = 0.0f;

			elapsed = (BlendAnimFrame()[i] * elapsed) / (BlendAnimFrame()[i] - oldFrame);
			continue;
		}

		if (BlendAnimRate()[i] < 0.0f)
		{
			float speed = length(Velocity());

			float adjustedRate = -speed * BlendAnimRate()[i];

			float minRate = BlendAnimLast()[i];
			if (adjustedRate > minRate)
				adjustedRate = minRate;

			BlendAnimFrame()[i] += adjustedRate * elapsed;
		}
		else
		{
			BlendAnimFrame()[i] += BlendAnimRate()[i] * elapsed;
		}

		if (BlendAnimFrame()[i] >= BlendAnimLast()[i])
		{
			float endFrame = BlendAnimLast()[i];

			BlendAnimFrame()[i] = endFrame;
			BlendAnimRate()[i] = 0.0f;

			elapsed = ((BlendAnimFrame()[i] - endFrame) * elapsed) / (BlendAnimFrame()[i] - oldFrame);

			if (RemoteRole() < ENetRole::ROLE_SimulatedProxy)
			{
				SimBlendAnim()[i].z = BlendAnimFrame()[i] * 10000.0f;

				float rate = BlendAnimRate()[i] * 5000.0f;
				if (rate > 32767.0f)
					rate = 32767.0f;

				SimBlendAnim()[i].w = rate;
			}
		}
	}
}

void UActor::SetTweenFromAnimFrame()
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(AnimSequence());
		if (seq)
		{
			float animFrame = std::max(AnimFrame(), 0.0f) * seq->NumFrames;
			int frame0 = (int)animFrame;
			int frame1 = frame0 + 1;
			frame0 = frame0 % seq->NumFrames;
			frame1 = frame1 % seq->NumFrames;
			TweenFromAnimFrame.V0 = (seq->StartFrame + frame0) * Mesh()->FrameVerts;
			TweenFromAnimFrame.V1 = (seq->StartFrame + frame1) * Mesh()->FrameVerts;
			TweenFromAnimFrame.T = animFrame - (float)frame0;
		}
		else // For safety. Should never happen.
		{
			TweenFromAnimFrame.V0 = 0;
			TweenFromAnimFrame.V1 = 0;
			TweenFromAnimFrame.T = -1.0f;
		}
	}
}

void UActor::SetTweenFromBlendAnimFrame(int slot)
{
	if (slot < 0 || slot > 3)
		return;

	if (!Mesh())
		return;

	if (BlendAnimSequence()[slot].IsNone())
	{
		TweenFromBlendAnimFrame[slot].V0 = 0;
		TweenFromBlendAnimFrame[slot].V1 = 0;
		TweenFromBlendAnimFrame[slot].T = -1.0f;
		return;
	}

	MeshAnimSeq* seq = Mesh()->GetSequence(BlendAnimSequence()[slot]);
	if (seq)
	{
		float frame = std::max(BlendAnimFrame()[slot], 0.0f) * seq->NumFrames;
		int frame0 = (int)frame;
		int frame1 = frame0 + 1;
		frame0 = frame0 % seq->NumFrames;
		frame1 = frame1 % seq->NumFrames;
		TweenFromBlendAnimFrame[slot].V0 = (seq->StartFrame + frame0) * Mesh()->FrameVerts;
		TweenFromBlendAnimFrame[slot].V1 = (seq->StartFrame + frame1) * Mesh()->FrameVerts;
		TweenFromBlendAnimFrame[slot].T = frame - (float)frame0;
	}
	else
	{
		TweenFromBlendAnimFrame[slot].V0 = 0;
		TweenFromBlendAnimFrame[slot].V1 = 0;
		TweenFromBlendAnimFrame[slot].T = -1.0f;
	}
}

UActor* UActor::CreateAnimChannel(UClass* NewClass, EAnimType Type, const NameString& RootBone, bool bTransient)
{
	auto animChannel = Spawn(NewClass, {}, {}, {}, {});
	LogUnimplemented("Actor.CreateAnimChannel");
	return animChannel;
}

int UActor::BoneNumber(const NameString& Bone)
{
	LogUnimplemented("Actor.BoneNumber");
	return 0;
}

NameString UActor::BoneName(int Bone)
{
	LogUnimplemented("Actor.BoneName");
	return {};
}

vec3 UActor::BonePos(const NameString& Bone)
{
	LogUnimplemented("Actor.BonePos");
	return vec3(0.0f);
}

UTexture* UActor::GetMultiskin(int index)
{
	if (engine->LaunchInfo.ue1Version > 219 && index >= 0 && index < 8)
		return MultiSkins()[index];
	else
		return nullptr;
}

vec3 UActor::GetRenderExtent()
{
	LogUnimplemented("Actor.GetRenderExtent");
	return vec3(100.0f);
}
