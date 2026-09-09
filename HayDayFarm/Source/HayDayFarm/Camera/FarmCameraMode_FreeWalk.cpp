// Copyright (c) 2026. Private personal project.

#include "Camera/FarmCameraMode_FreeWalk.h"

UFarmCameraMode_FreeWalk::UFarmCameraMode_FreeWalk()
{
	FieldOfView = 75.0f;

	// Free Walk is the mode everything else returns to, so it should settle
	// gently rather than snap.
	BlendTime = 0.6f;
	BlendFunction = EFarmCameraBlendFunction::EaseOut;
	BlendExponent = 2.0f;
}

void UFarmCameraMode_FreeWalk::OnActivation()
{
	Super::OnActivation();

	// Re-entering Free Walk (from a bench, say) should not inherit a stale
	// sway strength from before, or the camera lurches on the first frame.
	SwayStrength = 0.0f;
}

void UFarmCameraMode_FreeWalk::UpdateView(float DeltaTime)
{
	const FVector PivotLocation = GetPivotLocation();
	const FRotator PivotRotation = GetPivotRotation();

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.FieldOfView = FieldOfView;

	if (!bEnableStepSway)
	{
		return;
	}

	const float Strength = UpdateSwayState(DeltaTime, GetTargetHorizontalSpeed());
	if (Strength <= UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	// The vertical component runs at twice the step frequency: the head rises
	// once per footfall, but sways sideways once per full left-right cycle.
	const float VerticalOffset = FMath::Sin(SwayPhase * 2.0f) * VerticalAmplitude * Strength;
	const float LateralOffset = FMath::Sin(SwayPhase) * LateralAmplitude * Strength;
	const float RollOffset = FMath::Sin(SwayPhase) * RollAmplitude * Strength;

	const FVector RightVector = FRotationMatrix(PivotRotation).GetScaledAxis(EAxis::Y);

	View.Location += (RightVector * LateralOffset) + (FVector::UpVector * VerticalOffset);
	View.Rotation.Roll += RollOffset;
}

float UFarmCameraMode_FreeWalk::UpdateSwayState(float DeltaTime, float HorizontalSpeed)
{
	const float SafeReferenceSpeed = FMath::Max(ReferenceSpeed, 1.0f);
	const float NormalisedSpeed = FMath::Clamp(HorizontalSpeed / SafeReferenceSpeed, 0.0f, 1.0f);

	const float TargetStrength = (HorizontalSpeed > SwayMinSpeed) ? NormalisedSpeed : 0.0f;
	SwayStrength = FMath::FInterpTo(SwayStrength, TargetStrength, DeltaTime, SwayFadeSpeed);

	// Only advance the phase while actually moving, so stopping mid-stride
	// does not leave the camera drifting on the spot.
	SwayPhase += DeltaTime * StepFrequency * NormalisedSpeed * UE_TWO_PI;
	SwayPhase = FMath::Fmod(SwayPhase, UE_TWO_PI);

	return SwayStrength;
}
